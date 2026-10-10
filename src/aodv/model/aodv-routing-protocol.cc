#define NS_LOG_APPEND_CONTEXT                                                  \
  if (m_ipv4) {                                                                \
    std::clog << "[node " << m_ipv4->GetObject<Node>()->GetId() << "] ";       \
  }

#include "aodv-routing-protocol.h"

#include "ns3/adhoc-wifi-mac.h"
#include "ns3/boolean.h"
#include "ns3/inet-socket-address.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/random-variable-stream.h"
#include "ns3/string.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-header.h"
#include "ns3/udp-l4-protocol.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/wifi-mpdu.h"
#include "ns3/wifi-net-device.h"

#include <algorithm>
#include <limits>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("AodvRoutingProtocol");

namespace aodv {
NS_OBJECT_ENSURE_REGISTERED(RoutingProtocol);

const uint32_t RoutingProtocol::AODV_PORT = 654;

class DeferredRouteOutputTag : public Tag {
public:
  DeferredRouteOutputTag(int32_t o = -1) : Tag(), m_oif(o) {}

  static TypeId GetTypeId() {
    static TypeId tid = TypeId("ns3::aodv::DeferredRouteOutputTag")
                            .SetParent<Tag>()
                            .SetGroupName("Aodv")
                            .AddConstructor<DeferredRouteOutputTag>();
    return tid;
  }

  TypeId GetInstanceTypeId() const override { return GetTypeId(); }

  int32_t GetInterface() const { return m_oif; }

  void SetInterface(int32_t oif) { m_oif = oif; }

  uint32_t GetSerializedSize() const override { return sizeof(int32_t); }

  void Serialize(TagBuffer i) const override { i.WriteU32(m_oif); }

  void Deserialize(TagBuffer i) override { m_oif = i.ReadU32(); }

  void Print(std::ostream &os) const override {
    os << "DeferredRouteOutputTag: output interface = " << m_oif;
  }

private:
  int32_t m_oif;
};

NS_OBJECT_ENSURE_REGISTERED(DeferredRouteOutputTag);

RoutingProtocol::RoutingProtocol()
    : m_rreqRetries(2), m_ttlStart(1), m_ttlIncrement(2), m_ttlThreshold(7),
      m_timeoutBuffer(2), m_rreqRateLimit(10), m_rerrRateLimit(10),
      m_activeRouteTimeout(Seconds(3)), m_netDiameter(35),
      m_nodeTraversalTime(MilliSeconds(40)),
      m_netTraversalTime(Time((2 * m_netDiameter) * m_nodeTraversalTime)),
      m_pathDiscoveryTime(Time(2 * m_netTraversalTime)),
      m_myRouteTimeout(
          Time(2 * std::max(m_pathDiscoveryTime, m_activeRouteTimeout))),
      m_helloInterval(Seconds(1)), m_allowedHelloLoss(2),
      m_deletePeriod(Time(5 * std::max(m_activeRouteTimeout, m_helloInterval))),
      m_nextHopWait(m_nodeTraversalTime + MilliSeconds(10)),
      m_blackListTimeout(Time(m_rreqRetries * m_netTraversalTime)),
      m_maxQueueLen(64), m_maxQueueTime(Seconds(30)), m_destinationOnly(false),
      m_gratuitousReply(true), m_enableHello(false),
      m_routingTable(m_deletePeriod), m_queue(m_maxQueueLen, m_maxQueueTime),
      m_requestId(0), m_seqNo(0), m_rreqIdCache(m_pathDiscoveryTime),
      m_dpd(m_pathDiscoveryTime), m_nb(m_helloInterval), m_rreqCount(0),
      m_rerrCount(0), m_htimer(Timer::CANCEL_ON_DESTROY),
      m_rreqRateLimitTimer(Timer::CANCEL_ON_DESTROY),
      m_rerrRateLimitTimer(Timer::CANCEL_ON_DESTROY),
      m_lastBcastTime(Seconds(0)) {
  m_nb.SetCallback(
      MakeCallback(&RoutingProtocol::SendRerrWhenBreaksLinkToNextHop, this));
}

TypeId RoutingProtocol::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::aodv::RoutingProtocol")
          .SetParent<Ipv4RoutingProtocol>()
          .SetGroupName("Aodv")
          .AddConstructor<RoutingProtocol>()
          .AddAttribute("HelloInterval", "HELLO messages emission interval.",
                        TimeValue(Seconds(1)),
                        MakeTimeAccessor(&RoutingProtocol::m_helloInterval),
                        MakeTimeChecker())
          .AddAttribute("TtlStart", "Initial TTL value for RREQ.",
                        UintegerValue(1),
                        MakeUintegerAccessor(&RoutingProtocol::m_ttlStart),
                        MakeUintegerChecker<uint16_t>())
          .AddAttribute("TtlIncrement",
                        "TTL increment for each attempt using the expanding "
                        "ring search for RREQ "
                        "dissemination.",
                        UintegerValue(2),
                        MakeUintegerAccessor(&RoutingProtocol::m_ttlIncrement),
                        MakeUintegerChecker<uint16_t>())
          .AddAttribute("TtlThreshold",
                        "Maximum TTL value for expanding ring search, TTL = "
                        "NetDiameter is used "
                        "beyond this value.",
                        UintegerValue(7),
                        MakeUintegerAccessor(&RoutingProtocol::m_ttlThreshold),
                        MakeUintegerChecker<uint16_t>())
          .AddAttribute("TimeoutBuffer", "Provide a buffer for the timeout.",
                        UintegerValue(2),
                        MakeUintegerAccessor(&RoutingProtocol::m_timeoutBuffer),
                        MakeUintegerChecker<uint16_t>())
          .AddAttribute(
              "RreqRetries",
              "Maximum number of retransmissions of RREQ to discover a route",
              UintegerValue(2),
              MakeUintegerAccessor(&RoutingProtocol::m_rreqRetries),
              MakeUintegerChecker<uint32_t>())
          .AddAttribute("RreqRateLimit", "Maximum number of RREQ per second.",
                        UintegerValue(10),
                        MakeUintegerAccessor(&RoutingProtocol::m_rreqRateLimit),
                        MakeUintegerChecker<uint32_t>())
          .AddAttribute("RerrRateLimit", "Maximum number of RERR per second.",
                        UintegerValue(10),
                        MakeUintegerAccessor(&RoutingProtocol::m_rerrRateLimit),
                        MakeUintegerChecker<uint32_t>())
          .AddAttribute(
              "NodeTraversalTime",
              "Conservative estimate of the average one hop traversal time for "
              "packets "
              "and should include "
              "queuing delays, interrupt processing times and transfer times.",
              TimeValue(MilliSeconds(40)),
              MakeTimeAccessor(&RoutingProtocol::m_nodeTraversalTime),
              MakeTimeChecker())
          .AddAttribute("NextHopWait",
                        "Period of our waiting for the neighbour's RREP_ACK = "
                        "10 ms + NodeTraversalTime",
                        TimeValue(MilliSeconds(50)),
                        MakeTimeAccessor(&RoutingProtocol::m_nextHopWait),
                        MakeTimeChecker())
          .AddAttribute(
              "ActiveRouteTimeout",
              "Period of time during which the route is considered to be valid",
              TimeValue(Seconds(3)),
              MakeTimeAccessor(&RoutingProtocol::m_activeRouteTimeout),
              MakeTimeChecker())
          .AddAttribute(
              "MyRouteTimeout",
              "Value of lifetime field in RREP generating by this node = 2 * "
              "max(ActiveRouteTimeout, PathDiscoveryTime)",
              TimeValue(Seconds(11.2)),
              MakeTimeAccessor(&RoutingProtocol::m_myRouteTimeout),
              MakeTimeChecker())
          .AddAttribute("BlackListTimeout",
                        "Time for which the node is put into the blacklist = "
                        "RreqRetries * "
                        "NetTraversalTime",
                        TimeValue(Seconds(5.6)),
                        MakeTimeAccessor(&RoutingProtocol::m_blackListTimeout),
                        MakeTimeChecker())
          .AddAttribute("DeletePeriod",
                        "DeletePeriod is intended to provide an upper bound on "
                        "the time for "
                        "which an upstream node A "
                        "can have a neighbor B as an active next hop for "
                        "destination D, while B "
                        "has invalidated the route to D."
                        " = 5 * max (HelloInterval, ActiveRouteTimeout)",
                        TimeValue(Seconds(15)),
                        MakeTimeAccessor(&RoutingProtocol::m_deletePeriod),
                        MakeTimeChecker())
          .AddAttribute("NetDiameter",
                        "Net diameter measures the maximum possible number of "
                        "hops between two "
                        "nodes in the network",
                        UintegerValue(35),
                        MakeUintegerAccessor(&RoutingProtocol::m_netDiameter),
                        MakeUintegerChecker<uint32_t>())
          .AddAttribute("NetTraversalTime",
                        "Estimate of the average net traversal time = 2 * "
                        "NodeTraversalTime * NetDiameter",
                        TimeValue(Seconds(2.8)),
                        MakeTimeAccessor(&RoutingProtocol::m_netTraversalTime),
                        MakeTimeChecker())
          .AddAttribute("PathDiscoveryTime",
                        "Estimate of maximum time needed to find route in "
                        "network = 2 * NetTraversalTime",
                        TimeValue(Seconds(5.6)),
                        MakeTimeAccessor(&RoutingProtocol::m_pathDiscoveryTime),
                        MakeTimeChecker())
          .AddAttribute("MaxQueueLen",
                        "Maximum number of packets that we allow a routing "
                        "protocol to buffer.",
                        UintegerValue(64),
                        MakeUintegerAccessor(&RoutingProtocol::SetMaxQueueLen,
                                             &RoutingProtocol::GetMaxQueueLen),
                        MakeUintegerChecker<uint32_t>())
          .AddAttribute("MaxQueueTime",
                        "Maximum time packets can be queued (in seconds)",
                        TimeValue(Seconds(30)),
                        MakeTimeAccessor(&RoutingProtocol::SetMaxQueueTime,
                                         &RoutingProtocol::GetMaxQueueTime),
                        MakeTimeChecker())
          .AddAttribute(
              "AllowedHelloLoss",
              "Number of hello messages which may be loss for valid link.",
              UintegerValue(2),
              MakeUintegerAccessor(&RoutingProtocol::m_allowedHelloLoss),
              MakeUintegerChecker<uint16_t>())
          .AddAttribute(
              "GratuitousReply",
              "Indicates whether a gratuitous RREP should be unicast to the "
              "node "
              "originated route discovery.",
              BooleanValue(true),
              MakeBooleanAccessor(&RoutingProtocol::SetGratuitousReplyFlag,
                                  &RoutingProtocol::GetGratuitousReplyFlag),
              MakeBooleanChecker())
          .AddAttribute(
              "DestinationOnly",
              "Indicates only the destination may respond to this RREQ.",
              BooleanValue(false),
              MakeBooleanAccessor(&RoutingProtocol::SetDestinationOnlyFlag,
                                  &RoutingProtocol::GetDestinationOnlyFlag),
              MakeBooleanChecker())
          .AddAttribute("EnableHello",
                        "Indicates whether a hello messages enable.",
                        BooleanValue(true),
                        MakeBooleanAccessor(&RoutingProtocol::SetHelloEnable,
                                            &RoutingProtocol::GetHelloEnable),
                        MakeBooleanChecker())
          .AddAttribute(
              "EnableBroadcast",
              "Indicates whether a broadcast data packets forwarding enable.",
              BooleanValue(true),
              MakeBooleanAccessor(&RoutingProtocol::SetBroadcastEnable,
                                  &RoutingProtocol::GetBroadcastEnable),
              MakeBooleanChecker())
          .AddAttribute(
              "UniformRv", "Access to the underlying UniformRandomVariable",
              StringValue("ns3::UniformRandomVariable"),
              MakePointerAccessor(&RoutingProtocol::m_uniformRandomVariable),
              MakePointerChecker<UniformRandomVariable>());
  return tid;
}

