
#define NS_LOG_APPEND_CONTEXT                                                  \
  if (GetObject<Node>()) {                                                     \
    std::clog << "[node " << GetObject<Node>()->GetId() << "] ";               \
  }

#include "dsr-options.h"

#include "dsr-option-header.h"
#include "dsr-rcache.h"

#include "ns3/assert.h"
#include "ns3/fatal-error.h"
#include "ns3/icmpv4-l4-protocol.h"
#include "ns3/ip-l4-protocol.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4-header.h"
#include "ns3/ipv4-interface.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/ipv4-route.h"
#include "ns3/log.h"
#include "ns3/node-list.h"
#include "ns3/node.h"
#include "ns3/object-vector.h"
#include "ns3/pointer.h"
#include "ns3/ptr.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-header.h"
#include "ns3/uinteger.h"

#include <algorithm>
#include <ctime>
#include <list>
#include <map>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("DsrOptions");

namespace dsr {

NS_OBJECT_ENSURE_REGISTERED(DsrOptions);

TypeId DsrOptions::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::dsr::DsrOptions")
          .SetParent<Object>()
          .SetGroupName("Dsr")
          .AddAttribute("OptionNumber", "The Dsr option number.",
                        UintegerValue(0),
                        MakeUintegerAccessor(&DsrOptions::GetOptionNumber),
                        MakeUintegerChecker<uint8_t>())
          .AddTraceSource("Drop", "Packet dropped.",
                          MakeTraceSourceAccessor(&DsrOptions::m_dropTrace),
                          "ns3::Packet::TracedCallback")
          .AddTraceSource("Rx", "Receive DSR packet.",
                          MakeTraceSourceAccessor(&DsrOptions::m_rxPacketTrace),
                          "ns3::dsr::DsrOptionSRHeader::TracedCallback");
  return tid;
}

DsrOptions::DsrOptions() { NS_LOG_FUNCTION_NOARGS(); }

DsrOptions::~DsrOptions() { NS_LOG_FUNCTION_NOARGS(); }

void DsrOptions::SetNode(Ptr<Node> node) {
  NS_LOG_FUNCTION(this << node);
  m_node = node;
}

Ptr<Node> DsrOptions::GetNode() const {
  NS_LOG_FUNCTION_NOARGS();
  return m_node;
}

bool DsrOptions::ContainAddressAfter(Ipv4Address ipv4Address,
                                     Ipv4Address destAddress,
                                     std::vector<Ipv4Address> &nodeList) {
  NS_LOG_FUNCTION(this << ipv4Address << destAddress);
  auto it = find(nodeList.begin(), nodeList.end(), destAddress);

  for (auto i = it; i != nodeList.end(); ++i) {
    if ((ipv4Address == (*i)) && ((*i) != nodeList.back())) {
      return true;
    }
  }
  return false;
}

std::vector<Ipv4Address>
DsrOptions::CutRoute(Ipv4Address ipv4Address,
                     std::vector<Ipv4Address> &nodeList) {
  NS_LOG_FUNCTION(this << ipv4Address);
  auto it = find(nodeList.begin(), nodeList.end(), ipv4Address);
  std::vector<Ipv4Address> cutRoute;
  for (auto i = it; i != nodeList.end(); ++i) {
    cutRoute.push_back(*i);
  }
  return cutRoute;
}

Ptr<Ipv4Route> DsrOptions::SetRoute(Ipv4Address nextHop,
                                    Ipv4Address srcAddress) {
  NS_LOG_FUNCTION(this << nextHop << srcAddress);
  m_ipv4Route = Create<Ipv4Route>();
  m_ipv4Route->SetDestination(nextHop);
  m_ipv4Route->SetGateway(nextHop);
  m_ipv4Route->SetSource(srcAddress);
  return m_ipv4Route;
}

bool DsrOptions::ReverseRoutes(std::vector<Ipv4Address> &vec) {
  NS_LOG_FUNCTION(this);

  std::reverse(vec.begin(), vec.end());

  return true;
}

Ipv4Address DsrOptions::SearchNextHop(Ipv4Address ipv4Address,
                                      std::vector<Ipv4Address> &vec) {
  NS_LOG_FUNCTION(this << ipv4Address);
  Ipv4Address nextHop;
  NS_LOG_DEBUG("the vector size " << vec.size());
  if (vec.size() == 2) {
    NS_LOG_DEBUG("The two nodes are neighbors");
    nextHop = vec[1];
    return nextHop;
  }

  if (ipv4Address == vec.back()) {
    NS_LOG_DEBUG("We have reached to the final destination "
                 << ipv4Address << " " << vec.back());
    return ipv4Address;
  }
  for (auto i = vec.begin(); i != vec.end(); ++i) {
    if (ipv4Address == (*i)) {
      nextHop = *(++i);
      return nextHop;
    }
  }

  NS_LOG_DEBUG("next hop address not found, route corrupted");
  Ipv4Address none = "0.0.0.0";
  return none;
}

Ipv4Address DsrOptions::ReverseSearchNextHop(Ipv4Address ipv4Address,
                                             std::vector<Ipv4Address> &vec) {
  NS_LOG_FUNCTION(this << ipv4Address);
  Ipv4Address nextHop;
  if (vec.size() == 2) {
    NS_LOG_DEBUG("The two nodes are neighbors");
    nextHop = vec[0];
    return nextHop;
  }

  for (auto ri = vec.rbegin(); ri != vec.rend(); ++ri) {
    if (ipv4Address == (*ri)) {
      nextHop = *(++ri);
      return nextHop;
    }
  }

  NS_LOG_DEBUG("next hop address not found, route corrupted");
  Ipv4Address none = "0.0.0.0";
  return none;
}

Ipv4Address DsrOptions::ReverseSearchNextTwoHop(Ipv4Address ipv4Address,
                                                std::vector<Ipv4Address> &vec) {
  NS_LOG_FUNCTION(this << ipv4Address);
  Ipv4Address nextTwoHop;
  NS_LOG_DEBUG("The vector size " << vec.size());
  NS_ASSERT(vec.size() > 2);
  for (auto ri = vec.rbegin(); ri != vec.rend(); ++ri) {
    if (ipv4Address == (*ri)) {
      nextTwoHop = *(ri + 2);
      return nextTwoHop;
    }
  }
  NS_FATAL_ERROR("next hop address not found, route corrupted");
  Ipv4Address none = "0.0.0.0";
  return none;
}

