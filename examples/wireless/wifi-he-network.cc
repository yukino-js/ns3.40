
#include "ns3/boolean.h"
#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/double.h"
#include "ns3/enum.h"
#include "ns3/he-phy.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/log.h"
#include "ns3/mobility-helper.h"
#include "ns3/multi-model-spectrum-channel.h"
#include "ns3/on-off-helper.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/packet-sink.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/spectrum-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/string.h"
#include "ns3/udp-client-server-helper.h"
#include "ns3/uinteger.h"
#include "ns3/wifi-acknowledgment.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/yans-wifi-helper.h"

#include <functional>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("he-wifi-network");

int main(int argc, char *argv[]) {
  bool udp{true};
  bool downlink{true};
  bool useRts{false};
  bool useExtendedBlockAck{false};
  double simulationTime{10};
  double distance{1.0};
  double frequency{5};
  std::size_t nStations{1};
  std::string dlAckSeqType{"NO-OFDMA"};
  bool enableUlOfdma{false};
  bool enableBsrp{false};
  int mcs{-1};
  uint32_t payloadSize = 700;
  std::string phyModel{"Yans"};
  double minExpectedThroughput{0};
  double maxExpectedThroughput{0};
  Time accessReqInterval{0};

  CommandLine cmd(__FILE__);
  cmd.AddValue("frequency",
               "Whether working in the 2.4, 5 or 6 GHz band (other values gets "
               "rejected)",
               frequency);
  cmd.AddValue("distance",
               "Distance in meters between the station and the access point",
               distance);
  cmd.AddValue("simulationTime", "Simulation time in seconds", simulationTime);
  cmd.AddValue("udp", "UDP if set to 1, TCP otherwise", udp);
  cmd.AddValue("downlink",
               "Generate downlink flows if set to 1, uplink flows otherwise",
               downlink);
  cmd.AddValue("useRts", "Enable/disable RTS/CTS", useRts);
  cmd.AddValue("useExtendedBlockAck", "Enable/disable use of extended BACK",
               useExtendedBlockAck);
  cmd.AddValue("nStations", "Number of non-AP HE stations", nStations);
  cmd.AddValue("dlAckType",
               "Ack sequence type for DL OFDMA (NO-OFDMA, ACK-SU-FORMAT, "
               "MU-BAR, AGGR-MU-BAR)",
               dlAckSeqType);
  cmd.AddValue(
      "enableUlOfdma",
      "Enable UL OFDMA (useful if DL OFDMA is enabled and TCP is used)",
      enableUlOfdma);
  cmd.AddValue(
      "enableBsrp",
      "Enable BSRP (useful if DL and UL OFDMA are enabled and TCP is used)",
      enableBsrp);
  cmd.AddValue("muSchedAccessReqInterval",
               "Duration of the interval between two requests for channel "
               "access made by the MU scheduler",
               accessReqInterval);
  cmd.AddValue("mcs", "if set, limit testing to a specific MCS (0-11)", mcs);
  cmd.AddValue("payloadSize", "The application payload size in bytes",
               payloadSize);
  cmd.AddValue("phyModel",
               "PHY model to use when OFDMA is disabled (Yans or Spectrum). If "
               "OFDMA is enabled "
               "then Spectrum is automatically selected",
               phyModel);
  cmd.AddValue(
      "minExpectedThroughput",
      "if set, simulation fails if the lowest throughput is below this value",
      minExpectedThroughput);
  cmd.AddValue(
      "maxExpectedThroughput",
      "if set, simulation fails if the highest throughput is above this value",
      maxExpectedThroughput);
  cmd.Parse(argc, argv);

  if (useRts) {
    Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold",
                       StringValue("0"));
    Config::SetDefault("ns3::WifiDefaultProtectionManager::EnableMuRts",
                       BooleanValue(true));
  }

  if (dlAckSeqType == "ACK-SU-FORMAT") {
    Config::SetDefault("ns3::WifiDefaultAckManager::DlMuAckSequenceType",
                       EnumValue(WifiAcknowledgment::DL_MU_BAR_BA_SEQUENCE));
  } else if (dlAckSeqType == "MU-BAR") {
    Config::SetDefault("ns3::WifiDefaultAckManager::DlMuAckSequenceType",
                       EnumValue(WifiAcknowledgment::DL_MU_TF_MU_BAR));
  } else if (dlAckSeqType == "AGGR-MU-BAR") {
    Config::SetDefault("ns3::WifiDefaultAckManager::DlMuAckSequenceType",
                       EnumValue(WifiAcknowledgment::DL_MU_AGGREGATE_TF));
  } else if (dlAckSeqType != "NO-OFDMA") {
    NS_ABORT_MSG("Invalid DL ack sequence type (must be NO-OFDMA, "
                 "ACK-SU-FORMAT, MU-BAR or "
                 "AGGR-MU-BAR)");
  }

  if (phyModel != "Yans" && phyModel != "Spectrum") {
    NS_ABORT_MSG("Invalid PHY model (must be Yans or Spectrum)");
  }
  if (dlAckSeqType != "NO-OFDMA") {
    phyModel = "Spectrum";
  }

  double prevThroughput[12] = {0};

  std::cout << "MCS value"
            << "\t\t"
            << "Channel width"
            << "\t\t"
            << "GI"
            << "\t\t\t"
            << "Throughput" << '\n';
  int minMcs = 0;
  int maxMcs = 11;
  if (mcs >= 0 && mcs <= 11) {
    minMcs = mcs;
    maxMcs = mcs;
  }
  for (int mcs = minMcs; mcs <= maxMcs; mcs++) {
    uint8_t index = 0;
    double previous = 0;
    uint8_t maxChannelWidth = frequency == 2.4 ? 40 : 160;
    for (int channelWidth = 20; channelWidth <= maxChannelWidth;) {
      for (int gi = 3200; gi >= 800;) {
        if (!udp) {
          Config::SetDefault("ns3::TcpSocket::SegmentSize",
                             UintegerValue(payloadSize));
        }

        NodeContainer wifiStaNodes;
        wifiStaNodes.Create(nStations);
        NodeContainer wifiApNode;
        wifiApNode.Create(1);

        NetDeviceContainer apDevice;
        NetDeviceContainer staDevices;
        WifiMacHelper mac;
        WifiHelper wifi;
        std::string channelStr("{0, " + std::to_string(channelWidth) + ", ");
        StringValue ctrlRate;
        auto nonHtRefRateMbps = HePhy::GetNonHtReferenceRate(mcs) / 1e6;

        std::ostringstream ossDataMode;
        ossDataMode << "HeMcs" << mcs;

        if (frequency == 6) {
          wifi.SetStandard(WIFI_STANDARD_80211ax);
          ctrlRate = StringValue(ossDataMode.str());
          channelStr += "BAND_6GHZ, 0}";
          Config::SetDefault(
              "ns3::LogDistancePropagationLossModel::ReferenceLoss",
              DoubleValue(48));
        } else if (frequency == 5) {
          wifi.SetStandard(WIFI_STANDARD_80211ax);
          std::ostringstream ossControlMode;
          ossControlMode << "OfdmRate" << nonHtRefRateMbps << "Mbps";
          ctrlRate = StringValue(ossControlMode.str());
          channelStr += "BAND_5GHZ, 0}";
        } else if (frequency == 2.4) {
          wifi.SetStandard(WIFI_STANDARD_80211ax);
          std::ostringstream ossControlMode;
          ossControlMode << "ErpOfdmRate" << nonHtRefRateMbps << "Mbps";
          ctrlRate = StringValue(ossControlMode.str());
          channelStr += "BAND_2_4GHZ, 0}";
          Config::SetDefault(
              "ns3::LogDistancePropagationLossModel::ReferenceLoss",
              DoubleValue(40));
        } else {
          std::cout << "Wrong frequency value!" << std::endl;
          return 0;
        }

        wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode",
                                     StringValue(ossDataMode.str()),
                                     "ControlMode", ctrlRate);
        wifi.ConfigHeOptions("GuardInterval", TimeValue(NanoSeconds(gi)),
                             "MpduBufferSize",
                             UintegerValue(useExtendedBlockAck ? 256 : 64));

        Ssid ssid = Ssid("ns3-80211ax");

        if (phyModel == "Spectrum") {
          Ptr<MultiModelSpectrumChannel> spectrumChannel =
              CreateObject<MultiModelSpectrumChannel>();

          Ptr<LogDistancePropagationLossModel> lossModel =
              CreateObject<LogDistancePropagationLossModel>();
          spectrumChannel->AddPropagationLossModel(lossModel);

          SpectrumWifiPhyHelper phy;
          phy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);
          phy.SetChannel(spectrumChannel);

          mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid));
          phy.Set("ChannelSettings", StringValue(channelStr));
          staDevices = wifi.Install(phy, mac, wifiStaNodes);

          if (dlAckSeqType != "NO-OFDMA") {
            mac.SetMultiUserScheduler(
                "ns3::RrMultiUserScheduler", "EnableUlOfdma",
                BooleanValue(enableUlOfdma), "EnableBsrp",
                BooleanValue(enableBsrp), "AccessReqInterval",
                TimeValue(accessReqInterval));
          }
          mac.SetType("ns3::ApWifiMac", "EnableBeaconJitter",
                      BooleanValue(false), "Ssid", SsidValue(ssid));
          apDevice = wifi.Install(phy, mac, wifiApNode);
        } else {
          YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
          YansWifiPhyHelper phy;
          phy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);
          phy.SetChannel(channel.Create());

          mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid));
          phy.Set("ChannelSettings", StringValue(channelStr));
          staDevices = wifi.Install(phy, mac, wifiStaNodes);

          mac.SetType("ns3::ApWifiMac", "EnableBeaconJitter",
                      BooleanValue(false), "Ssid", SsidValue(ssid));
          apDevice = wifi.Install(phy, mac, wifiApNode);
        }

        RngSeedManager::SetSeed(1);
        RngSeedManager::SetRun(1);
        int64_t streamNumber = 150;
        streamNumber += wifi.AssignStreams(apDevice, streamNumber);
        streamNumber += wifi.AssignStreams(staDevices, streamNumber);

        MobilityHelper mobility;
        Ptr<ListPositionAllocator> positionAlloc =
            CreateObject<ListPositionAllocator>();

        positionAlloc->Add(Vector(0.0, 0.0, 0.0));
        positionAlloc->Add(Vector(distance, 0.0, 0.0));
        mobility.SetPositionAllocator(positionAlloc);

        mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

        mobility.Install(wifiApNode);
        mobility.Install(wifiStaNodes);

        InternetStackHelper stack;
        stack.Install(wifiApNode);
        stack.Install(wifiStaNodes);

        Ipv4AddressHelper address;
        address.SetBase("192.168.1.0", "255.255.255.0");
        Ipv4InterfaceContainer staNodeInterfaces;
        Ipv4InterfaceContainer apNodeInterface;

        staNodeInterfaces = address.Assign(staDevices);
        apNodeInterface = address.Assign(apDevice);

        ApplicationContainer serverApp;
        auto serverNodes =
            downlink ? std::ref(wifiStaNodes) : std::ref(wifiApNode);
        Ipv4InterfaceContainer serverInterfaces;
        NodeContainer clientNodes;
        for (std::size_t i = 0; i < nStations; i++) {
          serverInterfaces.Add(downlink ? staNodeInterfaces.Get(i)
                                        : apNodeInterface.Get(0));
          clientNodes.Add(downlink ? wifiApNode.Get(0) : wifiStaNodes.Get(i));
        }

        if (udp) {
          uint16_t port = 9;
          UdpServerHelper server(port);
          serverApp = server.Install(serverNodes.get());
          serverApp.Start(Seconds(0.0));
          serverApp.Stop(Seconds(simulationTime + 1));

          for (std::size_t i = 0; i < nStations; i++) {
            UdpClientHelper client(serverInterfaces.GetAddress(i), port);
            client.SetAttribute("MaxPackets", UintegerValue(4294967295U));
            client.SetAttribute("Interval", TimeValue(Time("0.00001")));
            client.SetAttribute("PacketSize", UintegerValue(payloadSize));
            ApplicationContainer clientApp = client.Install(clientNodes.Get(i));
            clientApp.Start(Seconds(1.0));
            clientApp.Stop(Seconds(simulationTime + 1));
          }
        } else {
          uint16_t port = 50000;
          Address localAddress(InetSocketAddress(Ipv4Address::GetAny(), port));
          PacketSinkHelper packetSinkHelper("ns3::TcpSocketFactory",
                                            localAddress);
          serverApp = packetSinkHelper.Install(serverNodes.get());
          serverApp.Start(Seconds(0.0));
          serverApp.Stop(Seconds(simulationTime + 1));

          for (std::size_t i = 0; i < nStations; i++) {
            OnOffHelper onoff("ns3::TcpSocketFactory", Ipv4Address::GetAny());
            onoff.SetAttribute(
                "OnTime",
                StringValue("ns3::ConstantRandomVariable[Constant=1]"));
            onoff.SetAttribute(
                "OffTime",
                StringValue("ns3::ConstantRandomVariable[Constant=0]"));
            onoff.SetAttribute("PacketSize", UintegerValue(payloadSize));
            onoff.SetAttribute("DataRate", DataRateValue(1000000000));
            AddressValue remoteAddress(
                InetSocketAddress(serverInterfaces.GetAddress(i), port));
            onoff.SetAttribute("Remote", remoteAddress);
            ApplicationContainer clientApp = onoff.Install(clientNodes.Get(i));
            clientApp.Start(Seconds(1.0));
            clientApp.Stop(Seconds(simulationTime + 1));
          }
        }

        Simulator::Schedule(Seconds(0),
                            &Ipv4GlobalRoutingHelper::PopulateRoutingTables);

        Simulator::Stop(Seconds(simulationTime + 1));
        Simulator::Run();

        double tolerance = 0.10;
        uint64_t rxBytes = 0;
        if (udp) {
          for (uint32_t i = 0; i < serverApp.GetN(); i++) {
            rxBytes += payloadSize *
                       DynamicCast<UdpServer>(serverApp.Get(i))->GetReceived();
          }
        } else {
          for (uint32_t i = 0; i < serverApp.GetN(); i++) {
            rxBytes += DynamicCast<PacketSink>(serverApp.Get(i))->GetTotalRx();
          }
        }
        double throughput = (rxBytes * 8) / (simulationTime * 1000000.0);

        Simulator::Destroy();

        std::cout << mcs << "\t\t\t" << channelWidth << " MHz\t\t\t" << gi
                  << " ns\t\t\t" << throughput << " Mbit/s" << std::endl;

        if (mcs == 0 && channelWidth == 20 && gi == 3200) {
          if (throughput * (1 + tolerance) < minExpectedThroughput) {
            NS_LOG_ERROR("Obtained throughput " << throughput
                                                << " is not expected!");
            exit(1);
          }
        }
        if (mcs == 11 && channelWidth == 160 && gi == 800) {
          if (maxExpectedThroughput > 0 &&
              throughput > maxExpectedThroughput * (1 + tolerance)) {
            NS_LOG_ERROR("Obtained throughput " << throughput
                                                << " is not expected!");
            exit(1);
          }
        }
        if (nStations == 1) {
          if (throughput * (1 + tolerance) > previous) {
            previous = throughput;
          } else if (throughput > 0) {
            NS_LOG_ERROR("Obtained throughput " << throughput
                                                << " is not expected!");
            exit(1);
          }
          if (throughput * (1 + tolerance) > prevThroughput[index]) {
            prevThroughput[index] = throughput;
          } else if (throughput > 0) {
            NS_LOG_ERROR("Obtained throughput " << throughput
                                                << " is not expected!");
            exit(1);
          }
        }
        index++;
        gi /= 2;
      }
      channelWidth *= 2;
    }
  }
  return 0;
}