void RoutingProtocol::SetMaxQueueLen(uint32_t len) {
  m_maxQueueLen = len;
  m_queue.SetMaxQueueLen(len);
}

void RoutingProtocol::SetMaxQueueTime(Time t) {
  m_maxQueueTime = t;
  m_queue.SetQueueTimeout(t);
}

RoutingProtocol::~RoutingProtocol() {}

void RoutingProtocol::DoDispose() {
  m_ipv4 = nullptr;
  for (auto iter = m_socketAddresses.begin(); iter != m_socketAddresses.end();
       iter++) {
    iter->first->Close();
  }
  m_socketAddresses.clear();
  for (auto iter = m_socketSubnetBroadcastAddresses.begin();
       iter != m_socketSubnetBroadcastAddresses.end(); iter++) {
    iter->first->Close();
  }
  m_socketSubnetBroadcastAddresses.clear();
  Ipv4RoutingProtocol::DoDispose();
}

void RoutingProtocol::PrintRoutingTable(Ptr<OutputStreamWrapper> stream,
                                        Time::Unit unit) const {
  *stream->GetStream() << "Node: " << m_ipv4->GetObject<Node>()->GetId()
                       << "; Time: " << Now().As(unit) << ", Local time: "
                       << m_ipv4->GetObject<Node>()->GetLocalTime().As(unit)
                       << ", AODV Routing table" << std::endl;

  m_routingTable.Print(stream, unit);
  *stream->GetStream() << std::endl;
}

int64_t RoutingProtocol::AssignStreams(int64_t stream) {
  NS_LOG_FUNCTION(this << stream);
  m_uniformRandomVariable->SetStream(stream);
  return 1;
}

void RoutingProtocol::Start() {
  NS_LOG_FUNCTION(this);
  if (m_enableHello) {
    m_nb.ScheduleTimer();
  }
  m_rreqRateLimitTimer.SetFunction(&RoutingProtocol::RreqRateLimitTimerExpire,
                                   this);
  m_rreqRateLimitTimer.Schedule(Seconds(1));

  m_rerrRateLimitTimer.SetFunction(&RoutingProtocol::RerrRateLimitTimerExpire,
                                   this);
  m_rerrRateLimitTimer.Schedule(Seconds(1));
}

Ptr<Ipv4Route> RoutingProtocol::RouteOutput(Ptr<Packet> p,
                                            const Ipv4Header &header,
                                            Ptr<NetDevice> oif,
                                            Socket::SocketErrno &sockerr) {
  NS_LOG_FUNCTION(this << header << (oif ? oif->GetIfIndex() : 0));
  if (!p) {
    NS_LOG_DEBUG("Packet is == 0");
    return LoopbackRoute(header, oif);
  }
  if (m_socketAddresses.empty()) {
    sockerr = Socket::ERROR_NOROUTETOHOST;
    NS_LOG_LOGIC("No aodv interfaces");
    Ptr<Ipv4Route> route;
    return route;
  }
  sockerr = Socket::ERROR_NOTERROR;
  Ptr<Ipv4Route> route;
  Ipv4Address dst = header.GetDestination();
  RoutingTableEntry rt;
  if (m_routingTable.LookupValidRoute(dst, rt)) {
    route = rt.GetRoute();
    NS_ASSERT(route);
    NS_LOG_DEBUG("Exist route to " << route->GetDestination()
                                   << " from interface " << route->GetSource());
    if (oif && route->GetOutputDevice() != oif) {
      NS_LOG_DEBUG("Output device doesn't match. Dropped.");
      sockerr = Socket::ERROR_NOROUTETOHOST;
      return Ptr<Ipv4Route>();
    }
    UpdateRouteLifeTime(dst, m_activeRouteTimeout);
    UpdateRouteLifeTime(route->GetGateway(), m_activeRouteTimeout);
    return route;
  }

  uint32_t iif = (oif ? m_ipv4->GetInterfaceForDevice(oif) : -1);
  DeferredRouteOutputTag tag(iif);
  NS_LOG_DEBUG("Valid Route not found");
  if (!p->PeekPacketTag(tag)) {
    p->AddPacketTag(tag);
  }
  return LoopbackRoute(header, oif);
}

void RoutingProtocol::DeferredRouteOutput(Ptr<const Packet> p,
                                          const Ipv4Header &header,
                                          UnicastForwardCallback ucb,
                                          ErrorCallback ecb) {
  NS_LOG_FUNCTION(this << p << header);
  NS_ASSERT(p && p != Ptr<Packet>());

  QueueEntry newEntry(p, header, ucb, ecb);
  bool result = m_queue.Enqueue(newEntry);
  if (result) {
    NS_LOG_LOGIC("Add packet " << p->GetUid() << " to queue. Protocol "
                               << (uint16_t)header.GetProtocol());
    RoutingTableEntry rt;
    bool result = m_routingTable.LookupRoute(header.GetDestination(), rt);
    if (!result || ((rt.GetFlag() != IN_SEARCH) && result)) {
      NS_LOG_LOGIC("Send new RREQ for outbound packet to "
                   << header.GetDestination());
      SendRequest(header.GetDestination());
    }
  }
}

bool RoutingProtocol::RouteInput(Ptr<const Packet> p, const Ipv4Header &header,
                                 Ptr<const NetDevice> idev,
                                 const UnicastForwardCallback &ucb,
                                 const MulticastForwardCallback &mcb,
                                 const LocalDeliverCallback &lcb,
                                 const ErrorCallback &ecb) {
  NS_LOG_FUNCTION(this << p->GetUid() << header.GetDestination()
                       << idev->GetAddress());
  if (m_socketAddresses.empty()) {
    NS_LOG_LOGIC("No aodv interfaces");
    return false;
  }
  NS_ASSERT(m_ipv4);
  NS_ASSERT(p);
  NS_ASSERT(m_ipv4->GetInterfaceForDevice(idev) >= 0);
  int32_t iif = m_ipv4->GetInterfaceForDevice(idev);

  Ipv4Address dst = header.GetDestination();
  Ipv4Address origin = header.GetSource();

  if (idev == m_lo) {
    DeferredRouteOutputTag tag;
    if (p->PeekPacketTag(tag)) {
      DeferredRouteOutput(p, header, ucb, ecb);
      return true;
    }
  }

  if (IsMyOwnAddress(origin)) {
    return true;
  }

  if (dst.IsMulticast()) {
    return false;
  }

  for (auto j = m_socketAddresses.begin(); j != m_socketAddresses.end(); ++j) {
    Ipv4InterfaceAddress iface = j->second;
    if (m_ipv4->GetInterfaceForAddress(iface.GetLocal()) == iif) {
      if (dst == iface.GetBroadcast() || dst.IsBroadcast()) {
        if (m_dpd.IsDuplicate(p, header)) {
          NS_LOG_DEBUG("Duplicated packet " << p->GetUid() << " from " << origin
                                            << ". Drop.");
          return true;
        }
        UpdateRouteLifeTime(origin, m_activeRouteTimeout);
        Ptr<Packet> packet = p->Copy();
        if (!lcb.IsNull()) {
          NS_LOG_LOGIC("Broadcast local delivery to " << iface.GetLocal());
          lcb(p, header, iif);
        } else {
          NS_LOG_ERROR("Unable to deliver packet locally due to null callback "
                       << p->GetUid() << " from " << origin);
          ecb(p, header, Socket::ERROR_NOROUTETOHOST);
        }
        if (!m_enableBroadcast) {
          return true;
        }
        if (header.GetProtocol() == UdpL4Protocol::PROT_NUMBER) {
          UdpHeader udpHeader;
          p->PeekHeader(udpHeader);
          if (udpHeader.GetDestinationPort() == AODV_PORT) {
            return true;
          }
        }
        if (header.GetTtl() > 1) {
          NS_LOG_LOGIC("Forward broadcast. TTL " << (uint16_t)header.GetTtl());
          RoutingTableEntry toBroadcast;
          if (m_routingTable.LookupRoute(dst, toBroadcast)) {
            Ptr<Ipv4Route> route = toBroadcast.GetRoute();
            ucb(route, packet, header);
          } else {
            NS_LOG_DEBUG("No route to forward broadcast. Drop packet "
                         << p->GetUid());
          }
        } else {
          NS_LOG_DEBUG("TTL exceeded. Drop packet " << p->GetUid());
        }
        return true;
      }
    }
  }

  if (m_ipv4->IsDestinationAddress(dst, iif)) {
    UpdateRouteLifeTime(origin, m_activeRouteTimeout);
    RoutingTableEntry toOrigin;
    if (m_routingTable.LookupValidRoute(origin, toOrigin)) {
      UpdateRouteLifeTime(toOrigin.GetNextHop(), m_activeRouteTimeout);
      m_nb.Update(toOrigin.GetNextHop(), m_activeRouteTimeout);
    }
    if (!lcb.IsNull()) {
      NS_LOG_LOGIC("Unicast local delivery to " << dst);
      lcb(p, header, iif);
    } else {
      NS_LOG_ERROR("Unable to deliver packet locally due to null callback "
                   << p->GetUid() << " from " << origin);
      ecb(p, header, Socket::ERROR_NOROUTETOHOST);
    }
    return true;
  }

  if (!m_ipv4->IsForwarding(iif)) {
    NS_LOG_LOGIC("Forwarding disabled for this interface");
    ecb(p, header, Socket::ERROR_NOROUTETOHOST);
    return true;
  }

  return Forwarding(p, header, ucb, ecb);
}

