

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("NeighborCacheExample");

class NeighborCacheExample {
public:
  NeighborCacheExample();

  void Run();

  void CommandSetup(int argc, char **argv);

private:
  void ReceivePacket(Ptr<const Packet> pkt, const Address &from,
                     const Address &dst, const SeqTsSizeHeader &header);

  bool m_useIpv6{false};
  bool m_enableLog{false};
  bool m_useChannel{false};
  bool m_useNetDeviceContainer{false};
  bool m_useInterfaceContainer{false};
  bool m_noGenerate{false};
  bool m_sendTraffic{false};
};

NeighborCacheExample::NeighborCacheExample() { NS_LOG_FUNCTION(this); }

void NeighborCacheExample::ReceivePacket(Ptr<const Packet> pkt,
                                         const Address &from,
                                         const Address &dst,
                                         const SeqTsSizeHeader &header) {
  std::cout << "Rx pkt from " << from << " to " << dst << " -> " << header
            << std::endl;
}

void NeighborCacheExample::CommandSetup(int argc, char **argv) {
  CommandLine cmd(__FILE__);
  cmd.AddValue("useIPv6", "Use IPv6 instead of IPv4", m_useIpv6);
  cmd.AddValue("enableLog", "Enable ArpL3Protocol and Icmpv6L4Protocol logging",
               m_enableLog);
  cmd.AddValue("useChannel", "Generate neighbor cache for specific Channel",
               m_useChannel);
  cmd.AddValue("useNetDeviceContainer",
               "Generate neighbor cache for specific netDeviceContainer",
               m_useNetDeviceContainer);
  cmd.AddValue("useInterfaceContainer",
               "Generate neighbor cache for specific interfaceContainer",
               m_useInterfaceContainer);
  cmd.AddValue("noGenerate", "do not generate neighbor cache automatically",
               m_noGenerate);
  cmd.AddValue("sendTraffic", "send data stream from n0 to n1", m_sendTraffic);

  cmd.Parse(argc, argv);
}

int main(int argc, char *argv[]) {
  NeighborCacheExample example;
  example.CommandSetup(argc, argv);
  example.Run();
  return 0;
}

