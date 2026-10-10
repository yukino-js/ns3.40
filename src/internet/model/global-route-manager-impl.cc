
#include "global-route-manager-impl.h"

#include "candidate-queue.h"
#include "global-router-interface.h"
#include "ipv4-global-routing.h"
#include "ipv4.h"

#include "ns3/assert.h"
#include "ns3/fatal-error.h"
#include "ns3/log.h"
#include "ns3/node-list.h"
#include "ns3/simulator.h"

#include <algorithm>
#include <iostream>
#include <queue>
#include <utility>
#include <vector>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("GlobalRouteManagerImpl");

std::ostream &operator<<(std::ostream &os, const SPFVertex::NodeExit_t &exit) {
  os << "(" << exit.first << " ," << exit.second << ")";
  return os;
}

std::ostream &operator<<(std::ostream &os,
                         const SPFVertex::ListOfSPFVertex_t &vs) {
  os << "{";
  for (auto iter = vs.begin(); iter != vs.end();) {
    os << (*iter)->m_vertexId;
    if (++iter != vs.end()) {
      os << ", ";
    } else {
      break;
    }
  }
  os << "}";
  return os;
}

SPFVertex::SPFVertex()
    : m_vertexType(VertexUnknown), m_vertexId("255.255.255.255"),
      m_lsa(nullptr), m_distanceFromRoot(SPF_INFINITY), m_rootOif(SPF_INFINITY),
      m_nextHop("0.0.0.0"), m_parents(), m_children(),
      m_vertexProcessed(false) {
  NS_LOG_FUNCTION(this);
}

SPFVertex::SPFVertex(GlobalRoutingLSA *lsa)
    : m_vertexId(lsa->GetLinkStateId()), m_lsa(lsa),
      m_distanceFromRoot(SPF_INFINITY), m_rootOif(SPF_INFINITY),
      m_nextHop("0.0.0.0"), m_parents(), m_children(),
      m_vertexProcessed(false) {
  NS_LOG_FUNCTION(this << lsa);

  if (lsa->GetLSType() == GlobalRoutingLSA::RouterLSA) {
    NS_LOG_LOGIC("Setting m_vertexType to VertexRouter");
    m_vertexType = SPFVertex::VertexRouter;
  } else if (lsa->GetLSType() == GlobalRoutingLSA::NetworkLSA) {
    NS_LOG_LOGIC("Setting m_vertexType to VertexNetwork");
    m_vertexType = SPFVertex::VertexNetwork;
  }
}

SPFVertex::~SPFVertex() {
  NS_LOG_FUNCTION(this);

  NS_LOG_LOGIC("Children vertices - " << m_children);
  NS_LOG_LOGIC("Parent verteices - " << m_parents);

  for (auto piter = m_parents.begin(); piter != m_parents.end(); piter++) {
    uint32_t orgCount = (*piter)->m_children.size();
    (*piter)->m_children.remove(this);
    uint32_t newCount = (*piter)->m_children.size();
    if (orgCount > newCount) {
      NS_ASSERT_MSG(
          orgCount > newCount,
          "Unable to find the current vertex from its parents --- impossible!");
    }
  }

  while (!m_children.empty()) {
    SPFVertex *p = m_children.front();
    if (p == nullptr) {
      continue;
    }
    NS_LOG_LOGIC("Parent vertex-" << m_vertexId << " deleting its child vertex-"
                                  << p->GetVertexId());
    delete p;
    p = nullptr;
  }
  m_children.clear();
  m_parents.clear();
  m_ecmpRootExits.clear();

  NS_LOG_LOGIC("Vertex-" << m_vertexId << " completed deleted");
}

void SPFVertex::SetVertexType(SPFVertex::VertexType type) {
  NS_LOG_FUNCTION(this << type);
  m_vertexType = type;
}

SPFVertex::VertexType SPFVertex::GetVertexType() const {
  NS_LOG_FUNCTION(this);
  return m_vertexType;
}

void SPFVertex::SetVertexId(Ipv4Address id) {
  NS_LOG_FUNCTION(this << id);
  m_vertexId = id;
}

Ipv4Address SPFVertex::GetVertexId() const {
  NS_LOG_FUNCTION(this);
  return m_vertexId;
}

void SPFVertex::SetLSA(GlobalRoutingLSA *lsa) {
  NS_LOG_FUNCTION(this << lsa);
  m_lsa = lsa;
}

GlobalRoutingLSA *SPFVertex::GetLSA() const {
  NS_LOG_FUNCTION(this);
  return m_lsa;
}

void SPFVertex::SetDistanceFromRoot(uint32_t distance) {
  NS_LOG_FUNCTION(this << distance);
  m_distanceFromRoot = distance;
}

uint32_t SPFVertex::GetDistanceFromRoot() const {
  NS_LOG_FUNCTION(this);
  return m_distanceFromRoot;
}

void SPFVertex::SetParent(SPFVertex *parent) {
  NS_LOG_FUNCTION(this << parent);

  m_parents.clear();
  m_parents.push_back(parent);
}

SPFVertex *SPFVertex::GetParent(uint32_t i) const {
  NS_LOG_FUNCTION(this << i);

  if (m_parents.size() <= i) {
    NS_LOG_LOGIC("Index to SPFVertex's parent is out-of-range.");
    return nullptr;
  }
  auto iter = m_parents.begin();
  while (i-- > 0) {
    iter++;
  }
  return *iter;
}

void SPFVertex::MergeParent(const SPFVertex *v) {
  NS_LOG_FUNCTION(this << v);

  NS_LOG_LOGIC("Before merge, list of parents = " << m_parents);
  m_parents.insert(m_parents.end(), v->m_parents.begin(), v->m_parents.end());
  m_parents.sort();
  m_parents.unique();
  NS_LOG_LOGIC("After merge, list of parents = " << m_parents);
}