bool RoutingProtocol::Forwarding(Ptr<const Packet> p, const Ipv4Header &header,
                                 UnicastForwardCallback ucb,
                                 ErrorCallback ecb) {
  NS_LOG_FUNCTION(this);
  Ipv4Address dst = header.GetDestination();
  Ipv4Address origin = header.GetSource();
  m_routingTable.Purge();
  RoutingTableEntry toDst;
  if (m_routingTable.LookupRoute(dst, toDst)) {
    if (toDst.GetFlag() == VALID) {
      Ptr<Ipv4Route> route = toDst.GetRoute();
      NS_LOG_LOGIC(route->GetSource() << " forwarding to " << dst << " from "
                                      << origin << " packet " << p->GetUid());

      UpdateRouteLifeTime(origin, m_activeRouteTimeout);
      UpdateRouteLifeTime(dst, m_activeRouteTimeout);
      UpdateRouteLifeTime(route->GetGateway(), m_activeRouteTimeout);
      RoutingTableEntry toOrigin;
      m_routingTable.LookupRoute(origin, toOrigin);
      UpdateRouteLifeTime(toOrigin.GetNextHop(), m_activeRouteTimeout);

      m_nb.Update(route->GetGateway(), m_activeRouteTimeout);
      m_nb.Update(toOrigin.GetNextHop(), m_activeRouteTimeout);

      ucb(route, p, header);
      return true;
    } else {
      if (toDst.GetValidSeqNo()) {
        SendRerrWhenNoRouteToForward(dst, toDst.GetSeqNo(), origin);
        NS_LOG_DEBUG("Drop packet " << p->GetUid()
                                    << " because no route to forward it.");
        return false;
      }
    }
  }
  NS_LOG_LOGIC("route not found to " << dst << ". Send RERR message.");
  NS_LOG_DEBUG("Drop packet " << p->GetUid()
                              << " because no route to forward it.");
  SendRerrWhenNoRouteToForward(dst, 0, origin);
  return false;
}

void RoutingProtocol::SetIpv4(Ptr<Ipv4> ipv4) {
  NS_ASSERT(ipv4);
  NS_ASSERT(!m_ipv4);

  m_ipv4 = ipv4;

  NS_ASSERT(m_ipv4->GetNInterfaces() == 1 &&
            m_ipv4->GetAddress(0, 0).GetLocal() == Ipv4Address("127.0.0.1"));
  m_lo = m_ipv4->GetNetDevice(0);
  NS_ASSERT(m_lo);
  RoutingTableEntry rt(
      m_lo, Ipv4Address::GetLoopback(), true, 0,
      Ipv4InterfaceAddress(Ipv4Address::GetLoopback(), Ipv4Mask("255.0.0.0")),
      1, Ipv4Address::GetLoopback(), Simulator::GetMaximumSimulationTime());
  m_routingTable.AddRoute(rt);

  Simulator::ScheduleNow(&RoutingProtocol::Start, this);
}

void RoutingProtocol::NotifyInterfaceUp(uint32_t i) {
  NS_LOG_FUNCTION(this << m_ipv4->GetAddress(i, 0).GetLocal());
  Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol>();
  if (l3->GetNAddresses(i) > 1) {
    NS_LOG_WARN(
        "AODV does not work with more then one address per each interface.");
  }
  Ipv4InterfaceAddress iface = l3->GetAddress(i, 0);
  if (iface.GetLocal() == Ipv4Address("127.0.0.1")) {
    return;
  }

  Ptr<Socket> socket =
      Socket::CreateSocket(GetObject<Node>(), UdpSocketFactory::GetTypeId());
  NS_ASSERT(socket);
  socket->SetRecvCallback(MakeCallback(&RoutingProtocol::RecvAodv, this));
  socket->BindToNetDevice(l3->GetNetDevice(i));
  socket->Bind(InetSocketAddress(iface.GetLocal(), AODV_PORT));
  socket->SetAllowBroadcast(true);
  socket->SetIpRecvTtl(true);
  m_socketAddresses.insert(std::make_pair(socket, iface));

  socket =
      Socket::CreateSocket(GetObject<Node>(), UdpSocketFactory::GetTypeId());
  NS_ASSERT(socket);
  socket->SetRecvCallback(MakeCallback(&RoutingProtocol::RecvAodv, this));
  socket->BindToNetDevice(l3->GetNetDevice(i));
  socket->Bind(InetSocketAddress(iface.GetBroadcast(), AODV_PORT));
  socket->SetAllowBroadcast(true);
  socket->SetIpRecvTtl(true);
  m_socketSubnetBroadcastAddresses.insert(std::make_pair(socket, iface));

  Ptr<NetDevice> dev =
      m_ipv4->GetNetDevice(m_ipv4->GetInterfaceForAddress(iface.GetLocal()));
  RoutingTableEntry rt(dev, iface.GetBroadcast(), true, 0, iface, 1,
                       iface.GetBroadcast(),
                       Simulator::GetMaximumSimulationTime());
  m_routingTable.AddRoute(rt);

  if (l3->GetInterface(i)->GetArpCache()) {
    m_nb.AddArpCache(l3->GetInterface(i)->GetArpCache());
  }

  Ptr<WifiNetDevice> wifi = dev->GetObject<WifiNetDevice>();
  if (!wifi) {
    return;
  }
  Ptr<WifiMac> mac = wifi->GetMac();
  if (!mac) {
    return;
  }

  mac->TraceConnectWithoutContext(
      "DroppedMpdu", MakeCallback(&RoutingProtocol::NotifyTxError, this));
}

void RoutingProtocol::NotifyTxError(WifiMacDropReason reason,
                                    Ptr<const WifiMpdu> mpdu) {
  m_nb.GetTxErrorCallback()(mpdu->GetHeader());
}

void RoutingProtocol::NotifyInterfaceDown(uint32_t i) {
  NS_LOG_FUNCTION(this << m_ipv4->GetAddress(i, 0).GetLocal());

  Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol>();
  Ptr<NetDevice> dev = l3->GetNetDevice(i);
  Ptr<WifiNetDevice> wifi = dev->GetObject<WifiNetDevice>();
  if (wifi) {
    Ptr<WifiMac> mac = wifi->GetMac()->GetObject<AdhocWifiMac>();
    if (mac) {
      mac->TraceDisconnectWithoutContext(
          "DroppedMpdu", MakeCallback(&RoutingProtocol::NotifyTxError, this));
      m_nb.DelArpCache(l3->GetInterface(i)->GetArpCache());
    }
  }

  Ptr<Socket> socket = FindSocketWithInterfaceAddress(m_ipv4->GetAddress(i, 0));
  NS_ASSERT(socket);
  socket->Close();
  m_socketAddresses.erase(socket);

  socket =
      FindSubnetBroadcastSocketWithInterfaceAddress(m_ipv4->GetAddress(i, 0));
  NS_ASSERT(socket);
  socket->Close();
  m_socketSubnetBroadcastAddresses.erase(socket);

  if (m_socketAddresses.empty()) {
    NS_LOG_LOGIC("No aodv interfaces");
    m_htimer.Cancel();
    m_nb.Clear();
    m_routingTable.Clear();
    return;
  }
  m_routingTable.DeleteAllRoutesFromInterface(m_ipv4->GetAddress(i, 0));
}

