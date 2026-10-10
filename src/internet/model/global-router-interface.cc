
#include "global-router-interface.h"

#include "ipv4-global-routing.h"
#include "ipv4.h"
#include "loopback-net-device.h"

#include "ns3/abort.h"
#include "ns3/assert.h"
#include "ns3/bridge-net-device.h"
#include "ns3/channel.h"
#include "ns3/log.h"
#include "ns3/net-device.h"
#include "ns3/node-list.h"
#include "ns3/node.h"

#include <vector>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("GlobalRouter");

GlobalRoutingLinkRecord::GlobalRoutingLinkRecord()
    : m_linkId("0.0.0.0"), m_linkData("0.0.0.0"), m_linkType(Unknown),
      m_metric(0) {
  NS_LOG_FUNCTION(this);
}

GlobalRoutingLinkRecord::GlobalRoutingLinkRecord(LinkType linkType,
                                                 Ipv4Address linkId,
                                                 Ipv4Address linkData,
                                                 uint16_t metric)
    : m_linkId(linkId), m_linkData(linkData), m_linkType(linkType),
      m_metric(metric) {
  NS_LOG_FUNCTION(this << linkType << linkId << linkData << metric);
}

GlobalRoutingLinkRecord::~GlobalRoutingLinkRecord() { NS_LOG_FUNCTION(this); }

Ipv4Address GlobalRoutingLinkRecord::GetLinkId() const {
  NS_LOG_FUNCTION(this);
  return m_linkId;
}

void GlobalRoutingLinkRecord::SetLinkId(Ipv4Address addr) {
  NS_LOG_FUNCTION(this << addr);
  m_linkId = addr;
}

Ipv4Address GlobalRoutingLinkRecord::GetLinkData() const {
  NS_LOG_FUNCTION(this);
  return m_linkData;
}

void GlobalRoutingLinkRecord::SetLinkData(Ipv4Address addr) {
  NS_LOG_FUNCTION(this << addr);
  m_linkData = addr;
}

GlobalRoutingLinkRecord::LinkType GlobalRoutingLinkRecord::GetLinkType() const {
  NS_LOG_FUNCTION(this);
  return m_linkType;
}

void GlobalRoutingLinkRecord::SetLinkType(
    GlobalRoutingLinkRecord::LinkType linkType) {
  NS_LOG_FUNCTION(this << linkType);
  m_linkType = linkType;
}

uint16_t GlobalRoutingLinkRecord::GetMetric() const {
  NS_LOG_FUNCTION(this);
  return m_metric;
}

void GlobalRoutingLinkRecord::SetMetric(uint16_t metric) {
  NS_LOG_FUNCTION(this << metric);
  m_metric = metric;
}

GlobalRoutingLSA::GlobalRoutingLSA()
    : m_lsType(GlobalRoutingLSA::Unknown), m_linkStateId("0.0.0.0"),
      m_advertisingRtr("0.0.0.0"), m_linkRecords(),
      m_networkLSANetworkMask("0.0.0.0"), m_attachedRouters(),
      m_status(GlobalRoutingLSA::LSA_SPF_NOT_EXPLORED), m_node_id(0) {
  NS_LOG_FUNCTION(this);
}

GlobalRoutingLSA::GlobalRoutingLSA(GlobalRoutingLSA::SPFStatus status,
                                   Ipv4Address linkStateId,
                                   Ipv4Address advertisingRtr)
    : m_lsType(GlobalRoutingLSA::Unknown), m_linkStateId(linkStateId),
      m_advertisingRtr(advertisingRtr), m_linkRecords(),
      m_networkLSANetworkMask("0.0.0.0"), m_attachedRouters(), m_status(status),
      m_node_id(0) {
  NS_LOG_FUNCTION(this << status << linkStateId << advertisingRtr);
}

GlobalRoutingLSA::GlobalRoutingLSA(GlobalRoutingLSA &lsa)
    : m_lsType(lsa.m_lsType), m_linkStateId(lsa.m_linkStateId),
      m_advertisingRtr(lsa.m_advertisingRtr),
      m_networkLSANetworkMask(lsa.m_networkLSANetworkMask),
      m_status(lsa.m_status), m_node_id(lsa.m_node_id) {
  NS_LOG_FUNCTION(this << &lsa);
  NS_ASSERT_MSG(
      IsEmpty(),
      "GlobalRoutingLSA::GlobalRoutingLSA (): Non-empty LSA in constructor");
  CopyLinkRecords(lsa);
}

GlobalRoutingLSA &GlobalRoutingLSA::operator=(const GlobalRoutingLSA &lsa) {
  NS_LOG_FUNCTION(this << &lsa);
  m_lsType = lsa.m_lsType;
  m_linkStateId = lsa.m_linkStateId;
  m_advertisingRtr = lsa.m_advertisingRtr;
  m_networkLSANetworkMask = lsa.m_networkLSANetworkMask,
  m_status = lsa.m_status;
  m_node_id = lsa.m_node_id;

  ClearLinkRecords();
  CopyLinkRecords(lsa);
  return *this;
}

void GlobalRoutingLSA::CopyLinkRecords(const GlobalRoutingLSA &lsa) {
  NS_LOG_FUNCTION(this << &lsa);
  for (auto i = lsa.m_linkRecords.begin(); i != lsa.m_linkRecords.end(); i++) {
    GlobalRoutingLinkRecord *pSrc = *i;
    auto pDst = new GlobalRoutingLinkRecord;

    pDst->SetLinkType(pSrc->GetLinkType());
    pDst->SetLinkId(pSrc->GetLinkId());
    pDst->SetLinkData(pSrc->GetLinkData());
    pDst->SetMetric(pSrc->GetMetric());

    m_linkRecords.push_back(pDst);
    pDst = nullptr;
  }

  m_attachedRouters = lsa.m_attachedRouters;
}

GlobalRoutingLSA::~GlobalRoutingLSA() {
  NS_LOG_FUNCTION(this);
  ClearLinkRecords();
}

void GlobalRoutingLSA::ClearLinkRecords() {
  NS_LOG_FUNCTION(this);
  for (auto i = m_linkRecords.begin(); i != m_linkRecords.end(); i++) {
    NS_LOG_LOGIC("Free link record");

    GlobalRoutingLinkRecord *p = *i;
    delete p;
    p = nullptr;

    *i = nullptr;
  }
  NS_LOG_LOGIC("Clear list");
  m_linkRecords.clear();
}