void SPFVertex::SetRootExitDirection(Ipv4Address nextHop, int32_t id) {
  NS_LOG_FUNCTION(this << nextHop << id);

  m_ecmpRootExits.clear();
  m_ecmpRootExits.emplace_back(nextHop, id);
  m_nextHop = nextHop;
  m_rootOif = id;
}

void SPFVertex::SetRootExitDirection(SPFVertex::NodeExit_t exit) {
  NS_LOG_FUNCTION(this << exit);
  SetRootExitDirection(exit.first, exit.second);
}

SPFVertex::NodeExit_t SPFVertex::GetRootExitDirection(uint32_t i) const {
  NS_LOG_FUNCTION(this << i);

  NS_ASSERT_MSG(
      i < m_ecmpRootExits.size(),
      "Index out-of-range when accessing SPFVertex::m_ecmpRootExits!");
  auto iter = m_ecmpRootExits.begin();
  while (i-- > 0) {
    iter++;
  }

  return *iter;
}

SPFVertex::NodeExit_t SPFVertex::GetRootExitDirection() const {
  NS_LOG_FUNCTION(this);

  NS_ASSERT_MSG(
      m_ecmpRootExits.size() <= 1,
      "Assumed there is at most one exit from the root to this vertex");
  return GetRootExitDirection(0);
}

void SPFVertex::MergeRootExitDirections(const SPFVertex *vertex) {
  NS_LOG_FUNCTION(this << vertex);

  const ListOfNodeExit_t &extList = vertex->m_ecmpRootExits;
  m_ecmpRootExits.insert(m_ecmpRootExits.end(), extList.begin(), extList.end());
  m_ecmpRootExits.sort();
  m_ecmpRootExits.unique();
}

void SPFVertex::InheritAllRootExitDirections(const SPFVertex *vertex) {
  NS_LOG_FUNCTION(this << vertex);

  if (!m_ecmpRootExits.empty()) {
    NS_LOG_WARN(
        "x root exit directions in this vertex are going to be discarded");
  }
  m_ecmpRootExits.clear();
  m_ecmpRootExits.insert(m_ecmpRootExits.end(), vertex->m_ecmpRootExits.begin(),
                         vertex->m_ecmpRootExits.end());
}

uint32_t SPFVertex::GetNRootExitDirections() const {
  NS_LOG_FUNCTION(this);
  return m_ecmpRootExits.size();
}

uint32_t SPFVertex::GetNChildren() const {
  NS_LOG_FUNCTION(this);
  return m_children.size();
}

SPFVertex *SPFVertex::GetChild(uint32_t n) const {
  NS_LOG_FUNCTION(this << n);
  uint32_t j = 0;

  for (auto i = m_children.begin(); i != m_children.end(); i++, j++) {
    if (j == n) {
      return *i;
    }
  }
  NS_ASSERT_MSG(false, "Index <n> out of range.");
  return nullptr;
}

uint32_t SPFVertex::AddChild(SPFVertex *child) {
  NS_LOG_FUNCTION(this << child);
  m_children.push_back(child);
  return m_children.size();
}

void SPFVertex::SetVertexProcessed(bool value) {
  NS_LOG_FUNCTION(this << value);
  m_vertexProcessed = value;
}

bool SPFVertex::IsVertexProcessed() const {
  NS_LOG_FUNCTION(this);
  return m_vertexProcessed;
}

void SPFVertex::ClearVertexProcessed() {
  NS_LOG_FUNCTION(this);
  for (uint32_t i = 0; i < this->GetNChildren(); i++) {
    this->GetChild(i)->ClearVertexProcessed();
  }
  this->SetVertexProcessed(false);
}

GlobalRouteManagerLSDB::GlobalRouteManagerLSDB()
    : m_database(), m_extdatabase() {
  NS_LOG_FUNCTION(this);
}

GlobalRouteManagerLSDB::~GlobalRouteManagerLSDB() {
  NS_LOG_FUNCTION(this);
  for (auto i = m_database.begin(); i != m_database.end(); i++) {
    NS_LOG_LOGIC("free LSA");
    GlobalRoutingLSA *temp = i->second;
    delete temp;
  }
  for (uint32_t j = 0; j < m_extdatabase.size(); j++) {
    NS_LOG_LOGIC("free ASexternalLSA");
    GlobalRoutingLSA *temp = m_extdatabase.at(j);
    delete temp;
  }
  NS_LOG_LOGIC("clear map");
  m_database.clear();
}

void GlobalRouteManagerLSDB::Initialize() {
  NS_LOG_FUNCTION(this);
  for (auto i = m_database.begin(); i != m_database.end(); i++) {
    GlobalRoutingLSA *temp = i->second;
    temp->SetStatus(GlobalRoutingLSA::LSA_SPF_NOT_EXPLORED);
  }
}

void GlobalRouteManagerLSDB::Insert(Ipv4Address addr, GlobalRoutingLSA *lsa) {
  NS_LOG_FUNCTION(this << addr << lsa);
  if (lsa->GetLSType() == GlobalRoutingLSA::ASExternalLSAs) {
    m_extdatabase.push_back(lsa);
  } else {
    m_database.insert(LSDBPair_t(addr, lsa));
  }
}

GlobalRoutingLSA *GlobalRouteManagerLSDB::GetExtLSA(uint32_t index) const {
  NS_LOG_FUNCTION(this << index);
  return m_extdatabase.at(index);
}

uint32_t GlobalRouteManagerLSDB::GetNumExtLSAs() const {
  NS_LOG_FUNCTION(this);
  return m_extdatabase.size();
}

