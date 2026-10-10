
#include "ns3/boolean.h"
#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-flow-classifier.h"
#include "ns3/on-off-helper.h"
#include "ns3/propagation-delay-model.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/string.h"
#include "ns3/udp-echo-helper.h"
#include "ns3/uinteger.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/yans-wifi-helper.h"

using namespace ns3;

void experiment(bool enableCtsRts, std::string wifiManager) {
  UintegerValue ctsThr =
      (enableCtsRts ? UintegerValue(100) : UintegerValue(2200));
  Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", ctsThr);

  NodeContainer nodes;
  nodes.Create(3);

  for (uint8_t i = 0; i < 3; ++i) {
    nodes.Get(i)->AggregateObject(
        CreateObject<ConstantPositionMobilityModel>());
  }

  Ptr<MatrixPropagationLossModel> lossModel =
      CreateObject<MatrixPropagationLossModel>();
  lossModel->SetDefaultLoss(200);
  lossModel->SetLoss(nodes.Get(0)->GetObject<MobilityModel>(),
                     nodes.Get(1)->GetObject<MobilityModel>(), 50);
  lossModel->SetLoss(nodes.Get(2)->GetObject<MobilityModel>(),
                     nodes.Get(1)->GetObject<MobilityModel>(), 50);

  Ptr<YansWifiChannel> wifiChannel = CreateObject<YansWifiChannel>();
  wifiChannel->SetPropagationLossModel(lossModel);
  wifiChannel->SetPropagationDelayModel(
      CreateObject<ConstantSpeedPropagationDelayModel>());

  WifiHelper wifi;
  wifi.SetStandard(WIFI_STANDARD_80211b);
  wifi.SetRemoteStationManager("ns3::" + wifiManager + "WifiManager");
  YansWifiPhyHelper wifiPhy;
  wifiPhy.SetChannel(wifiChannel);
  WifiMacHelper wifiMac;
  wifiMac.SetType("ns3::AdhocWifiMac");
  NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, nodes);

  InternetStackHelper internet;
  internet.Install(nodes);
  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.0.0.0", "255.0.0.0");
  ipv4.Assign(devices);

  ApplicationContainer cbrApps;
  uint16_t cbrPort = 12345;
  OnOffHelper onOffHelper("ns3::UdpSocketFactory",
                          InetSocketAddress(Ipv4Address("10.0.0.2"), cbrPort));
  onOffHelper.SetAttribute("PacketSize", UintegerValue(1400));
  onOffHelper.SetAttribute(
      "OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
  onOffHelper.SetAttribute(
      "OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));

  onOffHelper.SetAttribute("DataRate", StringValue("3000000bps"));
  onOffHelper.SetAttribute("StartTime", TimeValue(Seconds(1.000000)));
  cbrApps.Add(onOffHelper.Install(nodes.Get(0)));

  onOffHelper.SetAttribute("DataRate", StringValue("3001100bps"));
  onOffHelper.SetAttribute("StartTime", TimeValue(Seconds(1.001)));
  cbrApps.Add(onOffHelper.Install(nodes.Get(2)));

  uint16_t echoPort = 9;
  UdpEchoClientHelper echoClientHelper(Ipv4Address("10.0.0.2"), echoPort);
  echoClientHelper.SetAttribute("MaxPackets", UintegerValue(1));
  echoClientHelper.SetAttribute("Interval", TimeValue(Seconds(0.1)));
  echoClientHelper.SetAttribute("PacketSize", UintegerValue(10));
  ApplicationContainer pingApps;

  echoClientHelper.SetAttribute("StartTime", TimeValue(Seconds(0.001)));
  pingApps.Add(echoClientHelper.Install(nodes.Get(0)));
  echoClientHelper.SetAttribute("StartTime", TimeValue(Seconds(0.006)));
  pingApps.Add(echoClientHelper.Install(nodes.Get(2)));

  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll();

  Simulator::Stop(Seconds(10));
  Simulator::Run();

  monitor->CheckForLostPackets();
  Ptr<Ipv4FlowClassifier> classifier =
      DynamicCast<Ipv4FlowClassifier>(flowmon.GetClassifier());
  FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats();
  for (auto i = stats.begin(); i != stats.end(); ++i) {
    if (i->first > 2) {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(i->first);
      std::cout << "Flow " << i->first - 2 << " (" << t.sourceAddress << " -> "
                << t.destinationAddress << ")\n";
      std::cout << "  Tx Packets: " << i->second.txPackets << "\n";
      std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
      std::cout << "  TxOffered:  "
                << i->second.txBytes * 8.0 / 9.0 / 1000 / 1000 << " Mbps\n";
      std::cout << "  Rx Packets: " << i->second.rxPackets << "\n";
      std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
      std::cout << "  Throughput: "
                << i->second.rxBytes * 8.0 / 9.0 / 1000 / 1000 << " Mbps\n";
    }
  }

  Simulator::Destroy();
}

int main(int argc, char **argv) {
  std::string wifiManager("Arf");
  CommandLine cmd(__FILE__);
  cmd.AddValue("wifiManager",
               "Set wifi rate manager (Aarf, Aarfcd, Amrr, Arf, Cara, Ideal, "
               "Minstrel, Onoe, Rraa)",
               wifiManager);
  cmd.Parse(argc, argv);

  std::cout << "Hidden station experiment with RTS/CTS disabled:\n"
            << std::flush;
  experiment(false, wifiManager);
  std::cout << "------------------------------------------------\n";
  std::cout << "Hidden station experiment with RTS/CTS enabled:\n";
  experiment(true, wifiManager);

  return 0;
}
