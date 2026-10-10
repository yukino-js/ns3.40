
#include "click-internet-stack-helper.h"

#include "ns3/arp-l3-protocol.h"
#include "ns3/assert.h"
#include "ns3/callback.h"
#include "ns3/config.h"
#include "ns3/core-config.h"
#include "ns3/ipv4-click-routing.h"
#include "ns3/ipv4-l3-click-protocol.h"
#include "ns3/ipv4.h"
#include "ns3/log.h"
#include "ns3/names.h"
#include "ns3/net-device.h"
#include "ns3/node.h"
#include "ns3/object.h"
#include "ns3/packet-socket-factory.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/trace-helper.h"

#include <limits>
#include <map>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("ClickInternetStackHelper");

#define INTERFACE_CONTEXT

typedef std::pair<Ptr<Ipv4>, uint32_t> InterfacePairIpv4;
typedef std::map<InterfacePairIpv4, Ptr<PcapFileWrapper>> InterfaceFileMapIpv4;
typedef std::map<InterfacePairIpv4, Ptr<OutputStreamWrapper>>
    InterfaceStreamMapIpv4;

static InterfaceFileMapIpv4 g_interfaceFileMapIpv4;
static InterfaceStreamMapIpv4 g_interfaceStreamMapIpv4;

static void Ipv4L3ProtocolRxTxSink(Ptr<const Packet> p, Ptr<Ipv4> ipv4,
                                   uint32_t interface) {
  NS_LOG_FUNCTION(p << ipv4 << interface);

  InterfacePairIpv4 pair = std::make_pair(ipv4, interface);
  if (g_interfaceFileMapIpv4.find(pair) == g_interfaceFileMapIpv4.end()) {
    NS_LOG_INFO("Ignoring packet to/from interface " << interface);
    return;
  }

  Ptr<PcapFileWrapper> file = g_interfaceFileMapIpv4[pair];
  file->Write(Simulator::Now(), p);
}

static void Ipv4L3ProtocolDropSinkWithoutContext(
    Ptr<OutputStreamWrapper> stream, const Ipv4Header &header,
    Ptr<const Packet> packet, Ipv4L3Protocol::DropReason reason, Ptr<Ipv4> ipv4,
    uint32_t interface) {
  InterfacePairIpv4 pair = std::make_pair(ipv4, interface);
  if (g_interfaceStreamMapIpv4.find(pair) == g_interfaceStreamMapIpv4.end()) {
    NS_LOG_INFO("Ignoring packet to/from interface " << interface);
    return;
  }

  Ptr<Packet> p = packet->Copy();
  p->AddHeader(header);
  *stream->GetStream() << "d " << Simulator::Now().GetSeconds() << " " << *p
                       << std::endl;
}