GlobalRoutingLSA *GlobalRouteManagerLSDB::GetLSA(Ipv4Address addr) const {
  NS_LOG_FUNCTION(this << addr);
  for (auto i = m_database.begin(); i != m_database.end(); i++) {
    if (i->first == addr) {
      return i->second;
    }
  }
  return nullptr;
}

GlobalRoutingLSA *
GlobalRouteManagerLSDB::GetLSAByLinkData(Ipv4Address addr) const {
  NS_LOG_FUNCTION(this << addr);
  for (auto i = m_database.begin(); i != m_database.end(); i++) {
    GlobalRoutingLSA *temp = i->second;
    for (uint32_t j = 0; j < temp->GetNLinkRecords(); j++) {
      GlobalRoutingLinkRecord *lr = temp->GetLinkRecord(j);
      if (lr->GetLinkType() == GlobalRoutingLinkRecord::TransitNetwork &&
          lr->GetLinkData() == addr) {
        return temp;
      }
    }
  }
  return nullptr;
}

GlobalRouteManagerImpl::GlobalRouteManagerImpl() : m_spfroot(nullptr) {
  NS_LOG_FUNCTION(this);
  m_lsdb = new GlobalRouteManagerLSDB();
}

GlobalRouteManagerImpl::~GlobalRouteManagerImpl() {
  NS_LOG_FUNCTION(this);
  if (m_lsdb) {
    delete m_lsdb;
  }
}

void GlobalRouteManagerImpl::DebugUseLsdb(GlobalRouteManagerLSDB *lsdb) {
  NS_LOG_FUNCTION(this << lsdb);
  if (m_lsdb) {
    delete m_lsdb;
  }
  m_lsdb = lsdb;
}

void GlobalRouteManagerImpl::DeleteGlobalRoutes() {
  NS_LOG_FUNCTION(this);
  for (auto i = NodeList::Begin(); i != NodeList::End(); i++) {
    Ptr<Node> node = *i;
    Ptr<GlobalRouter> router = node->GetObject<GlobalRouter>();
    if (!router) {
      continue;
    }
    Ptr<Ipv4GlobalRouting> gr = router->GetRoutingProtocol();
    uint32_t j = 0;
    uint32_t nRoutes = gr->GetNRoutes();
    NS_LOG_LOGIC("Deleting " << gr->GetNRoutes() << " routes from node "
                             << node->GetId());
    for (j = 0; j < nRoutes; j++) {
      NS_LOG_LOGIC("Deleting global route " << j << " from node "
                                            << node->GetId());
      gr->RemoveRoute(0);
    }
    NS_LOG_LOGIC("Deleted " << j << " global routes from node "
                            << node->GetId());
  }
  if (m_lsdb) {
    NS_LOG_LOGIC("Deleting LSDB, creating new one");
    delete m_lsdb;
    m_lsdb = new GlobalRouteManagerLSDB();
  }
}

void GlobalRouteManagerImpl::BuildGlobalRoutingDatabase() {
  NS_LOG_FUNCTION(this);
  for (auto i = NodeList::Begin(); i != NodeList::End(); i++) {
    Ptr<Node> node = *i;

    Ptr<GlobalRouter> rtr = node->GetObject<GlobalRouter>();
    if (!rtr) {
      continue;
    }
    Ptr<Ipv4GlobalRouting> grouting = rtr->GetRoutingProtocol();
    uint32_t numLSAs = rtr->DiscoverLSAs();
    NS_LOG_LOGIC("Found " << numLSAs << " LSAs");

    for (uint32_t j = 0; j < numLSAs; ++j) {
      auto lsa = new GlobalRoutingLSA();
      rtr->GetLSA(j, *lsa);
      NS_LOG_LOGIC(*lsa);
      m_lsdb->Insert(lsa->GetLinkStateId(), lsa);
    }
  }
}

void GlobalRouteManagerImpl::InitializeRoutes() {
  NS_LOG_FUNCTION(this);
  NS_LOG_INFO("About to start SPF calculation");
  for (auto i = NodeList::Begin(); i != NodeList::End(); i++) {
    Ptr<Node> node = *i;
    Ptr<GlobalRouter> rtr = node->GetObject<GlobalRouter>();

#ifdef NS3_MPI
    uint32_t systemId = Simulator::GetSystemId();
    if (node->GetSystemId() != systemId) {
      continue;
    }
#endif

    if (rtr && rtr->GetNumLSAs()) {
      SPFCalculate(rtr->GetRouterId());
    }
  }
  NS_LOG_INFO("Finished SPF calculation");
}