void DsrOptions::PrintVector(std::vector<Ipv4Address> &vec) {
  NS_LOG_FUNCTION(this);
  if (vec.empty()) {
    NS_LOG_DEBUG("The vector is empty");
  } else {
    NS_LOG_DEBUG("Print all the elements in a vector");
    for (auto i = vec.begin(); i != vec.end(); ++i) {
      NS_LOG_DEBUG("The ip address " << *i);
    }
  }
}

bool DsrOptions::IfDuplicates(std::vector<Ipv4Address> &vec,
                              std::vector<Ipv4Address> &vec2) {
  NS_LOG_FUNCTION(this);
  for (auto i = vec.begin(); i != vec.end(); ++i) {
    for (auto j = vec2.begin(); j != vec2.end(); ++j) {
      if ((*i) == (*j)) {
        return true;
      }
    }
  }
  return false;
}

bool DsrOptions::CheckDuplicates(Ipv4Address ipv4Address,
                                 std::vector<Ipv4Address> &vec) {
  NS_LOG_FUNCTION(this << ipv4Address);
  for (auto i = vec.begin(); i != vec.end(); ++i) {
    if ((*i) == ipv4Address) {
      return true;
    }
  }
  return false;
}

void DsrOptions::RemoveDuplicates(std::vector<Ipv4Address> &vec) {
  NS_LOG_FUNCTION(this);
  std::vector<Ipv4Address> vec2(vec);
  PrintVector(vec2);
  vec.clear();
  for (auto i = vec2.begin(); i != vec2.end(); ++i) {
    if (vec.empty()) {
      vec.push_back(*i);
      continue;
    }

    for (auto j = vec.begin(); j != vec.end(); ++j) {
      if ((*i) == (*j)) {
        if ((j + 1) != vec.end()) {
          vec.erase(j + 1, vec.end());
        }

        break;
      } else if (j == (vec.end() - 1)) {
        vec.push_back(*i);
        break;
      }
    }
  }
}

uint32_t DsrOptions::GetIDfromIP(Ipv4Address address) {
  NS_LOG_FUNCTION(this << address);
  int32_t nNodes = NodeList::GetNNodes();
  for (int32_t i = 0; i < nNodes; ++i) {
    Ptr<Node> node = NodeList::GetNode(i);
    Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
    if (ipv4->GetAddress(1, 0).GetLocal() == address) {
      return i;
    }
  }
  return 255;
}

Ptr<Node> DsrOptions::GetNodeWithAddress(Ipv4Address ipv4Address) {
  NS_LOG_FUNCTION(this << ipv4Address);
  int32_t nNodes = NodeList::GetNNodes();
  for (int32_t i = 0; i < nNodes; ++i) {
    Ptr<Node> node = NodeList::GetNode(i);
    Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
    int32_t ifIndex = ipv4->GetInterfaceForAddress(ipv4Address);
    if (ifIndex != -1) {
      return node;
    }
  }
  return nullptr;
}

NS_OBJECT_ENSURE_REGISTERED(DsrOptionPad1);

TypeId DsrOptionPad1::GetTypeId() {
  static TypeId tid = TypeId("ns3::dsr::DsrOptionPad1")
                          .SetParent<DsrOptions>()
                          .SetGroupName("Dsr")
                          .AddConstructor<DsrOptionPad1>();
  return tid;
}

DsrOptionPad1::DsrOptionPad1() { NS_LOG_FUNCTION_NOARGS(); }

DsrOptionPad1::~DsrOptionPad1() { NS_LOG_FUNCTION_NOARGS(); }

uint8_t DsrOptionPad1::GetOptionNumber() const {
  NS_LOG_FUNCTION_NOARGS();

  return OPT_NUMBER;
}

uint8_t DsrOptionPad1::Process(Ptr<Packet> packet, Ptr<Packet> dsrP,
                               Ipv4Address ipv4Address, Ipv4Address source,
                               const Ipv4Header &ipv4Header, uint8_t protocol,
                               bool &isPromisc, Ipv4Address promiscSource) {
  NS_LOG_FUNCTION(this << packet << dsrP << ipv4Address << source << ipv4Header
                       << (uint32_t)protocol << isPromisc);
  Ptr<Packet> p = packet->Copy();
  DsrOptionPad1Header pad1Header;
  p->RemoveHeader(pad1Header);

  isPromisc = false;

  return pad1Header.GetSerializedSize();
}

NS_OBJECT_ENSURE_REGISTERED(DsrOptionPadn);

TypeId DsrOptionPadn::GetTypeId() {
  static TypeId tid = TypeId("ns3::dsr::DsrOptionPadn")
                          .SetParent<DsrOptions>()
                          .SetGroupName("Dsr")
                          .AddConstructor<DsrOptionPadn>();
  return tid;
}

DsrOptionPadn::DsrOptionPadn() { NS_LOG_FUNCTION_NOARGS(); }

DsrOptionPadn::~DsrOptionPadn() { NS_LOG_FUNCTION_NOARGS(); }

uint8_t DsrOptionPadn::GetOptionNumber() const {
  NS_LOG_FUNCTION_NOARGS();
  return OPT_NUMBER;
}

uint8_t DsrOptionPadn::Process(Ptr<Packet> packet, Ptr<Packet> dsrP,
                               Ipv4Address ipv4Address, Ipv4Address source,
                               const Ipv4Header &ipv4Header, uint8_t protocol,
                               bool &isPromisc, Ipv4Address promiscSource) {
  NS_LOG_FUNCTION(this << packet << dsrP << ipv4Address << source << ipv4Header
                       << (uint32_t)protocol << isPromisc);

  Ptr<Packet> p = packet->Copy();
  DsrOptionPadnHeader padnHeader;
  p->RemoveHeader(padnHeader);

  isPromisc = false;

  return padnHeader.GetSerializedSize();
}

NS_OBJECT_ENSURE_REGISTERED(DsrOptionRreq);

TypeId DsrOptionRreq::GetTypeId() {
  static TypeId tid = TypeId("ns3::dsr::DsrOptionRreq")
                          .SetParent<DsrOptions>()
                          .SetGroupName("Dsr")
                          .AddConstructor<DsrOptionRreq>();
  return tid;
}

