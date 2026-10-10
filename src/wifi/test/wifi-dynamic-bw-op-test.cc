
#include "ns3/channel-access-manager.h"
#include "ns3/config.h"
#include "ns3/mobility-helper.h"
#include "ns3/multi-model-spectrum-channel.h"
#include "ns3/packet-socket-client.h"
#include "ns3/packet-socket-helper.h"
#include "ns3/packet-socket-server.h"
#include "ns3/qos-txop.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/spectrum-wifi-helper.h"
#include "ns3/string.h"
#include "ns3/test.h"
#include "ns3/wifi-mac.h"
#include "ns3/wifi-net-device.h"
#include "ns3/wifi-psdu.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("WifiDynamicBwOpTestSuite");

class WifiUseAvailBwTest : public TestCase {
public:
  WifiUseAvailBwTest(std::initializer_list<std::string> channelStr,
                     uint16_t bss0Width);
  ~WifiUseAvailBwTest() override;

  void L7Receive(uint8_t bss, Ptr<const Packet> p, const Address &addr);
  void Transmit(uint8_t bss, WifiConstPsduMap psduMap, WifiTxVector txVector,
                double txPowerW);
  void CheckResults();

private:
  void DoRun() override;

  struct FrameInfo {
    Time txStart;
    Time txDuration;
    uint8_t bss;
    WifiMacHeader header;
    std::size_t nMpdus;
    WifiTxVector txVector;
  };

  std::vector<std::string> m_channelStr;
  uint16_t m_bss0Width;
  NetDeviceContainer m_staDevices;
  NetDeviceContainer m_apDevices;
  std::array<PacketSocketAddress, 2> m_sockets;
  std::vector<FrameInfo> m_txPsdus;
  uint8_t m_txPkts;
  std::array<uint8_t, 2> m_rcvPkts;
};

WifiUseAvailBwTest::WifiUseAvailBwTest(
    std::initializer_list<std::string> channelStr, uint16_t bss0Width)
    : TestCase("Check transmission on available bandwidth"),
      m_channelStr(channelStr), m_bss0Width(bss0Width),
      m_txPkts(bss0Width / 10), m_rcvPkts({0, 0}) {}

WifiUseAvailBwTest::~WifiUseAvailBwTest() {}

void WifiUseAvailBwTest::L7Receive(uint8_t bss, Ptr<const Packet> p,
                                   const Address &addr) {
  NS_LOG_INFO("Received " << p->GetSize() << " bytes in BSS " << +bss);
  m_rcvPkts[bss]++;
}

void WifiUseAvailBwTest::Transmit(uint8_t bss, WifiConstPsduMap psduMap,
                                  WifiTxVector txVector, double txPowerW) {
  auto psdu = psduMap.begin()->second;
  Time now = Simulator::Now();

  if (!psdu->GetHeader(0).IsMgt() && now > MilliSeconds(400)) {
    m_txPsdus.push_back(
        {now,
         WifiPhy::CalculateTxDuration(psduMap, txVector, WIFI_PHY_BAND_5GHZ),
         bss, psdu->GetHeader(0), psdu->GetNMpdus(), txVector});
  }

  NS_LOG_INFO(
      now << " BSS " << +bss << " " << psdu->GetHeader(0).GetTypeString()
          << " seq " << psdu->GetHeader(0).GetSequenceNumber() << " to "
          << psdu->GetAddr1() << " #MPDUs " << psdu->GetNMpdus() << " size "
          << psdu->GetSize() << " TX duration "
          << WifiPhy::CalculateTxDuration(psduMap, txVector, WIFI_PHY_BAND_5GHZ)
          << "\n"
          << "TXVECTOR " << txVector << "\n");

  if (bss == 1 && psdu->GetNMpdus() == m_txPkts && now >= Seconds(1.5)) {
    Ptr<PacketSocketClient> client = CreateObject<PacketSocketClient>();
    client->SetAttribute("PacketSize", UintegerValue(2000));
    client->SetAttribute("MaxPackets", UintegerValue(m_txPkts));
    client->SetAttribute("Interval", TimeValue(MicroSeconds(0)));
    client->SetRemote(m_sockets[0]);
    m_apDevices.Get(0)->GetNode()->AddApplication(client);
    client->SetStartTime(Seconds(0));
    client->SetStopTime(Seconds(1.0));
    client->Initialize();

    Simulator::Schedule(MicroSeconds(1), [&]() {
      auto mac = StaticCast<WifiNetDevice>(m_apDevices.Get(0))->GetMac();
      auto cam = mac->GetChannelAccessManager();
      NS_TEST_EXPECT_MSG_EQ(
          cam->GetLargestIdlePrimaryChannel(MicroSeconds(1), Simulator::Now()),
          m_bss0Width, "Unexpected width of the largest idle primary channel");
    });
  }
}

