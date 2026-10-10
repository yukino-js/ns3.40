

#include "ns3/config-store-module.h"
#include "ns3/core-module.h"
#include "ns3/energy-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/wifi-radio-energy-model-helper.h"
#include "ns3/yans-wifi-helper.h"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("EnergyWithHarvestingExample");

static inline std::string PrintReceivedPacket(Address &from) {
  InetSocketAddress iaddr = InetSocketAddress::ConvertFrom(from);

  std::ostringstream oss;
  oss << "--\nReceived one packet! Socket: " << iaddr.GetIpv4()
      << " port: " << iaddr.GetPort()
      << " at time = " << Simulator::Now().GetSeconds() << "\n--";

  return oss.str();
}

void ReceivePacket(Ptr<Socket> socket) {
  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom(from))) {
    if (packet->GetSize() > 0) {
      NS_LOG_UNCOND(PrintReceivedPacket(from));
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
  NS_LOG_UNCOND(Simulator::Now().GetSeconds()
                << "s Current remaining energy = " << remainingEnergy << "J");
}

void TotalEnergy(double oldValue, double totalEnergy) {
  NS_LOG_UNCOND(Simulator::Now().GetSeconds()
                << "s Total energy consumed by radio = " << totalEnergy << "J");
}

void HarvestedPower(double oldValue, double harvestedPower) {
  NS_LOG_UNCOND(Simulator::Now().GetSeconds()
                << "s Current harvested power = " << harvestedPower << " W");
}

void TotalEnergyHarvested(double oldValue, double totalEnergyHarvested) {
  NS_LOG_UNCOND(Simulator::Now().GetSeconds()
                << "s Total energy harvested by harvester = "
                << totalEnergyHarvested << " J");
}

int main(int argc, char *argv[]) {
  std::string phyMode("DsssRate1Mbps");
  double Prss = -80;
  uint32_t PacketSize = 200;
  bool verbose = false;

  uint32_t numPackets = 10000;
  double interval = 1;
  double startTime = 0.0;
  double distanceToRx = 100.0;

  double harvestingUpdateInterval = 1;

  CommandLine cmd(__FILE__);
  cmd.AddValue("phyMode", "Wifi Phy mode", phyMode);
  cmd.AddValue("Prss", "Intended primary RSS (dBm)", Prss);
  cmd.AddValue("PacketSize", "size of application packet sent", PacketSize);
  cmd.AddValue("numPackets", "Total number of packets to send", numPackets);
  cmd.AddValue("startTime", "Simulation start time", startTime);
  cmd.AddValue("distanceToRx", "X-Axis distance between nodes", distanceToRx);
  cmd.AddValue("verbose", "Turn on all device log components", verbose);
  cmd.Parse(argc, argv);

  Time interPacketInterval = Seconds(interval);

  Config::SetDefault("ns3::WifiRemoteStationManager::FragmentationThreshold",
                     StringValue("2200"));
  Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold",
                     StringValue("2200"));
  Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode",
                     StringValue(phyMode));

  NodeContainer c;
  c.Create(2);
  NodeContainer networkNodes;
  networkNodes.Add(c.Get(0));
  networkNodes.Add(c.Get(1));

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

  NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, networkNodes);

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc =
      CreateObject<ListPositionAllocator>();
  positionAlloc->Add(Vector(0.0, 0.0, 0.0));
  positionAlloc->Add(Vector(2 * distanceToRx, 0.0, 0.0));
  mobility.SetPositionAllocator(positionAlloc);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(c);

  BasicEnergySourceHelper basicSourceHelper;
  basicSourceHelper.Set("BasicEnergySourceInitialEnergyJ", DoubleValue(1.0));
  EnergySourceContainer sources = basicSourceHelper.Install(c);
  WifiRadioEnergyModelHelper radioEnergyHelper;
  radioEnergyHelper.Set("TxCurrentA", DoubleValue(0.0174));
  radioEnergyHelper.Set("RxCurrentA", DoubleValue(0.0197));
  DeviceEnergyModelContainer deviceModels =
      radioEnergyHelper.Install(devices, sources);

  BasicEnergyHarvesterHelper basicHarvesterHelper;
  basicHarvesterHelper.Set("PeriodicHarvestedPowerUpdateInterval",
                           TimeValue(Seconds(harvestingUpdateInterval)));
  basicHarvesterHelper.Set(
      "HarvestablePower",
      StringValue("ns3::UniformRandomVariable[Min=0.0|Max=0.1]"));
  EnergyHarvesterContainer harvesters = basicHarvesterHelper.Install(sources);

  InternetStackHelper internet;
  internet.Install(networkNodes);

  Ipv4AddressHelper ipv4;
  NS_LOG_INFO("Assign IP Addresses.");
  ipv4.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign(devices);

  TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
  Ptr<Socket> recvSink = Socket::CreateSocket(networkNodes.Get(1), tid);
  InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), 80);
  recvSink->Bind(local);
  recvSink->SetRecvCallback(MakeCallback(&ReceivePacket));

  Ptr<Socket> source = Socket::CreateSocket(networkNodes.Get(0), tid);
  InetSocketAddress remote = InetSocketAddress(Ipv4Address::GetBroadcast(), 80);
  source->SetAllowBroadcast(true);
  source->Connect(remote);

  Ptr<BasicEnergySource> basicSourcePtr =
      DynamicCast<BasicEnergySource>(sources.Get(1));
  basicSourcePtr->TraceConnectWithoutContext("RemainingEnergy",
                                             MakeCallback(&RemainingEnergy));
  Ptr<DeviceEnergyModel> basicRadioModelPtr =
      basicSourcePtr->FindDeviceEnergyModels("ns3::WifiRadioEnergyModel")
          .Get(0);
  NS_ASSERT(basicRadioModelPtr);
  basicRadioModelPtr->TraceConnectWithoutContext("TotalEnergyConsumption",
                                                 MakeCallback(&TotalEnergy));
  Ptr<BasicEnergyHarvester> basicHarvesterPtr =
      DynamicCast<BasicEnergyHarvester>(harvesters.Get(1));
  basicHarvesterPtr->TraceConnectWithoutContext("HarvestedPower",
                                                MakeCallback(&HarvestedPower));
  basicHarvesterPtr->TraceConnectWithoutContext(
      "TotalEnergyHarvested", MakeCallback(&TotalEnergyHarvested));

  Simulator::Schedule(Seconds(startTime), &GenerateTraffic, source, PacketSize,
                      networkNodes.Get(0), numPackets, interPacketInterval);

  Simulator::Stop(Seconds(10.0));
  Simulator::Run();

  for (auto iter = deviceModels.Begin(); iter != deviceModels.End(); iter++) {
    double energyConsumed = (*iter)->GetTotalEnergyConsumption();
    NS_LOG_UNCOND("End of simulation ("
                  << Simulator::Now().GetSeconds()
                  << "s) Total energy consumed by radio = " << energyConsumed
                  << "J");
    NS_ASSERT(energyConsumed <= 1.0);
  }

  Simulator::Destroy();

  return 0;
}
