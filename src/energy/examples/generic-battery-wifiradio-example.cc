

#include <ns3/core-module.h>
#include <ns3/energy-module.h>
#include <ns3/internet-module.h>
#include <ns3/mobility-module.h>
#include <ns3/network-module.h>
#include <ns3/wifi-module.h>

#include <sstream>
#include <string>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("GenericBatteryWifiRadioExample");

inline std::string PrintReceivedPacket(Address &from) {
  InetSocketAddress iaddr = InetSocketAddress::ConvertFrom(from);

  std::ostringstream oss;
  oss << " Received one packet! Socket: " << iaddr.GetIpv4()
      << " port: " << iaddr.GetPort() << "\n";

  return oss.str();
}

void ReceivePacket(Ptr<Socket> socket) {
  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom(from))) {
    if (packet->GetSize() > 0) {
      NS_LOG_DEBUG(PrintReceivedPacket(from));
    }
  }
}

static void GenerateTraffic(Ptr<Socket> socket, uint32_t pktSize, Ptr<Node> n,
                            uint32_t pktCount, Time pktInterval) {
  if (pktCount > 0) {
    socket->Send(Create<Packet>(pktSize));
    Simulator::Schedule(pktInterval, &GenerateTraffic, socket, pktSize, n,
                        pktCount - 1, pktInterval);
  } else {
    socket->Close();
  }
}

void RemainingEnergy(double oldValue, double remainingEnergy) {
  NS_LOG_DEBUG(" Remaining energy Node 1 = " << remainingEnergy << " J");
}

int main(int argc, char *argv[]) {
  LogComponentEnableAll(LogLevel(LOG_PREFIX_TIME | LOG_PREFIX_FUNC));
  LogComponentEnable("GenericBatteryWifiRadioExample", LOG_LEVEL_DEBUG);

  std::string phyMode("DsssRate1Mbps");
  double rss = -80;
  uint32_t packetSize = 200;
  bool verbose = false;

  uint32_t numPackets = 10000;
  double interval = 1;
  double startTime = 0.0;
  double distanceToRx = 100.0;

  CommandLine cmd(__FILE__);
  cmd.AddValue("phyMode", "Wifi Phy mode", phyMode);
  cmd.AddValue("rss", "Intended primary RSS (dBm)", rss);
  cmd.AddValue("packetSize", "size of application packet sent (Bytes)",
               packetSize);
  cmd.AddValue("numPackets", "Total number of packets to send", numPackets);
  cmd.AddValue("startTime", "Simulation start time (seconds)", startTime);
  cmd.AddValue("distanceToRx", "X-Axis distance between nodes (meters)",
               distanceToRx);
  cmd.AddValue("verbose", "Turn on all device log components", verbose);
  cmd.Parse(argc, argv);

  Time interPacketInterval = Seconds(interval);

  Config::SetDefault("ns3::WifiRemoteStationManager::FragmentationThreshold",
                     StringValue("2200"));
  Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold",
                     StringValue("2200"));
  Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode",
                     StringValue(phyMode));

  NodeContainer nodeContainer;
  nodeContainer.Create(2);

  WifiHelper wifi;
  if (verbose) {
    WifiHelper::EnableLogComponents();
  }
  wifi.SetStandard(WIFI_STANDARD_80211b);

  YansWifiPhyHelper wifiPhy;
  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel");

  Ptr<YansWifiChannel> wifiChannelPtr = wifiChannel.Create();
  wifiPhy.SetChannel(wifiChannelPtr);

  WifiMacHelper wifiMac;
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode",
                               StringValue(phyMode), "ControlMode",
                               StringValue(phyMode));

  wifiMac.SetType("ns3::AdhocWifiMac");
  NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, nodeContainer);

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc =
      CreateObject<ListPositionAllocator>();
  positionAlloc->Add(Vector(0.0, 0.0, 0.0));
  positionAlloc->Add(Vector(2 * distanceToRx, 0.0, 0.0));
  mobility.SetPositionAllocator(positionAlloc);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(nodeContainer);

  GenericBatteryModelHelper batteryHelper;
  EnergySourceContainer energySourceContainer =
      batteryHelper.Install(nodeContainer, PANASONIC_CGR18650DA_LION);
  batteryHelper.SetCellPack(energySourceContainer, 2, 2);

  Ptr<GenericBatteryModel> battery0 =
      DynamicCast<GenericBatteryModel>(energySourceContainer.Get(0));
  Ptr<GenericBatteryModel> battery1 =
      DynamicCast<GenericBatteryModel>(energySourceContainer.Get(1));

  WifiRadioEnergyModelHelper radioEnergyHelper;
  radioEnergyHelper.Set("TxCurrentA", DoubleValue(4.66));
  radioEnergyHelper.Set("RxCurrentA", DoubleValue(0.466));
  radioEnergyHelper.Set("IdleCurrentA", DoubleValue(0.466));
  DeviceEnergyModelContainer deviceModels =
      radioEnergyHelper.Install(devices, energySourceContainer);

  InternetStackHelper internet;
  internet.Install(nodeContainer);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign(devices);

  TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
  Ptr<Socket> recvSink = Socket::CreateSocket(nodeContainer.Get(1), tid);
  InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), 80);
  recvSink->Bind(local);
  recvSink->SetRecvCallback(MakeCallback(&ReceivePacket));

  Ptr<Socket> source = Socket::CreateSocket(nodeContainer.Get(0), tid);
  InetSocketAddress remote = InetSocketAddress(Ipv4Address::GetBroadcast(), 80);
  source->SetAllowBroadcast(true);
  source->Connect(remote);

  battery1->TraceConnectWithoutContext("RemainingEnergy",
                                       MakeCallback(&RemainingEnergy));

  Ptr<DeviceEnergyModel> radioConsumptionModel =
      battery1->FindDeviceEnergyModels("ns3::WifiRadioEnergyModel").Get(0);

  Simulator::Schedule(Seconds(startTime), &GenerateTraffic, source, packetSize,
                      nodeContainer.Get(0), numPackets, interPacketInterval);

  Simulator::Stop(Seconds(3600));
  Simulator::Run();

  NS_LOG_DEBUG(" *Remaining Capacity * "
               << "| Node 0: " << battery0->GetRemainingEnergy() << " J "
               << "| Node 1: " << battery1->GetRemainingEnergy() << " J");
  NS_LOG_DEBUG(
      " *SoC * " << "| Node 0: " << battery0->GetStateOfCharge() << " % "
                 << "| Node 1: " << battery1->GetStateOfCharge() << " % ");

  Simulator::Destroy();

  return 0;
}