void GlobalRouteManagerImpl::SPFNext(SPFVertex *v, CandidateQueue &candidate) {
  NS_LOG_FUNCTION(this << v << &candidate);

  SPFVertex *w = nullptr;
  GlobalRoutingLSA *w_lsa = nullptr;
  GlobalRoutingLinkRecord *l = nullptr;
  uint32_t distance = 0;
  uint32_t numRecordsInVertex = 0;
  if (v->GetVertexType() == SPFVertex::VertexRouter) {
    numRecordsInVertex = v->GetLSA()->GetNLinkRecords();
  }
  if (v->GetVertexType() == SPFVertex::VertexNetwork) {
    numRecordsInVertex = v->GetLSA()->GetNAttachedRouters();
  }

  for (uint32_t i = 0; i < numRecordsInVertex; i++) {
    if (v->GetVertexType() == SPFVertex::VertexRouter) {
      NS_LOG_LOGIC("Examining link " << i << " of " << v->GetVertexId() << "'s "
                                     << v->GetLSA()->GetNLinkRecords()
                                     << " link records");
      l = v->GetLSA()->GetLinkRecord(i);
      NS_ASSERT(l != nullptr);
      if (l->GetLinkType() == GlobalRoutingLinkRecord::StubNetwork) {
        NS_LOG_LOGIC("Found a Stub record to " << l->GetLinkId());
        continue;
      }
      if (l->GetLinkType() == GlobalRoutingLinkRecord::PointToPoint) {
        w_lsa = m_lsdb->GetLSA(l->GetLinkId());
        NS_ASSERT(w_lsa);
        NS_LOG_LOGIC("Found a P2P record from " << v->GetVertexId() << " to "
                                                << w_lsa->GetLinkStateId());
      } else if (l->GetLinkType() == GlobalRoutingLinkRecord::TransitNetwork) {
        w_lsa = m_lsdb->GetLSA(l->GetLinkId());
        NS_ASSERT(w_lsa);
        NS_LOG_LOGIC("Found a Transit record from "
                     << v->GetVertexId() << " to " << w_lsa->GetLinkStateId());
      } else {
        NS_ASSERT_MSG(0, "illegal Link Type");
      }
    }
    if (v->GetVertexType() == SPFVertex::VertexNetwork) {
      w_lsa = m_lsdb->GetLSAByLinkData(v->GetLSA()->GetAttachedRouter(i));
      if (!w_lsa) {
        continue;
      }
      NS_LOG_LOGIC("Found a Network LSA from " << v->GetVertexId() << " to "
                                               << w_lsa->GetLinkStateId());
    }

    if (w_lsa->GetStatus() == GlobalRoutingLSA::LSA_SPF_IN_SPFTREE) {
      NS_LOG_LOGIC("Skipping ->  LSA " << w_lsa->GetLinkStateId()
                                       << " already in SPF tree");
      continue;
    }
    if (v->GetLSA()->GetLSType() == GlobalRoutingLSA::RouterLSA) {
      NS_ASSERT(l != nullptr);
      distance = v->GetDistanceFromRoot() + l->GetMetric();
    } else {
      distance = v->GetDistanceFromRoot();
    }

    NS_LOG_LOGIC("Considering w_lsa " << w_lsa->GetLinkStateId());

    if (w_lsa->GetStatus() == GlobalRoutingLSA::LSA_SPF_NOT_EXPLORED) {

      w = new SPFVertex(w_lsa);
      if (SPFNexthopCalculation(v, w, l, distance)) {
        w_lsa->SetStatus(GlobalRoutingLSA::LSA_SPF_CANDIDATE);
        candidate.Push(w);
        NS_LOG_LOGIC("Pushing " << w->GetVertexId()
                                << ", parent vertexId: " << v->GetVertexId()
                                << ", distance: " << w->GetDistanceFromRoot());
      } else {
        NS_ASSERT_MSG(0, "SPFNexthopCalculation never "
                             << "return false, but it does now!");
      }
    } else if (w_lsa->GetStatus() == GlobalRoutingLSA::LSA_SPF_CANDIDATE) {

      SPFVertex *cw;
      cw = candidate.Find(w_lsa->GetLinkStateId());
      if (cw->GetDistanceFromRoot() < distance) {
        continue;
      } else if (cw->GetDistanceFromRoot() == distance) {
        NS_LOG_LOGIC("Equal cost multiple paths found.");

        w = new SPFVertex(w_lsa);
        SPFNexthopCalculation(v, w, l, distance);
        cw->MergeRootExitDirections(w);
        cw->MergeParent(w);
        SPFVertexAddParent(w);
        delete w;
      } else {
        if (SPFNexthopCalculation(v, cw, l, distance)) {
          candidate.Reorder();
        }
      }
    }
  }
}

int GlobalRouteManagerImpl::SPFNexthopCalculation(SPFVertex *v, SPFVertex *w,
                                                  GlobalRoutingLinkRecord *l,
                                                  uint32_t distance) {
  NS_LOG_FUNCTION(this << v << w << l << distance);

  if (v == m_spfroot) {
    if (w->GetVertexType() == SPFVertex::VertexRouter) {
      NS_ASSERT(l);
      GlobalRoutingLinkRecord *linkRemote = nullptr;
      linkRemote = SPFGetNextLink(w, v, linkRemote);
      Ipv4Address nextHop = linkRemote->GetLinkData();
      uint32_t outIf = FindOutgoingInterfaceId(l->GetLinkData());

      w->SetRootExitDirection(nextHop, outIf);
      w->SetDistanceFromRoot(distance);
      w->SetParent(v);
      NS_LOG_LOGIC("Next hop from "
                   << v->GetVertexId() << " to " << w->GetVertexId()
                   << " goes through next hop " << nextHop
                   << " via outgoing interface " << outIf << " with distance "
                   << distance);
    } else {
      NS_ASSERT(w->GetVertexType() == SPFVertex::VertexNetwork);
      GlobalRoutingLSA *w_lsa = w->GetLSA();
      NS_ASSERT(w_lsa->GetLSType() == GlobalRoutingLSA::NetworkLSA);
      uint32_t outIf = FindOutgoingInterfaceId(
          w_lsa->GetLinkStateId(), w_lsa->GetNetworkLSANetworkMask());
      Ipv4Address nextHop = Ipv4Address::GetZero();
      w->SetRootExitDirection(nextHop, outIf);
      w->SetDistanceFromRoot(distance);
      w->SetParent(v);
      NS_LOG_LOGIC("Next hop from "
                   << v->GetVertexId() << " to network " << w->GetVertexId()
                   << " via outgoing interface " << outIf << " with distance "
                   << distance);
      return 1;
    }
  } else if (v->GetVertexType() == SPFVertex::VertexNetwork) {
    if (v->GetParent() == m_spfroot) {
      NS_ASSERT(w->GetVertexType() == SPFVertex::VertexRouter);
      GlobalRoutingLinkRecord *linkRemote = nullptr;
      while ((linkRemote = SPFGetNextLink(w, v, linkRemote))) {
        Ipv4Address nextHop = linkRemote->GetLinkData();
        uint32_t outIf = v->GetRootExitDirection().second;
        w->SetRootExitDirection(nextHop, outIf);
        NS_LOG_LOGIC("Next hop from " << v->GetVertexId() << " to "
                                      << w->GetVertexId()
                                      << " goes through next hop " << nextHop
                                      << " via outgoing interface " << outIf);
      }
    } else {
      w->SetRootExitDirection(v->GetRootExitDirection());
    }
  } else {
    w->InheritAllRootExitDirections(v);
  }
  w->SetDistanceFromRoot(distance);
  w->SetParent(v);

  return 1;
}

