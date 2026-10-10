
#include "internet-stack-helper.h"

#include "ipv4-global-routing-helper.h"
#include "ipv4-list-routing-helper.h"
#include "ipv4-static-routing-helper.h"
#include "ipv6-static-routing-helper.h"

#include "ns3/arp-l3-protocol.h"
#include "ns3/assert.h"
#include "ns3/callback.h"
#include "ns3/config.h"
#include "ns3/global-router-interface.h"
#include "ns3/icmpv6-l4-protocol.h"
#include "ns3/ipv4-global-routing.h"
#include "ns3/ipv4.h"
#include "ns3/ipv6-extension-demux.h"
#include "ns3/ipv6-extension-header.h"
#include "ns3/ipv6-extension.h"
#include "ns3/ipv6.h"
#include "ns3/log.h"
#include "ns3/names.h"
#include "ns3/net-device.h"
#include "ns3/node-list.h"
#include "ns3/node.h"
#include "ns3/object.h"
#include "ns3/packet-socket-factory.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/traffic-control-layer.h"

#include <limits>
#include <map>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("InternetStackHelper");

#define INTERFACE_CONTEXT

typedef std::pair<uint32_t, uint32_t> InterfacePairIpv4;
typedef std::map<InterfacePairIpv4, Ptr<PcapFileWrapper>> InterfaceFileMapIpv4;
typedef std::map<InterfacePairIpv4, Ptr<OutputStreamWrapper>>
    InterfaceStreamMapIpv4;

static InterfaceFileMapIpv4 g_interfaceFileMapIpv4;
static InterfaceStreamMapIpv4 g_interfaceStreamMapIpv4;

typedef std::pair<uint32_t, uint32_t> InterfacePairIpv6;
typedef std::map<InterfacePairIpv6, Ptr<PcapFileWrapper>> InterfaceFileMapIpv6;
typedef std::map<InterfacePairIpv6, Ptr<OutputStreamWrapper>>
    InterfaceStreamMapIpv6;

static InterfaceFileMapIpv6 g_interfaceFileMapIpv6;
static InterfaceStreamMapIpv6 g_interfaceStreamMapIpv6;

InternetStackHelper::InternetStackHelper()
    : m_routing(nullptr), m_routingv6(nullptr), m_ipv4Enabled(true),
      m_ipv6Enabled(true), m_ipv4ArpJitterEnabled(true),
      m_ipv6NsRsJitterEnabled(true)

{
  Initialize();
}

void InternetStackHelper::Initialize() {
  Ipv4StaticRoutingHelper staticRouting;
  Ipv4GlobalRoutingHelper globalRouting;
  Ipv4ListRoutingHelper listRouting;
  Ipv6StaticRoutingHelper staticRoutingv6;
  listRouting.Add(staticRouting, 0);
  listRouting.Add(globalRouting, -10);
  SetRoutingHelper(listRouting);
  SetRoutingHelper(staticRoutingv6);
}

InternetStackHelper::~InternetStackHelper() {
  delete m_routing;
  delete m_routingv6;
}

InternetStackHelper::InternetStackHelper(const InternetStackHelper &o) {
  m_routing = o.m_routing->Copy();
  m_routingv6 = o.m_routingv6->Copy();
  m_ipv4Enabled = o.m_ipv4Enabled;
  m_ipv6Enabled = o.m_ipv6Enabled;
  m_ipv4ArpJitterEnabled = o.m_ipv4ArpJitterEnabled;
  m_ipv6NsRsJitterEnabled = o.m_ipv6NsRsJitterEnabled;
}

InternetStackHelper &
InternetStackHelper::operator=(const InternetStackHelper &o) {
  if (this == &o) {
    return *this;
  }
  m_routing = o.m_routing->Copy();
  m_routingv6 = o.m_routingv6->Copy();
  return *this;
}

void InternetStackHelper::Reset() {
  delete m_routing;
  m_routing = nullptr;
  delete m_routingv6;
  m_routingv6 = nullptr;
  m_ipv4Enabled = true;
  m_ipv6Enabled = true;
  m_ipv4ArpJitterEnabled = true;
  m_ipv6NsRsJitterEnabled = true;
  Initialize();
}

void InternetStackHelper::SetRoutingHelper(const Ipv4RoutingHelper &routing) {
  delete m_routing;
  m_routing = routing.Copy();
}