void RoutingProtocol::NotifyAddAddress(uint32_t i,
                                       Ipv4InterfaceAddress address) {
  NS_LOG_FUNCTION(this << " interface " << i << " address " << address);
  Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol>();
  if (!l3->IsUp(i)) {
    return;
  }
  if (l3->GetNAddresses(i) == 1) {
    Ipv4InterfaceAddress iface = l3->GetAddress(i, 0);
    Ptr<Socket> socket = FindSocketWithInterfaceAddress(iface);
    if (!socket) {
      if (iface.GetLocal() == Ipv4Address("127.0.0.1")) {
        return;
      }
      Ptr<Socket> socket = Socket::CreateSocket(GetObject<Node>(),
                                                UdpSocketFactory::GetTypeId());
      NS_ASSERT(socket);
      socket->SetRecvCallback(MakeCallback(&RoutingProtocol::RecvAodv, this));
      socket->BindToNetDevice(l3->GetNetDevice(i));
      socket->Bind(InetSocketAddress(iface.GetLocal(), AODV_PORT));
      socket->SetAllowBroadcast(true);
      m_socketAddresses.insert(std::make_pair(socket, iface));

      socket = Socket::CreateSocket(GetObject<Node>(),
                                    UdpSocketFactory::GetTypeId());
      NS_ASSERT(socket);
      socket->SetRecvCallback(MakeCallback(&RoutingProtocol::RecvAodv, this));
      socket->BindToNetDevice(l3->GetNetDevice(i));
      socket->Bind(InetSocketAddress(iface.GetBroadcast(), AODV_PORT));
      socket->SetAllowBroadcast(true);
      socket->SetIpRecvTtl(true);
      m_socketSubnetBroadcastAddresses.insert(std::make_pair(socket, iface));

      Ptr<NetDevice> dev = m_ipv4->GetNetDevice(
          m_ipv4->GetInterfaceForAddress(iface.GetLocal()));
      RoutingTableEntry rt(dev, iface.GetBroadcast(), true, 0, iface, 1,
                           iface.GetBroadcast(),
                           Simulator::GetMaximumSimulationTime());
      m_routingTable.AddRoute(rt);
    }
  } else {
    NS_LOG_LOGIC("AODV does not work with more then one address per each "
                 "interface. Ignore "
                 "added address");
  }
}

void RoutingProtocol::NotifyRemoveAddress(uint32_t i,
                                          Ipv4InterfaceAddress address) {
  NS_LOG_FUNCTION(this);
  Ptr<Socket> socket = FindSocketWithInterfaceAddress(address);
  if (socket) {
    m_routingTable.DeleteAllRoutesFromInterface(address);
    socket->Close();
    m_socketAddresses.erase(socket);

    Ptr<Socket> unicastSocket =
        FindSubnetBroadcastSocketWithInterfaceAddress(address);
    if (unicastSocket) {
      unicastSocket->Close();
      m_socketAddresses.erase(unicastSocket);
    }

    Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol>();
    if (l3->GetNAddresses(i)) {
      Ipv4InterfaceAddress iface = l3->GetAddress(i, 0);
      Ptr<Socket> socket = Socket::CreateSocket(GetObject<Node>(),
                                                UdpSocketFactory::GetTypeId());
      NS_ASSERT(socket);
      socket->SetRecvCallback(MakeCallback(&RoutingProtocol::RecvAodv, this));
      socket->BindToNetDevice(l3->GetNetDevice(i));
      socket->Bind(InetSocketAddress(iface.GetLocal(), AODV_PORT));
      socket->SetAllowBroadcast(true);
      socket->SetIpRecvTtl(true);
      m_socketAddresses.insert(std::make_pair(socket, iface));

      socket = Socket::CreateSocket(GetObject<Node>(),
                                    UdpSocketFactory::GetTypeId());
      NS_ASSERT(socket);
      socket->SetRecvCallback(MakeCallback(&RoutingProtocol::RecvAodv, this));
      socket->BindToNetDevice(l3->GetNetDevice(i));
      socket->Bind(InetSocketAddress(iface.GetBroadcast(), AODV_PORT));
      socket->SetAllowBroadcast(true);
      socket->SetIpRecvTtl(true);
      m_socketSubnetBroadcastAddresses.insert(std::make_pair(socket, iface));

      Ptr<NetDevice> dev = m_ipv4->GetNetDevice(
          m_ipv4->GetInterfaceForAddress(iface.GetLocal()));
      RoutingTableEntry rt(dev, iface.GetBroadcast(), true, 0, iface, 1,
                           iface.GetBroadcast(),
                           Simulator::GetMaximumSimulationTime());
      m_routingTable.AddRoute(rt);
    }
    if (m_socketAddresses.empty()) {
      NS_LOG_LOGIC("No aodv interfaces");
      m_htimer.Cancel();
      m_nb.Clear();
      m_routingTable.Clear();
      return;
    }
  } else {
    NS_LOG_LOGIC("Remove address not participating in AODV operation");
  }
}

bool RoutingProtocol::IsMyOwnAddress(Ipv4Address src) {
  NS_LOG_FUNCTION(this << src);
  for (auto j = m_socketAddresses.begin(); j != m_socketAddresses.end(); ++j) {
    Ipv4InterfaceAddress iface = j->second;
    if (src == iface.GetLocal()) {
      return true;
    }
  }
  return false;
}

Ptr<Ipv4Route> RoutingProtocol::LoopbackRoute(const Ipv4Header &hdr,
                                              Ptr<NetDevice> oif) const {
  NS_LOG_FUNCTION(this << hdr);
  NS_ASSERT(m_lo);
  Ptr<Ipv4Route> rt = Create<Ipv4Route>();
  rt->SetDestination(hdr.GetDestination());
  auto j = m_socketAddresses.begin();
  if (oif) {
    for (j = m_socketAddresses.begin(); j != m_socketAddresses.end(); ++j) {
      Ipv4Address addr = j->second.GetLocal();
      int32_t interface = m_ipv4->GetInterfaceForAddress(addr);
      if (oif == m_ipv4->GetNetDevice(static_cast<uint32_t>(interface))) {
        rt->SetSource(addr);
        break;
      }
    }
  } else {
    rt->SetSource(j->second.GetLocal());
  }
  NS_ASSERT_MSG(rt->GetSource() != Ipv4Address(),
                "Valid AODV source address not found");
  rt->SetGateway(Ipv4Address("127.0.0.1"));
  rt->SetOutputDevice(m_lo);
  return rt;
}

void RoutingProtocol::SendRequest(Ipv4Address dst) {
  NS_LOG_FUNCTION(this << dst);
  if (m_rreqCount == m_rreqRateLimit) {
    Simulator::Schedule(m_rreqRateLimitTimer.GetDelayLeft() + MicroSeconds(100),
                        &RoutingProtocol::SendRequest, this, dst);
    return;
  } else {
    m_rreqCount++;
  }
  RreqHeader rreqHeader;
  rreqHeader.SetDst(dst);

  RoutingTableEntry rt;
  uint16_t ttl = m_ttlStart;
  if (m_routingTable.LookupRoute(dst, rt)) {
    if (rt.GetFlag() != IN_SEARCH) {
      ttl = std::min<uint16_t>(rt.GetHop() + m_ttlIncrement, m_netDiameter);
    } else {
      ttl = rt.GetHop() + m_ttlIncrement;
      if (ttl > m_ttlThreshold) {
        ttl = m_netDiameter;
      }
    }
    if (ttl == m_netDiameter) {
      rt.IncrementRreqCnt();
    }
    if (rt.GetValidSeqNo()) {
      rreqHeader.SetDstSeqno(rt.GetSeqNo());
    } else {
      rreqHeader.SetUnknownSeqno(true);
    }
    rt.SetHop(ttl);
    rt.SetFlag(IN_SEARCH);
    rt.SetLifeTime(m_pathDiscoveryTime);
    m_routingTable.Update(rt);
  } else {
    rreqHeader.SetUnknownSeqno(true);
    Ptr<NetDevice> dev = nullptr;
    RoutingTableEntry newEntry(dev, dst, false, 0, Ipv4InterfaceAddress(), ttl,
                               Ipv4Address(), m_pathDiscoveryTime);
    if (ttl == m_netDiameter) {
      newEntry.IncrementRreqCnt();
    }
    newEntry.SetFlag(IN_SEARCH);
    m_routingTable.AddRoute(newEntry);
  }

  if (m_gratuitousReply) {
    rreqHeader.SetGratuitousRrep(true);
  }
  if (m_destinationOnly) {
    rreqHeader.SetDestinationOnly(true);
  }

  m_seqNo++;
  rreqHeader.SetOriginSeqno(m_seqNo);
  m_requestId++;
  rreqHeader.SetId(m_requestId);

  for (auto j = m_socketAddresses.begin(); j != m_socketAddresses.end(); ++j) {
    Ptr<Socket> socket = j->first;
    Ipv4InterfaceAddress iface = j->second;

    rreqHeader.SetOrigin(iface.GetLocal());
    m_rreqIdCache.IsDuplicate(iface.GetLocal(), m_requestId);

    Ptr<Packet> packet = Create<Packet>();
    SocketIpTtlTag tag;
    tag.SetTtl(ttl);
    packet->AddPacketTag(tag);
    packet->AddHeader(rreqHeader);
    TypeHeader tHeader(AODVTYPE_RREQ);
    packet->AddHeader(tHeader);
    Ipv4Address destination;
    if (iface.GetMask() == Ipv4Mask::GetOnes()) {
      destination = Ipv4Address("255.255.255.255");
    } else {
      destination = iface.GetBroadcast();
    }
    NS_LOG_DEBUG("Send RREQ with id " << rreqHeader.GetId() << " to socket");
    m_lastBcastTime = Simulator::Now();
    Simulator::Schedule(
        Time(MilliSeconds(m_uniformRandomVariable->GetInteger(0, 10))),
        &RoutingProtocol::SendTo, this, socket, packet, destination);
  }
  ScheduleRreqRetry(dst);
}

void RoutingProtocol::SendTo(Ptr<Socket> socket, Ptr<Packet> packet,
                             Ipv4Address destination) {
  socket->SendTo(packet, 0, InetSocketAddress(destination, AODV_PORT));
}