TypeId DsrOptionRreq::GetInstanceTypeId() const { return GetTypeId(); }

DsrOptionRreq::DsrOptionRreq() { NS_LOG_FUNCTION_NOARGS(); }

DsrOptionRreq::~DsrOptionRreq() { NS_LOG_FUNCTION_NOARGS(); }

uint8_t DsrOptionRreq::GetOptionNumber() const {
  NS_LOG_FUNCTION_NOARGS();

  return OPT_NUMBER;
}

uint8_t DsrOptionRreq::Process(Ptr<Packet> packet, Ptr<Packet> dsrP,
                               Ipv4Address ipv4Address, Ipv4Address source,
                               const Ipv4Header &ipv4Header, uint8_t protocol,
                               bool &isPromisc, Ipv4Address promiscSource) {
  NS_LOG_FUNCTION(this << packet << dsrP << ipv4Address << source << ipv4Header
                       << (uint32_t)protocol << isPromisc);
  Ipv4Address srcAddress = ipv4Header.GetSource();
  if (source == ipv4Address) {
    NS_LOG_DEBUG(
        "Discard the packet since it was originated from same source address");
    m_dropTrace(packet);
    return 0;
  }
  Ptr<Node> node = GetNodeWithAddress(ipv4Address);
  Ptr<dsr::DsrRouting> dsr = node->GetObject<dsr::DsrRouting>();

  Ptr<Packet> p = packet->Copy();
  uint8_t buf[2];
  p->CopyData(buf, sizeof(buf));
  uint8_t numberAddress = (buf[1] - 6) / 4;
  NS_LOG_DEBUG("The number of Ip addresses " << (uint32_t)numberAddress);
  if (numberAddress >= 255) {
    NS_LOG_DEBUG("Discard the packet, malformed header since two many ip "
                 "addresses in route");
    m_dropTrace(packet);
    return 0;
  }

  DsrOptionRreqHeader rreq;
  rreq.SetNumberAddress(numberAddress);
  p->RemoveHeader(rreq);
  uint8_t length = rreq.GetLength();
  if (length % 2 != 0) {
    NS_LOG_LOGIC("Malformed header. Drop!");
    m_dropTrace(packet);
    return 0;
  }
  uint16_t requestId = rreq.GetId();
  Ipv4Address targetAddress = rreq.GetTarget();
  std::vector<Ipv4Address> mainVector = rreq.GetNodesAddresses();
  std::vector<Ipv4Address> nodeList(mainVector);
  Ipv4Address sourceAddress = nodeList.front();
  PrintVector(nodeList);
  DsrRoutingHeader dsrRoutingHeader;
  dsrRoutingHeader.SetNextHeader(protocol);
  dsrRoutingHeader.SetMessageType(1);
  dsrRoutingHeader.SetSourceId(GetIDfromIP(source));
  dsrRoutingHeader.SetDestId(255);

  uint8_t ttl = ipv4Header.GetTtl();
  bool dupRequest = false;
  if (ttl) {
    dupRequest = dsr->FindSourceEntry(sourceAddress, targetAddress, requestId);
  }

  if (dupRequest) {
    NS_LOG_LOGIC("Duplicate request. Drop!");
    m_dropTrace(packet);
    return 0;
  }

  else if (CheckDuplicates(ipv4Address, nodeList)) {
    m_dropTrace(packet);
    NS_LOG_DEBUG(
        "Our node address is already seen in the route, drop the request");
    return 0;
  } else {
    DsrRouteCacheEntry toPrev;
    bool isRouteInCache = dsr->LookupRoute(targetAddress, toPrev);
    DsrRouteCacheEntry::IP_VECTOR ip = toPrev.GetVector();
    PrintVector(ip);
    std::vector<Ipv4Address> saveRoute(nodeList);
    PrintVector(saveRoute);
    bool areThereDuplicates = IfDuplicates(ip, saveRoute);

    NS_LOG_DEBUG("The target address over here "
                 << targetAddress << " and the ip address " << ipv4Address
                 << " and the source address " << mainVector[0]);
    if (targetAddress == ipv4Address) {
      Ipv4Address nextHop;
      if (nodeList.size() == 1) {
        NS_LOG_DEBUG("These two nodes are neighbors");
        m_finalRoute.clear();
        m_finalRoute.push_back(source);
        m_finalRoute.push_back(ipv4Address);
        nextHop = srcAddress;
      } else {
        std::vector<Ipv4Address> changeRoute(nodeList);
        changeRoute.push_back(ipv4Address);
        m_finalRoute.clear();
        for (auto i = changeRoute.begin(); i != changeRoute.end(); ++i) {
          m_finalRoute.push_back(*i);
        }
        PrintVector(m_finalRoute);
        nextHop = ReverseSearchNextHop(ipv4Address, m_finalRoute);
      }

      DsrOptionRrepHeader rrep;
      rrep.SetNodesAddress(m_finalRoute);
      NS_LOG_DEBUG("The nextHop address " << nextHop);
      Ipv4Address replyDst = m_finalRoute.front();
      DsrRoutingHeader dsrRoutingHeader;
      dsrRoutingHeader.SetNextHeader(protocol);
      dsrRoutingHeader.SetMessageType(1);
      dsrRoutingHeader.SetSourceId(GetIDfromIP(ipv4Address));
      dsrRoutingHeader.SetDestId(GetIDfromIP(replyDst));
      SetRoute(nextHop, ipv4Address);

      uint8_t length = rrep.GetLength();
      dsrRoutingHeader.SetPayloadLength(length + 2);
      dsrRoutingHeader.AddDsrOption(rrep);
      Ptr<Packet> newPacket = Create<Packet>();
      newPacket->AddHeader(dsrRoutingHeader);
      dsr->ScheduleInitialReply(newPacket, ipv4Address, nextHop, m_ipv4Route);
      PrintVector(m_finalRoute);
      if (ReverseRoutes(m_finalRoute)) {
        PrintVector(m_finalRoute);
        Ipv4Address dst = m_finalRoute.back();
        bool addRoute = false;
        if (numberAddress > 0) {
          DsrRouteCacheEntry toSource(m_finalRoute, dst, ActiveRouteTimeout);
          if (dsr->IsLinkCache()) {
            addRoute = dsr->AddRoute_Link(m_finalRoute, ipv4Address);
          } else {
            addRoute = dsr->AddRoute(toSource);
          }
        } else {
          NS_LOG_DEBUG("Abnormal RouteRequest");
          return 0;
        }

        if (addRoute) {
          DsrOptionSRHeader sourceRoute;
          NS_LOG_DEBUG("The route length " << m_finalRoute.size());
          sourceRoute.SetNodesAddress(m_finalRoute);

          sourceRoute.SetSegmentsLeft((m_finalRoute.size() - 2));
          sourceRoute.SetSalvage(0);
          Ipv4Address nextHop = SearchNextHop(ipv4Address, m_finalRoute);
          NS_LOG_DEBUG("The nextHop address " << nextHop);

          if (nextHop == "0.0.0.0") {
            dsr->PacketNewRoute(dsrP, ipv4Address, dst, protocol);
            return 0;
          }
          SetRoute(nextHop, ipv4Address);
          dsr->SendPacketFromBuffer(sourceRoute, nextHop, protocol);
          dsr->CancelRreqTimer(dst, true);
        } else {
          NS_LOG_DEBUG("The route is failed to add in cache");
          return 0;
        }
      } else {
        NS_LOG_DEBUG("Unable to reverse route");
        return 0;
      }
      isPromisc = false;
      return rreq.GetSerializedSize();
    }

    else if (isRouteInCache && !areThereDuplicates) {
      m_finalRoute.clear();
      for (auto i = saveRoute.begin(); i != saveRoute.end(); ++i) {
        m_finalRoute.push_back(*i);
      }
      for (auto j = ip.begin(); j != ip.end(); ++j) {
        m_finalRoute.push_back(*j);
      }
      bool addRoute = false;
      std::vector<Ipv4Address> reverseRoute(m_finalRoute);

      if (ReverseRoutes(reverseRoute)) {
        saveRoute.push_back(ipv4Address);
        ReverseRoutes(saveRoute);
        Ipv4Address dst = saveRoute.back();
        NS_LOG_DEBUG("This is the route save in route cache");
        PrintVector(saveRoute);

        DsrRouteCacheEntry toSource(saveRoute, dst, ActiveRouteTimeout);
        NS_ASSERT(saveRoute.front() == ipv4Address);
        if (dsr->IsLinkCache()) {
          addRoute = dsr->AddRoute_Link(saveRoute, ipv4Address);
        } else {
          addRoute = dsr->AddRoute(toSource);
        }

        if (addRoute) {
          NS_LOG_LOGIC(
              "We have added the route and search send buffer for packet with "
              "destination "
              << dst);
          DsrOptionSRHeader sourceRoute;
          PrintVector(saveRoute);

          sourceRoute.SetNodesAddress(saveRoute);
          sourceRoute.SetSegmentsLeft((saveRoute.size() - 2));
          uint8_t salvage = 0;
          sourceRoute.SetSalvage(salvage);
          Ipv4Address nextHop = SearchNextHop(ipv4Address, saveRoute);
          NS_LOG_DEBUG("The nextHop address " << nextHop);

          if (nextHop == "0.0.0.0") {
            dsr->PacketNewRoute(dsrP, ipv4Address, dst, protocol);
            return 0;
          }
          SetRoute(nextHop, ipv4Address);
          dsr->SendPacketFromBuffer(sourceRoute, nextHop, protocol);
          dsr->CancelRreqTimer(dst, true);
        } else {
          NS_LOG_DEBUG("The route is failed to add in cache");
          return 0;
        }
      } else {
        NS_LOG_DEBUG("Unable to reverse the route");
        return 0;
      }

      Ipv4Address nextHop = ReverseSearchNextHop(ipv4Address, m_finalRoute);
      SetRoute(nextHop, ipv4Address);

      uint16_t hops = m_finalRoute.size();
      DsrOptionRrepHeader rrep;
      rrep.SetNodesAddress(m_finalRoute);
      Ipv4Address realSource = m_finalRoute.back();
      PrintVector(m_finalRoute);
      NS_LOG_DEBUG("This is the full route from " << realSource << " to "
                                                  << m_finalRoute.front());
      DsrRoutingHeader dsrRoutingHeader;
      dsrRoutingHeader.SetNextHeader(protocol);
      dsrRoutingHeader.SetMessageType(1);
      dsrRoutingHeader.SetSourceId(GetIDfromIP(realSource));
      dsrRoutingHeader.SetDestId(255);

      uint8_t length = rrep.GetLength();
      dsrRoutingHeader.SetPayloadLength(length + 2);
      dsrRoutingHeader.AddDsrOption(rrep);
      Ptr<Packet> newPacket = Create<Packet>();
      newPacket->AddHeader(dsrRoutingHeader);
      dsr->ScheduleCachedReply(newPacket, ipv4Address, nextHop, m_ipv4Route,
                               hops);
      isPromisc = false;
      return rreq.GetSerializedSize();
    } else {
      mainVector.push_back(ipv4Address);
      NS_ASSERT(mainVector.front() == source);
      NS_LOG_DEBUG("Print out the main vector");
      PrintVector(mainVector);
      rreq.SetNodesAddress(mainVector);

      Ptr<Packet> errP = p->Copy();
      if (errP->GetSize()) {
        NS_LOG_DEBUG("Error header included");
        DsrOptionRerrUnreachHeader rerr;
        p->RemoveHeader(rerr);
        Ipv4Address errorSrc = rerr.GetErrorSrc();
        Ipv4Address unreachNode = rerr.GetUnreachNode();
        Ipv4Address errorDst = rerr.GetErrorDst();

        if ((errorSrc == srcAddress) && (unreachNode == ipv4Address)) {
          NS_LOG_DEBUG("The error link back to work again");
          uint16_t length = rreq.GetLength();
          NS_LOG_DEBUG("The RREQ header length " << length);
          dsrRoutingHeader.AddDsrOption(rreq);
          dsrRoutingHeader.SetPayloadLength(length + 2);
        } else {
          dsr->DeleteAllRoutesIncludeLink(errorSrc, unreachNode, ipv4Address);

          DsrOptionRerrUnreachHeader newUnreach;
          newUnreach.SetErrorType(1);
          newUnreach.SetErrorSrc(errorSrc);
          newUnreach.SetUnreachNode(unreachNode);
          newUnreach.SetErrorDst(errorDst);
          newUnreach.SetSalvage(rerr.GetSalvage());
          uint16_t length = rreq.GetLength() + newUnreach.GetLength();
          NS_LOG_DEBUG("The RREQ and newUnreach header length " << length);
          dsrRoutingHeader.SetPayloadLength(length + 4);
          dsrRoutingHeader.AddDsrOption(rreq);
          dsrRoutingHeader.AddDsrOption(newUnreach);
        }
      } else {
        uint16_t length = rreq.GetLength();
        NS_LOG_DEBUG("The RREQ header length " << length);
        dsrRoutingHeader.AddDsrOption(rreq);
        dsrRoutingHeader.SetPayloadLength(length + 2);
      }
      uint8_t ttl = ipv4Header.GetTtl();
      NS_LOG_DEBUG("The ttl value here " << (uint32_t)ttl);
      if (ttl) {
        Ptr<Packet> interP = Create<Packet>();
        SocketIpTtlTag tag;
        tag.SetTtl(ttl - 1);
        interP->AddPacketTag(tag);
        interP->AddHeader(dsrRoutingHeader);
        dsr->ScheduleInterRequest(interP);
        isPromisc = false;
      }
      return rreq.GetSerializedSize();
    }
  }
}