void WifiUseAvailBwTest::DoRun() {
  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(40);
  int64_t streamNumber = 100;

  NodeContainer wifiApNodes(2);
  NodeContainer wifiStaNodes(2);

  auto spectrumChannel = CreateObject<MultiModelSpectrumChannel>();
  Ptr<FriisPropagationLossModel> lossModel =
      CreateObject<FriisPropagationLossModel>();
  spectrumChannel->AddPropagationLossModel(lossModel);
  Ptr<ConstantSpeedPropagationDelayModel> delayModel =
      CreateObject<ConstantSpeedPropagationDelayModel>();
  spectrumChannel->SetPropagationDelayModel(delayModel);

  SpectrumWifiPhyHelper phy;
  phy.SetChannel(spectrumChannel);

  WifiHelper wifi;
  wifi.SetStandard(WIFI_STANDARD_80211ax);
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode",
                               StringValue("HeMcs0"), "ControlMode",
                               StringValue("OfdmRate6Mbps"));

  WifiMacHelper apMac;
  apMac.SetType("ns3::ApWifiMac", "Ssid",
                SsidValue(Ssid("dynamic-bw-op-ssid")));

  WifiMacHelper staMac;
  staMac.SetType("ns3::StaWifiMac", "Ssid",
                 SsidValue(Ssid("dynamic-bw-op-ssid")));

  phy.Set("ChannelSettings", StringValue(m_channelStr.at(0)));

  m_apDevices = wifi.Install(phy, apMac, wifiApNodes.Get(0));
  m_staDevices = wifi.Install(phy, staMac, wifiStaNodes.Get(0));

  phy.Set("ChannelSettings", StringValue(m_channelStr.at(1)));

  m_apDevices.Add(wifi.Install(phy, apMac, wifiApNodes.Get(1)));
  m_staDevices.Add(wifi.Install(phy, staMac, wifiStaNodes.Get(1)));

  Ptr<WifiNetDevice> dev;

  streamNumber += wifi.AssignStreams(m_apDevices, streamNumber);
  streamNumber += wifi.AssignStreams(m_staDevices, streamNumber);

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc =
      CreateObject<ListPositionAllocator>();

  positionAlloc->Add(Vector(0.0, 0.0, 0.0));
  positionAlloc->Add(Vector(50.0, 0.0, 0.0));
  positionAlloc->Add(Vector(0.0, 50.0, 0.0));
  positionAlloc->Add(Vector(50.0, 50.0, 0.0));
  mobility.SetPositionAllocator(positionAlloc);

  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(wifiApNodes);
  mobility.Install(wifiStaNodes);

  NS_LOG_INFO("Position of AP (BSS 0) = "
              << wifiApNodes.Get(0)->GetObject<MobilityModel>()->GetPosition());
  NS_LOG_INFO("Position of AP (BSS 1) = "
              << wifiApNodes.Get(1)->GetObject<MobilityModel>()->GetPosition());
  NS_LOG_INFO(
      "Position of STA (BSS 0) = "
      << wifiStaNodes.Get(0)->GetObject<MobilityModel>()->GetPosition());
  NS_LOG_INFO(
      "Position of STA (BSS 1) = "
      << wifiStaNodes.Get(1)->GetObject<MobilityModel>()->GetPosition());

  PacketSocketHelper packetSocket;
  packetSocket.Install(wifiApNodes);
  packetSocket.Install(wifiStaNodes);

  for (uint8_t bss : {0, 1}) {
    m_sockets[bss].SetSingleDevice(m_apDevices.Get(bss)->GetIfIndex());
    m_sockets[bss].SetPhysicalAddress(m_staDevices.Get(bss)->GetAddress());
    m_sockets[bss].SetProtocol(1);

    Ptr<PacketSocketClient> client1 = CreateObject<PacketSocketClient>();
    client1->SetAttribute("PacketSize", UintegerValue(500));
    client1->SetAttribute("MaxPackets", UintegerValue(2));
    client1->SetAttribute("Interval", TimeValue(MicroSeconds(0)));
    client1->SetRemote(m_sockets[bss]);
    wifiApNodes.Get(bss)->AddApplication(client1);
    client1->SetStartTime(Seconds(0.5) + bss * MilliSeconds(500));
    client1->SetStopTime(Seconds(2.0));

    if (bss == 1) {
      Ptr<PacketSocketClient> client2 = CreateObject<PacketSocketClient>();
      client2->SetAttribute("PacketSize", UintegerValue(2000));
      client2->SetAttribute("MaxPackets", UintegerValue(m_txPkts));
      client2->SetAttribute("Interval", TimeValue(MicroSeconds(0)));
      client2->SetRemote(m_sockets[bss]);
      wifiApNodes.Get(bss)->AddApplication(client2);
      client2->SetStartTime(Seconds(1.5));
      client2->SetStopTime(Seconds(2.0));
    }

    Ptr<PacketSocketServer> server = CreateObject<PacketSocketServer>();
    server->SetLocal(m_sockets[bss]);
    wifiStaNodes.Get(bss)->AddApplication(server);
    server->SetStartTime(Seconds(0.0));
    server->SetStopTime(Seconds(2.0));

    Config::ConnectWithoutContext(
        "/NodeList/" + std::to_string(2 + bss) +
            "/ApplicationList/*/$ns3::PacketSocketServer/Rx",
        MakeCallback(&WifiUseAvailBwTest::L7Receive, this).Bind(bss));
    Config::ConnectWithoutContext(
        "/NodeList/" + std::to_string(bss) +
            "/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyTxPsduBegin",
        MakeCallback(&WifiUseAvailBwTest::Transmit, this).Bind(bss));
    Config::ConnectWithoutContext(
        "/NodeList/" + std::to_string(2 + bss) +
            "/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyTxPsduBegin",
        MakeCallback(&WifiUseAvailBwTest::Transmit, this).Bind(bss));
  }

  Simulator::Stop(Seconds(2));
  Simulator::Run();

  CheckResults();

  Simulator::Destroy();
}