void RoutingProtocol::ScheduleRreqRetry(Ipv4Address dst) {
  NS_LOG_FUNCTION(this << dst);
  if (m_addressReqTimer.find(dst) == m_addressReqTimer.end()) {
    Timer timer(Timer::CANCEL_ON_DESTROY);
    m_addressReqTimer[dst] = timer;
  }
  m_addressReqTimer[dst].SetFunction(&RoutingProtocol::RouteRequestTimerExpire,
                                     this);
  m_addressReqTimer[dst].Cancel();
  m_addressReqTimer[dst].SetArguments(dst);
  RoutingTableEntry rt;
  m_routingTable.LookupRoute(dst, rt);
  Time retry;
  if (rt.GetHop() < m_netDiameter) {
    retry = 2 * m_nodeTraversalTime * (rt.GetHop() + m_timeoutBuffer);
  } else {
    NS_ABORT_MSG_UNLESS(rt.GetRreqCnt() > 0,
                        "Unexpected value for GetRreqCount ()");
    uint16_t backoffFactor = rt.GetRreqCnt() - 1;
    NS_LOG_LOGIC("Applying binary exponential backoff factor "
                 << backoffFactor);
    retry = m_netTraversalTime * (1 << backoffFactor);
  }
  m_addressReqTimer[dst].Schedule(retry);
  NS_LOG_LOGIC("Scheduled RREQ retry in " << retry.As(Time::S));
}

void RoutingProtocol::RecvAodv(Ptr<Socket> socket) {
  NS_LOG_FUNCTION(this << socket);
  Address sourceAddress;
  Ptr<Packet> packet = socket->RecvFrom(sourceAddress);
  InetSocketAddress inetSourceAddr =
      InetSocketAddress::ConvertFrom(sourceAddress);
  Ipv4Address sender = inetSourceAddr.GetIpv4();
  Ipv4Address receiver;

  if (m_socketAddresses.find(socket) != m_socketAddresses.end()) {
    receiver = m_socketAddresses[socket].GetLocal();
  } else if (m_socketSubnetBroadcastAddresses.find(socket) !=
             m_socketSubnetBroadcastAddresses.end()) {
    receiver = m_socketSubnetBroadcastAddresses[socket].GetLocal();
  } else {
    NS_ASSERT_MSG(false, "Received a packet from an unknown socket");
  }
  NS_LOG_DEBUG("AODV node " << this << " received a AODV packet from " << sender
                            << " to " << receiver);

  UpdateRouteToNeighbor(sender, receiver);
  TypeHeader tHeader(AODVTYPE_RREQ);
  packet->RemoveHeader(tHeader);
  if (!tHeader.IsValid()) {
    NS_LOG_DEBUG("AODV message " << packet->GetUid()
                                 << " with unknown type received: "
                                 << tHeader.Get() << ". Drop");
    return;
  }
  switch (tHeader.Get()) {
  case AODVTYPE_RREQ: {
    RecvRequest(packet, receiver, sender);
    break;
  }
  case AODVTYPE_RREP: {
    RecvReply(packet, receiver, sender);
    break;
  }
  case AODVTYPE_RERR: {
    RecvError(packet, sender);
    break;
  }
  case AODVTYPE_RREP_ACK: {
    RecvReplyAck(sender);
    break;
  }
  }
}

bool RoutingProtocol::UpdateRouteLifeTime(Ipv4Address addr, Time lifetime) {
  NS_LOG_FUNCTION(this << addr << lifetime);
  RoutingTableEntry rt;
  if (m_routingTable.LookupRoute(addr, rt)) {
    if (rt.GetFlag() == VALID) {
      NS_LOG_DEBUG("Updating VALID route");
      rt.SetRreqCnt(0);
      rt.SetLifeTime(std::max(lifetime, rt.GetLifeTime()));
      m_routingTable.Update(rt);
      return true;
    }
  }
  return false;
}

void RoutingProtocol::UpdateRouteToNeighbor(Ipv4Address sender,
                                            Ipv4Address receiver) {
  NS_LOG_FUNCTION(this << "sender " << sender << " receiver " << receiver);
  RoutingTableEntry toNeighbor;
  if (!m_routingTable.LookupRoute(sender, toNeighbor)) {
    Ptr<NetDevice> dev =
        m_ipv4->GetNetDevice(m_ipv4->GetInterfaceForAddress(receiver));
    RoutingTableEntry newEntry(
        dev, sender, false, 0,
        m_ipv4->GetAddress(m_ipv4->GetInterfaceForAddress(receiver), 0), 1,
        sender, m_activeRouteTimeout);
    m_routingTable.AddRoute(newEntry);
  } else {
    Ptr<NetDevice> dev =
        m_ipv4->GetNetDevice(m_ipv4->GetInterfaceForAddress(receiver));
    if (toNeighbor.GetValidSeqNo() && (toNeighbor.GetHop() == 1) &&
        (toNeighbor.GetOutputDevice() == dev)) {
      toNeighbor.SetLifeTime(
          std::max(m_activeRouteTimeout, toNeighbor.GetLifeTime()));
    } else {
      RoutingTableEntry newEntry(
          dev, sender, false, 0,
          m_ipv4->GetAddress(m_ipv4->GetInterfaceForAddress(receiver), 0), 1,
          sender, std::max(m_activeRouteTimeout, toNeighbor.GetLifeTime()));
      m_routingTable.Update(newEntry);
    }
  }
}

void RoutingProtocol::RecvRequest(Ptr<Packet> p, Ipv4Address receiver,
                                  Ipv4Address src) {
  NS_LOG_FUNCTION(this);
  RreqHeader rreqHeader;
  p->RemoveHeader(rreqHeader);

  RoutingTableEntry toPrev;
  if (m_routingTable.LookupRoute(src, toPrev)) {
    if (toPrev.IsUnidirectional()) {
      NS_LOG_DEBUG("Ignoring RREQ from node in blacklist");
      return;
    }
  }

  uint32_t id = rreqHeader.GetId();
  Ipv4Address origin = rreqHeader.GetOrigin();

  if (m_rreqIdCache.IsDuplicate(origin, id)) {
    NS_LOG_DEBUG("Ignoring RREQ due to duplicate");
    return;
  }

  uint8_t hop = rreqHeader.GetHopCount() + 1;
  rreqHeader.SetHopCount(hop);

  RoutingTableEntry toOrigin;
  if (!m_routingTable.LookupRoute(origin, toOrigin)) {
    Ptr<NetDevice> dev =
        m_ipv4->GetNetDevice(m_ipv4->GetInterfaceForAddress(receiver));
    RoutingTableEntry newEntry(
        dev, origin, true, rreqHeader.GetOriginSeqno(),
        m_ipv4->GetAddress(m_ipv4->GetInterfaceForAddress(receiver), 0), hop,
        src, Time((2 * m_netTraversalTime - 2 * hop * m_nodeTraversalTime)));
    m_routingTable.AddRoute(newEntry);
  } else {
    if (toOrigin.GetValidSeqNo()) {
      if (int32_t(rreqHeader.GetOriginSeqno()) - int32_t(toOrigin.GetSeqNo()) >
          0) {
        toOrigin.SetSeqNo(rreqHeader.GetOriginSeqno());
      }
    } else {
      toOrigin.SetSeqNo(rreqHeader.GetOriginSeqno());
    }
    toOrigin.SetValidSeqNo(true);
    toOrigin.SetNextHop(src);
    toOrigin.SetOutputDevice(
        m_ipv4->GetNetDevice(m_ipv4->GetInterfaceForAddress(receiver)));
    toOrigin.SetInterface(
        m_ipv4->GetAddress(m_ipv4->GetInterfaceForAddress(receiver), 0));
    toOrigin.SetHop(hop);
    toOrigin.SetLifeTime(
        std::max(Time(2 * m_netTraversalTime - 2 * hop * m_nodeTraversalTime),
                 toOrigin.GetLifeTime()));
    m_routingTable.Update(toOrigin);
  }

  RoutingTableEntry toNeighbor;
  if (!m_routingTable.LookupRoute(src, toNeighbor)) {
    NS_LOG_DEBUG(
        "Neighbor:" << src << " not found in routing table. Creating an entry");
    Ptr<NetDevice> dev =
        m_ipv4->GetNetDevice(m_ipv4->GetInterfaceForAddress(receiver));
    RoutingTableEntry newEntry(
        dev, src, false, rreqHeader.GetOriginSeqno(),
        m_ipv4->GetAddress(m_ipv4->GetInterfaceForAddress(receiver), 0), 1, src,
        m_activeRouteTimeout);
    m_routingTable.AddRoute(newEntry);
  } else {
    toNeighbor.SetLifeTime(m_activeRouteTimeout);
    toNeighbor.SetValidSeqNo(false);
    toNeighbor.SetSeqNo(rreqHeader.GetOriginSeqno());
    toNeighbor.SetFlag(VALID);
    toNeighbor.SetOutputDevice(
        m_ipv4->GetNetDevice(m_ipv4->GetInterfaceForAddress(receiver)));
    toNeighbor.SetInterface(
        m_ipv4->GetAddress(m_ipv4->GetInterfaceForAddress(receiver), 0));
    toNeighbor.SetHop(1);
    toNeighbor.SetNextHop(src);
    m_routingTable.Update(toNeighbor);
  }
  m_nb.Update(src, Time(m_allowedHelloLoss * m_helloInterval));

  NS_LOG_LOGIC(receiver << " receive RREQ with hop count "
                        << static_cast<uint32_t>(rreqHeader.GetHopCount())
                        << " ID " << rreqHeader.GetId() << " to destination "
                        << rreqHeader.GetDst());

  if (IsMyOwnAddress(rreqHeader.GetDst())) {
    m_routingTable.LookupRoute(origin, toOrigin);
    NS_LOG_DEBUG("Send reply since I am the destination");
    SendReply(rreqHeader, toOrigin);
    return;
  }
  RoutingTableEntry toDst;
  Ipv4Address dst = rreqHeader.GetDst();
  if (m_routingTable.LookupRoute(dst, toDst)) {
    if (toDst.GetNextHop() == src) {
      NS_LOG_DEBUG("Drop RREQ from " << src << ", dest next hop "
                                     << toDst.GetNextHop());
      return;
    }
    if ((rreqHeader.GetUnknownSeqno() ||
         (int32_t(toDst.GetSeqNo()) - int32_t(rreqHeader.GetDstSeqno()) >=
          0)) &&
        toDst.GetValidSeqNo()) {
      if (!rreqHeader.GetDestinationOnly() && toDst.GetFlag() == VALID) {
        m_routingTable.LookupRoute(origin, toOrigin);
        SendReplyByIntermediateNode(toDst, toOrigin,
                                    rreqHeader.GetGratuitousRrep());
        return;
      }
      rreqHeader.SetDstSeqno(toDst.GetSeqNo());
      rreqHeader.SetUnknownSeqno(false);
    }
  }

  SocketIpTtlTag tag;
  p->RemovePacketTag(tag);
  if (tag.GetTtl() < 2) {
    NS_LOG_DEBUG("TTL exceeded. Drop RREQ origin " << src << " destination "
                                                   << dst);
    return;
  }

  for (auto j = m_socketAddresses.begin(); j != m_socketAddresses.end(); ++j) {
    Ptr<Socket> socket = j->first;
    Ipv4InterfaceAddress iface = j->second;
    Ptr<Packet> packet = Create<Packet>();
    SocketIpTtlTag ttl;
    ttl.SetTtl(tag.GetTtl() - 1);
    packet->AddPacketTag(ttl);
    packet->AddHeader(rreqHeader);
    TypeHeader tHeader(AODVTYPE_RREQ);
    packet->AddHeader(tHeader);
    Ipv4Address destination;
    if (iface.GetMask() == Ipv4Mask::GetOnes()) {
      destination = Ipv4Address("255.255.255.255");
    } else {
      destination = iface.GetBroadcast();
    }
    m_lastBcastTime = Simulator::Now();
    Simulator::Schedule(
        Time(MilliSeconds(m_uniformRandomVariable->GetInteger(0, 10))),
        &RoutingProtocol::SendTo, this, socket, packet, destination);
  }
}