uint32_t GlobalRoutingLSA::AddLinkRecord(GlobalRoutingLinkRecord *lr) {
  NS_LOG_FUNCTION(this << lr);
  m_linkRecords.push_back(lr);
  return m_linkRecords.size();
}

uint32_t GlobalRoutingLSA::GetNLinkRecords() const {
  NS_LOG_FUNCTION(this);
  return m_linkRecords.size();
}

GlobalRoutingLinkRecord *GlobalRoutingLSA::GetLinkRecord(uint32_t n) const {
  NS_LOG_FUNCTION(this << n);
  uint32_t j = 0;
  for (auto i = m_linkRecords.begin(); i != m_linkRecords.end(); i++, j++) {
    if (j == n) {
      return *i;
    }
  }
  NS_ASSERT_MSG(false, "GlobalRoutingLSA::GetLinkRecord (): invalid index");
  return nullptr;
}

bool GlobalRoutingLSA::IsEmpty() const {
  NS_LOG_FUNCTION(this);
  return m_linkRecords.empty();
}

GlobalRoutingLSA::LSType GlobalRoutingLSA::GetLSType() const {
  NS_LOG_FUNCTION(this);
  return m_lsType;
}

void GlobalRoutingLSA::SetLSType(GlobalRoutingLSA::LSType typ) {
  NS_LOG_FUNCTION(this << typ);
  m_lsType = typ;
}

Ipv4Address GlobalRoutingLSA::GetLinkStateId() const {
  NS_LOG_FUNCTION(this);
  return m_linkStateId;
}

void GlobalRoutingLSA::SetLinkStateId(Ipv4Address addr) {
  NS_LOG_FUNCTION(this << addr);
  m_linkStateId = addr;
}

Ipv4Address GlobalRoutingLSA::GetAdvertisingRouter() const {
  NS_LOG_FUNCTION(this);
  return m_advertisingRtr;
}

void GlobalRoutingLSA::SetAdvertisingRouter(Ipv4Address addr) {
  NS_LOG_FUNCTION(this << addr);
  m_advertisingRtr = addr;
}

void GlobalRoutingLSA::SetNetworkLSANetworkMask(Ipv4Mask mask) {
  NS_LOG_FUNCTION(this << mask);
  m_networkLSANetworkMask = mask;
}

Ipv4Mask GlobalRoutingLSA::GetNetworkLSANetworkMask() const {
  NS_LOG_FUNCTION(this);
  return m_networkLSANetworkMask;
}

GlobalRoutingLSA::SPFStatus GlobalRoutingLSA::GetStatus() const {
  NS_LOG_FUNCTION(this);
  return m_status;
}

uint32_t GlobalRoutingLSA::AddAttachedRouter(Ipv4Address addr) {
  NS_LOG_FUNCTION(this << addr);
  m_attachedRouters.push_back(addr);
  return m_attachedRouters.size();
}

uint32_t GlobalRoutingLSA::GetNAttachedRouters() const {
  NS_LOG_FUNCTION(this);
  return m_attachedRouters.size();
}

Ipv4Address GlobalRoutingLSA::GetAttachedRouter(uint32_t n) const {
  NS_LOG_FUNCTION(this << n);
  uint32_t j = 0;
  for (auto i = m_attachedRouters.begin(); i != m_attachedRouters.end();
       i++, j++) {
    if (j == n) {
      return *i;
    }
  }
  NS_ASSERT_MSG(false, "GlobalRoutingLSA::GetAttachedRouter (): invalid index");
  return Ipv4Address("0.0.0.0");
}

void GlobalRoutingLSA::SetStatus(GlobalRoutingLSA::SPFStatus status) {
  NS_LOG_FUNCTION(this << status);
  m_status = status;
}

Ptr<Node> GlobalRoutingLSA::GetNode() const {
  NS_LOG_FUNCTION(this);
  return NodeList::GetNode(m_node_id);
}

void GlobalRoutingLSA::SetNode(Ptr<Node> node) {
  NS_LOG_FUNCTION(this << node);
  m_node_id = node->GetId();
}

void GlobalRoutingLSA::Print(std::ostream &os) const {
  NS_LOG_FUNCTION(this << &os);
  os << std::endl;
  os << "========== Global Routing LSA ==========" << std::endl;
  os << "m_lsType = " << m_lsType;
  if (m_lsType == GlobalRoutingLSA::RouterLSA) {
    os << " (GlobalRoutingLSA::RouterLSA)";
  } else if (m_lsType == GlobalRoutingLSA::NetworkLSA) {
    os << " (GlobalRoutingLSA::NetworkLSA)";
  } else if (m_lsType == GlobalRoutingLSA::ASExternalLSAs) {
    os << " (GlobalRoutingLSA::ASExternalLSA)";
  } else {
    os << "(Unknown LSType)";
  }
  os << std::endl;

  os << "m_linkStateId = " << m_linkStateId << " (Router ID)" << std::endl;
  os << "m_advertisingRtr = " << m_advertisingRtr << " (Router ID)"
     << std::endl;

  if (m_lsType == GlobalRoutingLSA::RouterLSA) {
    for (auto i = m_linkRecords.begin(); i != m_linkRecords.end(); i++) {
      GlobalRoutingLinkRecord *p = *i;

      os << "---------- RouterLSA Link Record ----------" << std::endl;
      os << "m_linkType = " << p->m_linkType;
      if (p->m_linkType == GlobalRoutingLinkRecord::PointToPoint) {
        os << " (GlobalRoutingLinkRecord::PointToPoint)" << std::endl;
        os << "m_linkId = " << p->m_linkId << std::endl;
        os << "m_linkData = " << p->m_linkData << std::endl;
        os << "m_metric = " << p->m_metric << std::endl;
      } else if (p->m_linkType == GlobalRoutingLinkRecord::TransitNetwork) {
        os << " (GlobalRoutingLinkRecord::TransitNetwork)" << std::endl;
        os << "m_linkId = " << p->m_linkId << " (Designated router for network)"
           << std::endl;
        os << "m_linkData = " << p->m_linkData << " (This router's IP address)"
           << std::endl;
        os << "m_metric = " << p->m_metric << std::endl;
      } else if (p->m_linkType == GlobalRoutingLinkRecord::StubNetwork) {
        os << " (GlobalRoutingLinkRecord::StubNetwork)" << std::endl;
        os << "m_linkId = " << p->m_linkId
           << " (Network number of attached network)" << std::endl;
        os << "m_linkData = " << p->m_linkData
           << " (Network mask of attached network)" << std::endl;
        os << "m_metric = " << p->m_metric << std::endl;
      } else {
        os << " (Unknown LinkType)" << std::endl;
        os << "m_linkId = " << p->m_linkId << std::endl;
        os << "m_linkData = " << p->m_linkData << std::endl;
        os << "m_metric = " << p->m_metric << std::endl;
      }
      os << "---------- End RouterLSA Link Record ----------" << std::endl;
    }
  } else if (m_lsType == GlobalRoutingLSA::NetworkLSA) {
    os << "---------- NetworkLSA Link Record ----------" << std::endl;
    os << "m_networkLSANetworkMask = " << m_networkLSANetworkMask << std::endl;
    for (auto i = m_attachedRouters.begin(); i != m_attachedRouters.end();
         i++) {
      Ipv4Address p = *i;
      os << "attachedRouter = " << p << std::endl;
    }
    os << "---------- End NetworkLSA Link Record ----------" << std::endl;
  } else if (m_lsType == GlobalRoutingLSA::ASExternalLSAs) {
    os << "---------- ASExternalLSA Link Record --------" << std::endl;
    os << "m_linkStateId = " << m_linkStateId << std::endl;
    os << "m_networkLSANetworkMask = " << m_networkLSANetworkMask << std::endl;
  } else {
    NS_ASSERT_MSG(0, "Illegal LSA LSType: " << m_lsType);
  }
  os << "========== End Global Routing LSA ==========" << std::endl;
}