NS_OBJECT_ENSURE_REGISTERED(DsrOptionRrep);

TypeId DsrOptionRrep::GetTypeId() {
  static TypeId tid = TypeId("ns3::dsr::DsrOptionRrep")
                          .SetParent<DsrOptions>()
                          .SetGroupName("Dsr")
                          .AddConstructor<DsrOptionRrep>();
  return tid;
}

DsrOptionRrep::DsrOptionRrep() { NS_LOG_FUNCTION_NOARGS(); }

DsrOptionRrep::~DsrOptionRrep() { NS_LOG_FUNCTION_NOARGS(); }

TypeId DsrOptionRrep::GetInstanceTypeId() const { return GetTypeId(); }

uint8_t DsrOptionRrep::GetOptionNumber() const {
  NS_LOG_FUNCTION_NOARGS();

  return OPT_NUMBER;
}

uint8_t DsrOptionRrep::Process(Ptr<Packet> packet, Ptr<Packet> dsrP,
                               Ipv4Address ipv4Address, Ipv4Address source,
                               const Ipv4Header &ipv4Header, uint8_t protocol,
                               bool &isPromisc, Ipv4Address promiscSource) {
  NS_LOG_FUNCTION(this << packet << dsrP << ipv4Address << source << ipv4Header
                       << (uint32_t)protocol << isPromisc);

  Ptr<Packet> p = packet->Copy();

  uint8_t buf[2];
  p->CopyData(buf, sizeof(buf));
  uint8_t numberAddress = (buf[1] - 2) / 4;

  DsrOptionRrepHeader rrep;
  rrep.SetNumberAddress(numberAddress);
  p->RemoveHeader(rrep);

  Ptr<Node> node = GetNodeWithAddress(ipv4Address);
  Ptr<dsr::DsrRouting> dsr = node->GetObject<dsr::DsrRouting>();

  NS_LOG_DEBUG("The next header value " << (uint32_t)protocol);

  std::vector<Ipv4Address> nodeList = rrep.GetNodesAddress();
  Ipv4Address targetAddress = nodeList.front();
  if (targetAddress == ipv4Address) {
    RemoveDuplicates(nodeList);
    if (nodeList.empty()) {
      NS_LOG_DEBUG("The route we have contains 0 entries");
      return 0;
    }
    Ipv4Address dst = nodeList.back();
    DsrRouteCacheEntry toDestination(nodeList, dst, ActiveRouteTimeout);
    NS_ASSERT(nodeList.front() == ipv4Address);
    bool addRoute = false;
    if (dsr->IsLinkCache()) {
      addRoute = dsr->AddRoute_Link(nodeList, ipv4Address);
    } else {
      addRoute = dsr->AddRoute(toDestination);
    }

    if (addRoute) {
      NS_LOG_DEBUG("We have added the route and search send buffer for packet "
                   "with destination "
                   << dst);
      DsrOptionSRHeader sourceRoute;
      NS_LOG_DEBUG("The route length " << nodeList.size());
      sourceRoute.SetNodesAddress(nodeList);
      sourceRoute.SetSegmentsLeft((nodeList.size() - 2));
      sourceRoute.SetSalvage(0);
      Ipv4Address nextHop = SearchNextHop(ipv4Address, nodeList);
      NS_LOG_DEBUG("The nextHop address " << nextHop);
      if (nextHop == "0.0.0.0") {
        dsr->PacketNewRoute(dsrP, ipv4Address, dst, protocol);
        return 0;
      }
      PrintVector(nodeList);
      SetRoute(nextHop, ipv4Address);
      dsr->CancelRreqTimer(dst, true);
      dsr->SendPacketFromBuffer(sourceRoute, nextHop, protocol);
    } else {
      NS_LOG_DEBUG("Failed to add the route");
      return 0;
    }
  } else {
    uint8_t length = rrep.GetLength() - 2;
    NS_LOG_DEBUG("The length of rrep option " << (uint32_t)length);

    if (length % 2 != 0) {
      NS_LOG_LOGIC("Malformed header. Drop!");
      m_dropTrace(packet);
      return 0;
    }
    PrintVector(nodeList);
    std::vector<Ipv4Address> routeCopy = nodeList;
    std::vector<Ipv4Address> cutRoute = CutRoute(ipv4Address, nodeList);
    PrintVector(cutRoute);
    if (cutRoute.size() >= 2) {
      Ipv4Address dst = cutRoute.back();
      NS_LOG_DEBUG("The route destination after cut " << dst);
      DsrRouteCacheEntry toDestination(cutRoute, dst, ActiveRouteTimeout);
      NS_ASSERT(cutRoute.front() == ipv4Address);
      bool addRoute = false;
      if (dsr->IsLinkCache()) {
        addRoute = dsr->AddRoute_Link(nodeList, ipv4Address);
      } else {
        addRoute = dsr->AddRoute(toDestination);
      }
      if (addRoute) {
        dsr->CancelRreqTimer(dst, true);
      } else {
        NS_LOG_DEBUG("The route not added");
      }
    } else {
      NS_LOG_DEBUG("The route is corrupted");
    }
    Ipv4Address nextHop = ReverseSearchNextHop(ipv4Address, routeCopy);
    NS_ASSERT(routeCopy.back() == source);
    PrintVector(routeCopy);
    NS_LOG_DEBUG("The nextHop address "
                 << nextHop << " and the source in the route reply " << source);
    SetRoute(nextHop, ipv4Address);
    DsrRoutingHeader dsrRoutingHeader;
    dsrRoutingHeader.SetNextHeader(protocol);

    length = rrep.GetLength();
    NS_LOG_DEBUG("The reply header length " << (uint32_t)length);
    dsrRoutingHeader.SetPayloadLength(length + 2);
    dsrRoutingHeader.SetMessageType(1);
    dsrRoutingHeader.SetSourceId(GetIDfromIP(source));
    dsrRoutingHeader.SetDestId(GetIDfromIP(targetAddress));
    dsrRoutingHeader.AddDsrOption(rrep);
    Ptr<Packet> newPacket = Create<Packet>();
    newPacket->AddHeader(dsrRoutingHeader);
    dsr->SendReply(newPacket, ipv4Address, nextHop, m_ipv4Route);
    isPromisc = false;
  }
  return rrep.GetSerializedSize();
}