void RoutingProtocol::SendReply(const RreqHeader &rreqHeader,
                                const RoutingTableEntry &toOrigin) {
  NS_LOG_FUNCTION(this << toOrigin.GetDestination());
  if (!rreqHeader.GetUnknownSeqno() &&
      (rreqHeader.GetDstSeqno() == m_seqNo + 1)) {
    m_seqNo++;
  }
  RrepHeader rrepHeader(0, 0, rreqHeader.GetDst(), m_seqNo,
                        toOrigin.GetDestination(), m_myRouteTimeout);
  Ptr<Packet> packet = Create<Packet>();
  SocketIpTtlTag tag;
  tag.SetTtl(toOrigin.GetHop());
  packet->AddPacketTag(tag);
  packet->AddHeader(rrepHeader);
  TypeHeader tHeader(AODVTYPE_RREP);
  packet->AddHeader(tHeader);
  Ptr<Socket> socket = FindSocketWithInterfaceAddress(toOrigin.GetInterface());
  NS_ASSERT(socket);
  socket->SendTo(packet, 0,
                 InetSocketAddress(toOrigin.GetNextHop(), AODV_PORT));
}

void RoutingProtocol::SendReplyByIntermediateNode(RoutingTableEntry &toDst,
                                                  RoutingTableEntry &toOrigin,
                                                  bool gratRep) {
  NS_LOG_FUNCTION(this);
  RrepHeader rrepHeader(0, toDst.GetHop(), toDst.GetDestination(),
                        toDst.GetSeqNo(), toOrigin.GetDestination(),
                        toDst.GetLifeTime());
  if (toDst.GetHop() == 1) {
    rrepHeader.SetAckRequired(true);
    RoutingTableEntry toNextHop;
    m_routingTable.LookupRoute(toOrigin.GetNextHop(), toNextHop);
    toNextHop.m_ackTimer.SetFunction(&RoutingProtocol::AckTimerExpire, this);
    toNextHop.m_ackTimer.SetArguments(toNextHop.GetDestination(),
                                      m_blackListTimeout);
    toNextHop.m_ackTimer.SetDelay(m_nextHopWait);
  }
  toDst.InsertPrecursor(toOrigin.GetNextHop());
  toOrigin.InsertPrecursor(toDst.GetNextHop());
  m_routingTable.Update(toDst);
  m_routingTable.Update(toOrigin);

  Ptr<Packet> packet = Create<Packet>();
  SocketIpTtlTag tag;
  tag.SetTtl(toOrigin.GetHop());
  packet->AddPacketTag(tag);
  packet->AddHeader(rrepHeader);
  TypeHeader tHeader(AODVTYPE_RREP);
  packet->AddHeader(tHeader);
  Ptr<Socket> socket = FindSocketWithInterfaceAddress(toOrigin.GetInterface());
  NS_ASSERT(socket);
  socket->SendTo(packet, 0,
                 InetSocketAddress(toOrigin.GetNextHop(), AODV_PORT));

  if (gratRep) {
    RrepHeader gratRepHeader(0, toOrigin.GetHop(), toOrigin.GetDestination(),
                             toOrigin.GetSeqNo(), toDst.GetDestination(),
                             toOrigin.GetLifeTime());
    Ptr<Packet> packetToDst = Create<Packet>();
    SocketIpTtlTag gratTag;
    gratTag.SetTtl(toDst.GetHop());
    packetToDst->AddPacketTag(gratTag);
    packetToDst->AddHeader(gratRepHeader);
    TypeHeader type(AODVTYPE_RREP);
    packetToDst->AddHeader(type);
    Ptr<Socket> socket = FindSocketWithInterfaceAddress(toDst.GetInterface());
    NS_ASSERT(socket);
    NS_LOG_LOGIC("Send gratuitous RREP " << packet->GetUid());
    socket->SendTo(packetToDst, 0,
                   InetSocketAddress(toDst.GetNextHop(), AODV_PORT));
  }
}

void RoutingProtocol::SendReplyAck(Ipv4Address neighbor) {
  NS_LOG_FUNCTION(this << " to " << neighbor);
  RrepAckHeader h;
  TypeHeader typeHeader(AODVTYPE_RREP_ACK);
  Ptr<Packet> packet = Create<Packet>();
  SocketIpTtlTag tag;
  tag.SetTtl(1);
  packet->AddPacketTag(tag);
  packet->AddHeader(h);
  packet->AddHeader(typeHeader);
  RoutingTableEntry toNeighbor;
  m_routingTable.LookupRoute(neighbor, toNeighbor);
  Ptr<Socket> socket =
      FindSocketWithInterfaceAddress(toNeighbor.GetInterface());
  NS_ASSERT(socket);
  socket->SendTo(packet, 0, InetSocketAddress(neighbor, AODV_PORT));
}

void RoutingProtocol::RecvReply(Ptr<Packet> p, Ipv4Address receiver,
                                Ipv4Address sender) {
  NS_LOG_FUNCTION(this << " src " << sender);
  RrepHeader rrepHeader;
  p->RemoveHeader(rrepHeader);
  Ipv4Address dst = rrepHeader.GetDst();
  NS_LOG_LOGIC("RREP destination " << dst << " RREP origin "
                                   << rrepHeader.GetOrigin());

  uint8_t hop = rrepHeader.GetHopCount() + 1;
  rrepHeader.SetHopCount(hop);

  if (dst == rrepHeader.GetOrigin()) {
    ProcessHello(rrepHeader, receiver);
    return;
  }

  Ptr<NetDevice> dev =
      m_ipv4->GetNetDevice(m_ipv4->GetInterfaceForAddress(receiver));
  RoutingTableEntry newEntry(
      dev, dst, true, rrepHeader.GetDstSeqno(),
      m_ipv4->GetAddress(m_ipv4->GetInterfaceForAddress(receiver), 0), hop,
      sender, rrepHeader.GetLifeTime());
  RoutingTableEntry toDst;
  if (m_routingTable.LookupRoute(dst, toDst)) {
    if (!toDst.GetValidSeqNo()) {
      m_routingTable.Update(newEntry);
    } else if ((int32_t(rrepHeader.GetDstSeqno()) - int32_t(toDst.GetSeqNo())) >
               0) {
      m_routingTable.Update(newEntry);
    } else {
      if ((rrepHeader.GetDstSeqno() == toDst.GetSeqNo()) &&
          (toDst.GetFlag() != VALID)) {
        m_routingTable.Update(newEntry);
      } else if ((rrepHeader.GetDstSeqno() == toDst.GetSeqNo()) &&
                 (hop < toDst.GetHop())) {
        m_routingTable.Update(newEntry);
      }
    }
  } else {
    NS_LOG_LOGIC("add new route");
    m_routingTable.AddRoute(newEntry);
  }
  if (rrepHeader.GetAckRequired()) {
    SendReplyAck(sender);
    rrepHeader.SetAckRequired(false);
  }
  NS_LOG_LOGIC("receiver " << receiver << " origin " << rrepHeader.GetOrigin());
  if (IsMyOwnAddress(rrepHeader.GetOrigin())) {
    if (toDst.GetFlag() == IN_SEARCH) {
      m_routingTable.Update(newEntry);
      m_addressReqTimer[dst].Cancel();
      m_addressReqTimer.erase(dst);
    }
    m_routingTable.LookupRoute(dst, toDst);
    SendPacketFromQueue(dst, toDst.GetRoute());
    return;
  }

  RoutingTableEntry toOrigin;
  if (!m_routingTable.LookupRoute(rrepHeader.GetOrigin(), toOrigin) ||
      toOrigin.GetFlag() == IN_SEARCH) {
    return;
  }
  toOrigin.SetLifeTime(std::max(m_activeRouteTimeout, toOrigin.GetLifeTime()));
  m_routingTable.Update(toOrigin);

  if (m_routingTable.LookupValidRoute(rrepHeader.GetDst(), toDst)) {
    toDst.InsertPrecursor(toOrigin.GetNextHop());
    m_routingTable.Update(toDst);

    RoutingTableEntry toNextHopToDst;
    m_routingTable.LookupRoute(toDst.GetNextHop(), toNextHopToDst);
    toNextHopToDst.InsertPrecursor(toOrigin.GetNextHop());
    m_routingTable.Update(toNextHopToDst);

    toOrigin.InsertPrecursor(toDst.GetNextHop());
    m_routingTable.Update(toOrigin);

    RoutingTableEntry toNextHopToOrigin;
    m_routingTable.LookupRoute(toOrigin.GetNextHop(), toNextHopToOrigin);
    toNextHopToOrigin.InsertPrecursor(toDst.GetNextHop());
    m_routingTable.Update(toNextHopToOrigin);
  }
  SocketIpTtlTag tag;
  p->RemovePacketTag(tag);
  if (tag.GetTtl() < 2) {
    NS_LOG_DEBUG("TTL exceeded. Drop RREP destination "
                 << dst << " origin " << rrepHeader.GetOrigin());
    return;
  }

  Ptr<Packet> packet = Create<Packet>();
  SocketIpTtlTag ttl;
  ttl.SetTtl(tag.GetTtl() - 1);
  packet->AddPacketTag(ttl);
  packet->AddHeader(rrepHeader);
  TypeHeader tHeader(AODVTYPE_RREP);
  packet->AddHeader(tHeader);
  Ptr<Socket> socket = FindSocketWithInterfaceAddress(toOrigin.GetInterface());
  NS_ASSERT(socket);
  socket->SendTo(packet, 0,
                 InetSocketAddress(toOrigin.GetNextHop(), AODV_PORT));
}