void InternetStackHelper::SetRoutingHelper(const Ipv6RoutingHelper &routing) {
  delete m_routingv6;
  m_routingv6 = routing.Copy();
}

void InternetStackHelper::SetIpv4StackInstall(bool enable) {
  m_ipv4Enabled = enable;
}

void InternetStackHelper::SetIpv6StackInstall(bool enable) {
  m_ipv6Enabled = enable;
}

void InternetStackHelper::SetIpv4ArpJitter(bool enable) {
  m_ipv4ArpJitterEnabled = enable;
}

void InternetStackHelper::SetIpv6NsRsJitter(bool enable) {
  m_ipv6NsRsJitterEnabled = enable;
}

int64_t InternetStackHelper::AssignStreams(NodeContainer c, int64_t stream) {
  int64_t currentStream = stream;
  for (auto i = c.Begin(); i != c.End(); ++i) {
    Ptr<Node> node = *i;
    Ptr<GlobalRouter> router = node->GetObject<GlobalRouter>();
    if (router) {
      Ptr<Ipv4GlobalRouting> gr = router->GetRoutingProtocol();
      if (gr) {
        currentStream += gr->AssignStreams(currentStream);
      }
    }
    Ptr<Ipv6ExtensionDemux> demux = node->GetObject<Ipv6ExtensionDemux>();
    if (demux) {
      Ptr<Ipv6Extension> fe =
          demux->GetExtension(Ipv6ExtensionFragment::EXT_NUMBER);
      NS_ASSERT(fe);
      currentStream += fe->AssignStreams(currentStream);
    }
    Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
    if (ipv4) {
      Ptr<ArpL3Protocol> arpL3Protocol = ipv4->GetObject<ArpL3Protocol>();
      if (arpL3Protocol) {
        currentStream += arpL3Protocol->AssignStreams(currentStream);
      }
    }
    Ptr<Ipv6> ipv6 = node->GetObject<Ipv6>();
    if (ipv6) {
      Ptr<Icmpv6L4Protocol> icmpv6L4Protocol =
          ipv6->GetObject<Icmpv6L4Protocol>();
      if (icmpv6L4Protocol) {
        currentStream += icmpv6L4Protocol->AssignStreams(currentStream);
      }
    }
  }
  return (currentStream - stream);
}

void InternetStackHelper::Install(NodeContainer c) const {
  for (auto i = c.Begin(); i != c.End(); ++i) {
    Install(*i);
  }
}

void InternetStackHelper::InstallAll() const {
  Install(NodeContainer::GetGlobal());
}

void InternetStackHelper::CreateAndAggregateObjectFromTypeId(
    Ptr<Node> node, const std::string typeId) {
  TypeId tid = TypeId::LookupByName(typeId);
  if (node->GetObject<Object>(tid)) {
    return;
  }

  ObjectFactory factory;
  factory.SetTypeId(typeId);
  Ptr<Object> protocol = factory.Create<Object>();
  node->AggregateObject(protocol);
}

void InternetStackHelper::Install(Ptr<Node> node) const {
  if (m_ipv4Enabled) {
    CreateAndAggregateObjectFromTypeId(node, "ns3::ArpL3Protocol");
    CreateAndAggregateObjectFromTypeId(node, "ns3::Ipv4L3Protocol");
    CreateAndAggregateObjectFromTypeId(node, "ns3::Icmpv4L4Protocol");
    if (!m_ipv4ArpJitterEnabled) {
      Ptr<ArpL3Protocol> arp = node->GetObject<ArpL3Protocol>();
      NS_ASSERT(arp);
      arp->SetAttribute(
          "RequestJitter",
          StringValue("ns3::ConstantRandomVariable[Constant=0.0]"));
    }

    Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
    if (!ipv4->GetRoutingProtocol()) {
      Ptr<Ipv4RoutingProtocol> ipv4Routing = m_routing->Create(node);
      ipv4->SetRoutingProtocol(ipv4Routing);
    }
  }

  if (m_ipv6Enabled) {
    CreateAndAggregateObjectFromTypeId(node, "ns3::Ipv6L3Protocol");
    CreateAndAggregateObjectFromTypeId(node, "ns3::Icmpv6L4Protocol");
    if (!m_ipv6NsRsJitterEnabled) {
      Ptr<Icmpv6L4Protocol> icmpv6l4 = node->GetObject<Icmpv6L4Protocol>();
      NS_ASSERT(icmpv6l4);
      icmpv6l4->SetAttribute(
          "SolicitationJitter",
          StringValue("ns3::ConstantRandomVariable[Constant=0.0]"));
    }
    Ptr<Ipv6> ipv6 = node->GetObject<Ipv6>();
    if (!ipv6->GetRoutingProtocol()) {
      Ptr<Ipv6RoutingProtocol> ipv6Routing = m_routingv6->Create(node);
      ipv6->SetRoutingProtocol(ipv6Routing);
    }
    ipv6->RegisterExtensions();
    ipv6->RegisterOptions();
  }

  if (m_ipv4Enabled || m_ipv6Enabled) {
    CreateAndAggregateObjectFromTypeId(node, "ns3::TrafficControlLayer");
    CreateAndAggregateObjectFromTypeId(node, "ns3::UdpL4Protocol");
    CreateAndAggregateObjectFromTypeId(node, "ns3::TcpL4Protocol");
    if (!node->GetObject<PacketSocketFactory>()) {
      Ptr<PacketSocketFactory> factory = CreateObject<PacketSocketFactory>();
      node->AggregateObject(factory);
    }
  }

  if (m_ipv4Enabled) {
    Ptr<ArpL3Protocol> arp = node->GetObject<ArpL3Protocol>();
    Ptr<TrafficControlLayer> tc = node->GetObject<TrafficControlLayer>();
    NS_ASSERT(arp);
    NS_ASSERT(tc);
    arp->SetTrafficControl(tc);
  }
}

