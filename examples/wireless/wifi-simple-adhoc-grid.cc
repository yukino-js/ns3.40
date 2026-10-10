

#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/double.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/log.h"
#include "ns3/mobility-helper.h"
#include "ns3/mobility-model.h"
#include "ns3/olsr-helper.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/yans-wifi-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("WifiSimpleAdhocGrid");

void ReceivePacket(Ptr<Socket> socket) {
  while (socket->Recv()) {
    NS_LOG_UNCOND("Received one packet!");
  }
}

static void GenerateTraffic(Ptr<Socket> socket, uint32_t pktSize,
                            uint32_t pktCount, Time pktInterval) {
  if (pktCount > 0) {
    socket->Send(Create<Packet>(pktSize));
    Simulator::Schedule(pktInterval, &GenerateTraffic, socket, pktSize,
                        pktCount - 1, pktInterval);
  } else {
    socket->Close();
  }
}

int main(int argc, char *argv[]) {
  std::string phyMode("DsssRate1Mbps");
  double distance = 100;
  uint32_t packetSize = 1000;
  uint32_t numPackets = 1;
  uint32_t numNodes = 25;
  uint32_t sinkNode = 0;
  uint32_t sourceNode = 24;
  double interval = 1.0;
  bool verbose = false;
  bool tracing = false;

  CommandLine cmd(__FILE__);
  cmd.AddValue("phyMode", "Wifi Phy mode", phyMode);
  cmd.AddValue("distance", "distance (m)", distance);
  cmd.AddValue("packetSize", "size of application packet sent", packetSize);
  cmd.AddValue("numPackets", "number of packets generated", numPackets);
  cmd.AddValue("interval", "interval (seconds) between packets", interval);
  cmd.AddValue("verbose", "turn on all WifiNetDevice log components", verbose);
  cmd.AddValue("tracing", "turn on ascii and pcap tracing", tracing);
  cmd.AddValue("numNodes", "number of nodes", numNodes);
  cmd.AddValue("sinkNode", "Receiver node number", sinkNode);
  cmd.AddValue("sourceNode", "Sender node number", sourceNode);
  cmd.Parse(argc, argv);
  Time interPacketInterval = Seconds(interval);

  Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode",
                     StringValue(phyMode));

  NodeContainer c;
  c.Create(numNodes);

  WifiHelper wifi;
  if (verbose) {
    WifiHelper::EnableLogComponents();
  }

  YansWifiPhyHelper wifiPhy;
  wifiPhy.Set("RxGain", DoubleValue(-10));
  wifiPhy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);

  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel");
  wifiPhy.SetChannel(wifiChannel.Create());

  WifiMacHelper wifiMac;
  wifi.SetStandard(WIFI_STANDARD_80211b);
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode",
                               StringValue(phyMode), "ControlMode",
                               StringValue(phyMode));
  wifiMac.SetType("ns3::AdhocWifiMac");
  NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, c);

  MobilityHelper mobility;
  mobility.SetPositionAllocator(
      "ns3::GridPositionAllocator", "MinX", DoubleValue(0.0), "MinY",
      DoubleValue(0.0), "DeltaX", DoubleValue(distance), "DeltaY",
      DoubleValue(distance), "GridWidth", UintegerValue(5), "LayoutType",
      StringValue("RowFirst"));
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(c);

  OlsrHelper olsr;
  Ipv4StaticRoutingHelper staticRouting;

  Ipv4ListRoutingHelper list;
  list.Add(staticRouting, 0);
  list.Add(olsr, 10);

  InternetStackHelper internet;
  internet.SetRoutingHelper(list);
  internet.Install(c);

  Ipv4AddressHelper ipv4;
  NS_LOG_INFO("Assign IP Addresses.");
  ipv4.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign(devices);

  TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
  Ptr<Socket> recvSink = Socket::CreateSocket(c.Get(sinkNode), tid);
  InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), 80);
  recvSink->Bind(local);
  recvSink->SetRecvCallback(MakeCallback(&ReceivePacket));

  Ptr<Socket> source = Socket::CreateSocket(c.Get(sourceNode), tid);
  InetSocketAddress remote = InetSocketAddress(i.GetAddress(sinkNode, 0), 80);
  source->Connect(remote);

  if (tracing) {
    AsciiTraceHelper ascii;
    wifiPhy.EnableAsciiAll(ascii.CreateFileStream("wifi-simple-adhoc-grid.tr"));
    wifiPhy.EnablePcap("wifi-simple-adhoc-grid", devices);
    Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper>(
        "wifi-simple-adhoc-grid.routes", std::ios::out);
    Ipv4RoutingHelper::PrintRoutingTableAllEvery(Seconds(2), routingStream);
    Ptr<OutputStreamWrapper> neighborStream = Create<OutputStreamWrapper>(
        "wifi-simple-adhoc-grid.neighbors", std::ios::out);
    Ipv4RoutingHelper::PrintNeighborCacheAllEvery(Seconds(2), neighborStream);
  }

  Simulator::Schedule(Seconds(30.0), &GenerateTraffic, source, packetSize,
                      numPackets, interPacketInterval);

  NS_LOG_UNCOND("Testing from node " << sourceNode << " to " << sinkNode
                                     << " with grid distance " << distance);

  Simulator::Stop(Seconds(33.0));
  Simulator::Run();
  Simulator::Destroy();

  return 0;
}
