

#include "ns3/boolean.h"
#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/double.h"
#include "ns3/gnuplot.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/log.h"
#include "ns3/mobility-helper.h"
#include "ns3/mobility-model.h"
#include "ns3/on-off-helper.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/ssid.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/yans-wifi-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("RateAdaptationDistance");

class NodeStatistics {
public:
  NodeStatistics(NetDeviceContainer aps, NetDeviceContainer stas);

  void RxCallback(std::string path, Ptr<const Packet> packet,
                  const Address &from);
  void SetPosition(Ptr<Node> node, Vector position);
  void AdvancePosition(Ptr<Node> node, int stepsSize, int stepsTime);
  Vector GetPosition(Ptr<Node> node);
  Gnuplot2dDataset GetDatafile();

private:
  uint32_t m_bytesTotal;
  Gnuplot2dDataset m_output;
};

NodeStatistics::NodeStatistics(NetDeviceContainer aps,
                               NetDeviceContainer stas) {
  m_bytesTotal = 0;
}

void NodeStatistics::RxCallback(std::string path, Ptr<const Packet> packet,
                                const Address &from) {
  m_bytesTotal += packet->GetSize();
}

void NodeStatistics::SetPosition(Ptr<Node> node, Vector position) {
  Ptr<MobilityModel> mobility = node->GetObject<MobilityModel>();
  mobility->SetPosition(position);
}

Vector NodeStatistics::GetPosition(Ptr<Node> node) {
  Ptr<MobilityModel> mobility = node->GetObject<MobilityModel>();
  return mobility->GetPosition();
}

void NodeStatistics::AdvancePosition(Ptr<Node> node, int stepsSize,
                                     int stepsTime) {
  Vector pos = GetPosition(node);
  double mbs = ((m_bytesTotal * 8.0) / (1000000 * stepsTime));
  m_bytesTotal = 0;
  m_output.Add(pos.x, mbs);
  pos.x += stepsSize;
  SetPosition(node, pos);
  Simulator::Schedule(Seconds(stepsTime), &NodeStatistics::AdvancePosition,
                      this, node, stepsSize, stepsTime);
}

Gnuplot2dDataset NodeStatistics::GetDatafile() { return m_output; }

void RateCallback(uint64_t oldRate, uint64_t newRate) {
  NS_LOG_INFO("Rate " << newRate / 1000000.0 << " Mbps");
}