NS_OBJECT_ENSURE_REGISTERED(DsrOptionSR);

TypeId DsrOptionSR::GetTypeId() {
  static TypeId tid = TypeId("ns3::dsr::DsrOptionSR")
                          .SetParent<DsrOptions>()
                          .SetGroupName("Dsr")
                          .AddConstructor<DsrOptionSR>();
  return tid;
}

DsrOptionSR::DsrOptionSR() { NS_LOG_FUNCTION_NOARGS(); }

DsrOptionSR::~DsrOptionSR() { NS_LOG_FUNCTION_NOARGS(); }

TypeId DsrOptionSR::GetInstanceTypeId() const { return GetTypeId(); }

uint8_t DsrOptionSR::GetOptionNumber() const {
  NS_LOG_FUNCTION_NOARGS();
  return OPT_NUMBER;
}

uint8_t DsrOptionSR::Process(Ptr<Packet> packet, Ptr<Packet> dsrP,
                             Ipv4Address ipv4Address, Ipv4Address source,
                             const Ipv4Header &ipv4Header, uint8_t protocol,
                             bool &isPromisc, Ipv4Address promiscSource) {
  NS_LOG_FUNCTION(this << packet << dsrP << ipv4Address << source << ipv4Address
                       << ipv4Header << (uint32_t)protocol << isPromisc);
  Ptr<Packet> p = packet->Copy();
  uint8_t buf[2];
  p->CopyData(buf, sizeof(buf));
  uint8_t numberAddress = (buf[1] - 2) / 4;
  DsrOptionSRHeader sourceRoute;
  sourceRoute.SetNumberAddress(numberAddress);
  p->RemoveHeader(sourceRoute);

  std::vector<Ipv4Address> nodeList = sourceRoute.GetNodesAddress();
  uint8_t segsLeft = sourceRoute.GetSegmentsLeft();
  uint8_t salvage = sourceRoute.GetSalvage();
  Ptr<Node> node = GetNodeWithAddress(ipv4Address);
  Ptr<dsr::DsrRouting> dsr = node->GetObject<dsr::DsrRouting>();
  Ipv4Address srcAddress = ipv4Header.GetSource();
  Ipv4Address destAddress = ipv4Header.GetDestination();

  Ipv4Address destination = nodeList.back();
  if (isPromisc) {
    NS_LOG_LOGIC("We process promiscuous receipt data packet");
    if (ContainAddressAfter(ipv4Address, destAddress, nodeList)) {
      NS_LOG_LOGIC("Send back the gratuitous reply");
      dsr->SendGratuitousReply(source, srcAddress, nodeList, protocol);
    }

    uint16_t fragmentOffset = ipv4Header.GetFragmentOffset();
    uint16_t identification = ipv4Header.GetIdentification();

    if (destAddress != destination) {
      NS_LOG_DEBUG("Process the promiscuously received packet");
      bool findPassive = false;
      int32_t nNodes = NodeList::GetNNodes();
      for (int32_t i = 0; i < nNodes; ++i) {
        NS_LOG_DEBUG("Working with node " << i);

        Ptr<Node> node = NodeList::GetNode(i);
        Ptr<dsr::DsrRouting> dsrNode = node->GetObject<dsr::DsrRouting>();
        findPassive =
            dsrNode->PassiveEntryCheck(packet, source, destination, segsLeft,
                                       fragmentOffset, identification, false);
        if (findPassive) {
          break;
        }
      }

      if (findPassive) {
        NS_LOG_DEBUG("We find one previously received passive entry");
        PrintVector(nodeList);

        NS_LOG_DEBUG("promisc source " << promiscSource);
        Ptr<Node> node = GetNodeWithAddress(promiscSource);
        Ptr<dsr::DsrRouting> dsrSrc = node->GetObject<dsr::DsrRouting>();
        dsrSrc->CancelPassiveTimer(packet, source, destination, segsLeft);
      } else {
        NS_LOG_DEBUG("Saved the entry for further use");
        dsr->PassiveEntryCheck(packet, source, destination, segsLeft,
                               fragmentOffset, identification, true);
      }
    }
    return 0;
  } else {
    uint8_t length = sourceRoute.GetLength();
    uint8_t nextAddressIndex;
    Ipv4Address nextAddress;

    uint32_t size = p->GetSize();
    auto data = new uint8_t[size];
    p->CopyData(data, size);
    uint8_t optionType = 0;
    optionType = *(data);
    if (optionType == 160) {
      NS_LOG_LOGIC(
          "Remove the ack request header and add ack header to the packet");
      DsrOptionAckReqHeader ackReq;
      p->RemoveHeader(ackReq);
      uint16_t ackId = ackReq.GetAckId();
      Ipv4Address ackAddress = srcAddress;
      if (!nodeList.empty()) {
        if (segsLeft > numberAddress) {
          NS_LOG_LOGIC("Malformed header. Drop!");
          m_dropTrace(packet);
          return 0;
        }
        if (numberAddress - segsLeft < 2) {
          NS_LOG_LOGIC("Malformed header. Drop!");
          m_dropTrace(packet);
          return 0;
        }
        ackAddress = nodeList[numberAddress - segsLeft - 2];
      }
      m_ipv4Route = SetRoute(ackAddress, ipv4Address);
      NS_LOG_DEBUG("Send back ACK to the earlier hop "
                   << ackAddress << " from us " << ipv4Address);
      dsr->SendAck(ackId, ackAddress, source, destination, protocol,
                   m_ipv4Route);
    }
    if (segsLeft == 0) {
      NS_LOG_DEBUG("This is the final destination");
      isPromisc = false;
      return sourceRoute.GetSerializedSize();
    }

    if (length % 2 != 0) {
      NS_LOG_LOGIC("Malformed header. Drop!");
      m_dropTrace(packet);
      return 0;
    }

    if (segsLeft > numberAddress) {
      NS_LOG_LOGIC("Malformed header. Drop!");
      m_dropTrace(packet);
      return 0;
    }

    DsrOptionSRHeader newSourceRoute;
    newSourceRoute.SetSegmentsLeft(segsLeft - 1);
    newSourceRoute.SetSalvage(salvage);
    newSourceRoute.SetNodesAddress(nodeList);
    nextAddressIndex = numberAddress - segsLeft;
    nextAddress = newSourceRoute.GetNodeAddress(nextAddressIndex);
    NS_LOG_DEBUG("The next address of source route option "
                 << nextAddress
                 << " and the nextAddressIndex: " << (uint32_t)nextAddressIndex
                 << " and the segments left : " << (uint32_t)segsLeft);
    Ipv4Address targetAddress = nodeList.back();
    Ipv4Address realSource = nodeList.front();
    Ipv4Address nextHop = SearchNextHop(ipv4Address, nodeList);
    PrintVector(nodeList);

    if (nextHop == "0.0.0.0") {
      NS_LOG_DEBUG("Before new packet " << *dsrP);
      dsr->PacketNewRoute(dsrP, realSource, targetAddress, protocol);
      return 0;
    }

    if (ipv4Address == nextHop) {
      NS_LOG_DEBUG("We have reached the destination");
      newSourceRoute.SetSegmentsLeft(0);
      return newSourceRoute.GetSerializedSize();
    }
    if (nextAddress.IsMulticast() || destAddress.IsMulticast()) {
      m_dropTrace(packet);
      return 0;
    }
    SetRoute(nextAddress, ipv4Address);
    NS_LOG_DEBUG("dsr packet size " << dsrP->GetSize());
    dsr->ForwardPacket(dsrP, newSourceRoute, ipv4Header, realSource,
                       nextAddress, targetAddress, protocol, m_ipv4Route);
  }
  return sourceRoute.GetSerializedSize();
}

