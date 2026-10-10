

#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/double.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/log.h"
#include "ns3/mobility-helper.h"
#include "ns3/mobility-model.h"
#include "ns3/ssid.h"
#include "ns3/string.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/yans-wifi-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("WifiSimpleInterference");

static inline std::string PrintReceivedPacket(Ptr<Socket> socket) {
  Address addr;

  std::ostringstream oss;

  while (socket->Recv()) {
    socket->GetSockName(addr);
    InetSocketAddress iaddr = InetSocketAddress::ConvertFrom(addr);

    oss << "Received one packet!  Socket: " << iaddr.GetIpv4()
        << " port: " << iaddr.GetPort();
  }

  return oss.str();
}

static void ReceivePacket(Ptr<Socket> socket) {
  NS_LOG_UNCOND(PrintReceivedPacket(socket));
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
  double Prss = -80;
  double Irss = -95;
  double delta = 0;
  uint32_t PpacketSize = 1000;
  uint32_t IpacketSize = 1000;
  bool verbose = false;

  uint32_t numPackets = 1;
  double interval = 1.0;
  double startTime = 10.0;
  double distanceToRx = 100.0;

  double offset = 91;
  CommandLine cmd(__FILE__);
  cmd.AddValue("phyMode", "Wifi Phy mode", phyMode);
  cmd.AddValue("Prss", "Intended primary received signal strength (dBm)", Prss);
  cmd.AddValue("Irss", "Intended interfering received signal strength (dBm)",
               Irss);
  cmd.AddValue("delta", "time offset (microseconds) for interfering signal",
               delta);
  cmd.AddValue("PpacketSize", "size of application packet sent", PpacketSize);
  cmd.AddValue("IpacketSize", "size of interfering packet sent", IpacketSize);
  cmd.AddValue("verbose", "turn on all WifiNetDevice log components", verbose);
  cmd.Parse(argc, argv);
  Time interPacketInterval = Seconds(interval);

  Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode",
                     StringValue(phyMode));

  NodeContainer c;
  c.Create(3);

  WifiHelper wifi;
  if (verbose) {
    WifiHelper::EnableLogComponents();
  }
  wifi.SetStandard(WIFI_STANDARD_80211b);

  YansWifiPhyHelper wifiPhy;

  wifiPhy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);

  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss("ns3::LogDistancePropagationLossModel");
  wifiPhy.SetChannel(wifiChannel.Create());

  WifiMacHelper wifiMac;
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode",
                               StringValue(phyMode), "ControlMode",
                               StringValue(phyMode));
  wifiMac.SetType("ns3::AdhocWifiMac");
  NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, c.Get(0));
  wifiPhy.Set("TxGain", DoubleValue(offset + Prss));
  devices.Add(wifi.Install(wifiPhy, wifiMac, c.Get(1)));
  wifiPhy.Set("TxGain", DoubleValue(offset + Irss));
  devices.Add(wifi.Install(wifiPhy, wifiMac, c.Get(2)));

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc =
      CreateObject<ListPositionAllocator>();
  positionAlloc->Add(Vector(0.0, 0.0, 0.0));
  positionAlloc->Add(Vector(distanceToRx, 0.0, 0.0));
  positionAlloc->Add(Vector(-1 * distanceToRx, 0.0, 0.0));
  mobility.SetPositionAllocator(positionAlloc);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(c);

  InternetStackHelper internet;
  internet.Install(c);

  TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
  Ptr<Socket> recvSink = Socket::CreateSocket(c.Get(0), tid);
  InetSocketAddress local = InetSocketAddress(Ipv4Address("10.1.1.1"), 80);
  recvSink->Bind(local);
  recvSink->SetRecvCallback(MakeCallback(&ReceivePacket));

  Ptr<Socket> source = Socket::CreateSocket(c.Get(1), tid);
  InetSocketAddress remote =
      InetSocketAddress(Ipv4Address("255.255.255.255"), 80);
  source->SetAllowBroadcast(true);
  source->Connect(remote);

  Ptr<Socket> interferer = Socket::CreateSocket(c.Get(2), tid);
  InetSocketAddress interferingAddr =
      InetSocketAddress(Ipv4Address("255.255.255.255"), 49000);
  interferer->SetAllowBroadcast(true);
  interferer->Connect(interferingAddr);

  wifiPhy.EnablePcap("wifi-simple-interference", devices.Get(0));

  NS_LOG_UNCOND("Primary packet RSS=" << Prss << " dBm and interferer RSS="
                                      << Irss << " dBm at time offset=" << delta
                                      << " ms");

  Simulator::ScheduleWithContext(source->GetNode()->GetId(), Seconds(startTime),
                                 &GenerateTraffic, source, PpacketSize,
                                 numPackets, interPacketInterval);

  Simulator::ScheduleWithContext(interferer->GetNode()->GetId(),
                                 Seconds(startTime + delta / 1000000.0),
                                 &GenerateTraffic, interferer, IpacketSize,
                                 numPackets, interPacketInterval);

  Simulator::Run();
  Simulator::Destroy();

  return 0;
}