GlobalRoutingLinkRecord *
GlobalRouteManagerImpl::SPFGetNextLink(SPFVertex *v, SPFVertex *w,
                                       GlobalRoutingLinkRecord *prev_link) {
  NS_LOG_FUNCTION(this << v << w << prev_link);

  bool skip = true;
  bool found_prev_link = false;
  GlobalRoutingLinkRecord *l;
  if (prev_link == nullptr) {
    skip = false;
    found_prev_link = true;
  }
  for (uint32_t i = 0; i < v->GetLSA()->GetNLinkRecords(); ++i) {
    l = v->GetLSA()->GetLinkRecord(i);
    if (l->GetLinkId() == w->GetVertexId()) {
      if (!found_prev_link) {
        NS_LOG_LOGIC("Skipping links before prev_link found");
        found_prev_link = true;
        continue;
      }

      NS_LOG_LOGIC("Found matching link l:  linkId = "
                   << l->GetLinkId() << " linkData = " << l->GetLinkData());
      if (!skip) {
        NS_LOG_LOGIC("Returning the found link");
        return l;
      } else {
        NS_LOG_LOGIC("Skipping the found link");
        skip = false;
        continue;
      }
    }
  }
  return nullptr;
}

void GlobalRouteManagerImpl::DebugSPFCalculate(Ipv4Address root) {
  NS_LOG_FUNCTION(this << root);
  SPFCalculate(root);
}

bool GlobalRouteManagerImpl::CheckForStubNode(Ipv4Address root) {
  NS_LOG_FUNCTION(this << root);
  GlobalRoutingLSA *rlsa = m_lsdb->GetLSA(root);
  Ipv4Address myRouterId = rlsa->GetLinkStateId();
  int transits = 0;
  GlobalRoutingLinkRecord *transitLink = nullptr;
  for (uint32_t i = 0; i < rlsa->GetNLinkRecords(); i++) {
    GlobalRoutingLinkRecord *l = rlsa->GetLinkRecord(i);
    if (l->GetLinkType() == GlobalRoutingLinkRecord::TransitNetwork) {
      transits++;
      transitLink = l;
    } else if (l->GetLinkType() == GlobalRoutingLinkRecord::PointToPoint) {
      transits++;
      transitLink = l;
    }
  }
  if (transits == 0) {
    NS_LOG_WARN("all nodes should have at least one transit link:" << root);
    return true;
  }
  if (transits == 1) {
    if (transitLink->GetLinkType() == GlobalRoutingLinkRecord::TransitNetwork) {
      NS_LOG_LOGIC("TBD: Would have inserted default for transit");
      return false;
    } else if (transitLink->GetLinkType() ==
               GlobalRoutingLinkRecord::PointToPoint) {
      GlobalRoutingLSA *w_lsa = m_lsdb->GetLSA(transitLink->GetLinkId());
      uint32_t nLinkRecords = w_lsa->GetNLinkRecords();
      for (uint32_t j = 0; j < nLinkRecords; ++j) {
        GlobalRoutingLinkRecord *lr = w_lsa->GetLinkRecord(j);
        if (lr->GetLinkType() != GlobalRoutingLinkRecord::PointToPoint) {
          continue;
        }
        if (lr->GetLinkId() == myRouterId) {
          Ptr<GlobalRouter> router = rlsa->GetNode()->GetObject<GlobalRouter>();
          NS_ASSERT(router);
          Ptr<Ipv4GlobalRouting> gr = router->GetRoutingProtocol();
          NS_ASSERT(gr);
          gr->AddNetworkRouteTo(
              Ipv4Address("0.0.0.0"), Ipv4Mask("0.0.0.0"), lr->GetLinkData(),
              FindOutgoingInterfaceId(transitLink->GetLinkData()));
          NS_LOG_LOGIC("Inserting default route for node "
                       << myRouterId << " to next hop " << lr->GetLinkData()
                       << " via interface "
                       << FindOutgoingInterfaceId(transitLink->GetLinkData()));
          return true;
        }
      }
    }
  }
  return false;
}