std::ostream &operator<<(std::ostream &os, GlobalRoutingLSA &lsa) {
  lsa.Print(os);
  return os;
}

NS_OBJECT_ENSURE_REGISTERED(GlobalRouter);

TypeId GlobalRouter::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::GlobalRouter").SetParent<Object>().SetGroupName("Internet");
  return tid;
}

GlobalRouter::GlobalRouter() : m_LSAs() {
  NS_LOG_FUNCTION(this);
  m_routerId.Set(GlobalRouteManager::AllocateRouterId());
}

GlobalRouter::~GlobalRouter() {
  NS_LOG_FUNCTION(this);
  ClearLSAs();
}

void GlobalRouter::SetRoutingProtocol(Ptr<Ipv4GlobalRouting> routing) {
  NS_LOG_FUNCTION(this << routing);
  m_routingProtocol = routing;
}

Ptr<Ipv4GlobalRouting> GlobalRouter::GetRoutingProtocol() {
  NS_LOG_FUNCTION(this);
  return m_routingProtocol;
}

void GlobalRouter::DoDispose() {
  NS_LOG_FUNCTION(this);
  m_routingProtocol = nullptr;
  for (auto k = m_injectedRoutes.begin(); k != m_injectedRoutes.end();
       k = m_injectedRoutes.erase(k)) {
    delete (*k);
  }
  Object::DoDispose();
}

void GlobalRouter::ClearLSAs() {
  NS_LOG_FUNCTION(this);
  for (auto i = m_LSAs.begin(); i != m_LSAs.end(); i++) {
    NS_LOG_LOGIC("Free LSA");

    GlobalRoutingLSA *p = *i;
    delete p;
    p = nullptr;

    *i = nullptr;
  }
  NS_LOG_LOGIC("Clear list of LSAs");
  m_LSAs.clear();
}

Ipv4Address GlobalRouter::GetRouterId() const {
  NS_LOG_FUNCTION(this);
  return m_routerId;
}

uint32_t GlobalRouter::DiscoverLSAs() {
  NS_LOG_FUNCTION(this);
  Ptr<Node> node = GetObject<Node>();
  NS_ABORT_MSG_UNLESS(
      node,
      "GlobalRouter::DiscoverLSAs (): GetObject for <Node> interface failed");
  NS_LOG_LOGIC("For node " << node->GetId());

  ClearLSAs();

  NetDeviceContainer c;

  Ptr<Ipv4> ipv4Local = node->GetObject<Ipv4>();
  NS_ABORT_MSG_UNLESS(
      ipv4Local,
      "GlobalRouter::DiscoverLSAs (): GetObject for <Ipv4> interface failed");

  auto pLSA = new GlobalRoutingLSA;
  pLSA->SetLSType(GlobalRoutingLSA::RouterLSA);
  pLSA->SetLinkStateId(m_routerId);
  pLSA->SetAdvertisingRouter(m_routerId);
  pLSA->SetStatus(GlobalRoutingLSA::LSA_SPF_NOT_EXPLORED);
  pLSA->SetNode(node);

  uint32_t numDevices = node->GetNDevices();

  for (uint32_t i = 0; i < numDevices; ++i) {
    Ptr<NetDevice> ndLocal = node->GetDevice(i);

    if (DynamicCast<LoopbackNetDevice>(ndLocal)) {
      continue;
    }

    if (NetDeviceIsBridged(ndLocal)) {
      int32_t ifIndex = ipv4Local->GetInterfaceForDevice(ndLocal);
      NS_ABORT_MSG_IF(ifIndex != -1,
                      "GlobalRouter::DiscoverLSAs(): Bridge ports must not "
                      "have an IPv4 interface index");
    }

    int32_t interfaceNumber = ipv4Local->GetInterfaceForDevice(ndLocal);
    if (interfaceNumber == -1 || !(ipv4Local->IsUp(interfaceNumber) &&
                                   ipv4Local->IsForwarding(interfaceNumber))) {
      NS_LOG_LOGIC(
          "Net device "
          << ndLocal
          << "has no IP interface or is not enabled for forwarding, skipping");
      continue;
    }

    if (ndLocal->IsBroadcast() && !ndLocal->IsPointToPoint()) {
      NS_LOG_LOGIC("Broadcast link");
      ProcessBroadcastLink(ndLocal, pLSA, c);
    } else if (ndLocal->IsPointToPoint()) {
      NS_LOG_LOGIC("Point=to-point link");
      ProcessPointToPointLink(ndLocal, pLSA);
    } else {
      NS_ASSERT_MSG(0, "GlobalRouter::DiscoverLSAs (): unknown link type");
    }
  }

  NS_LOG_LOGIC("========== LSA for node " << node->GetId() << " ==========");
  NS_LOG_LOGIC(*pLSA);
  m_LSAs.push_back(pLSA);
  pLSA = nullptr;

  uint32_t nDesignatedRouters = c.GetN();
  if (nDesignatedRouters > 0) {
    NS_LOG_LOGIC("Build Network LSAs");
    BuildNetworkLSAs(c);
  }

  for (auto i = m_injectedRoutes.begin(); i != m_injectedRoutes.end(); i++) {
    auto pLSA = new GlobalRoutingLSA;
    pLSA->SetLSType(GlobalRoutingLSA::ASExternalLSAs);
    pLSA->SetLinkStateId((*i)->GetDestNetwork());
    pLSA->SetAdvertisingRouter(m_routerId);
    pLSA->SetNetworkLSANetworkMask((*i)->GetDestNetworkMask());
    pLSA->SetStatus(GlobalRoutingLSA::LSA_SPF_NOT_EXPLORED);
    m_LSAs.push_back(pLSA);
  }
  return m_LSAs.size();
}

