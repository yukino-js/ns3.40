

#include "ns3/abort.h"
#include "ns3/ap-wifi-mac.h"
#include "ns3/application-container.h"
#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/double.h"
#include "ns3/he-configuration.h"
#include "ns3/mobility-helper.h"
#include "ns3/multi-model-spectrum-channel.h"
#include "ns3/packet-socket-client.h"
#include "ns3/packet-socket-helper.h"
#include "ns3/packet-socket-server.h"
#include "ns3/spectrum-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/string.h"
#include "ns3/wifi-net-device.h"

using namespace ns3;

std::vector<uint32_t> bytesReceived(4);

uint32_t ContextToNodeId(std::string context) {
  std::string sub = context.substr(10);
  uint32_t pos = sub.find("/Device");
  return std::stoi(sub.substr(0, pos));
}

void SocketRx(std::string context, Ptr<const Packet> p, const Address &addr) {
  uint32_t nodeId = ContextToNodeId(context);
  bytesReceived[nodeId] += p->GetSize();
}

int main(int argc, char *argv[]) {
  double duration = 10.0;
  double d1 = 30.0;
  double d2 = 30.0;
  double d3 = 150.0;
  double powSta1 = 10.0;
  double powSta2 = 10.0;
  double powAp1 = 21.0;
  double powAp2 = 21.0;
  double ccaEdTrSta1 = -62;
  double ccaEdTrSta2 = -62;
  double ccaEdTrAp1 = -62;
  double ccaEdTrAp2 = -62;
  double minimumRssi = -82;
  int channelWidth = 20;
  uint32_t payloadSize = 1500;
  uint32_t mcs = 0;
  double interval = 0.001;
  bool enableObssPd = true;
  double obssPdThreshold = -72.0;

  CommandLine cmd(__FILE__);
  cmd.AddValue("duration", "Duration of simulation (s)", duration);
  cmd.AddValue("interval", "Inter packet interval (s)", interval);
  cmd.AddValue("enableObssPd", "Enable/disable OBSS_PD", enableObssPd);
  cmd.AddValue("d1", "Distance between STA1 and AP1 (m)", d1);
  cmd.AddValue("d2", "Distance between STA2 and AP2 (m)", d2);
  cmd.AddValue("d3", "Distance between AP1 and AP2 (m)", d3);
  cmd.AddValue("powSta1", "Power of STA1 (dBm)", powSta1);
  cmd.AddValue("powSta2", "Power of STA2 (dBm)", powSta2);
  cmd.AddValue("powAp1", "Power of AP1 (dBm)", powAp1);
  cmd.AddValue("powAp2", "Power of AP2 (dBm)", powAp2);
  cmd.AddValue("ccaEdTrSta1", "CCA-ED Threshold of STA1 (dBm)", ccaEdTrSta1);
  cmd.AddValue("ccaEdTrSta2", "CCA-ED Threshold of STA2 (dBm)", ccaEdTrSta2);
  cmd.AddValue("ccaEdTrAp1", "CCA-ED Threshold of AP1 (dBm)", ccaEdTrAp1);
  cmd.AddValue("ccaEdTrAp2", "CCA-ED Threshold of AP2 (dBm)", ccaEdTrAp2);
  cmd.AddValue("minimumRssi",
               "Minimum RSSI for the ThresholdPreambleDetectionModel",
               minimumRssi);
  cmd.AddValue("channelWidth",
               "Bandwidth of the channel in MHz [20, 40, or 80]", channelWidth);
  cmd.AddValue("obssPdThreshold", "Threshold for the OBSS PD Algorithm",
               obssPdThreshold);
  cmd.AddValue("mcs", "The constant MCS value to transmit HE PPDUs", mcs);
  cmd.Parse(argc, argv);

  NodeContainer wifiStaNodes;
  wifiStaNodes.Create(2);

  NodeContainer wifiApNodes;
  wifiApNodes.Create(2);

  SpectrumWifiPhyHelper spectrumPhy;
  Ptr<MultiModelSpectrumChannel> spectrumChannel =
      CreateObject<MultiModelSpectrumChannel>();
  Ptr<FriisPropagationLossModel> lossModel =
      CreateObject<FriisPropagationLossModel>();
  spectrumChannel->AddPropagationLossModel(lossModel);
  Ptr<ConstantSpeedPropagationDelayModel> delayModel =
      CreateObject<ConstantSpeedPropagationDelayModel>();
  spectrumChannel->SetPropagationDelayModel(delayModel);

  spectrumPhy.SetChannel(spectrumChannel);
  spectrumPhy.SetErrorRateModel("ns3::YansErrorRateModel");
  switch (channelWidth) {
  case 20:
    spectrumPhy.Set("ChannelSettings", StringValue("{36, 20, BAND_5GHZ, 0}"));
    break;
  case 40:
    spectrumPhy.Set("ChannelSettings", StringValue("{62, 40, BAND_5GHZ, 0}"));
    break;
  case 80:
    spectrumPhy.Set("ChannelSettings", StringValue("{171, 80, BAND_5GHZ, 0}"));
    break;
  default:
    NS_ABORT_MSG("Unrecognized channel width: " << channelWidth);
    break;
  }
  spectrumPhy.SetPreambleDetectionModel("ns3::ThresholdPreambleDetectionModel",
                                        "MinimumRssi",
                                        DoubleValue(minimumRssi));

  WifiHelper wifi;
  wifi.SetStandard(WIFI_STANDARD_80211ax);
  if (enableObssPd) {
    wifi.SetObssPdAlgorithm("ns3::ConstantObssPdAlgorithm", "ObssPdLevel",
                            DoubleValue(obssPdThreshold));
  }

  WifiMacHelper mac;
  std::ostringstream oss;
  oss << "HeMcs" << mcs;
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode",
                               StringValue(oss.str()), "ControlMode",
                               StringValue(oss.str()));

  spectrumPhy.Set("TxPowerStart", DoubleValue(powSta1));
  spectrumPhy.Set("TxPowerEnd", DoubleValue(powSta1));
  spectrumPhy.Set("CcaEdThreshold", DoubleValue(ccaEdTrSta1));
  spectrumPhy.Set("RxSensitivity", DoubleValue(-92.0));

  Ssid ssidA = Ssid("A");
  mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssidA));
  NetDeviceContainer staDeviceA =
      wifi.Install(spectrumPhy, mac, wifiStaNodes.Get(0));

  spectrumPhy.Set("TxPowerStart", DoubleValue(powAp1));
  spectrumPhy.Set("TxPowerEnd", DoubleValue(powAp1));
  spectrumPhy.Set("CcaEdThreshold", DoubleValue(ccaEdTrAp1));
  spectrumPhy.Set("RxSensitivity", DoubleValue(-92.0));

  mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssidA));
  NetDeviceContainer apDeviceA =
      wifi.Install(spectrumPhy, mac, wifiApNodes.Get(0));

  Ptr<WifiNetDevice> apDevice = apDeviceA.Get(0)->GetObject<WifiNetDevice>();
  Ptr<ApWifiMac> apWifiMac = apDevice->GetMac()->GetObject<ApWifiMac>();
  if (enableObssPd) {
    apDevice->GetHeConfiguration()->SetAttribute("BssColor", UintegerValue(1));
  }

  spectrumPhy.Set("TxPowerStart", DoubleValue(powSta2));
  spectrumPhy.Set("TxPowerEnd", DoubleValue(powSta2));
  spectrumPhy.Set("CcaEdThreshold", DoubleValue(ccaEdTrSta2));
  spectrumPhy.Set("RxSensitivity", DoubleValue(-92.0));

  Ssid ssidB = Ssid("B");
  mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssidB));
  NetDeviceContainer staDeviceB =
      wifi.Install(spectrumPhy, mac, wifiStaNodes.Get(1));

  spectrumPhy.Set("TxPowerStart", DoubleValue(powAp2));
  spectrumPhy.Set("TxPowerEnd", DoubleValue(powAp2));
  spectrumPhy.Set("CcaEdThreshold", DoubleValue(ccaEdTrAp2));
  spectrumPhy.Set("RxSensitivity", DoubleValue(-92.0));

  mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssidB));
  NetDeviceContainer apDeviceB =
      wifi.Install(spectrumPhy, mac, wifiApNodes.Get(1));

  Ptr<WifiNetDevice> ap2Device = apDeviceB.Get(0)->GetObject<WifiNetDevice>();
  apWifiMac = ap2Device->GetMac()->GetObject<ApWifiMac>();
  if (enableObssPd) {
    ap2Device->GetHeConfiguration()->SetAttribute("BssColor", UintegerValue(2));
  }

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc =
      CreateObject<ListPositionAllocator>();
  positionAlloc->Add(Vector(0.0, 0.0, 0.0));
  positionAlloc->Add(Vector(d3, 0.0, 0.0));
  positionAlloc->Add(Vector(0.0, d1, 0.0));
  positionAlloc->Add(Vector(d3, d2, 0.0));
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator(positionAlloc);
  mobility.Install(wifiApNodes);
  mobility.Install(wifiStaNodes);

  PacketSocketHelper packetSocket;
  packetSocket.Install(wifiApNodes);
  packetSocket.Install(wifiStaNodes);
  ApplicationContainer apps;

  {
    PacketSocketAddress socketAddr;
    socketAddr.SetSingleDevice(staDeviceA.Get(0)->GetIfIndex());
    socketAddr.SetPhysicalAddress(apDeviceA.Get(0)->GetAddress());
    socketAddr.SetProtocol(1);
    Ptr<PacketSocketClient> client = CreateObject<PacketSocketClient>();
    client->SetRemote(socketAddr);
    wifiStaNodes.Get(0)->AddApplication(client);
    client->SetAttribute("PacketSize", UintegerValue(payloadSize));
    client->SetAttribute("MaxPackets", UintegerValue(0));
    client->SetAttribute("Interval", TimeValue(Seconds(interval)));
    Ptr<PacketSocketServer> server = CreateObject<PacketSocketServer>();
    server->SetLocal(socketAddr);
    wifiApNodes.Get(0)->AddApplication(server);
  }

  {
    PacketSocketAddress socketAddr;
    socketAddr.SetSingleDevice(staDeviceB.Get(0)->GetIfIndex());
    socketAddr.SetPhysicalAddress(apDeviceB.Get(0)->GetAddress());
    socketAddr.SetProtocol(1);
    Ptr<PacketSocketClient> client = CreateObject<PacketSocketClient>();
    client->SetRemote(socketAddr);
    wifiStaNodes.Get(1)->AddApplication(client);
    client->SetAttribute("PacketSize", UintegerValue(payloadSize));
    client->SetAttribute("MaxPackets", UintegerValue(0));
    client->SetAttribute("Interval", TimeValue(Seconds(interval)));
    Ptr<PacketSocketServer> server = CreateObject<PacketSocketServer>();
    server->SetLocal(socketAddr);
    wifiApNodes.Get(1)->AddApplication(server);
  }

  Config::Connect("/NodeList/*/ApplicationList/*/$ns3::PacketSocketServer/Rx",
                  MakeCallback(&SocketRx));

  Simulator::Stop(Seconds(duration));
  Simulator::Run();

  Simulator::Destroy();

  for (uint32_t i = 0; i < 2; i++) {
    double throughput =
        static_cast<double>(bytesReceived[2 + i]) * 8 / 1000 / 1000 / duration;
    std::cout << "Throughput for BSS " << i + 1 << ": " << throughput
              << " Mbit/s" << std::endl;
  }

  return 0;
}