void GlobalRouteManagerImpl::SPFCalculate(Ipv4Address root) {
  NS_LOG_FUNCTION(this << root);

  SPFVertex *v;
  m_lsdb->Initialize();
  CandidateQueue candidate;
  NS_ASSERT(candidate.Size() == 0);
  v = new SPFVertex(m_lsdb->GetLSA(root));
  m_spfroot = v;
  v->SetDistanceFromRoot(0);
  v->GetLSA()->SetStatus(GlobalRoutingLSA::LSA_SPF_IN_SPFTREE);
  NS_LOG_LOGIC("Starting SPFCalculate for node " << root);

  if (NodeList::GetNNodes() > 0 && CheckForStubNode(root)) {
    NS_LOG_LOGIC("SPFCalculate truncated for stub node " << root);
    delete m_spfroot;
    return;
  }

  for (;;) {
    SPFNext(v, candidate);
    if (candidate.Size() == 0) {
      break;
    }
    NS_LOG_LOGIC(candidate);
    v = candidate.Pop();
    NS_LOG_LOGIC("Popped vertex " << v->GetVertexId());
    v->GetLSA()->SetStatus(GlobalRoutingLSA::LSA_SPF_IN_SPFTREE);
    SPFVertexAddParent(v);
    if (v->GetVertexType() == SPFVertex::VertexRouter) {
      SPFIntraAddRouter(v);
    } else if (v->GetVertexType() == SPFVertex::VertexNetwork) {
      SPFIntraAddTransit(v);
    } else {
      NS_ASSERT_MSG(0, "illegal SPFVertex type");
    }
  }

  SPFProcessStubs(m_spfroot);
  for (uint32_t i = 0; i < m_lsdb->GetNumExtLSAs(); i++) {
    m_spfroot->ClearVertexProcessed();
    GlobalRoutingLSA *extlsa = m_lsdb->GetExtLSA(i);
    NS_LOG_LOGIC("Processing External LSA with id "
                 << extlsa->GetLinkStateId());
    ProcessASExternals(m_spfroot, extlsa);
  }

  delete m_spfroot;
  m_spfroot = nullptr;
}

void GlobalRouteManagerImpl::ProcessASExternals(SPFVertex *v,
                                                GlobalRoutingLSA *extlsa) {
  NS_LOG_FUNCTION(this << v << extlsa);
  NS_LOG_LOGIC("Processing external for destination "
               << extlsa->GetLinkStateId() << ", for router "
               << v->GetVertexId() << ", advertised by "
               << extlsa->GetAdvertisingRouter());
  if (v->GetVertexType() == SPFVertex::VertexRouter) {
    GlobalRoutingLSA *rlsa = v->GetLSA();
    NS_LOG_LOGIC("Processing router LSA with id " << rlsa->GetLinkStateId());
    if ((rlsa->GetLinkStateId()) == (extlsa->GetAdvertisingRouter())) {
      NS_LOG_LOGIC("Found advertising router to destination");
      SPFAddASExternal(extlsa, v);
    }
  }
  for (uint32_t i = 0; i < v->GetNChildren(); i++) {
    if (!v->GetChild(i)->IsVertexProcessed()) {
      NS_LOG_LOGIC("Vertex's child " << i
                                     << " not yet processed, processing...");
      ProcessASExternals(v->GetChild(i), extlsa);
      v->GetChild(i)->SetVertexProcessed(true);
    }
  }
}

void GlobalRouteManagerImpl::SPFAddASExternal(GlobalRoutingLSA *extlsa,
                                              SPFVertex *v) {
  NS_LOG_FUNCTION(this << extlsa << v);

  NS_ASSERT_MSG(
      m_spfroot,
      "GlobalRouteManagerImpl::SPFAddASExternal (): Root pointer not set");
  if (v->GetVertexId() == m_spfroot->GetVertexId()) {
    NS_LOG_LOGIC("External is on local host: " << v->GetVertexId()
                                               << "; returning");
    return;
  }
  NS_LOG_LOGIC("External is on remote host: " << extlsa->GetAdvertisingRouter()
                                              << "; installing");

  Ipv4Address routerId = m_spfroot->GetVertexId();

  NS_LOG_LOGIC("Vertex ID = " << routerId);
  for (auto i = NodeList::Begin(); i != NodeList::End(); i++) {
    Ptr<Node> node = *i;
    Ptr<GlobalRouter> rtr = node->GetObject<GlobalRouter>();

    if (!rtr) {
      NS_LOG_LOGIC("No GlobalRouter interface on node " << node->GetId());
      continue;
    }
    NS_LOG_LOGIC("Considering router " << rtr->GetRouterId());

    if (rtr->GetRouterId() == routerId) {
      NS_LOG_LOGIC("Setting routes for node " << node->GetId());
      Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
      NS_ASSERT_MSG(ipv4, "GlobalRouteManagerImpl::SPFIntraAddRouter (): "
                          "QI for <Ipv4> interface failed");
      NS_ASSERT_MSG(v->GetLSA(),
                    "GlobalRouteManagerImpl::SPFIntraAddRouter (): "
                    "Expected valid LSA in SPFVertex* v");
      Ipv4Mask tempmask = extlsa->GetNetworkLSANetworkMask();
      Ipv4Address tempip = extlsa->GetLinkStateId();
      tempip = tempip.CombineMask(tempmask);

      Ptr<GlobalRouter> router = node->GetObject<GlobalRouter>();
      if (!router) {
        continue;
      }
      Ptr<Ipv4GlobalRouting> gr = router->GetRoutingProtocol();
      NS_ASSERT(gr);
      for (uint32_t i = 0; i < v->GetNRootExitDirections(); i++) {
        SPFVertex::NodeExit_t exit = v->GetRootExitDirection(i);
        Ipv4Address nextHop = exit.first;
        int32_t outIf = exit.second;
        if (outIf >= 0) {
          gr->AddASExternalRouteTo(tempip, tempmask, nextHop, outIf);
          NS_LOG_LOGIC("(Route " << i << ") Node " << node->GetId()
                                 << " add external network route to " << tempip
                                 << " using next hop " << nextHop
                                 << " via interface " << outIf);
        } else {
          NS_LOG_LOGIC("(Route " << i << ") Node " << node->GetId()
                                 << " NOT able to add network route to "
                                 << tempip << " using next hop " << nextHop
                                 << " since outgoing interface id is negative");
        }
      }
      return;
    }
  }
}

