
#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/log.h"
#include "ns3/mobility-helper.h"
#include "ns3/mobility-model.h"
#include "ns3/on-off-helper.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/packet-sink.h"
#include "ns3/ssid.h"
#include "ns3/string.h"
#include "ns3/tcp-westwood-plus.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/yans-wifi-helper.h"

NS_LOG_COMPONENT_DEFINE("wifi-tcp");

using namespace ns3;

Ptr<PacketSink> sink;
uint64_t lastTotalRx = 0;

void CalculateThroughput() {
  Time now = Simulator::Now();
  double cur = (sink->GetTotalRx() - lastTotalRx) * 8.0 / 1e5;
  std::cout << now.GetSeconds() << "s: \t" << cur << " Mbit/s" << std::endl;
  lastTotalRx = sink->GetTotalRx();
  Simulator::Schedule(MilliSeconds(100), &CalculateThroughput);
}

int main(int argc, char *argv[]) {
  uint32_t payloadSize = 1472;
  std::string dataRate = "100Mbps";
  std::string tcpVariant = "TcpNewReno";
  std::string phyRate = "HtMcs7";
  double simulationTime = 10;
  bool pcapTracing = false;

  CommandLine cmd(__FILE__);
  cmd.AddValue("payloadSize", "Payload size in bytes", payloadSize);
  cmd.AddValue("dataRate", "Application data ate", dataRate);
  cmd.AddValue(
      "tcpVariant",
      "Transport protocol to use: TcpNewReno, "
      "TcpHybla, TcpHighSpeed, TcpHtcp, TcpVegas, TcpScalable, TcpVeno, "
      "TcpBic, TcpYeah, TcpIllinois, TcpWestwood, TcpWestwoodPlus, TcpLedbat ",
      tcpVariant);
  cmd.AddValue("phyRate", "Physical layer bitrate", phyRate);
  cmd.AddValue("simulationTime", "Simulation time in seconds", simulationTime);
  cmd.AddValue("pcap", "Enable/disable PCAP Tracing", pcapTracing);
  cmd.Parse(argc, argv);

  tcpVariant = std::string("ns3::") + tcpVariant;
  TypeId tcpTid;
  NS_ABORT_MSG_UNLESS(TypeId::LookupByNameFailSafe(tcpVariant, &tcpTid),
                      "TypeId " << tcpVariant << " not found");
  Config::SetDefault("ns3::TcpL4Protocol::SocketType",
                     TypeIdValue(TypeId::LookupByName(tcpVariant)));

  Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(payloadSize));

  WifiMacHelper wifiMac;
  WifiHelper wifiHelper;
  wifiHelper.SetStandard(WIFI_STANDARD_80211n);

  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel", "Frequency",
                                 DoubleValue(5e9));

  YansWifiPhyHelper wifiPhy;
  wifiPhy.SetChannel(wifiChannel.Create());
  wifiPhy.SetErrorRateModel("ns3::YansErrorRateModel");
  wifiHelper.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode",
                                     StringValue(phyRate), "ControlMode",
                                     StringValue("HtMcs0"));

  NodeContainer networkNodes;
  networkNodes.Create(2);
  Ptr<Node> apWifiNode = networkNodes.Get(0);
  Ptr<Node> staWifiNode = networkNodes.Get(1);

  Ssid ssid = Ssid("network");
  wifiMac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));

  NetDeviceContainer apDevice;
  apDevice = wifiHelper.Install(wifiPhy, wifiMac, apWifiNode);

  wifiMac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid));

  NetDeviceContainer staDevices;
  staDevices = wifiHelper.Install(wifiPhy, wifiMac, staWifiNode);

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc =
      CreateObject<ListPositionAllocator>();
  positionAlloc->Add(Vector(0.0, 0.0, 0.0));
  positionAlloc->Add(Vector(1.0, 1.0, 0.0));

  mobility.SetPositionAllocator(positionAlloc);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(apWifiNode);
  mobility.Install(staWifiNode);

  InternetStackHelper stack;
  stack.Install(networkNodes);

  Ipv4AddressHelper address;
  address.SetBase("10.0.0.0", "255.255.255.0");
  Ipv4InterfaceContainer apInterface;
  apInterface = address.Assign(apDevice);
  Ipv4InterfaceContainer staInterface;
  staInterface = address.Assign(staDevices);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  PacketSinkHelper sinkHelper("ns3::TcpSocketFactory",
                              InetSocketAddress(Ipv4Address::GetAny(), 9));
  ApplicationContainer sinkApp = sinkHelper.Install(apWifiNode);
  sink = StaticCast<PacketSink>(sinkApp.Get(0));

  OnOffHelper server("ns3::TcpSocketFactory",
                     (InetSocketAddress(apInterface.GetAddress(0), 9)));
  server.SetAttribute("PacketSize", UintegerValue(payloadSize));
  server.SetAttribute("OnTime",
                      StringValue("ns3::ConstantRandomVariable[Constant=1]"));
  server.SetAttribute("OffTime",
                      StringValue("ns3::ConstantRandomVariable[Constant=0]"));
  server.SetAttribute("DataRate", DataRateValue(DataRate(dataRate)));
  ApplicationContainer serverApp = server.Install(staWifiNode);

  sinkApp.Start(Seconds(0.0));
  serverApp.Start(Seconds(1.0));
  Simulator::Schedule(Seconds(1.1), &CalculateThroughput);

  if (pcapTracing) {
    wifiPhy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);
    wifiPhy.EnablePcap("AccessPoint", apDevice);
    wifiPhy.EnablePcap("Station", staDevices);
  }

  Simulator::Stop(Seconds(simulationTime + 1));
  Simulator::Run();

  double averageThroughput =
      ((sink->GetTotalRx() * 8) / (1e6 * simulationTime));

  Simulator::Destroy();

  if (averageThroughput < 50) {
    NS_LOG_ERROR("Obtained throughput is not in the expected boundaries!");
    exit(1);
  }
  std::cout << "\nAverage throughput: " << averageThroughput << " Mbit/s"
            << std::endl;
  return 0;
}