void RoutingProtocol::RecvReplyAck(Ipv4Address neighbor) {
  NS_LOG_FUNCTION(this);
  RoutingTableEntry rt;
  if (m_routingTable.LookupRoute(neighbor, rt)) {
    rt.m_ackTimer.Cancel();
    rt.SetFlag(VALID);
    m_routingTable.Update(rt);
  }
}

void RoutingProtocol::ProcessHello(const RrepHeader &rrepHeader,
                                   Ipv4Address receiver) {
  NS_LOG_FUNCTION(this << "from " << rrepHeader.GetDst());
  RoutingTableEntry toNeighbor;
  if (!m_routingTable.LookupRoute(rrepHeader.GetDst(), toNeighbor)) {
    Ptr<NetDevice> dev =
        m_ipv4->GetNetDevice(m_ipv4->GetInterfaceForAddress(receiver));
    RoutingTableEntry newEntry(
        dev, rrepHeader.GetDst(), true, rrepHeader.GetDstSeqno(),
        m_ipv4->GetAddress(m_ipv4->GetInterfaceForAddress(receiver), 0), 1,
        rrepHeader.GetDst(), rrepHeader.GetLifeTime());
    m_routingTable.AddRoute(newEntry);
  } else {
    toNeighbor.SetLifeTime(std::max(Time(m_allowedHelloLoss * m_helloInterval),
                                    toNeighbor.GetLifeTime()));
    toNeighbor.SetSeqNo(rrepHeader.GetDstSeqno());
    toNeighbor.SetValidSeqNo(true);
    toNeighbor.SetFlag(VALID);
    toNeighbor.SetOutputDevice(
        m_ipv4->GetNetDevice(m_ipv4->GetInterfaceForAddress(receiver)));
    toNeighbor.SetInterface(
        m_ipv4->GetAddress(m_ipv4->GetInterfaceForAddress(receiver), 0));
    toNeighbor.SetHop(1);
    toNeighbor.SetNextHop(rrepHeader.GetDst());
    m_routingTable.Update(toNeighbor);
  }
  if (m_enableHello) {
    m_nb.Update(rrepHeader.GetDst(),
                Time(m_allowedHelloLoss * m_helloInterval));
  }
}

void RoutingProtocol::RecvError(Ptr<Packet> p, Ipv4Address src) {
  NS_LOG_FUNCTION(this << " from " << src);
  RerrHeader rerrHeader;
  p->RemoveHeader(rerrHeader);
  std::map<Ipv4Address, uint32_t> dstWithNextHopSrc;
  std::map<Ipv4Address, uint32_t> unreachable;
  m_routingTable.GetListOfDestinationWithNextHop(src, dstWithNextHopSrc);
  std::pair<Ipv4Address, uint32_t> un;
  while (rerrHeader.RemoveUnDestination(un)) {
    for (auto i = dstWithNextHopSrc.begin(); i != dstWithNextHopSrc.end();
         ++i) {
      if (i->first == un.first) {
        unreachable.insert(un);
      }
    }
  }

  std::vector<Ipv4Address> precursors;
  for (auto i = unreachable.begin(); i != unreachable.end();) {
    if (!rerrHeader.AddUnDestination(i->first, i->second)) {
      TypeHeader typeHeader(AODVTYPE_RERR);
      Ptr<Packet> packet = Create<Packet>();
      SocketIpTtlTag tag;
      tag.SetTtl(1);
      packet->AddPacketTag(tag);
      packet->AddHeader(rerrHeader);
      packet->AddHeader(typeHeader);
      SendRerrMessage(packet, precursors);
      rerrHeader.Clear();
    } else {
      RoutingTableEntry toDst;
      m_routingTable.LookupRoute(i->first, toDst);
      toDst.GetPrecursors(precursors);
      ++i;
    }
  }
  if (rerrHeader.GetDestCount() != 0) {
    TypeHeader typeHeader(AODVTYPE_RERR);
    Ptr<Packet> packet = Create<Packet>();
    SocketIpTtlTag tag;
    tag.SetTtl(1);
    packet->AddPacketTag(tag);
    packet->AddHeader(rerrHeader);
    packet->AddHeader(typeHeader);
    SendRerrMessage(packet, precursors);
  }
  m_routingTable.InvalidateRoutesWithDst(unreachable);
}

void RoutingProtocol::RouteRequestTimerExpire(Ipv4Address dst) {
  NS_LOG_LOGIC(this);
  RoutingTableEntry toDst;
  if (m_routingTable.LookupValidRoute(dst, toDst)) {
    SendPacketFromQueue(dst, toDst.GetRoute());
    NS_LOG_LOGIC("route to " << dst << " found");
    return;
  }
  if (toDst.GetRreqCnt() == m_rreqRetries) {
    NS_LOG_LOGIC("route discovery to "
                 << dst << " has been attempted RreqRetries (" << m_rreqRetries
                 << ") times with ttl " << m_netDiameter);
    m_addressReqTimer.erase(dst);
    m_routingTable.DeleteRoute(dst);
    NS_LOG_DEBUG("Route not found. Drop all packets with dst " << dst);
    m_queue.DropPacketWithDst(dst);
    return;
  }

  if (toDst.GetFlag() == IN_SEARCH) {
    NS_LOG_LOGIC("Resend RREQ to " << dst << " previous ttl "
                                   << toDst.GetHop());
    SendRequest(dst);
  } else {
    NS_LOG_DEBUG("Route down. Stop search. Drop packet with destination "
                 << dst);
    m_addressReqTimer.erase(dst);
    m_routingTable.DeleteRoute(dst);
    m_queue.DropPacketWithDst(dst);
  }
}

void RoutingProtocol::HelloTimerExpire() {
  NS_LOG_FUNCTION(this);
  Time offset = Time(Seconds(0));
  if (m_lastBcastTime > Time(Seconds(0))) {
    offset = Simulator::Now() - m_lastBcastTime;
    NS_LOG_DEBUG("Hello deferred due to last bcast at:" << m_lastBcastTime);
  } else {
    SendHello();
  }
  m_htimer.Cancel();
  Time diff = m_helloInterval - offset;
  m_htimer.Schedule(std::max(Time(Seconds(0)), diff));
  m_lastBcastTime = Time(Seconds(0));
}

void RoutingProtocol::RreqRateLimitTimerExpire() {
  NS_LOG_FUNCTION(this);
  m_rreqCount = 0;
  m_rreqRateLimitTimer.Schedule(Seconds(1));
}

void RoutingProtocol::RerrRateLimitTimerExpire() {
  NS_LOG_FUNCTION(this);
  m_rerrCount = 0;
  m_rerrRateLimitTimer.Schedule(Seconds(1));
}

void RoutingProtocol::AckTimerExpire(Ipv4Address neighbor,
                                     Time blacklistTimeout) {
  NS_LOG_FUNCTION(this);
  m_routingTable.MarkLinkAsUnidirectional(neighbor, blacklistTimeout);
}

void RoutingProtocol::SendHello() {
  NS_LOG_FUNCTION(this);
  for (auto j = m_socketAddresses.begin(); j != m_socketAddresses.end(); ++j) {
    Ptr<Socket> socket = j->first;
    Ipv4InterfaceAddress iface = j->second;
    RrepHeader helloHeader(0, 0, iface.GetLocal(), m_seqNo, iface.GetLocal(),
                           Time(m_allowedHelloLoss * m_helloInterval));
    Ptr<Packet> packet = Create<Packet>();
    SocketIpTtlTag tag;
    tag.SetTtl(1);
    packet->AddPacketTag(tag);
    packet->AddHeader(helloHeader);
    TypeHeader tHeader(AODVTYPE_RREP);
    packet->AddHeader(tHeader);
    Ipv4Address destination;
    if (iface.GetMask() == Ipv4Mask::GetOnes()) {
      destination = Ipv4Address("255.255.255.255");
    } else {
      destination = iface.GetBroadcast();
    }
    Time jitter =
        Time(MilliSeconds(m_uniformRandomVariable->GetInteger(0, 10)));
    Simulator::Schedule(jitter, &RoutingProtocol::SendTo, this, socket, packet,
                        destination);
  }
}