void GlobalRouteManagerImpl::SPFProcessStubs(SPFVertex *v) {
  NS_LOG_FUNCTION(this << v);
  NS_LOG_LOGIC("Processing stubs for " << v->GetVertexId());
  if (v->GetVertexType() == SPFVertex::VertexRouter) {
    GlobalRoutingLSA *rlsa = v->GetLSA();
    NS_LOG_LOGIC("Processing router LSA with id " << rlsa->GetLinkStateId());
    for (uint32_t i = 0; i < rlsa->GetNLinkRecords(); i++) {
      NS_LOG_LOGIC("Examining link " << i << " of " << v->GetVertexId() << "'s "
                                     << v->GetLSA()->GetNLinkRecords()
                                     << " link records");
      GlobalRoutingLinkRecord *l = v->GetLSA()->GetLinkRecord(i);
      if (l->GetLinkType() == GlobalRoutingLinkRecord::StubNetwork) {
        NS_LOG_LOGIC("Found a Stub record to " << l->GetLinkId());
        SPFIntraAddStub(l, v);
        continue;
      }
    }
  }
  for (uint32_t i = 0; i < v->GetNChildren(); i++) {
    if (!v->GetChild(i)->IsVertexProcessed()) {
      SPFProcessStubs(v->GetChild(i));
      v->GetChild(i)->SetVertexProcessed(true);
    }
  }
}

void GlobalRouteManagerImpl::SPFIntraAddStub(GlobalRoutingLinkRecord *l,
                                             SPFVertex *v) {
  NS_LOG_FUNCTION(this << l << v);

  NS_ASSERT_MSG(
      m_spfroot,
      "GlobalRouteManagerImpl::SPFIntraAddStub (): Root pointer not set");

  if (v->GetVertexId() == m_spfroot->GetVertexId()) {
    NS_LOG_LOGIC("Stub is on local host: " << v->GetVertexId()
                                           << "; returning");
    return;
  }
  NS_LOG_LOGIC("Stub is on remote host: " << v->GetVertexId()
                                          << "; installing");
  Ipv4Address routerId = m_spfroot->GetVertexId();

  NS_LOG_LOGIC("Vertex ID = " << routerId);
  for (auto i = NodeList::Begin(); i != NodeList::End(); i++) {
    Ptr<Node> node = *i;
    Ptr<GlobalRouter> rtr = node->GetObject<GlobalRouter>();

    if (!rtr) {
      NS_LOG_LOGIC("No GlobalRouter interface on node " << node->GetId());
      continue;
    }
    NS_LOG_LOGIC("Considering router " << rtr->GetRouterId());

    if (rtr->GetRouterId() == routerId) {
      NS_LOG_LOGIC("Setting routes for node " << node->GetId());
      Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
      NS_ASSERT_MSG(ipv4, "GlobalRouteManagerImpl::SPFIntraAddRouter (): "
                          "QI for <Ipv4> interface failed");
      NS_ASSERT_MSG(v->GetLSA(),
                    "GlobalRouteManagerImpl::SPFIntraAddRouter (): "
                    "Expected valid LSA in SPFVertex* v");
      Ipv4Mask tempmask(l->GetLinkData().Get());
      Ipv4Address tempip = l->GetLinkId();
      tempip = tempip.CombineMask(tempmask);

      Ptr<GlobalRouter> router = node->GetObject<GlobalRouter>();
      if (!router) {
        continue;
      }
      Ptr<Ipv4GlobalRouting> gr = router->GetRoutingProtocol();
      NS_ASSERT(gr);
      for (uint32_t i = 0; i < v->GetNRootExitDirections(); i++) {
        SPFVertex::NodeExit_t exit = v->GetRootExitDirection(i);
        Ipv4Address nextHop = exit.first;
        int32_t outIf = exit.second;
        if (outIf >= 0) {
          gr->AddNetworkRouteTo(tempip, tempmask, nextHop, outIf);
          NS_LOG_LOGIC("(Route " << i << ") Node " << node->GetId()
                                 << " add network route to " << tempip
                                 << " using next hop " << nextHop
                                 << " via interface " << outIf);
        } else {
          NS_LOG_LOGIC("(Route " << i << ") Node " << node->GetId()
                                 << " NOT able to add network route to "
                                 << tempip << " using next hop " << nextHop
                                 << " since outgoing interface id is negative");
        }
      }
      return;
    }
  }
}

int32_t GlobalRouteManagerImpl::FindOutgoingInterfaceId(Ipv4Address a,
                                                        Ipv4Mask amask) {
  NS_LOG_FUNCTION(this << a << amask);
  Ipv4Address routerId = m_spfroot->GetVertexId();
  for (auto i = NodeList::Begin(); i != NodeList::End(); i++) {
    Ptr<Node> node = *i;

    Ptr<GlobalRouter> rtr = node->GetObject<GlobalRouter>();
    if (!rtr) {
      continue;
    }

    if (rtr->GetRouterId() == routerId) {
      Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
      NS_ASSERT_MSG(ipv4, "GlobalRouteManagerImpl::FindOutgoingInterfaceId (): "
                          "GetObject for <Ipv4> interface failed");
      int32_t interface = ipv4->GetInterfaceForPrefix(a, amask);

#if 0
          if (interface < 0)
            {
              NS_FATAL_ERROR ("GlobalRouteManagerImpl::FindOutgoingInterfaceId(): "
                              "Expected an interface associated with address a:" << a);
            }
#endif
      return interface;
    }
  }
  NS_LOG_LOGIC("FindOutgoingInterfaceId():Can't find root node " << routerId);
  return -1;
}