void InternetStackHelper::Install(std::string nodeName) const {
  Ptr<Node> node = Names::Find<Node>(nodeName);
  Install(node);
}

static void Ipv4L3ProtocolRxTxSink(Ptr<const Packet> p, Ptr<Ipv4> ipv4,
                                   uint32_t interface) {
  NS_LOG_FUNCTION(p << ipv4 << interface);

  InterfacePairIpv4 pair =
      std::make_pair(ipv4->GetObject<Node>()->GetId(), interface);
  if (g_interfaceFileMapIpv4.find(pair) == g_interfaceFileMapIpv4.end()) {
    NS_LOG_INFO("Ignoring packet to/from interface " << interface);
    return;
  }

  Ptr<PcapFileWrapper> file = g_interfaceFileMapIpv4[pair];
  file->Write(Simulator::Now(), p);
}

bool InternetStackHelper::PcapHooked(Ptr<Ipv4> ipv4) {
  auto id = ipv4->GetObject<Node>()->GetId();

  for (auto i = g_interfaceFileMapIpv4.begin();
       i != g_interfaceFileMapIpv4.end(); ++i) {
    if ((*i).first.first == id) {
      return true;
    }
  }
  return false;
}

void InternetStackHelper::EnablePcapIpv4Internal(std::string prefix,
                                                 Ptr<Ipv4> ipv4,
                                                 uint32_t interface,
                                                 bool explicitFilename) {
  NS_LOG_FUNCTION(prefix << ipv4 << interface);

  if (!m_ipv4Enabled) {
    NS_LOG_INFO("Call to enable Ipv4 pcap tracing but Ipv4 not enabled");
    return;
  }

  PcapHelper pcapHelper;

  std::string filename;
  if (explicitFilename) {
    filename = prefix;
  } else {
    filename = pcapHelper.GetFilenameFromInterfacePair(prefix, ipv4, interface);
  }

  Ptr<PcapFileWrapper> file =
      pcapHelper.CreateFile(filename, std::ios::out, PcapHelper::DLT_RAW);

  if (!PcapHooked(ipv4)) {
    Ptr<Ipv4L3Protocol> ipv4L3Protocol = ipv4->GetObject<Ipv4L3Protocol>();
    NS_ASSERT_MSG(ipv4L3Protocol,
                  "InternetStackHelper::EnablePcapIpv4Internal(): "
                  "m_ipv4Enabled and ipv4L3Protocol inconsistent");

    bool result = ipv4L3Protocol->TraceConnectWithoutContext(
        "Tx", MakeCallback(&Ipv4L3ProtocolRxTxSink));
    NS_ASSERT_MSG(result == true,
                  "InternetStackHelper::EnablePcapIpv4Internal():  "
                  "Unable to connect ipv4L3Protocol \"Tx\"");

    result = ipv4L3Protocol->TraceConnectWithoutContext(
        "Rx", MakeCallback(&Ipv4L3ProtocolRxTxSink));
    NS_ASSERT_MSG(result == true,
                  "InternetStackHelper::EnablePcapIpv4Internal():  "
                  "Unable to connect ipv4L3Protocol \"Rx\"");
  }

  g_interfaceFileMapIpv4[std::make_pair(ipv4->GetObject<Node>()->GetId(),
                                        interface)] = file;
}

