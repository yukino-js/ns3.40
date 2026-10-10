
#include "ns3/boolean.h"
#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/double.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/log.h"
#include "ns3/mobility-helper.h"
#include "ns3/ssid.h"
#include "ns3/string.h"
#include "ns3/udp-client-server-helper.h"
#include "ns3/uinteger.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/yans-wifi-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SimplesHtHiddenStations");

int main(int argc, char *argv[]) {
  uint32_t payloadSize = 1472;
  double simulationTime = 10;
  uint32_t nMpdus = 1;
  uint32_t maxAmpduSize = 0;
  bool enableRts = false;
  double minExpectedThroughput = 0;
  double maxExpectedThroughput = 0;

  CommandLine cmd(__FILE__);
  cmd.AddValue("nMpdus", "Number of aggregated MPDUs", nMpdus);
  cmd.AddValue("payloadSize", "Payload size in bytes", payloadSize);
  cmd.AddValue("enableRts", "Enable RTS/CTS", enableRts);
  cmd.AddValue("simulationTime", "Simulation time in seconds", simulationTime);
  cmd.AddValue(
      "minExpectedThroughput",
      "if set, simulation fails if the lowest throughput is below this value",
      minExpectedThroughput);
  cmd.AddValue(
      "maxExpectedThroughput",
      "if set, simulation fails if the highest throughput is above this value",
      maxExpectedThroughput);
  cmd.Parse(argc, argv);

  if (!enableRts) {
    Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold",
                       StringValue("999999"));
  } else {
    Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold",
                       StringValue("0"));
  }

  maxAmpduSize = nMpdus * (payloadSize + 200);

  Config::SetDefault("ns3::RangePropagationLossModel::MaxRange",
                     DoubleValue(5));

  NodeContainer wifiStaNodes;
  wifiStaNodes.Create(2);
  NodeContainer wifiApNode;
  wifiApNode.Create(1);

  YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
  channel.AddPropagationLoss("ns3::RangePropagationLossModel");

  YansWifiPhyHelper phy;
  phy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);
  phy.SetChannel(channel.Create());
  phy.Set("ChannelSettings", StringValue("{36, 0, BAND_5GHZ, 0}"));

  WifiHelper wifi;
  wifi.SetStandard(WIFI_STANDARD_80211n);
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode",
                               StringValue("HtMcs7"), "ControlMode",
                               StringValue("HtMcs0"));
  WifiMacHelper mac;

  Ssid ssid = Ssid("simple-mpdu-aggregation");
  mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid));

  NetDeviceContainer staDevices;
  staDevices = wifi.Install(phy, mac, wifiStaNodes);

  mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid), "EnableBeaconJitter",
              BooleanValue(false));

  NetDeviceContainer apDevice;
  apDevice = wifi.Install(phy, mac, wifiApNode);

  Config::Set(
      "/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/BE_MaxAmpduSize",
      UintegerValue(maxAmpduSize));

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc =
      CreateObject<ListPositionAllocator>();

  positionAlloc->Add(Vector(5.0, 0.0, 0.0));
  positionAlloc->Add(Vector(0.0, 0.0, 0.0));
  positionAlloc->Add(Vector(10.0, 0.0, 0.0));
  mobility.SetPositionAllocator(positionAlloc);

  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

  mobility.Install(wifiApNode);
  mobility.Install(wifiStaNodes);

  InternetStackHelper stack;
  stack.Install(wifiApNode);
  stack.Install(wifiStaNodes);

  Ipv4AddressHelper address;
  address.SetBase("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer StaInterface;
  StaInterface = address.Assign(staDevices);
  Ipv4InterfaceContainer ApInterface;
  ApInterface = address.Assign(apDevice);

  uint16_t port = 9;
  UdpServerHelper server(port);
  ApplicationContainer serverApp = server.Install(wifiApNode);
  serverApp.Start(Seconds(0.0));
  serverApp.Stop(Seconds(simulationTime + 1));

  UdpClientHelper client(ApInterface.GetAddress(0), port);
  client.SetAttribute("MaxPackets", UintegerValue(4294967295U));
  client.SetAttribute("Interval", TimeValue(Time("0.0001")));
  client.SetAttribute("PacketSize", UintegerValue(payloadSize));

  ApplicationContainer clientApp1 = client.Install(wifiStaNodes);
  clientApp1.Start(Seconds(1.0));
  clientApp1.Stop(Seconds(simulationTime + 1));

  phy.EnablePcap("SimpleHtHiddenStations_Ap", apDevice.Get(0));
  phy.EnablePcap("SimpleHtHiddenStations_Sta1", staDevices.Get(0));
  phy.EnablePcap("SimpleHtHiddenStations_Sta2", staDevices.Get(1));

  AsciiTraceHelper ascii;
  phy.EnableAsciiAll(ascii.CreateFileStream("SimpleHtHiddenStations.tr"));

  Simulator::Stop(Seconds(simulationTime + 1));

  Simulator::Run();

  uint64_t totalPacketsThrough =
      DynamicCast<UdpServer>(serverApp.Get(0))->GetReceived();

  Simulator::Destroy();

  double throughput =
      totalPacketsThrough * payloadSize * 8 / (simulationTime * 1000000.0);
  std::cout << "Throughput: " << throughput << " Mbit/s" << '\n';
  if (throughput < minExpectedThroughput ||
      (maxExpectedThroughput > 0 && throughput > maxExpectedThroughput)) {
    NS_LOG_ERROR("Obtained throughput "
                 << throughput << " is not in the expected boundaries!");
    exit(1);
  }
  return 0;
}
