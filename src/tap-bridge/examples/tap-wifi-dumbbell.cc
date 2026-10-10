

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/tap-bridge-module.h"
#include "ns3/wifi-module.h"

#include <fstream>
#include <iostream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TapDumbbellExample");

int main(int argc, char *argv[]) {
  std::string mode = "ConfigureLocal";
  std::string tapName = "thetap";

  CommandLine cmd(__FILE__);
  cmd.AddValue("mode", "Mode setting of TapBridge", mode);
  cmd.AddValue("tapName", "Name of the OS tap device", tapName);
  cmd.Parse(argc, argv);

  GlobalValue::Bind("SimulatorImplementationType",
                    StringValue("ns3::RealtimeSimulatorImpl"));
  GlobalValue::Bind("ChecksumEnabled", BooleanValue(true));

  NodeContainer nodesLeft;
  nodesLeft.Create(4);

  YansWifiPhyHelper wifiPhy;
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
  wifiPhy.SetChannel(wifiChannel.Create());

  Ssid ssid = Ssid("left");
  WifiHelper wifi;
  WifiMacHelper wifiMac;
  wifi.SetRemoteStationManager("ns3::ArfWifiManager");

  wifiMac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
  NetDeviceContainer devicesLeft =
      wifi.Install(wifiPhy, wifiMac, nodesLeft.Get(0));

  wifiMac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid), "ActiveProbing",
                  BooleanValue(false));
  devicesLeft.Add(wifi.Install(
      wifiPhy, wifiMac,
      NodeContainer(nodesLeft.Get(1), nodesLeft.Get(2), nodesLeft.Get(3))));

  MobilityHelper mobility;
  mobility.Install(nodesLeft);

  InternetStackHelper internetLeft;
  internetLeft.Install(nodesLeft);

  Ipv4AddressHelper ipv4Left;
  ipv4Left.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfacesLeft = ipv4Left.Assign(devicesLeft);

  TapBridgeHelper tapBridge(interfacesLeft.GetAddress(1));
  tapBridge.SetAttribute("Mode", StringValue(mode));
  tapBridge.SetAttribute("DeviceName", StringValue(tapName));
  tapBridge.Install(nodesLeft.Get(0), devicesLeft.Get(0));

  NodeContainer nodesRight;
  nodesRight.Create(4);

  CsmaHelper csmaRight;
  csmaRight.SetChannelAttribute("DataRate", DataRateValue(5000000));
  csmaRight.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));

  NetDeviceContainer devicesRight = csmaRight.Install(nodesRight);

  InternetStackHelper internetRight;
  internetRight.Install(nodesRight);

  Ipv4AddressHelper ipv4Right;
  ipv4Right.SetBase("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer interfacesRight = ipv4Right.Assign(devicesRight);

  PointToPointHelper p2p;
  p2p.SetDeviceAttribute("DataRate", StringValue("512kbps"));
  p2p.SetChannelAttribute("Delay", StringValue("10ms"));

  NodeContainer nodes = NodeContainer(nodesLeft.Get(3), nodesRight.Get(0));
  NetDeviceContainer devices = p2p.Install(nodes);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.2.0", "255.255.255.192");
  Ipv4InterfaceContainer interfaces = ipv4.Assign(devices);

  uint16_t port = 9;
  OnOffHelper onoff("ns3::UdpSocketFactory",
                    InetSocketAddress(interfaces.GetAddress(1), port));
  onoff.SetConstantRate(DataRate("500kb/s"));

  ApplicationContainer apps = onoff.Install(nodesLeft.Get(3));
  apps.Start(Seconds(1.0));

  PacketSinkHelper sink("ns3::UdpSocketFactory",
                        InetSocketAddress(Ipv4Address::GetAny(), port));

  apps = sink.Install(nodesRight.Get(0));
  apps.Start(Seconds(1.0));

  wifiPhy.EnablePcapAll("tap-wifi-dumbbell");

  csmaRight.EnablePcapAll("tap-wifi-dumbbell", false);
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  Simulator::Stop(Seconds(60.));
  Simulator::Run();
  Simulator::Destroy();

  return 0;
}