void NeighborCacheExample::Run() {
  if (m_enableLog) {
    LogComponentEnable("ArpL3Protocol", LOG_LEVEL_LOGIC);
    LogComponentEnable("Icmpv6L4Protocol", LOG_LEVEL_LOGIC);
  }
  uint32_t nCsmaLeft = 2;
  uint32_t nCsmaRight = 2;

  NodeContainer csmaNodesLeft;
  csmaNodesLeft.Create(nCsmaLeft);
  NodeContainer csmaNodesRight;
  csmaNodesRight.Add(csmaNodesLeft.Get(1));
  csmaNodesRight.Create(nCsmaRight);

  CsmaHelper csmaLeft;
  csmaLeft.SetChannelAttribute("DataRate", StringValue("20Gbps"));
  csmaLeft.SetChannelAttribute("Delay", TimeValue(MicroSeconds(1)));
  CsmaHelper csmaRight;
  csmaRight.SetChannelAttribute("DataRate", StringValue("20Gbps"));
  csmaRight.SetChannelAttribute("Delay", TimeValue(MicroSeconds(1)));

  NetDeviceContainer csmaDevicesLeft;
  csmaDevicesLeft = csmaLeft.Install(csmaNodesLeft);
  NetDeviceContainer csmaDevicesRight;
  csmaDevicesRight = csmaRight.Install(csmaNodesRight);

  InternetStackHelper stack;
  if (!m_useIpv6) {
    stack.SetIpv6StackInstall(false);
  } else {
    stack.SetIpv4StackInstall(false);
  }
  stack.SetIpv4ArpJitter(false);
  stack.SetIpv6NsRsJitter(false);
  stack.Install(csmaNodesLeft.Get(0));
  stack.Install(csmaNodesRight);

  if (!m_useIpv6) {
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer csmaInterfacesLeft;
    csmaInterfacesLeft = address.Assign(csmaDevicesLeft);
    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer csmaInterfacesRight;
    csmaInterfacesRight = address.Assign(csmaDevicesRight);

    NeighborCacheHelper neighborCache;
    if (m_useChannel) {
      Ptr<Channel> csmaChannel = csmaDevicesLeft.Get(0)->GetChannel();
      neighborCache.PopulateNeighborCache(csmaChannel);
    } else if (m_useNetDeviceContainer) {
      neighborCache.PopulateNeighborCache(csmaDevicesRight);
    } else if (m_useInterfaceContainer) {
      std::pair<Ptr<Ipv4>, uint32_t> txInterface = csmaInterfacesLeft.Get(0);
      Ptr<Ipv4> ipv41 = txInterface.first;
      uint32_t index1 = txInterface.second;
      std::pair<Ptr<Ipv4>, uint32_t> rxInterface =
          csmaInterfacesRight.Get(nCsmaRight);
      Ptr<Ipv4> ipv42 = rxInterface.first;
      uint32_t index2 = rxInterface.second;

      Ipv4InterfaceContainer interfaces;
      interfaces.Add(ipv41, index1);
      interfaces.Add(ipv42, index2);
      neighborCache.PopulateNeighborCache(interfaces);
    } else if (!m_noGenerate) {
      neighborCache.PopulateNeighborCache();
    }

    if (m_sendTraffic) {
      uint16_t port = 9;
      OnOffHelper onoff(
          "ns3::UdpSocketFactory",
          Address(InetSocketAddress(csmaInterfacesLeft.GetAddress(1), port)));
      onoff.SetConstantRate(DataRate("10Gbps"));
      onoff.SetAttribute("EnableSeqTsSizeHeader", BooleanValue(true));
      ApplicationContainer apps = onoff.Install(csmaNodesLeft.Get(0));
      apps.Start(Seconds(1.0));

      PacketSinkHelper sink(
          "ns3::UdpSocketFactory",
          Address(InetSocketAddress(Ipv4Address::GetAny(), port)));
      sink.SetAttribute("EnableSeqTsSizeHeader", BooleanValue(true));
      apps = sink.Install(csmaNodesLeft);
      apps.Get(1)->TraceConnectWithoutContext(
          "RxWithSeqTsSize",
          MakeCallback(&NeighborCacheExample::ReceivePacket, this));
      AsciiTraceHelper ascii;
      Ptr<OutputStreamWrapper> stream =
          ascii.CreateFileStream("neighbor-cache-example.tr");
      csmaLeft.EnableAsciiAll(stream);
      csmaLeft.EnablePcapAll("neighbor-cache-example");
    } else {
      Ptr<OutputStreamWrapper> outputStream =
          Create<OutputStreamWrapper>(&std::cout);
      Ipv4RoutingHelper::PrintNeighborCacheAllAt(Seconds(0), outputStream);
    }
  } else {
    Ipv6AddressHelper address;
    address.SetBase(Ipv6Address("2001:1::"), Ipv6Prefix(64));
    Ipv6InterfaceContainer csmaInterfacesLeft;
    csmaInterfacesLeft = address.Assign(csmaDevicesLeft);
    csmaInterfacesLeft.SetForwarding(1, true);
    csmaInterfacesLeft.SetDefaultRouteInAllNodes(1);

    address.SetBase(Ipv6Address("2001:2::"), Ipv6Prefix(64));
    Ipv6InterfaceContainer csmaInterfacesRight;
    csmaInterfacesRight = address.Assign(csmaDevicesRight);
    csmaInterfacesRight.SetForwarding(0, true);
    csmaInterfacesRight.SetDefaultRouteInAllNodes(0);

    NeighborCacheHelper neighborCache;
    if (m_useChannel) {
      Ptr<Channel> csmaChannel = csmaDevicesLeft.Get(0)->GetChannel();
      neighborCache.PopulateNeighborCache(csmaChannel);
    } else if (m_useNetDeviceContainer) {
      neighborCache.PopulateNeighborCache(csmaDevicesRight);
    } else if (m_useInterfaceContainer) {
      std::pair<Ptr<Ipv6>, uint32_t> txInterface = csmaInterfacesLeft.Get(0);
      Ptr<Ipv6> ipv61 = txInterface.first;
      uint32_t index1 = txInterface.second;
      std::pair<Ptr<Ipv6>, uint32_t> rxInterface =
          csmaInterfacesRight.Get(nCsmaRight);
      Ptr<Ipv6> ipv62 = rxInterface.first;
      uint32_t index2 = rxInterface.second;

      Ipv6InterfaceContainer interfaces;
      interfaces.Add(ipv61, index1);
      interfaces.Add(ipv62, index2);
      neighborCache.PopulateNeighborCache(interfaces);
    } else if (!m_noGenerate) {
      neighborCache.PopulateNeighborCache();
    }

    if (m_sendTraffic) {
      uint16_t port = 9;
      OnOffHelper onoff("ns3::UdpSocketFactory",
                        Address(Inet6SocketAddress(
                            csmaInterfacesLeft.GetAddress(1, 1), port)));
      onoff.SetConstantRate(DataRate("10Gbps"));
      onoff.SetAttribute("EnableSeqTsSizeHeader", BooleanValue(true));
      ApplicationContainer apps = onoff.Install(csmaNodesLeft.Get(0));
      apps.Start(Seconds(1.0));

      PacketSinkHelper sink(
          "ns3::UdpSocketFactory",
          Address(Inet6SocketAddress(Ipv6Address::GetAny(), port)));
      sink.SetAttribute("EnableSeqTsSizeHeader", BooleanValue(true));
      apps = sink.Install(csmaNodesLeft);
      apps.Get(1)->TraceConnectWithoutContext(
          "RxWithSeqTsSize",
          MakeCallback(&NeighborCacheExample::ReceivePacket, this));
      AsciiTraceHelper ascii;
      Ptr<OutputStreamWrapper> stream =
          ascii.CreateFileStream("neighbor-cache-example.tr");
      csmaLeft.EnableAsciiAll(stream);
      csmaLeft.EnablePcapAll("neighbor-cache-example");
    } else {
      Ptr<OutputStreamWrapper> outputStream =
          Create<OutputStreamWrapper>(&std::cout);
      Ipv6RoutingHelper::PrintNeighborCacheAllAt(Seconds(0), outputStream);
    }
  }
  Simulator::Stop(Seconds(1.00002));
  Simulator::Run();
  Simulator::Destroy();
}