NS_OBJECT_ENSURE_REGISTERED(DsrOptionRerr);

TypeId DsrOptionRerr::GetTypeId() {
  static TypeId tid = TypeId("ns3::dsr::DsrOptionRerr")
                          .SetParent<DsrOptions>()
                          .SetGroupName("Dsr")
                          .AddConstructor<DsrOptionRerr>();
  return tid;
}

DsrOptionRerr::DsrOptionRerr() { NS_LOG_FUNCTION_NOARGS(); }

DsrOptionRerr::~DsrOptionRerr() { NS_LOG_FUNCTION_NOARGS(); }

TypeId DsrOptionRerr::GetInstanceTypeId() const { return GetTypeId(); }

uint8_t DsrOptionRerr::GetOptionNumber() const {
  NS_LOG_FUNCTION_NOARGS();
  return OPT_NUMBER;
}

uint8_t DsrOptionRerr::Process(Ptr<Packet> packet, Ptr<Packet> dsrP,
                               Ipv4Address ipv4Address, Ipv4Address source,
                               const Ipv4Header &ipv4Header, uint8_t protocol,
                               bool &isPromisc, Ipv4Address promiscSource) {
  NS_LOG_FUNCTION(this << packet << dsrP << ipv4Address << source << ipv4Header
                       << (uint32_t)protocol << isPromisc);
  Ptr<Packet> p = packet->Copy();
  uint32_t size = p->GetSize();
  auto data = new uint8_t[size];
  p->CopyData(data, size);
  uint8_t errorType = *(data + 2);
  Ptr<Node> node = GetNodeWithAddress(ipv4Address);
  Ptr<dsr::DsrRouting> dsr = node->GetObject<dsr::DsrRouting>();
  NS_LOG_DEBUG("The error type value here " << (uint32_t)errorType);
  if (errorType == 1) {
    DsrOptionRerrUnreachHeader rerrUnreach;
    p->RemoveHeader(rerrUnreach);
    Ipv4Address unreachAddress = rerrUnreach.GetUnreachNode();
    Ipv4Address errorSource = rerrUnreach.GetErrorSrc();

    NS_LOG_DEBUG("The error source is " << rerrUnreach.GetErrorDst()
                                        << "and the unreachable node is "
                                        << unreachAddress);
    uint32_t rerrSize = rerrUnreach.GetSerializedSize();
    Ptr<Node> node = GetNodeWithAddress(ipv4Address);
    dsr->DeleteAllRoutesIncludeLink(errorSource, unreachAddress, ipv4Address);

    Ptr<Packet> newP = p->Copy();
    uint32_t serialized =
        DoSendError(newP, rerrUnreach, rerrSize, ipv4Address, protocol);
    return serialized;
  } else {
    DsrOptionRerrUnsupportedHeader rerrUnsupported;
    p->RemoveHeader(rerrUnsupported);

    uint32_t serialized = 0;
    return serialized;
  }
}