void GlobalRouter::ProcessBroadcastLink(Ptr<NetDevice> nd,
                                        GlobalRoutingLSA *pLSA,
                                        NetDeviceContainer &c) {
  NS_LOG_FUNCTION(this << nd << pLSA << &c);

  if (nd->IsBridge()) {
    ProcessBridgedBroadcastLink(nd, pLSA, c);
  } else {
    ProcessSingleBroadcastLink(nd, pLSA, c);
  }
}

void GlobalRouter::ProcessSingleBroadcastLink(Ptr<NetDevice> nd,
                                              GlobalRoutingLSA *pLSA,
                                              NetDeviceContainer &c) {
  NS_LOG_FUNCTION(this << nd << pLSA << &c);

  auto plr = new GlobalRoutingLinkRecord;
  NS_ABORT_MSG_IF(
      plr == nullptr,
      "GlobalRouter::ProcessSingleBroadcastLink(): Can't alloc link record");

  Ptr<Node> node = nd->GetNode();

  Ptr<Ipv4> ipv4Local = node->GetObject<Ipv4>();
  NS_ABORT_MSG_UNLESS(ipv4Local, "GlobalRouter::ProcessSingleBroadcastLink (): "
                                 "GetObject for <Ipv4> interface failed");

  int32_t interfaceLocal = ipv4Local->GetInterfaceForDevice(nd);
  NS_ABORT_MSG_IF(interfaceLocal == -1,
                  "GlobalRouter::ProcessSingleBroadcastLink(): No interface "
                  "index associated with device");

  if (ipv4Local->GetNAddresses(interfaceLocal) > 1) {
    NS_LOG_WARN("Warning, interface has multiple IP addresses; using only the "
                "primary one");
  }
  Ipv4Address addrLocal = ipv4Local->GetAddress(interfaceLocal, 0).GetLocal();
  Ipv4Mask maskLocal = ipv4Local->GetAddress(interfaceLocal, 0).GetMask();
  NS_LOG_LOGIC("Working with local address " << addrLocal);
  uint16_t metricLocal = ipv4Local->GetMetric(interfaceLocal);

  ClearBridgesVisited();
  if (!AnotherRouterOnLink(nd)) {
    NS_LOG_LOGIC("Router-LSA Stub Network");
    plr->SetLinkType(GlobalRoutingLinkRecord::StubNetwork);

    plr->SetLinkId(addrLocal.CombineMask(maskLocal));

    Ipv4Address maskLocalAddr;
    maskLocalAddr.Set(maskLocal.Get());
    plr->SetLinkData(maskLocalAddr);
    plr->SetMetric(metricLocal);
    pLSA->AddLinkRecord(plr);
    plr = nullptr;
  } else {
    NS_LOG_LOGIC("Router-LSA Transit Network");
    plr->SetLinkType(GlobalRoutingLinkRecord::TransitNetwork);

    ClearBridgesVisited();
    Ipv4Address designatedRtr;
    designatedRtr = FindDesignatedRouterForLink(nd);

    if (designatedRtr != "255.255.255.255") {
      Ipv4Address networkHere = addrLocal.CombineMask(maskLocal);
      Ipv4Address networkThere = designatedRtr.CombineMask(maskLocal);
      NS_ABORT_MSG_UNLESS(networkHere == networkThere,
                          "GlobalRouter::ProcessSingleBroadcastLink(): Network "
                          "number confusion ("
                              << addrLocal << "/" << maskLocal.GetPrefixLength()
                              << ", " << designatedRtr << "/"
                              << maskLocal.GetPrefixLength() << ")");
    }
    if (designatedRtr == addrLocal) {
      c.Add(nd);
      NS_LOG_LOGIC("Node " << node->GetId() << " elected a designated router");
    }
    plr->SetLinkId(designatedRtr);

    plr->SetLinkData(addrLocal);
    plr->SetMetric(metricLocal);
    pLSA->AddLinkRecord(plr);
    plr = nullptr;
  }
}

