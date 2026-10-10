

#include "ns3/animation-interface.h"
#include "ns3/command-line.h"
#include "ns3/csma-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/olsr-helper.h"
#include "ns3/on-off-helper.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/qos-txop.h"
#include "ns3/ssid.h"
#include "ns3/string.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/yans-wifi-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("MixedWireless");

static void CourseChangeCallback(std::string path,
                                 Ptr<const MobilityModel> model) {
  Vector position = model->GetPosition();
  std::cout << "CourseChange " << path << " x=" << position.x
            << ", y=" << position.y << ", z=" << position.z << std::endl;
}

int main(int argc, char *argv[]) {
  uint32_t backboneNodes = 10;
  uint32_t infraNodes = 2;
  uint32_t lanNodes = 2;
  uint32_t stopTime = 20;
  bool useCourseChangeCallback = false;

  Config::SetDefault("ns3::OnOffApplication::PacketSize", StringValue("1472"));
  Config::SetDefault("ns3::OnOffApplication::DataRate", StringValue("100kb/s"));

  CommandLine cmd(__FILE__);
  cmd.AddValue("backboneNodes", "number of backbone nodes", backboneNodes);
  cmd.AddValue("infraNodes", "number of leaf nodes", infraNodes);
  cmd.AddValue("lanNodes", "number of LAN nodes", lanNodes);
  cmd.AddValue("stopTime", "simulation stop time (seconds)", stopTime);
  cmd.AddValue("useCourseChangeCallback",
               "whether to enable course change tracing",
               useCourseChangeCallback);

  cmd.Parse(argc, argv);

  if (stopTime < 10) {
    std::cout << "Use a simulation stop time >= 10 seconds" << std::endl;
    exit(1);
  }

  NodeContainer backbone;
  backbone.Create(backboneNodes);
  WifiHelper wifi;
  WifiMacHelper mac;
  mac.SetType("ns3::AdhocWifiMac");
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode",
                               StringValue("OfdmRate54Mbps"));
  YansWifiPhyHelper wifiPhy;
  wifiPhy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
  wifiPhy.SetChannel(wifiChannel.Create());
  NetDeviceContainer backboneDevices = wifi.Install(wifiPhy, mac, backbone);

  NS_LOG_INFO("Enabling OLSR routing on all backbone nodes");
  OlsrHelper olsr;
  InternetStackHelper internet;
  internet.SetRoutingHelper(olsr);
  internet.Install(backbone);

  Ipv4AddressHelper ipAddrs;
  ipAddrs.SetBase("192.168.0.0", "255.255.255.0");
  ipAddrs.Assign(backboneDevices);

  MobilityHelper mobility;
  mobility.SetPositionAllocator(
      "ns3::GridPositionAllocator", "MinX", DoubleValue(20.0), "MinY",
      DoubleValue(20.0), "DeltaX", DoubleValue(20.0), "DeltaY",
      DoubleValue(20.0), "GridWidth", UintegerValue(5), "LayoutType",
      StringValue("RowFirst"));
  mobility.SetMobilityModel(
      "ns3::RandomDirection2dMobilityModel", "Bounds",
      RectangleValue(Rectangle(-500, 500, -500, 500)), "Speed",
      StringValue("ns3::ConstantRandomVariable[Constant=2]"), "Pause",
      StringValue("ns3::ConstantRandomVariable[Constant=0.2]"));
  mobility.Install(backbone);

  ipAddrs.SetBase("172.16.0.0", "255.255.255.0");

  for (uint32_t i = 0; i < backboneNodes; ++i) {
    NS_LOG_INFO("Configuring local area network for backbone node " << i);
    NodeContainer newLanNodes;
    newLanNodes.Create(lanNodes - 1);
    NodeContainer lan(backbone.Get(i), newLanNodes);
    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", DataRateValue(DataRate(5000000)));
    csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));
    NetDeviceContainer lanDevices = csma.Install(lan);
    internet.Install(newLanNodes);
    ipAddrs.Assign(lanDevices);
    ipAddrs.NewNetwork();
    MobilityHelper mobilityLan;
    Ptr<ListPositionAllocator> subnetAlloc =
        CreateObject<ListPositionAllocator>();
    for (uint32_t j = 0; j < newLanNodes.GetN(); ++j) {
      subnetAlloc->Add(Vector(0.0, j * 10 + 10, 0.0));
    }
    mobilityLan.PushReferenceMobilityModel(backbone.Get(i));
    mobilityLan.SetPositionAllocator(subnetAlloc);
    mobilityLan.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobilityLan.Install(newLanNodes);
  }

  ipAddrs.SetBase("10.0.0.0", "255.255.255.0");

  for (uint32_t i = 0; i < backboneNodes; ++i) {
    NS_LOG_INFO("Configuring wireless network for backbone node " << i);
    NodeContainer stas;
    stas.Create(infraNodes - 1);
    NodeContainer infra(backbone.Get(i), stas);
    WifiHelper wifiInfra;
    WifiMacHelper macInfra;
    wifiPhy.SetChannel(wifiChannel.Create());
    std::string ssidString("wifi-infra");
    std::stringstream ss;
    ss << i;
    ssidString += ss.str();
    Ssid ssid = Ssid(ssidString);
    macInfra.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid));
    NetDeviceContainer staDevices = wifiInfra.Install(wifiPhy, macInfra, stas);
    macInfra.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
    NetDeviceContainer apDevices =
        wifiInfra.Install(wifiPhy, macInfra, backbone.Get(i));
    NetDeviceContainer infraDevices(apDevices, staDevices);

    internet.Install(stas);
    ipAddrs.Assign(infraDevices);
    ipAddrs.NewNetwork();
    Ptr<ListPositionAllocator> subnetAlloc =
        CreateObject<ListPositionAllocator>();
    for (uint32_t j = 0; j < infra.GetN(); ++j) {
      subnetAlloc->Add(Vector(0.0, j, 0.0));
    }
    mobility.PushReferenceMobilityModel(backbone.Get(i));
    mobility.SetPositionAllocator(subnetAlloc);
    mobility.SetMobilityModel(
        "ns3::RandomDirection2dMobilityModel", "Bounds",
        RectangleValue(Rectangle(-10, 10, -10, 10)), "Speed",
        StringValue("ns3::ConstantRandomVariable[Constant=3]"), "Pause",
        StringValue("ns3::ConstantRandomVariable[Constant=0.4]"));
    mobility.Install(stas);
  }

  NS_LOG_INFO("Create Applications.");
  uint16_t port = 9;

  NS_ASSERT(lanNodes > 1 && infraNodes > 1);
  Ptr<Node> appSource = NodeList::GetNode(backboneNodes);
  uint32_t lastNodeIndex = backboneNodes + backboneNodes * (lanNodes - 1) +
                           backboneNodes * (infraNodes - 1) - 1;
  Ptr<Node> appSink = NodeList::GetNode(lastNodeIndex);
  Ipv4Address remoteAddr =
      appSink->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();

  OnOffHelper onoff("ns3::UdpSocketFactory",
                    Address(InetSocketAddress(remoteAddr, port)));

  ApplicationContainer apps = onoff.Install(appSource);
  apps.Start(Seconds(3));
  apps.Stop(Seconds(stopTime - 1));

  PacketSinkHelper sink("ns3::UdpSocketFactory",
                        InetSocketAddress(Ipv4Address::GetAny(), port));
  apps = sink.Install(appSink);
  apps.Start(Seconds(3));

  NS_LOG_INFO("Configure Tracing.");
  CsmaHelper csma;

  AsciiTraceHelper ascii;
  Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream("mixed-wireless.tr");
  wifiPhy.EnableAsciiAll(stream);
  csma.EnableAsciiAll(stream);
  internet.EnableAsciiIpv4All(stream);

  csma.EnablePcapAll("mixed-wireless", false);
  wifiPhy.EnablePcap("mixed-wireless", backboneDevices, false);
  wifiPhy.EnablePcap("mixed-wireless", appSink->GetId(), 0);

  if (useCourseChangeCallback) {
    Config::Connect("/NodeList/*/$ns3::MobilityModel/CourseChange",
                    MakeCallback(&CourseChangeCallback));
  }

  AnimationInterface anim("mixed-wireless.xml");

  NS_LOG_INFO("Run Simulation.");
  Simulator::Stop(Seconds(stopTime));
  Simulator::Run();
  Simulator::Destroy();

  return 0;
}