static void Ipv6L3ProtocolRxTxSink(Ptr<const Packet> p, Ptr<Ipv6> ipv6,
                                   uint32_t interface) {
  NS_LOG_FUNCTION(p << ipv6 << interface);

  InterfacePairIpv6 pair =
      std::make_pair(ipv6->GetObject<Node>()->GetId(), interface);
  if (g_interfaceFileMapIpv6.find(pair) == g_interfaceFileMapIpv6.end()) {
    NS_LOG_INFO("Ignoring packet to/from interface " << interface);
    return;
  }

  Ptr<PcapFileWrapper> file = g_interfaceFileMapIpv6[pair];
  file->Write(Simulator::Now(), p);
}

bool InternetStackHelper::PcapHooked(Ptr<Ipv6> ipv6) {
  auto id = ipv6->GetObject<Node>()->GetId();

  for (auto i = g_interfaceFileMapIpv6.begin();
       i != g_interfaceFileMapIpv6.end(); ++i) {
    if ((*i).first.first == id) {
      return true;
    }
  }
  return false;
}

void InternetStackHelper::EnablePcapIpv6Internal(std::string prefix,
                                                 Ptr<Ipv6> ipv6,
                                                 uint32_t interface,
                                                 bool explicitFilename) {
  NS_LOG_FUNCTION(prefix << ipv6 << interface);

  if (!m_ipv6Enabled) {
    NS_LOG_INFO("Call to enable Ipv6 pcap tracing but Ipv6 not enabled");
    return;
  }

  PcapHelper pcapHelper;

  std::string filename;
  if (explicitFilename) {
    filename = prefix;
  } else {
    filename = pcapHelper.GetFilenameFromInterfacePair(prefix, ipv6, interface);
  }

  Ptr<PcapFileWrapper> file =
      pcapHelper.CreateFile(filename, std::ios::out, PcapHelper::DLT_RAW);

  if (!PcapHooked(ipv6)) {
    Ptr<Ipv6L3Protocol> ipv6L3Protocol = ipv6->GetObject<Ipv6L3Protocol>();
    NS_ASSERT_MSG(ipv6L3Protocol,
                  "InternetStackHelper::EnablePcapIpv6Internal(): "
                  "m_ipv6Enabled and ipv6L3Protocol inconsistent");

    bool result = ipv6L3Protocol->TraceConnectWithoutContext(
        "Tx", MakeCallback(&Ipv6L3ProtocolRxTxSink));
    NS_ASSERT_MSG(result == true,
                  "InternetStackHelper::EnablePcapIpv6Internal():  "
                  "Unable to connect ipv6L3Protocol \"Tx\"");

    result = ipv6L3Protocol->TraceConnectWithoutContext(
        "Rx", MakeCallback(&Ipv6L3ProtocolRxTxSink));
    NS_ASSERT_MSG(result == true,
                  "InternetStackHelper::EnablePcapIpv6Internal():  "
                  "Unable to connect ipv6L3Protocol \"Rx\"");
  }

  g_interfaceFileMapIpv6[std::make_pair(ipv6->GetObject<Node>()->GetId(),
                                        interface)] = file;
}

static void Ipv4L3ProtocolDropSinkWithoutContext(
    Ptr<OutputStreamWrapper> stream, const Ipv4Header &header,
    Ptr<const Packet> packet, Ipv4L3Protocol::DropReason reason, Ptr<Ipv4> ipv4,
    uint32_t interface) {
  InterfacePairIpv4 pair =
      std::make_pair(ipv4->GetObject<Node>()->GetId(), interface);
  if (g_interfaceStreamMapIpv4.find(pair) == g_interfaceStreamMapIpv4.end()) {
    NS_LOG_INFO("Ignoring packet to/from interface " << interface);
    return;
  }

  Ptr<Packet> p = packet->Copy();
  p->AddHeader(header);
  *stream->GetStream() << "d " << Simulator::Now().GetSeconds() << " " << *p
                       << std::endl;
}