void GlobalRouter::ProcessBridgedBroadcastLink(Ptr<NetDevice> nd,
                                               GlobalRoutingLSA *pLSA,
                                               NetDeviceContainer &c) {
  NS_LOG_FUNCTION(this << nd << pLSA << &c);
  NS_ASSERT_MSG(nd->IsBridge(), "GlobalRouter::ProcessBridgedBroadcastLink(): "
                                "Called with non-bridge net device");

#if 0

  Ptr<BridgeNetDevice> bnd = nd->GetObject<BridgeNetDevice> ();
  NS_ABORT_MSG_UNLESS (bnd, "GlobalRouter::DiscoverLSAs (): GetObject for <BridgeNetDevice> failed");

  Ptr<Node> node = nd->GetNode ();
  Ptr<Ipv4> ipv4Local = node->GetObject<Ipv4> ();
  NS_ABORT_MSG_UNLESS (ipv4Local, "GlobalRouter::ProcessBridgedBroadcastLink (): GetObject for <Ipv4> interface failed");

  int32_t interfaceLocal = ipv4Local->GetInterfaceForDevice (nd);
  NS_ABORT_MSG_IF (interfaceLocal == -1, "GlobalRouter::ProcessBridgedBroadcastLink(): No interface index associated with device");

  if (ipv4Local->GetNAddresses (interfaceLocal) > 1)
    {
      NS_LOG_WARN ("Warning, interface has multiple IP addresses; using only the primary one");
    }
  Ipv4Address addrLocal = ipv4Local->GetAddress (interfaceLocal, 0).GetLocal ();
  Ipv4Mask maskLocal = ipv4Local->GetAddress (interfaceLocal, 0).GetMask ();
  NS_LOG_LOGIC ("Working with local address " << addrLocal);
  uint16_t metricLocal = ipv4Local->GetMetric (interfaceLocal);


  bool areTransitNetwork = false;
  Ipv4Address designatedRtr ("255.255.255.255");

  for (uint32_t i = 0; i < bnd->GetNBridgePorts (); ++i)
    {
      Ptr<NetDevice> ndTemp = bnd->GetBridgePort (i);

      ClearBridgesVisited ();
      if (AnotherRouterOnLink (ndTemp))
        {
          areTransitNetwork = true;

          ClearBridgesVisited ();
          Ipv4Address designatedRtrTemp = FindDesignatedRouterForLink (ndTemp);

          if (designatedRtrTemp != "255.255.255.255")
            {
              Ipv4Address networkHere = addrLocal.CombineMask (maskLocal);
              Ipv4Address networkThere = designatedRtrTemp.CombineMask (maskLocal);
              NS_ABORT_MSG_UNLESS (networkHere == networkThere,
                                   "GlobalRouter::ProcessSingleBroadcastLink(): Network number confusion (" <<
                                   addrLocal << "/" << maskLocal.GetPrefixLength () << ", " <<
                                   designatedRtrTemp << "/" << maskLocal.GetPrefixLength () << ")");
            }
          if (designatedRtrTemp < designatedRtr)
            {
              designatedRtr = designatedRtrTemp;
            }
        }
    }

  GlobalRoutingLinkRecord *plr = new GlobalRoutingLinkRecord;
  NS_ABORT_MSG_IF (plr == 0, "GlobalRouter::ProcessBridgedBroadcastLink(): Can't alloc link record");

  if (areTransitNetwork == false)
    {
      NS_LOG_LOGIC ("Router-LSA Stub Network");
      plr->SetLinkType (GlobalRoutingLinkRecord::StubNetwork);

      plr->SetLinkId (addrLocal.CombineMask (maskLocal));

      Ipv4Address maskLocalAddr;
      maskLocalAddr.Set (maskLocal.Get ());
      plr->SetLinkData (maskLocalAddr);
      plr->SetMetric (metricLocal);
      pLSA->AddLinkRecord (plr);
      plr = 0;
    }
  else
    {
      NS_LOG_LOGIC ("Router-LSA Transit Network");
      plr->SetLinkType (GlobalRoutingLinkRecord::TransitNetwork);

      if (designatedRtr == addrLocal)
        {
          c.Add (nd);
          NS_LOG_LOGIC ("Node " << node->GetId () << " elected a designated router");
        }
      plr->SetLinkId (designatedRtr);

      plr->SetLinkData (addrLocal);
      plr->SetMetric (metricLocal);
      pLSA->AddLinkRecord (plr);
      plr = 0;
    }
#endif
}

void GlobalRouter::ProcessPointToPointLink(Ptr<NetDevice> ndLocal,
                                           GlobalRoutingLSA *pLSA) {
  NS_LOG_FUNCTION(this << ndLocal << pLSA);

  Ptr<Node> nodeLocal = ndLocal->GetNode();

  Ptr<Ipv4> ipv4Local = nodeLocal->GetObject<Ipv4>();
  NS_ABORT_MSG_UNLESS(ipv4Local, "GlobalRouter::ProcessPointToPointLink (): "
                                 "GetObject for <Ipv4> interface failed");

  int32_t interfaceLocal = ipv4Local->GetInterfaceForDevice(ndLocal);
  NS_ABORT_MSG_IF(interfaceLocal == -1,
                  "GlobalRouter::ProcessPointToPointLink (): No interface "
                  "index associated with device");

  if (ipv4Local->GetNAddresses(interfaceLocal) > 1) {
    NS_LOG_WARN("Warning, interface has multiple IP addresses; using only the "
                "primary one");
  }
  Ipv4Address addrLocal = ipv4Local->GetAddress(interfaceLocal, 0).GetLocal();
  NS_LOG_LOGIC("Working with local address " << addrLocal);
  uint16_t metricLocal = ipv4Local->GetMetric(interfaceLocal);

  Ptr<Channel> ch = ndLocal->GetChannel();

  Ptr<NetDevice> ndRemote = GetAdjacent(ndLocal, ch);

  Ptr<Node> nodeRemote = ndRemote->GetNode();
  Ptr<Ipv4> ipv4Remote = nodeRemote->GetObject<Ipv4>();
  NS_ABORT_MSG_UNLESS(ipv4Remote, "GlobalRouter::ProcessPointToPointLink(): "
                                  "GetObject for remote <Ipv4> failed");

  Ptr<GlobalRouter> rtrRemote = nodeRemote->GetObject<GlobalRouter>();
  if (!rtrRemote) {
    return;
  }
  Ipv4Address rtrIdRemote = rtrRemote->GetRouterId();
  NS_LOG_LOGIC("Working with remote router " << rtrIdRemote);

  int32_t interfaceRemote = ipv4Remote->GetInterfaceForDevice(ndRemote);
  NS_ABORT_MSG_IF(interfaceRemote == -1,
                  "GlobalRouter::ProcessPointToPointLinks(): No interface "
                  "index associated with "
                  "remote device");

  if (ipv4Remote->GetNAddresses(interfaceRemote) > 1) {
    NS_LOG_WARN("Warning, interface has multiple IP addresses; using only the "
                "primary one");
  }
  Ipv4Address addrRemote =
      ipv4Remote->GetAddress(interfaceRemote, 0).GetLocal();
  Ipv4Mask maskRemote = ipv4Remote->GetAddress(interfaceRemote, 0).GetMask();
  NS_LOG_LOGIC("Working with remote address " << addrRemote);

  GlobalRoutingLinkRecord *plr;
  if (ipv4Remote->IsUp(interfaceRemote)) {
    NS_LOG_LOGIC("Remote side interface " << interfaceRemote
                                          << " is up-- add a type 1 link");

    plr = new GlobalRoutingLinkRecord;
    NS_ABORT_MSG_IF(
        plr == nullptr,
        "GlobalRouter::ProcessPointToPointLink(): Can't alloc link record");
    plr->SetLinkType(GlobalRoutingLinkRecord::PointToPoint);
    plr->SetLinkId(rtrIdRemote);
    plr->SetLinkData(addrLocal);
    plr->SetMetric(metricLocal);
    pLSA->AddLinkRecord(plr);
    plr = nullptr;
  }

  plr = new GlobalRoutingLinkRecord;
  NS_ABORT_MSG_IF(
      plr == nullptr,
      "GlobalRouter::ProcessPointToPointLink(): Can't alloc link record");
  plr->SetLinkType(GlobalRoutingLinkRecord::StubNetwork);
  plr->SetLinkId(addrRemote);
  plr->SetLinkData(Ipv4Address(maskRemote.Get()));
  plr->SetMetric(metricLocal);
  pLSA->AddLinkRecord(plr);
  plr = nullptr;
}