void WifiUseAvailBwTest::CheckResults() {
  NS_TEST_ASSERT_MSG_EQ(m_txPsdus.size(), 12, "Expected 12 transmitted frames");

  auto psduIt = m_txPsdus.begin();
  NS_TEST_EXPECT_MSG_EQ(psduIt->header.IsAck(), true,
                        "Expected Ack after ADDBA Request");
  NS_TEST_EXPECT_MSG_EQ(+psduIt->bss, 0, "Expected a transmission in BSS 0");
  psduIt++;

  NS_TEST_EXPECT_MSG_EQ(psduIt->header.IsAck(), true,
                        "Expected Ack after ADDBA Response");
  NS_TEST_EXPECT_MSG_EQ(+psduIt->bss, 0, "Expected a transmission in BSS 0");
  psduIt++;

  NS_TEST_EXPECT_MSG_EQ(psduIt->header.IsQosData(), true,
                        "Expected a QoS data frame");
  NS_TEST_EXPECT_MSG_EQ(+psduIt->bss, 0, "Expected a transmission in BSS 0");
  NS_TEST_EXPECT_MSG_EQ(psduIt->nMpdus, 2,
                        "Expected an A-MPDU of 2 MPDUs after Block Ack");
  NS_TEST_EXPECT_MSG_EQ(psduIt->txVector.GetChannelWidth(),
                        StaticCast<WifiNetDevice>(m_apDevices.Get(0))
                            ->GetPhy()
                            ->GetChannelWidth(),
                        "Expected a transmission on the whole channel width");
  psduIt++;

  NS_TEST_EXPECT_MSG_EQ(psduIt->header.IsBlockAck(), true,
                        "Expected Block Ack after data frame");
  NS_TEST_EXPECT_MSG_EQ(+psduIt->bss, 0, "Expected a transmission in BSS 0");
  psduIt++;

  NS_TEST_EXPECT_MSG_EQ(psduIt->header.IsAck(), true,
                        "Expected Ack after ADDBA Request");
  NS_TEST_EXPECT_MSG_EQ(+psduIt->bss, 1, "Expected a transmission in BSS 1");
  psduIt++;

  NS_TEST_EXPECT_MSG_EQ(psduIt->header.IsAck(), true,
                        "Expected Ack after ADDBA Response");
  NS_TEST_EXPECT_MSG_EQ(+psduIt->bss, 1, "Expected a transmission in BSS 1");
  psduIt++;

  NS_TEST_EXPECT_MSG_EQ(psduIt->header.IsQosData(), true,
                        "Expected a QoS data frame");
  NS_TEST_EXPECT_MSG_EQ(+psduIt->bss, 1, "Expected a transmission in BSS 1");
  NS_TEST_EXPECT_MSG_EQ(psduIt->nMpdus, 2,
                        "Expected an A-MPDU of 2 MPDUs after Block Ack");
  NS_TEST_EXPECT_MSG_EQ(psduIt->txVector.GetChannelWidth(),
                        StaticCast<WifiNetDevice>(m_apDevices.Get(1))
                            ->GetPhy()
                            ->GetChannelWidth(),
                        "Expected a transmission on the whole channel width");
  psduIt++;

  NS_TEST_EXPECT_MSG_EQ(psduIt->header.IsBlockAck(), true,
                        "Expected Block Ack after data frame");
  NS_TEST_EXPECT_MSG_EQ(+psduIt->bss, 1, "Expected a transmission in BSS 1");
  psduIt++;

  NS_TEST_EXPECT_MSG_EQ(psduIt->header.IsQosData(), true,
                        "Expected a QoS data frame");
  NS_TEST_EXPECT_MSG_EQ(+psduIt->bss, 1, "Expected a transmission in BSS 1");
  NS_TEST_EXPECT_MSG_EQ(psduIt->nMpdus, +m_txPkts,
                        "Expected an A-MPDU of " << +m_txPkts << " MPDUs");
  NS_TEST_EXPECT_MSG_EQ(psduIt->txVector.GetChannelWidth(),
                        StaticCast<WifiNetDevice>(m_apDevices.Get(1))
                            ->GetPhy()
                            ->GetChannelWidth(),
                        "Expected a transmission on the whole channel width");
  psduIt++;

  NS_TEST_EXPECT_MSG_EQ(psduIt->header.IsQosData(), true,
                        "Expected a QoS data frame");
  NS_TEST_EXPECT_MSG_EQ(+psduIt->bss, 0, "Expected a transmission in BSS 0");
  NS_TEST_EXPECT_MSG_EQ(psduIt->nMpdus, +m_txPkts,
                        "Expected an A-MPDU of " << +m_txPkts << " MPDUs");
  NS_TEST_EXPECT_MSG_EQ(psduIt->txVector.GetChannelWidth(), m_bss0Width,
                        "Unexpected transmission width");
  NS_TEST_EXPECT_MSG_LT(
      psduIt->txStart,
      std::prev(psduIt)->txStart + std::prev(psduIt)->txDuration,
      "AP 0 is expected to transmit before the end of transmission of AP 1");
  psduIt++;

  NS_TEST_EXPECT_MSG_EQ(psduIt->header.IsBlockAck(), true,
                        "Expected Block Ack after data frame");
  NS_TEST_EXPECT_MSG_EQ(+psduIt->bss, 1, "Expected a transmission in BSS 1");
  psduIt++;

  NS_TEST_EXPECT_MSG_EQ(psduIt->header.IsBlockAck(), true,
                        "Expected Block Ack after data frame");
  NS_TEST_EXPECT_MSG_EQ(+psduIt->bss, 0, "Expected a transmission in BSS 0");
  psduIt++;

  NS_TEST_EXPECT_MSG_EQ(+m_rcvPkts[0], 2 + m_txPkts,
                        "Unexpected number of packets received by STA 0");
  NS_TEST_EXPECT_MSG_EQ(+m_rcvPkts[1], 2 + m_txPkts,
                        "Unexpected number of packets received by STA 1");
}

class WifiDynamicBwOpTestSuite : public TestSuite {
public:
  WifiDynamicBwOpTestSuite();
};

WifiDynamicBwOpTestSuite::WifiDynamicBwOpTestSuite()
    : TestSuite("wifi-dynamic-bw-op", UNIT) {
  AddTestCase(new WifiUseAvailBwTest(
                  {"{54, 40, BAND_5GHZ, 1}", "{52, 20, BAND_5GHZ, 0}"}, 20),
              TestCase::QUICK);
  AddTestCase(new WifiUseAvailBwTest(
                  {"{58, 80, BAND_5GHZ, 0}", "{62, 40, BAND_5GHZ, 1}"}, 40),
              TestCase::QUICK);
  AddTestCase(new WifiUseAvailBwTest(
                  {"{50, 160, BAND_5GHZ, 5}", "{42, 80, BAND_5GHZ, 2}"}, 80),
              TestCase::QUICK);
}

static WifiDynamicBwOpTestSuite g_wifiDynamicBwOpTestSuite;