static void Ipv4L3ProtocolTxSinkWithoutContext(Ptr<OutputStreamWrapper> stream,
                                               Ptr<const Packet> packet,
                                               Ptr<Ipv4> ipv4,
                                               uint32_t interface) {
  InterfacePairIpv4 pair =
      std::make_pair(ipv4->GetObject<Node>()->GetId(), interface);

  if (g_interfaceStreamMapIpv4.find(pair) == g_interfaceStreamMapIpv4.end()) {
    NS_LOG_INFO("Ignoring packet to/from interface " << interface);
    return;
  }
  *stream->GetStream() << "t " << Simulator::Now().GetSeconds() << " "
                       << *packet << std::endl;
}

static void Ipv4L3ProtocolRxSinkWithoutContext(Ptr<OutputStreamWrapper> stream,
                                               Ptr<const Packet> packet,
                                               Ptr<Ipv4> ipv4,
                                               uint32_t interface) {
  InterfacePairIpv4 pair =
      std::make_pair(ipv4->GetObject<Node>()->GetId(), interface);
  if (g_interfaceStreamMapIpv4.find(pair) == g_interfaceStreamMapIpv4.end()) {
    NS_LOG_INFO("Ignoring packet to/from interface " << interface);
    return;
  }

  *stream->GetStream() << "r " << Simulator::Now().GetSeconds() << " "
                       << *packet << std::endl;
}

static void Ipv4L3ProtocolDropSinkWithContext(
    Ptr<OutputStreamWrapper> stream, std::string context,
    const Ipv4Header &header, Ptr<const Packet> packet,
    Ipv4L3Protocol::DropReason reason, Ptr<Ipv4> ipv4, uint32_t interface) {
  InterfacePairIpv4 pair =
      std::make_pair(ipv4->GetObject<Node>()->GetId(), interface);
  if (g_interfaceStreamMapIpv4.find(pair) == g_interfaceStreamMapIpv4.end()) {
    NS_LOG_INFO("Ignoring packet to/from interface " << interface);
    return;
  }

  Ptr<Packet> p = packet->Copy();
  p->AddHeader(header);
#ifdef INTERFACE_CONTEXT
  *stream->GetStream() << "d " << Simulator::Now().GetSeconds() << " "
                       << context << "(" << interface << ") " << *p
                       << std::endl;
#else
  *stream->GetStream() << "d " << Simulator::Now().GetSeconds() << " "
                       << context << " " << *p << std::endl;
#endif
}

static void Ipv4L3ProtocolTxSinkWithContext(Ptr<OutputStreamWrapper> stream,
                                            std::string context,
                                            Ptr<const Packet> packet,
                                            Ptr<Ipv4> ipv4,
                                            uint32_t interface) {
  InterfacePairIpv4 pair =
      std::make_pair(ipv4->GetObject<Node>()->GetId(), interface);
  if (g_interfaceStreamMapIpv4.find(pair) == g_interfaceStreamMapIpv4.end()) {
    NS_LOG_INFO("Ignoring packet to/from interface " << interface);
    return;
  }

#ifdef INTERFACE_CONTEXT
  *stream->GetStream() << "t " << Simulator::Now().GetSeconds() << " "
                       << context << "(" << interface << ") " << *packet
                       << std::endl;
#else
  *stream->GetStream() << "t " << Simulator::Now().GetSeconds() << " "
                       << context << " " << *packet << std::endl;
#endif
}

static void Ipv4L3ProtocolRxSinkWithContext(Ptr<OutputStreamWrapper> stream,
                                            std::string context,
                                            Ptr<const Packet> packet,
                                            Ptr<Ipv4> ipv4,
                                            uint32_t interface) {
  InterfacePairIpv4 pair =
      std::make_pair(ipv4->GetObject<Node>()->GetId(), interface);
  if (g_interfaceStreamMapIpv4.find(pair) == g_interfaceStreamMapIpv4.end()) {
    NS_LOG_INFO("Ignoring packet to/from interface " << interface);
    return;
  }

#ifdef INTERFACE_CONTEXT
  *stream->GetStream() << "r " << Simulator::Now().GetSeconds() << " "
                       << context << "(" << interface << ") " << *packet
                       << std::endl;
#else
  *stream->GetStream() << "r " << Simulator::Now().GetSeconds() << " "
                       << context << " " << *packet << std::endl;
#endif
}