void GlobalRouter::BuildNetworkLSAs(NetDeviceContainer c) {
  NS_LOG_FUNCTION(this << &c);

  uint32_t nDesignatedRouters = c.GetN();
  NS_LOG_DEBUG("Number of designated routers: " << nDesignatedRouters);

  for (uint32_t i = 0; i < nDesignatedRouters; ++i) {
    Ptr<NetDevice> ndLocal = c.Get(i);
    Ptr<Node> node = ndLocal->GetNode();

    Ptr<Ipv4> ipv4Local = node->GetObject<Ipv4>();
    NS_ABORT_MSG_UNLESS(ipv4Local, "GlobalRouter::ProcessPointToPointLink (): "
                                   "GetObject for <Ipv4> interface failed");

    int32_t interfaceLocal = ipv4Local->GetInterfaceForDevice(ndLocal);
    NS_ABORT_MSG_IF(interfaceLocal == -1,
                    "GlobalRouter::BuildNetworkLSAs (): No interface index "
                    "associated with device");

    if (ipv4Local->GetNAddresses(interfaceLocal) > 1) {
      NS_LOG_WARN("Warning, interface has multiple IP addresses; using only "
                  "the primary one");
    }
    Ipv4Address addrLocal = ipv4Local->GetAddress(interfaceLocal, 0).GetLocal();
    Ipv4Mask maskLocal = ipv4Local->GetAddress(interfaceLocal, 0).GetMask();

    auto pLSA = new GlobalRoutingLSA;
    NS_ABORT_MSG_IF(
        pLSA == nullptr,
        "GlobalRouter::BuildNetworkLSAs(): Can't alloc link record");

    pLSA->SetLSType(GlobalRoutingLSA::NetworkLSA);
    pLSA->SetLinkStateId(addrLocal);
    pLSA->SetAdvertisingRouter(m_routerId);
    pLSA->SetNetworkLSANetworkMask(maskLocal);
    pLSA->SetStatus(GlobalRoutingLSA::LSA_SPF_NOT_EXPLORED);
    pLSA->SetNode(node);

    ClearBridgesVisited();
    Ptr<Channel> ch = ndLocal->GetChannel();
    std::size_t nDevices = ch->GetNDevices();
    NS_ASSERT(nDevices);
    NetDeviceContainer deviceList = FindAllNonBridgedDevicesOnLink(ch);
    NS_LOG_LOGIC("Found " << deviceList.GetN()
                          << " non-bridged devices on channel");

    for (uint32_t i = 0; i < deviceList.GetN(); i++) {
      Ptr<NetDevice> tempNd = deviceList.Get(i);
      NS_ASSERT(tempNd);
      if (tempNd == ndLocal) {
        NS_LOG_LOGIC("Adding " << addrLocal << " to Network LSA");
        pLSA->AddAttachedRouter(addrLocal);
        continue;
      }
      Ptr<Node> tempNode = tempNd->GetNode();

      Ptr<GlobalRouter> rtr = tempNode->GetObject<GlobalRouter>();
      if (!rtr) {
        NS_LOG_LOGIC("Node "
                     << tempNode->GetId()
                     << " does not have GlobalRouter interface--skipping");
        continue;
      }

      Ptr<Ipv4> tempIpv4 = tempNode->GetObject<Ipv4>();
      int32_t tempInterface = tempIpv4->GetInterfaceForDevice(tempNd);

      if (tempInterface != -1) {
        Ptr<Ipv4> tempIpv4 = tempNode->GetObject<Ipv4>();
        NS_ASSERT(tempIpv4);
        if (!tempIpv4->IsUp(tempInterface)) {
          NS_LOG_LOGIC("Remote side interface " << tempInterface << " not up");
        } else {
          if (tempIpv4->GetNAddresses(tempInterface) > 1) {
            NS_LOG_WARN(
                "Warning, interface has multiple IP addresses; using only the "
                "primary one");
          }
          Ipv4Address tempAddr =
              tempIpv4->GetAddress(tempInterface, 0).GetLocal();
          NS_LOG_LOGIC("Adding " << tempAddr << " to Network LSA");
          pLSA->AddAttachedRouter(tempAddr);
        }
      } else {
        NS_LOG_LOGIC("Node " << tempNode->GetId() << " device " << tempNd
                             << " does not have IPv4 interface; skipping");
      }
    }
    m_LSAs.push_back(pLSA);
    NS_LOG_LOGIC("========== LSA for node " << node->GetId() << " ==========");
    NS_LOG_LOGIC(*pLSA);
    pLSA = nullptr;
  }
}

NetDeviceContainer
GlobalRouter::FindAllNonBridgedDevicesOnLink(Ptr<Channel> ch) const {
  NS_LOG_FUNCTION(this << ch);
  NetDeviceContainer c;

  for (std::size_t i = 0; i < ch->GetNDevices(); i++) {
    Ptr<NetDevice> nd = ch->GetDevice(i);
    NS_LOG_LOGIC("checking to see if the device " << nd << " is bridged");
    Ptr<BridgeNetDevice> bnd = NetDeviceIsBridged(nd);
    if (bnd && !BridgeHasAlreadyBeenVisited(bnd)) {
      NS_LOG_LOGIC("Device is bridged by BridgeNetDevice "
                   << bnd << " with " << bnd->GetNBridgePorts() << " ports");
      MarkBridgeAsVisited(bnd);
      for (uint32_t j = 0; j < bnd->GetNBridgePorts(); j++) {
        Ptr<NetDevice> bridgedDevice = bnd->GetBridgePort(j);
        if (bridgedDevice->GetChannel() == ch) {
          NS_LOG_LOGIC("Skipping my own device/channel");
          continue;
        }
        NS_LOG_LOGIC("Calling on channel " << bridgedDevice->GetChannel());
        c.Add(FindAllNonBridgedDevicesOnLink(bridgedDevice->GetChannel()));
      }
    } else {
      NS_LOG_LOGIC("Device is not bridged; adding");
      c.Add(nd);
    }
  }
  NS_LOG_LOGIC("Found " << c.GetN() << " devices");
  return c;
}