uint8_t DsrOptionRerr::DoSendError(Ptr<Packet> p,
                                   DsrOptionRerrUnreachHeader &rerr,
                                   uint32_t rerrSize, Ipv4Address ipv4Address,
                                   uint8_t protocol) {
  uint8_t buf[2];
  p->CopyData(buf, sizeof(buf));
  uint8_t numberAddress = (buf[1] - 2) / 4;

  NS_LOG_DEBUG("The number of addresses " << (uint32_t)numberAddress);
  DsrOptionSRHeader sourceRoute;
  sourceRoute.SetNumberAddress(numberAddress);
  p->RemoveHeader(sourceRoute);
  NS_ASSERT(p->GetSize() == 0);
  Ptr<Node> node = GetNodeWithAddress(ipv4Address);
  Ptr<dsr::DsrRouting> dsr = node->GetObject<dsr::DsrRouting>();
  uint8_t segmentsLeft = sourceRoute.GetSegmentsLeft();
  uint8_t length = sourceRoute.GetLength();
  uint8_t nextAddressIndex;
  Ipv4Address nextAddress;
  std::vector<Ipv4Address> nodeList = sourceRoute.GetNodesAddress();
  Ipv4Address targetAddress = nodeList.back();
  uint32_t serializedSize = rerrSize + sourceRoute.GetSerializedSize();

  if (length % 2 != 0) {
    NS_LOG_LOGIC("Malformed header. Drop!");
    m_dropTrace(p);
    return 0;
  }

  if (segmentsLeft > numberAddress) {
    NS_LOG_LOGIC("Malformed header. Drop!");
    m_dropTrace(p);
    return 0;
  }
  if (segmentsLeft == 0 && targetAddress == ipv4Address) {
    NS_LOG_INFO("This is the destination of the error, send error request");
    dsr->SendErrorRequest(rerr, protocol);
    return serializedSize;
  }

  DsrOptionSRHeader newSourceRoute;
  newSourceRoute.SetSegmentsLeft(segmentsLeft - 1);
  nextAddressIndex = numberAddress - segmentsLeft;
  nextAddress = sourceRoute.GetNodeAddress(nextAddressIndex);
  newSourceRoute.SetSalvage(sourceRoute.GetSalvage());
  newSourceRoute.SetNodesAddress(nodeList);
  nextAddress = newSourceRoute.GetNodeAddress(nextAddressIndex);

  if (nextAddress.IsMulticast() || targetAddress.IsMulticast()) {
    m_dropTrace(p);
    return serializedSize;
  }

  SetRoute(nextAddress, ipv4Address);
  dsr->ForwardErrPacket(rerr, newSourceRoute, nextAddress, protocol,
                        m_ipv4Route);
  return serializedSize;
}