bool InternetStackHelper::AsciiHooked(Ptr<Ipv4> ipv4) {
  auto id = ipv4->GetObject<Node>()->GetId();

  for (auto i = g_interfaceStreamMapIpv4.begin();
       i != g_interfaceStreamMapIpv4.end(); ++i) {
    if ((*i).first.first == id) {
      return true;
    }
  }
  return false;
}

void InternetStackHelper::EnableAsciiIpv4Internal(
    Ptr<OutputStreamWrapper> stream, std::string prefix, Ptr<Ipv4> ipv4,
    uint32_t interface, bool explicitFilename) {
  if (!m_ipv4Enabled) {
    NS_LOG_INFO("Call to enable Ipv4 ascii tracing but Ipv4 not enabled");
    return;
  }

  Packet::EnablePrinting();

  if (!stream) {
    AsciiTraceHelper asciiTraceHelper;

    std::string filename;
    if (explicitFilename) {
      filename = prefix;
    } else {
      filename = asciiTraceHelper.GetFilenameFromInterfacePair(prefix, ipv4,
                                                               interface);
    }

    Ptr<OutputStreamWrapper> theStream =
        asciiTraceHelper.CreateFileStream(filename);

    if (!AsciiHooked(ipv4)) {
      Ptr<ArpL3Protocol> arpL3Protocol = ipv4->GetObject<ArpL3Protocol>();
      asciiTraceHelper.HookDefaultDropSinkWithoutContext<ArpL3Protocol>(
          arpL3Protocol, "Drop", theStream);

      Ptr<Ipv4L3Protocol> ipv4L3Protocol = ipv4->GetObject<Ipv4L3Protocol>();
      bool result = ipv4L3Protocol->TraceConnectWithoutContext(
          "Drop",
          MakeBoundCallback(&Ipv4L3ProtocolDropSinkWithoutContext, theStream));
      NS_ASSERT_MSG(result == true,
                    "InternetStackHelper::EnableAsciiIpv4Internal():  "
                    "Unable to connect ipv4L3Protocol \"Drop\"");
      result = ipv4L3Protocol->TraceConnectWithoutContext(
          "Tx",
          MakeBoundCallback(&Ipv4L3ProtocolTxSinkWithoutContext, theStream));
      NS_ASSERT_MSG(result == true,
                    "InternetStackHelper::EnableAsciiIpv4Internal():  "
                    "Unable to connect ipv4L3Protocol \"Tx\"");
      result = ipv4L3Protocol->TraceConnectWithoutContext(
          "Rx",
          MakeBoundCallback(&Ipv4L3ProtocolRxSinkWithoutContext, theStream));
      NS_ASSERT_MSG(result == true,
                    "InternetStackHelper::EnableAsciiIpv4Internal():  "
                    "Unable to connect ipv4L3Protocol \"Rx\"");
    }

    g_interfaceStreamMapIpv4[std::make_pair(ipv4->GetObject<Node>()->GetId(),
                                            interface)] = theStream;
    return;
  }

  if (!AsciiHooked(ipv4)) {
    Ptr<Node> node = ipv4->GetObject<Node>();
    std::ostringstream oss;

    oss << "/NodeList/" << node->GetId() << "/$ns3::ArpL3Protocol/Drop";
    Config::Connect(oss.str(),
                    MakeBoundCallback(
                        &AsciiTraceHelper::DefaultDropSinkWithContext, stream));

    oss.str("");
    oss << "/NodeList/" << node->GetId() << "/$ns3::Ipv4L3Protocol/Drop";
    Config::Connect(oss.str(), MakeBoundCallback(
                                   &Ipv4L3ProtocolDropSinkWithContext, stream));
    oss.str("");
    oss << "/NodeList/" << node->GetId() << "/$ns3::Ipv4L3Protocol/Tx";
    Config::Connect(
        oss.str(), MakeBoundCallback(&Ipv4L3ProtocolTxSinkWithContext, stream));
    oss.str("");
    oss << "/NodeList/" << node->GetId() << "/$ns3::Ipv4L3Protocol/Rx";
    Config::Connect(
        oss.str(), MakeBoundCallback(&Ipv4L3ProtocolRxSinkWithContext, stream));
  }

  g_interfaceStreamMapIpv4[std::make_pair(ipv4->GetObject<Node>()->GetId(),
                                          interface)] = stream;
}