Ipv4Address
GlobalRouter::FindDesignatedRouterForLink(Ptr<NetDevice> ndLocal) const {
  NS_LOG_FUNCTION(this << ndLocal);

  Ptr<Channel> ch = ndLocal->GetChannel();
  uint32_t nDevices = ch->GetNDevices();
  NS_ASSERT(nDevices);

  NS_LOG_LOGIC("Looking for designated router off of net device "
               << ndLocal << " on node " << ndLocal->GetNode()->GetId());

  Ipv4Address designatedRtr("255.255.255.255");

  for (uint32_t i = 0; i < nDevices; i++) {
    Ptr<NetDevice> ndOther = ch->GetDevice(i);
    NS_ASSERT(ndOther);

    Ptr<Node> nodeOther = ndOther->GetNode();

    NS_LOG_LOGIC("Examine channel device " << i << " on node "
                                           << nodeOther->GetId());

    NS_LOG_LOGIC("checking to see if the device is bridged");
    Ptr<BridgeNetDevice> bnd = NetDeviceIsBridged(ndOther);
    if (bnd) {
      NS_LOG_LOGIC("Device is bridged by BridgeNetDevice " << bnd);

      if (ndLocal == ndOther) {
        NS_LOG_LOGIC("Skip -- it is where we came from.");
        continue;
      }

      NS_LOG_LOGIC("Checking for router on bridge net device " << bnd);
      Ptr<GlobalRouter> rtr = nodeOther->GetObject<GlobalRouter>();
      Ptr<Ipv4> ipv4 = nodeOther->GetObject<Ipv4>();
      if (rtr && ipv4) {
        int32_t interfaceOther = ipv4->GetInterfaceForDevice(bnd);
        if (interfaceOther != -1) {
          NS_LOG_LOGIC("Found router on bridge net device " << bnd);
          if (!ipv4->IsUp(interfaceOther)) {
            NS_LOG_LOGIC("Remote side interface " << interfaceOther
                                                  << " not up");
            continue;
          }
          if (ipv4->GetNAddresses(interfaceOther) > 1) {
            NS_LOG_WARN(
                "Warning, interface has multiple IP addresses; using only the "
                "primary one");
          }
          Ipv4Address addrOther =
              ipv4->GetAddress(interfaceOther, 0).GetLocal();
          designatedRtr = addrOther < designatedRtr ? addrOther : designatedRtr;
          NS_LOG_LOGIC("designated router now " << designatedRtr);
        }
      }

      if (BridgeHasAlreadyBeenVisited(bnd)) {
        NS_ABORT_MSG("ERROR: L2 forwarding loop detected!");
      }

      MarkBridgeAsVisited(bnd);

      NS_LOG_LOGIC("Looking through bridge ports of bridge net device " << bnd);
      for (uint32_t j = 0; j < bnd->GetNBridgePorts(); ++j) {
        Ptr<NetDevice> ndBridged = bnd->GetBridgePort(j);
        NS_LOG_LOGIC("Examining bridge port " << j << " device " << ndBridged);
        if (ndBridged == ndOther) {
          NS_LOG_LOGIC("That bridge port is me, don't walk backward");
          continue;
        }

        NS_LOG_LOGIC("Recursively looking for routers down bridge port "
                     << ndBridged);
        Ipv4Address addrOther = FindDesignatedRouterForLink(ndBridged);
        designatedRtr = addrOther < designatedRtr ? addrOther : designatedRtr;
        NS_LOG_LOGIC("designated router now " << designatedRtr);
      }
    } else {
      NS_LOG_LOGIC("This device is not bridged");
      Ptr<Node> nodeOther = ndOther->GetNode();
      NS_ASSERT(nodeOther);

      Ptr<GlobalRouter> rtr = nodeOther->GetObject<GlobalRouter>();
      Ptr<Ipv4> ipv4 = nodeOther->GetObject<Ipv4>();
      if (rtr && ipv4) {
        int32_t interfaceOther = ipv4->GetInterfaceForDevice(ndOther);
        if (interfaceOther != -1) {
          if (!ipv4->IsUp(interfaceOther)) {
            NS_LOG_LOGIC("Remote side interface " << interfaceOther
                                                  << " not up");
            continue;
          }
          NS_LOG_LOGIC("Found router on net device " << ndOther);
          if (ipv4->GetNAddresses(interfaceOther) > 1) {
            NS_LOG_WARN(
                "Warning, interface has multiple IP addresses; using only the "
                "primary one");
          }
          Ipv4Address addrOther =
              ipv4->GetAddress(interfaceOther, 0).GetLocal();
          designatedRtr = addrOther < designatedRtr ? addrOther : designatedRtr;
          NS_LOG_LOGIC("designated router now " << designatedRtr);
        }
      }
    }
  }
  return designatedRtr;
}

bool GlobalRouter::AnotherRouterOnLink(Ptr<NetDevice> nd) const {
  NS_LOG_FUNCTION(this << nd);

  Ptr<Channel> ch = nd->GetChannel();
  if (!ch) {
    return false;
  }
  uint32_t nDevices = ch->GetNDevices();
  NS_ASSERT(nDevices);

  NS_LOG_LOGIC("Looking for routers off of net device "
               << nd << " on node " << nd->GetNode()->GetId());

  for (uint32_t i = 0; i < nDevices; i++) {
    Ptr<NetDevice> ndOther = ch->GetDevice(i);
    NS_ASSERT(ndOther);

    NS_LOG_LOGIC("Examine channel device " << i << " on node "
                                           << ndOther->GetNode()->GetId());

    if (ndOther == nd) {
      NS_LOG_LOGIC("Myself, skip");
      continue;
    }

    NS_LOG_LOGIC("checking to see if device is bridged");
    Ptr<BridgeNetDevice> bnd = NetDeviceIsBridged(ndOther);
    if (bnd) {
      NS_LOG_LOGIC("Device is bridged by net device " << bnd);

      if (BridgeHasAlreadyBeenVisited(bnd)) {
        NS_ABORT_MSG("ERROR: L2 forwarding loop detected!");
      }

      MarkBridgeAsVisited(bnd);

      NS_LOG_LOGIC("Looking through bridge ports of bridge net device " << bnd);
      for (uint32_t j = 0; j < bnd->GetNBridgePorts(); ++j) {
        Ptr<NetDevice> ndBridged = bnd->GetBridgePort(j);
        NS_LOG_LOGIC("Examining bridge port " << j << " device " << ndBridged);
        if (ndBridged == ndOther) {
          NS_LOG_LOGIC("That bridge port is me, skip");
          continue;
        }

        NS_LOG_LOGIC("Recursively looking for routers on bridge port "
                     << ndBridged);
        if (AnotherRouterOnLink(ndBridged)) {
          NS_LOG_LOGIC("Found routers on bridge port, return true");
          return true;
        }
      }
      NS_LOG_LOGIC("No routers on bridged net device, return false");
      return false;
    }

    NS_LOG_LOGIC("This device is not bridged");
    Ptr<Node> nodeTemp = ndOther->GetNode();
    NS_ASSERT(nodeTemp);

    Ptr<GlobalRouter> rtr = nodeTemp->GetObject<GlobalRouter>();
    if (rtr) {
      NS_LOG_LOGIC("Found GlobalRouter interface, return true");
      return true;
    } else {
      NS_LOG_LOGIC("No GlobalRouter interface on device, continue search");
    }
  }
  NS_LOG_LOGIC("No routers found, return false");
  return false;
}