int main(int argc, char *argv[]) {
  uint32_t rtsThreshold = 65535;
  std::string staManager = "ns3::MinstrelHtWifiManager";
  std::string apManager = "ns3::MinstrelHtWifiManager";
  std::string standard = "802.11n-5GHz";
  std::string outputFileName = "minstrelHT";
  uint32_t BeMaxAmpduSize = 65535;
  bool shortGuardInterval = false;
  uint32_t chWidth = 20;
  int ap1_x = 0;
  int ap1_y = 0;
  int sta1_x = 5;
  int sta1_y = 0;
  int steps = 100;
  int stepsSize = 1;
  int stepsTime = 1;

  CommandLine cmd(__FILE__);
  cmd.AddValue("staManager", "Rate adaptation manager of the STA", staManager);
  cmd.AddValue("apManager", "Rate adaptation manager of the AP", apManager);
  cmd.AddValue("standard", "Wifi standard (a/b/g/n/ac only)", standard);
  cmd.AddValue("shortGuardInterval",
               "Enable Short Guard Interval in all stations",
               shortGuardInterval);
  cmd.AddValue("channelWidth", "Channel width of all the stations", chWidth);
  cmd.AddValue("rtsThreshold", "RTS threshold", rtsThreshold);
  cmd.AddValue("BeMaxAmpduSize", "BE Mac A-MPDU size", BeMaxAmpduSize);
  cmd.AddValue("outputFileName", "Output filename", outputFileName);
  cmd.AddValue("steps", "How many different distances to try", steps);
  cmd.AddValue("stepsTime", "Time on each step", stepsTime);
  cmd.AddValue("stepsSize", "Distance between steps", stepsSize);
  cmd.AddValue("AP1_x", "Position of AP1 in x coordinate", ap1_x);
  cmd.AddValue("AP1_y", "Position of AP1 in y coordinate", ap1_y);
  cmd.AddValue("STA1_x", "Position of STA1 in x coordinate", sta1_x);
  cmd.AddValue("STA1_y", "Position of STA1 in y coordinate", sta1_y);
  cmd.Parse(argc, argv);

  int simuTime = steps * stepsTime;

  if (standard != "802.11a" && standard != "802.11b" && standard != "802.11g" &&
      standard == "802.11n-2.4GHz" && standard != "802.11n-5GHz" &&
      standard != "802.11ac") {
    NS_FATAL_ERROR("Standard " << standard
                               << " is not supported by this program");
  }

  NodeContainer wifiApNodes;
  wifiApNodes.Create(1);

  NodeContainer wifiStaNodes;
  wifiStaNodes.Create(1);

  YansWifiPhyHelper wifiPhy;
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
  wifiPhy.SetChannel(wifiChannel.Create());
  std::string frequencyBand;
  if (standard == "802.11b" || standard == "802.11g" ||
      standard == "802.11n-2.4GHz") {
    frequencyBand = "BAND_2_4GHZ";
  } else {
    frequencyBand = "BAND_5GHZ";
  }
  wifiPhy.Set("ChannelSettings", StringValue("{0, " + std::to_string(chWidth) +
                                             ", " + frequencyBand + ", 0}"));

  wifiPhy.Set("CcaSensitivity", DoubleValue(-110));
  wifiPhy.Set("RxNoiseFigure", DoubleValue(0));
  wifiPhy.DisablePreambleDetectionModel();

  NetDeviceContainer wifiApDevices;
  NetDeviceContainer wifiStaDevices;
  NetDeviceContainer wifiDevices;

  WifiHelper wifi;
  if (standard == "802.11a" || standard == "802.11b" || standard == "802.11g") {
    if (standard == "802.11a") {
      wifi.SetStandard(WIFI_STANDARD_80211a);
    } else if (standard == "802.11b") {
      wifi.SetStandard(WIFI_STANDARD_80211b);
    } else if (standard == "802.11g") {
      wifi.SetStandard(WIFI_STANDARD_80211g);
    }
    WifiMacHelper wifiMac;

    wifi.SetRemoteStationManager(staManager, "RtsCtsThreshold",
                                 UintegerValue(rtsThreshold));

    Ssid ssid = Ssid("AP");
    wifiMac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid));
    wifiStaDevices.Add(wifi.Install(wifiPhy, wifiMac, wifiStaNodes.Get(0)));

    wifi.SetRemoteStationManager(apManager, "RtsCtsThreshold",
                                 UintegerValue(rtsThreshold));

    ssid = Ssid("AP");
    wifiMac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
    wifiApDevices.Add(wifi.Install(wifiPhy, wifiMac, wifiApNodes.Get(0)));
  } else if (standard == "802.11n-2.4GHz" || standard == "802.11n-5GHz" ||
             standard == "802.11ac") {
    if (standard == "802.11n-2.4GHz" || standard == "802.11n-5GHz") {
      wifi.SetStandard(WIFI_STANDARD_80211n);
    } else if (standard == "802.11ac") {
      wifi.SetStandard(WIFI_STANDARD_80211ac);
    }

    WifiMacHelper wifiMac;

    wifi.SetRemoteStationManager(staManager, "RtsCtsThreshold",
                                 UintegerValue(rtsThreshold));

    Ssid ssid = Ssid("AP");
    wifiMac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid));
    wifiStaDevices.Add(wifi.Install(wifiPhy, wifiMac, wifiStaNodes.Get(0)));

    wifi.SetRemoteStationManager(apManager, "RtsCtsThreshold",
                                 UintegerValue(rtsThreshold));

    ssid = Ssid("AP");
    wifiMac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
    wifiApDevices.Add(wifi.Install(wifiPhy, wifiMac, wifiApNodes.Get(0)));

    Config::Set(
        "/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/BE_MaxAmpduSize",
        UintegerValue(BeMaxAmpduSize));
  }

  wifiDevices.Add(wifiStaDevices);
  wifiDevices.Add(wifiApDevices);

  Config::Set("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/HtConfiguration/"
              "ShortGuardIntervalSupported",
              BooleanValue(shortGuardInterval));

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc =
      CreateObject<ListPositionAllocator>();
  positionAlloc->Add(Vector(ap1_x, ap1_y, 0.0));
  positionAlloc->Add(Vector(sta1_x, sta1_y, 0.0));
  mobility.SetPositionAllocator(positionAlloc);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(wifiApNodes.Get(0));
  mobility.Install(wifiStaNodes.Get(0));

  NodeStatistics atpCounter = NodeStatistics(wifiApDevices, wifiStaDevices);

  Simulator::Schedule(Seconds(0.5 + stepsTime),
                      &NodeStatistics::AdvancePosition, &atpCounter,
                      wifiStaNodes.Get(0), stepsSize, stepsTime);

  InternetStackHelper stack;
  stack.Install(wifiApNodes);
  stack.Install(wifiStaNodes);
  Ipv4AddressHelper address;
  address.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = address.Assign(wifiDevices);
  Ipv4Address sinkAddress = i.GetAddress(0);
  uint16_t port = 9;

  PacketSinkHelper sink("ns3::UdpSocketFactory",
                        InetSocketAddress(sinkAddress, port));
  ApplicationContainer apps_sink = sink.Install(wifiStaNodes.Get(0));

  OnOffHelper onoff("ns3::UdpSocketFactory",
                    InetSocketAddress(sinkAddress, port));
  onoff.SetConstantRate(DataRate("400Mb/s"), 1420);
  onoff.SetAttribute("StartTime", TimeValue(Seconds(0.5)));
  onoff.SetAttribute("StopTime", TimeValue(Seconds(simuTime)));
  ApplicationContainer apps_source = onoff.Install(wifiApNodes.Get(0));

  apps_sink.Start(Seconds(0.5));
  apps_sink.Stop(Seconds(simuTime));

  Config::Connect("/NodeList/1/ApplicationList/*/$ns3::PacketSink/Rx",
                  MakeCallback(&NodeStatistics::RxCallback, &atpCounter));

  Config::ConnectWithoutContextFailSafe(
      "/NodeList/0/DeviceList/*/$ns3::WifiNetDevice/RemoteStationManager/$" +
          apManager + "/Rate",
      MakeCallback(RateCallback));

  Simulator::Stop(Seconds(simuTime));
  Simulator::Run();

  std::ofstream outfile("throughput-" + outputFileName + ".plt");
  Gnuplot gnuplot =
      Gnuplot("throughput-" + outputFileName + ".eps", "Throughput");
  gnuplot.SetTerminal("post eps color enhanced");
  gnuplot.SetLegend("Distance (meters)", "Throughput (Mb/s)");
  gnuplot.SetTitle("Throughput (AP to STA) vs distance");
  gnuplot.AddDataset(atpCounter.GetDatafile());
  gnuplot.GenerateOutput(outfile);

  Simulator::Destroy();

  return 0;
}