static void Ipv6L3ProtocolDropSinkWithoutContext(
    Ptr<OutputStreamWrapper> stream, const Ipv6Header &header,
    Ptr<const Packet> packet, Ipv6L3Protocol::DropReason reason, Ptr<Ipv6> ipv6,
    uint32_t interface) {
  InterfacePairIpv6 pair =
      std::make_pair(ipv6->GetObject<Node>()->GetId(), interface);
  if (g_interfaceStreamMapIpv6.find(pair) == g_interfaceStreamMapIpv6.end()) {
    NS_LOG_INFO("Ignoring packet to/from interface " << interface);
    return;
  }

  Ptr<Packet> p = packet->Copy();
  p->AddHeader(header);
  *stream->GetStream() << "d " << Simulator::Now().GetSeconds() << " " << *p
                       << std::endl;
}

static void Ipv6L3ProtocolTxSinkWithoutContext(Ptr<OutputStreamWrapper> stream,
                                               Ptr<const Packet> packet,
                                               Ptr<Ipv6> ipv6,
                                               uint32_t interface) {
  InterfacePairIpv6 pair =
      std::make_pair(ipv6->GetObject<Node>()->GetId(), interface);
  if (g_interfaceStreamMapIpv6.find(pair) == g_interfaceStreamMapIpv6.end()) {
    NS_LOG_INFO("Ignoring packet to/from interface " << interface);
    return;
  }

  *stream->GetStream() << "t " << Simulator::Now().GetSeconds() << " "
                       << *packet << std::endl;
}

static void Ipv6L3ProtocolRxSinkWithoutContext(Ptr<OutputStreamWrapper> stream,
                                               Ptr<const Packet> packet,
                                               Ptr<Ipv6> ipv6,
                                               uint32_t interface) {
  InterfacePairIpv6 pair =
      std::make_pair(ipv6->GetObject<Node>()->GetId(), interface);
  if (g_interfaceStreamMapIpv6.find(pair) == g_interfaceStreamMapIpv6.end()) {
    NS_LOG_INFO("Ignoring packet to/from interface " << interface);
    return;
  }

  *stream->GetStream() << "r " << Simulator::Now().GetSeconds() << " "
                       << *packet << std::endl;
}

static void Ipv6L3ProtocolDropSinkWithContext(
    Ptr<OutputStreamWrapper> stream, std::string context,
    const Ipv6Header &header, Ptr<const Packet> packet,
    Ipv6L3Protocol::DropReason reason, Ptr<Ipv6> ipv6, uint32_t interface) {
  InterfacePairIpv6 pair =
      std::make_pair(ipv6->GetObject<Node>()->GetId(), interface);
  if (g_interfaceStreamMapIpv6.find(pair) == g_interfaceStreamMapIpv6.end()) {
    NS_LOG_INFO("Ignoring packet to/from interface " << interface);
    return;
  }

  Ptr<Packet> p = packet->Copy();
  p->AddHeader(header);
#ifdef INTERFACE_CONTEXT
  *stream->GetStream() << "d " << Simulator::Now().GetSeconds() << " "
                       << context << "(" << interface << ") " << *p
                       << std::endl;
#else
  *stream->GetStream() << "d " << Simulator::Now().GetSeconds() << " "
                       << context << " " << *p << std::endl;
#endif
}

static void Ipv6L3ProtocolTxSinkWithContext(Ptr<OutputStreamWrapper> stream,
                                            std::string context,
                                            Ptr<const Packet> packet,
                                            Ptr<Ipv6> ipv6,
                                            uint32_t interface) {
  InterfacePairIpv6 pair =
      std::make_pair(ipv6->GetObject<Node>()->GetId(), interface);
  if (g_interfaceStreamMapIpv6.find(pair) == g_interfaceStreamMapIpv6.end()) {
    NS_LOG_INFO("Ignoring packet to/from interface " << interface);
    return;
  }

#ifdef INTERFACE_CONTEXT
  *stream->GetStream() << "t " << Simulator::Now().GetSeconds() << " "
                       << context << "(" << interface << ") " << *packet
                       << std::endl;
#else
  *stream->GetStream() << "t " << Simulator::Now().GetSeconds() << " "
                       << context << " " << *packet << std::endl;
#endif
}