uint32_t GlobalRouter::GetNumLSAs() const {
  NS_LOG_FUNCTION(this);
  return m_LSAs.size();
}

bool GlobalRouter::GetLSA(uint32_t n, GlobalRoutingLSA &lsa) const {
  NS_LOG_FUNCTION(this << n << &lsa);
  NS_ASSERT_MSG(lsa.IsEmpty(), "GlobalRouter::GetLSA (): Must pass empty LSA");
  auto i = m_LSAs.begin();
  uint32_t j = 0;

  for (; i != m_LSAs.end(); i++, j++) {
    if (j == n) {
      GlobalRoutingLSA *p = *i;
      lsa = *p;
      return true;
    }
  }

  return false;
}

void GlobalRouter::InjectRoute(Ipv4Address network, Ipv4Mask networkMask) {
  NS_LOG_FUNCTION(this << network << networkMask);
  auto route = new Ipv4RoutingTableEntry();
  *route = Ipv4RoutingTableEntry::CreateNetworkRouteTo(network, networkMask, 1);
  m_injectedRoutes.push_back(route);
}

Ipv4RoutingTableEntry *GlobalRouter::GetInjectedRoute(uint32_t index) {
  NS_LOG_FUNCTION(this << index);
  if (index < m_injectedRoutes.size()) {
    uint32_t tmp = 0;
    for (auto i = m_injectedRoutes.begin(); i != m_injectedRoutes.end(); i++) {
      if (tmp == index) {
        return *i;
      }
      tmp++;
    }
  }
  NS_ASSERT(false);
  return nullptr;
}

uint32_t GlobalRouter::GetNInjectedRoutes() {
  NS_LOG_FUNCTION(this);
  return m_injectedRoutes.size();
}

void GlobalRouter::RemoveInjectedRoute(uint32_t index) {
  NS_LOG_FUNCTION(this << index);
  NS_ASSERT(index < m_injectedRoutes.size());
  uint32_t tmp = 0;
  for (auto i = m_injectedRoutes.begin(); i != m_injectedRoutes.end(); i++) {
    if (tmp == index) {
      NS_LOG_LOGIC("Removing route " << index
                                     << "; size = " << m_injectedRoutes.size());
      delete *i;
      m_injectedRoutes.erase(i);
      return;
    }
    tmp++;
  }
}

bool GlobalRouter::WithdrawRoute(Ipv4Address network, Ipv4Mask networkMask) {
  NS_LOG_FUNCTION(this << network << networkMask);
  for (auto i = m_injectedRoutes.begin(); i != m_injectedRoutes.end(); i++) {
    if ((*i)->GetDestNetwork() == network &&
        (*i)->GetDestNetworkMask() == networkMask) {
      NS_LOG_LOGIC("Withdrawing route to network/mask " << network << "/"
                                                        << networkMask);
      delete *i;
      m_injectedRoutes.erase(i);
      return true;
    }
  }
  return false;
}

Ptr<NetDevice> GlobalRouter::GetAdjacent(Ptr<NetDevice> nd,
                                         Ptr<Channel> ch) const {
  NS_LOG_FUNCTION(this << nd << ch);
  NS_ASSERT_MSG(
      ch->GetNDevices() == 2,
      "GlobalRouter::GetAdjacent (): Channel with other than two devices");
  Ptr<NetDevice> nd1 = ch->GetDevice(0);
  Ptr<NetDevice> nd2 = ch->GetDevice(1);
  if (nd1 == nd) {
    return nd2;
  } else if (nd2 == nd) {
    return nd1;
  } else {
    NS_ASSERT_MSG(false,
                  "GlobalRouter::GetAdjacent (): Wrong or confused channel?");
    return nullptr;
  }
}

Ptr<BridgeNetDevice> GlobalRouter::NetDeviceIsBridged(Ptr<NetDevice> nd) const {
  NS_LOG_FUNCTION(this << nd);

  Ptr<Node> node = nd->GetNode();
  uint32_t nDevices = node->GetNDevices();

  for (uint32_t i = 0; i < nDevices; ++i) {
    Ptr<NetDevice> ndTest = node->GetDevice(i);
    NS_LOG_LOGIC("Examine device " << i << " " << ndTest);

    if (ndTest->IsBridge()) {
      NS_LOG_LOGIC("device " << i << " is a bridge net device");
      Ptr<BridgeNetDevice> bnd = ndTest->GetObject<BridgeNetDevice>();
      NS_ABORT_MSG_UNLESS(bnd, "GlobalRouter::DiscoverLSAs (): GetObject for "
                               "<BridgeNetDevice> failed");

      for (uint32_t j = 0; j < bnd->GetNBridgePorts(); ++j) {
        NS_LOG_LOGIC("Examine bridge port " << j << " "
                                            << bnd->GetBridgePort(j));
        if (bnd->GetBridgePort(j) == nd) {
          NS_LOG_LOGIC("Net device " << nd << " is bridged by " << bnd);
          return bnd;
        }
      }
    }
  }
  NS_LOG_LOGIC("Net device " << nd << " is not bridged");
  return nullptr;
}

void GlobalRouter::ClearBridgesVisited() const { m_bridgesVisited.clear(); }

bool GlobalRouter::BridgeHasAlreadyBeenVisited(
    Ptr<BridgeNetDevice> bridgeNetDevice) const {
  for (auto iter = m_bridgesVisited.begin(); iter != m_bridgesVisited.end();
       ++iter) {
    if (bridgeNetDevice == *iter) {
      NS_LOG_LOGIC("Bridge " << bridgeNetDevice << " has been visited.");
      return true;
    }
  }
  return false;
}

void GlobalRouter::MarkBridgeAsVisited(
    Ptr<BridgeNetDevice> bridgeNetDevice) const {
  NS_LOG_FUNCTION(this << bridgeNetDevice);
  m_bridgesVisited.push_back(bridgeNetDevice);
}

} // namespace ns3
