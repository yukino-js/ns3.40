

#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/double.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/log.h"
#include "ns3/mobility-helper.h"
#include "ns3/mobility-model.h"
#include "ns3/string.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/yans-wifi-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("WifiSimpleAdhoc");

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
  double rss = -80;
  uint32_t packetSize = 1000;
  uint32_t numPackets = 1;
  double interval = 1.0;
  bool verbose = false;

  CommandLine cmd(__FILE__);
  cmd.AddValue("phyMode", "Wifi Phy mode", phyMode);
  cmd.AddValue("rss", "received signal strength", rss);
  cmd.AddValue("packetSize", "size of application packet sent", packetSize);
  cmd.AddValue("numPackets", "number of packets generated", numPackets);
  cmd.AddValue("interval", "interval (seconds) between packets", interval);
  cmd.AddValue("verbose", "turn on all WifiNetDevice log components", verbose);
  cmd.Parse(argc, argv);
  Time interPacketInterval = Seconds(interval);

  Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode",
                     StringValue(phyMode));

  NodeContainer c;
  c.Create(2);

  WifiHelper wifi;
  if (verbose) {
    WifiHelper::EnableLogComponents();
  }
  wifi.SetStandard(WIFI_STANDARD_80211b);

  YansWifiPhyHelper wifiPhy;
  wifiPhy.Set("RxGain", DoubleValue(0));
  wifiPhy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);

  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss("ns3::FixedRssLossModel", "Rss",
                                 DoubleValue(rss));
  wifiPhy.SetChannel(wifiChannel.Create());

  WifiMacHelper wifiMac;
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode",
                               StringValue(phyMode), "ControlMode",
                               StringValue(phyMode));
  wifiMac.SetType("ns3::AdhocWifiMac");
  NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, c);

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc =
      CreateObject<ListPositionAllocator>();
  positionAlloc->Add(Vector(0.0, 0.0, 0.0));
  positionAlloc->Add(Vector(5.0, 0.0, 0.0));
  mobility.SetPositionAllocator(positionAlloc);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(c);

  InternetStackHelper internet;
  internet.Install(c);

  Ipv4AddressHelper ipv4;
  NS_LOG_INFO("Assign IP Addresses.");
  ipv4.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign(devices);

  TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
  Ptr<Socket> recvSink = Socket::CreateSocket(c.Get(0), tid);
  InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), 80);
  recvSink->Bind(local);
  recvSink->SetRecvCallback(MakeCallback(&ReceivePacket));

  Ptr<Socket> source = Socket::CreateSocket(c.Get(1), tid);
  InetSocketAddress remote =
      InetSocketAddress(Ipv4Address("255.255.255.255"), 80);
  source->SetAllowBroadcast(true);
  source->Connect(remote);

  wifiPhy.EnablePcap("wifi-simple-adhoc", devices);

  NS_LOG_UNCOND("Testing " << numPackets << " packets sent with receiver rss "
                           << rss);

  Simulator::ScheduleWithContext(source->GetNode()->GetId(), Seconds(1.0),
                                 &GenerateTraffic, source, packetSize,
                                 numPackets, interPacketInterval);

  Simulator::Run();
  Simulator::Destroy();

  return 0;
}