static void Ipv6L3ProtocolRxSinkWithContext(Ptr<OutputStreamWrapper> stream,
                                            std::string context,
                                            Ptr<const Packet> packet,
                                            Ptr<Ipv6> ipv6,
                                            uint32_t interface) {
  InterfacePairIpv6 pair =
      std::make_pair(ipv6->GetObject<Node>()->GetId(), interface);
  if (g_interfaceStreamMapIpv6.find(pair) == g_interfaceStreamMapIpv6.end()) {
    NS_LOG_INFO("Ignoring packet to/from interface " << interface);
    return;
  }

#ifdef INTERFACE_CONTEXT
  *stream->GetStream() << "r " << Simulator::Now().GetSeconds() << " "
                       << context << "(" << interface << ") " << *packet
                       << std::endl;
#else
  *stream->GetStream() << "r " << Simulator::Now().GetSeconds() << " "
                       << context << " " << *packet << std::endl;
#endif
}

bool InternetStackHelper::AsciiHooked(Ptr<Ipv6> ipv6) {
  auto id = ipv6->GetObject<Node>()->GetId();

  for (auto i = g_interfaceStreamMapIpv6.begin();
       i != g_interfaceStreamMapIpv6.end(); ++i) {
    if ((*i).first.first == id) {
      return true;
    }
  }
  return false;
}

void InternetStackHelper::EnableAsciiIpv6Internal(
    Ptr<OutputStreamWrapper> stream, std::string prefix, Ptr<Ipv6> ipv6,
    uint32_t interface, bool explicitFilename) {
  if (!m_ipv6Enabled) {
    NS_LOG_INFO("Call to enable Ipv6 ascii tracing but Ipv6 not enabled");
    return;
  }

  Packet::EnablePrinting();

  if (!stream) {
    AsciiTraceHelper asciiTraceHelper;

    std::string filename;
    if (explicitFilename) {
      filename = prefix;
    } else {
      filename = asciiTraceHelper.GetFilenameFromInterfacePair(prefix, ipv6,
                                                               interface);
    }

    Ptr<OutputStreamWrapper> theStream =
        asciiTraceHelper.CreateFileStream(filename);

    if (!AsciiHooked(ipv6)) {
      Ptr<Ipv6L3Protocol> ipv6L3Protocol = ipv6->GetObject<Ipv6L3Protocol>();
      bool result = ipv6L3Protocol->TraceConnectWithoutContext(
          "Drop",
          MakeBoundCallback(&Ipv6L3ProtocolDropSinkWithoutContext, theStream));
      NS_ASSERT_MSG(result == true,
                    "InternetStackHelper::EnableAsciiIpv6Internal():  "
                    "Unable to connect ipv6L3Protocol \"Drop\"");
      result = ipv6L3Protocol->TraceConnectWithoutContext(
          "Tx",
          MakeBoundCallback(&Ipv6L3ProtocolTxSinkWithoutContext, theStream));
      NS_ASSERT_MSG(result == true,
                    "InternetStackHelper::EnableAsciiIpv6Internal():  "
                    "Unable to connect ipv6L3Protocol \"Tx\"");
      result = ipv6L3Protocol->TraceConnectWithoutContext(
          "Rx",
          MakeBoundCallback(&Ipv6L3ProtocolRxSinkWithoutContext, theStream));
      NS_ASSERT_MSG(result == true,
                    "InternetStackHelper::EnableAsciiIpv6Internal():  "
                    "Unable to connect ipv6L3Protocol \"Rx\"");
    }

    g_interfaceStreamMapIpv6[std::make_pair(ipv6->GetObject<Node>()->GetId(),
                                            interface)] = theStream;
    return;
  }

  if (!AsciiHooked(ipv6)) {
    Ptr<Node> node = ipv6->GetObject<Node>();
    std::ostringstream oss;

    oss.str("");
    oss << "/NodeList/" << node->GetId() << "/$ns3::Ipv6L3Protocol/Drop";
    Config::Connect(oss.str(), MakeBoundCallback(
                                   &Ipv6L3ProtocolDropSinkWithContext, stream));
    oss.str("");
    oss << "/NodeList/" << node->GetId() << "/$ns3::Ipv6L3Protocol/Tx";
    Config::Connect(
        oss.str(), MakeBoundCallback(&Ipv6L3ProtocolTxSinkWithContext, stream));
    oss.str("");
    oss << "/NodeList/" << node->GetId() << "/$ns3::Ipv6L3Protocol/Rx";
    Config::Connect(
        oss.str(), MakeBoundCallback(&Ipv6L3ProtocolRxSinkWithContext, stream));
  }

  g_interfaceStreamMapIpv6[std::make_pair(ipv6->GetObject<Node>()->GetId(),
                                          interface)] = stream;
}

} // namespace ns3