static void Ipv4L3ProtocolDropSinkWithContext(
    Ptr<OutputStreamWrapper> stream, std::string context,
    const Ipv4Header &header, Ptr<const Packet> packet,
    Ipv4L3Protocol::DropReason reason, Ptr<Ipv4> ipv4, uint32_t interface) {
  InterfacePairIpv4 pair = std::make_pair(ipv4, interface);
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

ClickInternetStackHelper::ClickInternetStackHelper() : m_ipv4Enabled(true) {
  Initialize();
}

void ClickInternetStackHelper::Initialize() {}

ClickInternetStackHelper::~ClickInternetStackHelper() {}

ClickInternetStackHelper::ClickInternetStackHelper(
    const ClickInternetStackHelper &o) {
  m_ipv4Enabled = o.m_ipv4Enabled;
}

ClickInternetStackHelper &
ClickInternetStackHelper::operator=(const ClickInternetStackHelper &o) {
  if (this != &o) {
    m_ipv4Enabled = o.m_ipv4Enabled;
  }
  return *this;
}

void ClickInternetStackHelper::Reset() {
  m_ipv4Enabled = true;
  Initialize();
}

void ClickInternetStackHelper::SetClickFile(NodeContainer c,
                                            std::string clickfile) {
  for (auto i = c.Begin(); i != c.End(); ++i) {
    SetClickFile(*i, clickfile);
  }
}

void ClickInternetStackHelper::SetClickFile(Ptr<Node> node,
                                            std::string clickfile) {
  m_nodeToClickFileMap.insert(std::make_pair(node, clickfile));
}

void ClickInternetStackHelper::SetDefines(
    NodeContainer c, std::map<std::string, std::string> defines) {
  for (auto i = c.Begin(); i != c.End(); ++i) {
    SetDefines(*i, defines);
  }
}

void ClickInternetStackHelper::SetDefines(
    Ptr<Node> node, std::map<std::string, std::string> defines) {
  m_nodeToDefinesMap.insert(std::make_pair(node, defines));
}

void ClickInternetStackHelper::SetRoutingTableElement(NodeContainer c,
                                                      std::string rt) {
  for (auto i = c.Begin(); i != c.End(); ++i) {
    SetRoutingTableElement(*i, rt);
  }
}

void ClickInternetStackHelper::SetRoutingTableElement(Ptr<Node> node,
                                                      std::string rt) {
  m_nodeToRoutingTableElementMap.insert(std::make_pair(node, rt));
}

void ClickInternetStackHelper::Install(NodeContainer c) const {
  for (auto i = c.Begin(); i != c.End(); ++i) {
    Install(*i);
  }
}

void ClickInternetStackHelper::InstallAll() const {
  Install(NodeContainer::GetGlobal());
}

void ClickInternetStackHelper::CreateAndAggregateObjectFromTypeId(
    Ptr<Node> node, const std::string typeId) {
  ObjectFactory factory;
  factory.SetTypeId(typeId);
  Ptr<Object> protocol = factory.Create<Object>();
  node->AggregateObject(protocol);
}

void ClickInternetStackHelper::Install(Ptr<Node> node) const {
  if (m_ipv4Enabled) {
    if (node->GetObject<Ipv4>()) {
      NS_FATAL_ERROR("ClickInternetStackHelper::Install (): Aggregating "
                     "an InternetStack to a node with an existing Ipv4 object");
      return;
    }

    CreateAndAggregateObjectFromTypeId(node, "ns3::ArpL3Protocol");
    CreateAndAggregateObjectFromTypeId(node, "ns3::Ipv4L3ClickProtocol");
    CreateAndAggregateObjectFromTypeId(node, "ns3::Icmpv4L4Protocol");
    CreateAndAggregateObjectFromTypeId(node, "ns3::UdpL4Protocol");
    CreateAndAggregateObjectFromTypeId(node, "ns3::TcpL4Protocol");
    Ptr<PacketSocketFactory> factory = CreateObject<PacketSocketFactory>();
    node->AggregateObject(factory);
    Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
    Ptr<Ipv4ClickRouting> ipv4Routing = CreateObject<Ipv4ClickRouting>();
    auto it = m_nodeToClickFileMap.find(node);

    if (it != m_nodeToClickFileMap.end()) {
      ipv4Routing->SetClickFile(it->second);
    }

    auto definesIt = m_nodeToDefinesMap.find(node);
    if (definesIt != m_nodeToDefinesMap.end()) {
      ipv4Routing->SetDefines(definesIt->second);
    }

    it = m_nodeToRoutingTableElementMap.find(node);
    if (it != m_nodeToRoutingTableElementMap.end()) {
      ipv4Routing->SetClickRoutingTableElement(it->second);
    }
    ipv4->SetRoutingProtocol(ipv4Routing);
    node->AggregateObject(ipv4Routing);
  }
}

void ClickInternetStackHelper::Install(std::string nodeName) const {
  Ptr<Node> node = Names::Find<Node>(nodeName);
  Install(node);
}

bool ClickInternetStackHelper::PcapHooked(Ptr<Ipv4> ipv4) {
  for (auto i = g_interfaceFileMapIpv4.begin();
       i != g_interfaceFileMapIpv4.end(); ++i) {
    if ((*i).first.first == ipv4) {
      return true;
    }
  }
  return false;
}

void ClickInternetStackHelper::EnablePcapIpv4Internal(std::string prefix,
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
                  "ClickInternetStackHelper::EnablePcapIpv4Internal(): "
                  "m_ipv4Enabled and ipv4L3Protocol inconsistent");

    bool result = ipv4L3Protocol->TraceConnectWithoutContext(
        "Tx", MakeCallback(&Ipv4L3ProtocolRxTxSink));
    NS_ASSERT_MSG(result == true,
                  "ClickInternetStackHelper::EnablePcapIpv4Internal():  "
                  "Unable to connect ipv4L3Protocol \"Tx\"");

    result = ipv4L3Protocol->TraceConnectWithoutContext(
        "Rx", MakeCallback(&Ipv4L3ProtocolRxTxSink));
    NS_ASSERT_MSG(result == true,
                  "ClickInternetStackHelper::EnablePcapIpv4Internal():  "
                  "Unable to connect ipv4L3Protocol \"Rx\"");
  }

  g_interfaceFileMapIpv4[std::make_pair(ipv4, interface)] = file;
}

bool ClickInternetStackHelper::AsciiHooked(Ptr<Ipv4> ipv4) {
  for (auto i = g_interfaceStreamMapIpv4.begin();
       i != g_interfaceStreamMapIpv4.end(); ++i) {
    if ((*i).first.first == ipv4) {
      return true;
    }
  }
  return false;
}

void ClickInternetStackHelper::EnableAsciiIpv4Internal(
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
                    "ClickInternetStackHelper::EnableAsciiIpv4Internal():  "
                    "Unable to connect ipv4L3Protocol \"Drop\"");
    }

    g_interfaceStreamMapIpv4[std::make_pair(ipv4, interface)] = theStream;
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
  }

  g_interfaceStreamMapIpv4[std::make_pair(ipv4, interface)] = stream;
}

} // namespace ns3