void GlobalRouteManagerImpl::SPFIntraAddRouter(SPFVertex *v) {
  NS_LOG_FUNCTION(this << v);

  NS_ASSERT_MSG(
      m_spfroot,
      "GlobalRouteManagerImpl::SPFIntraAddRouter (): Root pointer not set");
  Ipv4Address routerId = m_spfroot->GetVertexId();

  NS_LOG_LOGIC("Vertex ID = " << routerId);
  for (auto i = NodeList::Begin(); i != NodeList::End(); i++) {
    Ptr<Node> node = *i;
    Ptr<GlobalRouter> rtr = node->GetObject<GlobalRouter>();

    if (!rtr) {
      NS_LOG_LOGIC("No GlobalRouter interface on node " << node->GetId());
      continue;
    }
    NS_LOG_LOGIC("Considering router " << rtr->GetRouterId());

    if (rtr->GetRouterId() == routerId) {
      NS_LOG_LOGIC("Setting routes for node " << node->GetId());
      Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
      NS_ASSERT_MSG(ipv4, "GlobalRouteManagerImpl::SPFIntraAddRouter (): "
                          "GetObject for <Ipv4> interface failed");
      GlobalRoutingLSA *lsa = v->GetLSA();
      NS_ASSERT_MSG(lsa, "GlobalRouteManagerImpl::SPFIntraAddRouter (): "
                         "Expected valid LSA in SPFVertex* v");

      uint32_t nLinkRecords = lsa->GetNLinkRecords();
      NS_LOG_LOGIC(" Node " << node->GetId() << " found " << nLinkRecords
                            << " link records in LSA " << lsa
                            << "with LinkStateId " << lsa->GetLinkStateId());
      for (uint32_t j = 0; j < nLinkRecords; ++j) {
        GlobalRoutingLinkRecord *lr = lsa->GetLinkRecord(j);
        if (lr->GetLinkType() != GlobalRoutingLinkRecord::PointToPoint) {
          continue;
        }
        Ptr<GlobalRouter> router = node->GetObject<GlobalRouter>();
        if (!router) {
          continue;
        }
        Ptr<Ipv4GlobalRouting> gr = router->GetRoutingProtocol();
        NS_ASSERT(gr);
        for (uint32_t i = 0; i < v->GetNRootExitDirections(); i++) {
          SPFVertex::NodeExit_t exit = v->GetRootExitDirection(i);
          Ipv4Address nextHop = exit.first;
          int32_t outIf = exit.second;
          if (outIf >= 0) {
            gr->AddHostRouteTo(lr->GetLinkData(), nextHop, outIf);
            NS_LOG_LOGIC("(Route " << i << ") Node " << node->GetId()
                                   << " adding host route to "
                                   << lr->GetLinkData() << " using next hop "
                                   << nextHop << " and outgoing interface "
                                   << outIf);
          } else {
            NS_LOG_LOGIC("(Route "
                         << i << ") Node " << node->GetId()
                         << " NOT able to add host route to "
                         << lr->GetLinkData() << " using next hop " << nextHop
                         << " since outgoing interface id is negative "
                         << outIf);
          }
        }
      }
      return;
    }
  }
}

void GlobalRouteManagerImpl::SPFIntraAddTransit(SPFVertex *v) {
  NS_LOG_FUNCTION(this << v);

  NS_ASSERT_MSG(
      m_spfroot,
      "GlobalRouteManagerImpl::SPFIntraAddTransit (): Root pointer not set");
  Ipv4Address routerId = m_spfroot->GetVertexId();

  NS_LOG_LOGIC("Vertex ID = " << routerId);
  for (auto i = NodeList::Begin(); i != NodeList::End(); i++) {
    Ptr<Node> node = *i;
    Ptr<GlobalRouter> rtr = node->GetObject<GlobalRouter>();

    if (!rtr) {
      NS_LOG_LOGIC("No GlobalRouter interface on node " << node->GetId());
      continue;
    }
    NS_LOG_LOGIC("Considering router " << rtr->GetRouterId());

    if (rtr->GetRouterId() == routerId) {
      NS_LOG_LOGIC("setting routes for node " << node->GetId());
      Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
      NS_ASSERT_MSG(ipv4, "GlobalRouteManagerImpl::SPFIntraAddTransit (): "
                          "GetObject for <Ipv4> interface failed");
      GlobalRoutingLSA *lsa = v->GetLSA();
      NS_ASSERT_MSG(lsa, "GlobalRouteManagerImpl::SPFIntraAddTransit (): "
                         "Expected valid LSA in SPFVertex* v");
      Ipv4Mask tempmask = lsa->GetNetworkLSANetworkMask();
      Ipv4Address tempip = lsa->GetLinkStateId();
      tempip = tempip.CombineMask(tempmask);
      Ptr<GlobalRouter> router = node->GetObject<GlobalRouter>();
      if (!router) {
        continue;
      }
      Ptr<Ipv4GlobalRouting> gr = router->GetRoutingProtocol();
      NS_ASSERT(gr);
      for (uint32_t i = 0; i < v->GetNRootExitDirections(); i++) {
        SPFVertex::NodeExit_t exit = v->GetRootExitDirection(i);
        Ipv4Address nextHop = exit.first;
        int32_t outIf = exit.second;

        if (outIf >= 0) {
          gr->AddNetworkRouteTo(tempip, tempmask, nextHop, outIf);
          NS_LOG_LOGIC("(Route " << i << ") Node " << node->GetId()
                                 << " add network route to " << tempip
                                 << " using next hop " << nextHop
                                 << " via interface " << outIf);
        } else {
          NS_LOG_LOGIC("(Route " << i << ") Node " << node->GetId()
                                 << " NOT able to add network route to "
                                 << tempip << " using next hop " << nextHop
                                 << " since outgoing interface id is negative "
                                 << outIf);
        }
      }
    }
  }
}

void GlobalRouteManagerImpl::SPFVertexAddParent(SPFVertex *v) {
  NS_LOG_FUNCTION(this << v);

  for (uint32_t i = 0;;) {
    SPFVertex *parent;
    if ((parent = v->GetParent(i++)) == nullptr) {
      break;
    }
    parent->AddChild(v);
  }
}

} // namespace ns3