NS_OBJECT_ENSURE_REGISTERED(DsrOptionAckReq);

TypeId DsrOptionAckReq::GetTypeId() {
  static TypeId tid = TypeId("ns3::dsr::DsrOptionAckReq")
                          .SetParent<DsrOptions>()
                          .SetGroupName("Dsr")
                          .AddConstructor<DsrOptionAckReq>();
  return tid;
}

DsrOptionAckReq::DsrOptionAckReq() { NS_LOG_FUNCTION_NOARGS(); }

DsrOptionAckReq::~DsrOptionAckReq() { NS_LOG_FUNCTION_NOARGS(); }

TypeId DsrOptionAckReq::GetInstanceTypeId() const { return GetTypeId(); }

uint8_t DsrOptionAckReq::GetOptionNumber() const {
  NS_LOG_FUNCTION_NOARGS();
  return OPT_NUMBER;
}

uint8_t DsrOptionAckReq::Process(Ptr<Packet> packet, Ptr<Packet> dsrP,
                                 Ipv4Address ipv4Address, Ipv4Address source,
                                 const Ipv4Header &ipv4Header, uint8_t protocol,
                                 bool &isPromisc, Ipv4Address promiscSource) {
  NS_LOG_FUNCTION(this << packet << dsrP << ipv4Address << source << ipv4Header
                       << (uint32_t)protocol << isPromisc);
  Ptr<Packet> p = packet->Copy();
  DsrOptionAckReqHeader ackReq;
  p->RemoveHeader(ackReq);
  Ptr<Node> node = GetNodeWithAddress(ipv4Address);
  Ptr<dsr::DsrRouting> dsr = node->GetObject<dsr::DsrRouting>();

  NS_LOG_DEBUG("The next header value " << (uint32_t)protocol);

  return ackReq.GetSerializedSize();
}

NS_OBJECT_ENSURE_REGISTERED(DsrOptionAck);

TypeId DsrOptionAck::GetTypeId() {
  static TypeId tid = TypeId("ns3::dsr::DsrOptionAck")
                          .SetParent<DsrOptions>()
                          .SetGroupName("Dsr")
                          .AddConstructor<DsrOptionAck>();
  return tid;
}

DsrOptionAck::DsrOptionAck() { NS_LOG_FUNCTION_NOARGS(); }

DsrOptionAck::~DsrOptionAck() { NS_LOG_FUNCTION_NOARGS(); }

TypeId DsrOptionAck::GetInstanceTypeId() const { return GetTypeId(); }

uint8_t DsrOptionAck::GetOptionNumber() const {
  NS_LOG_FUNCTION_NOARGS();
  return OPT_NUMBER;
}

uint8_t DsrOptionAck::Process(Ptr<Packet> packet, Ptr<Packet> dsrP,
                              Ipv4Address ipv4Address, Ipv4Address source,
                              const Ipv4Header &ipv4Header, uint8_t protocol,
                              bool &isPromisc, Ipv4Address promiscSource) {
  NS_LOG_FUNCTION(this << packet << dsrP << ipv4Address << source << ipv4Header
                       << (uint32_t)protocol << isPromisc);
  Ptr<Packet> p = packet->Copy();
  DsrOptionAckHeader ack;
  p->RemoveHeader(ack);
  Ipv4Address realSrc = ack.GetRealSrc();
  Ipv4Address realDst = ack.GetRealDst();
  uint16_t ackId = ack.GetAckId();
  Ptr<Node> node = GetNodeWithAddress(ipv4Address);
  Ptr<dsr::DsrRouting> dsr = node->GetObject<dsr::DsrRouting>();
  dsr->UpdateRouteEntry(realDst);
  dsr->CallCancelPacketTimer(ackId, ipv4Header, realSrc, realDst);
  return ack.GetSerializedSize();
}

} // namespace dsr
} // namespace ns3