void RoutingProtocol::SendPacketFromQueue(Ipv4Address dst,
                                          Ptr<Ipv4Route> route) {
  NS_LOG_FUNCTION(this);
  QueueEntry queueEntry;
  while (m_queue.Dequeue(dst, queueEntry)) {
    DeferredRouteOutputTag tag;
    Ptr<Packet> p = ConstCast<Packet>(queueEntry.GetPacket());
    if (p->RemovePacketTag(tag) && tag.GetInterface() != -1 &&
        tag.GetInterface() !=
            m_ipv4->GetInterfaceForDevice(route->GetOutputDevice())) {
      NS_LOG_DEBUG("Output device doesn't match. Dropped.");
      return;
    }
    UnicastForwardCallback ucb = queueEntry.GetUnicastForwardCallback();
    Ipv4Header header = queueEntry.GetIpv4Header();
    header.SetSource(route->GetSource());
    header.SetTtl(header.GetTtl() + 1);
    ucb(route, p, header);
  }
}

void RoutingProtocol::SendRerrWhenBreaksLinkToNextHop(Ipv4Address nextHop) {
  NS_LOG_FUNCTION(this << nextHop);
  RerrHeader rerrHeader;
  std::vector<Ipv4Address> precursors;
  std::map<Ipv4Address, uint32_t> unreachable;

  RoutingTableEntry toNextHop;
  if (!m_routingTable.LookupRoute(nextHop, toNextHop)) {
    return;
  }
  toNextHop.GetPrecursors(precursors);
  rerrHeader.AddUnDestination(nextHop, toNextHop.GetSeqNo());
  m_routingTable.GetListOfDestinationWithNextHop(nextHop, unreachable);
  for (auto i = unreachable.begin(); i != unreachable.end();) {
    if (!rerrHeader.AddUnDestination(i->first, i->second)) {
      NS_LOG_LOGIC("Send RERR message with maximum size.");
      TypeHeader typeHeader(AODVTYPE_RERR);
      Ptr<Packet> packet = Create<Packet>();
      SocketIpTtlTag tag;
      tag.SetTtl(1);
      packet->AddPacketTag(tag);
      packet->AddHeader(rerrHeader);
      packet->AddHeader(typeHeader);
      SendRerrMessage(packet, precursors);
      rerrHeader.Clear();
    } else {
      RoutingTableEntry toDst;
      m_routingTable.LookupRoute(i->first, toDst);
      toDst.GetPrecursors(precursors);
      ++i;
    }
  }
  if (rerrHeader.GetDestCount() != 0) {
    TypeHeader typeHeader(AODVTYPE_RERR);
    Ptr<Packet> packet = Create<Packet>();
    SocketIpTtlTag tag;
    tag.SetTtl(1);
    packet->AddPacketTag(tag);
    packet->AddHeader(rerrHeader);
    packet->AddHeader(typeHeader);
    SendRerrMessage(packet, precursors);
  }
  unreachable.insert(std::make_pair(nextHop, toNextHop.GetSeqNo()));
  m_routingTable.InvalidateRoutesWithDst(unreachable);
}

void RoutingProtocol::SendRerrWhenNoRouteToForward(Ipv4Address dst,
                                                   uint32_t dstSeqNo,
                                                   Ipv4Address origin) {
  NS_LOG_FUNCTION(this);
  if (m_rerrCount == m_rerrRateLimit) {
    NS_ASSERT(m_rerrRateLimitTimer.IsRunning());
    NS_LOG_LOGIC("RerrRateLimit reached at "
                 << Simulator::Now().As(Time::S) << " with timer delay left "
                 << m_rerrRateLimitTimer.GetDelayLeft().As(Time::S)
                 << "; suppressing RERR");
    return;
  }
  RerrHeader rerrHeader;
  rerrHeader.AddUnDestination(dst, dstSeqNo);
  RoutingTableEntry toOrigin;
  Ptr<Packet> packet = Create<Packet>();
  SocketIpTtlTag tag;
  tag.SetTtl(1);
  packet->AddPacketTag(tag);
  packet->AddHeader(rerrHeader);
  packet->AddHeader(TypeHeader(AODVTYPE_RERR));
  if (m_routingTable.LookupValidRoute(origin, toOrigin)) {
    Ptr<Socket> socket =
        FindSocketWithInterfaceAddress(toOrigin.GetInterface());
    NS_ASSERT(socket);
    NS_LOG_LOGIC("Unicast RERR to the source of the data transmission");
    socket->SendTo(packet, 0,
                   InetSocketAddress(toOrigin.GetNextHop(), AODV_PORT));
  } else {
    for (auto i = m_socketAddresses.begin(); i != m_socketAddresses.end();
         ++i) {
      Ptr<Socket> socket = i->first;
      Ipv4InterfaceAddress iface = i->second;
      NS_ASSERT(socket);
      NS_LOG_LOGIC("Broadcast RERR message from interface "
                   << iface.GetLocal());
      Ipv4Address destination;
      if (iface.GetMask() == Ipv4Mask::GetOnes()) {
        destination = Ipv4Address("255.255.255.255");
      } else {
        destination = iface.GetBroadcast();
      }
      socket->SendTo(packet->Copy(), 0,
                     InetSocketAddress(destination, AODV_PORT));
    }
  }
}

void RoutingProtocol::SendRerrMessage(Ptr<Packet> packet,
                                      std::vector<Ipv4Address> precursors) {
  NS_LOG_FUNCTION(this);

  if (precursors.empty()) {
    NS_LOG_LOGIC("No precursors");
    return;
  }
  if (m_rerrCount == m_rerrRateLimit) {
    NS_ASSERT(m_rerrRateLimitTimer.IsRunning());
    NS_LOG_LOGIC("RerrRateLimit reached at "
                 << Simulator::Now().As(Time::S) << " with timer delay left "
                 << m_rerrRateLimitTimer.GetDelayLeft().As(Time::S)
                 << "; suppressing RERR");
    return;
  }
  if (precursors.size() == 1) {
    RoutingTableEntry toPrecursor;
    if (m_routingTable.LookupValidRoute(precursors.front(), toPrecursor)) {
      Ptr<Socket> socket =
          FindSocketWithInterfaceAddress(toPrecursor.GetInterface());
      NS_ASSERT(socket);
      NS_LOG_LOGIC("one precursor => unicast RERR to "
                   << toPrecursor.GetDestination() << " from "
                   << toPrecursor.GetInterface().GetLocal());
      Simulator::Schedule(
          Time(MilliSeconds(m_uniformRandomVariable->GetInteger(0, 10))),
          &RoutingProtocol::SendTo, this, socket, packet, precursors.front());
      m_rerrCount++;
    }
    return;
  }

  std::vector<Ipv4InterfaceAddress> ifaces;
  RoutingTableEntry toPrecursor;
  for (auto i = precursors.begin(); i != precursors.end(); ++i) {
    if (m_routingTable.LookupValidRoute(*i, toPrecursor) &&
        std::find(ifaces.begin(), ifaces.end(), toPrecursor.GetInterface()) ==
            ifaces.end()) {
      ifaces.push_back(toPrecursor.GetInterface());
    }
  }

  for (auto i = ifaces.begin(); i != ifaces.end(); ++i) {
    Ptr<Socket> socket = FindSocketWithInterfaceAddress(*i);
    NS_ASSERT(socket);
    NS_LOG_LOGIC("Broadcast RERR message from interface " << i->GetLocal());
    Ptr<Packet> p = packet->Copy();
    Ipv4Address destination;
    if (i->GetMask() == Ipv4Mask::GetOnes()) {
      destination = Ipv4Address("255.255.255.255");
    } else {
      destination = i->GetBroadcast();
    }
    Simulator::Schedule(
        Time(MilliSeconds(m_uniformRandomVariable->GetInteger(0, 10))),
        &RoutingProtocol::SendTo, this, socket, p, destination);
  }
}

Ptr<Socket> RoutingProtocol::FindSocketWithInterfaceAddress(
    Ipv4InterfaceAddress addr) const {
  NS_LOG_FUNCTION(this << addr);
  for (auto j = m_socketAddresses.begin(); j != m_socketAddresses.end(); ++j) {
    Ptr<Socket> socket = j->first;
    Ipv4InterfaceAddress iface = j->second;
    if (iface == addr) {
      return socket;
    }
  }
  Ptr<Socket> socket;
  return socket;
}

Ptr<Socket> RoutingProtocol::FindSubnetBroadcastSocketWithInterfaceAddress(
    Ipv4InterfaceAddress addr) const {
  NS_LOG_FUNCTION(this << addr);
  for (auto j = m_socketSubnetBroadcastAddresses.begin();
       j != m_socketSubnetBroadcastAddresses.end(); ++j) {
    Ptr<Socket> socket = j->first;
    Ipv4InterfaceAddress iface = j->second;
    if (iface == addr) {
      return socket;
    }
  }
  Ptr<Socket> socket;
  return socket;
}

void RoutingProtocol::DoInitialize() {
  NS_LOG_FUNCTION(this);
  uint32_t startTime;
  if (m_enableHello) {
    m_htimer.SetFunction(&RoutingProtocol::HelloTimerExpire, this);
    startTime = m_uniformRandomVariable->GetInteger(0, 100);
    NS_LOG_DEBUG("Starting at time " << startTime << "ms");
    m_htimer.Schedule(MilliSeconds(startTime));
  }
  Ipv4RoutingProtocol::DoInitialize();
}

} // namespace aodv
} // namespace ns3
