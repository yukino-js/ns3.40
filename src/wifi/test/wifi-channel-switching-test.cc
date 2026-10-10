
#include "ns3/ap-wifi-mac.h"
#include "ns3/boolean.h"
#include "ns3/config.h"
#include "ns3/mobility-helper.h"
#include "ns3/multi-model-spectrum-channel.h"
#include "ns3/packet-socket-client.h"
#include "ns3/packet-socket-helper.h"
#include "ns3/packet-socket-server.h"
#include "ns3/qos-utils.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/spectrum-wifi-helper.h"
#include "ns3/string.h"
#include "ns3/test.h"
#include "ns3/wifi-net-device.h"
#include "ns3/wifi-psdu.h"

#include <array>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("WifiChannelSwitchingTest");

class WifiChannelSwitchingTest : public TestCase {
public:
  WifiChannelSwitchingTest();
  ~WifiChannelSwitchingTest() override;

  void DoRun() override;

  void Associated(Mac48Address bssid);
  void Transmit(WifiConstPsduMap psduMap, WifiTxVector txVector,
                double txPowerW);
  void L7Receive(Ptr<const Packet> p, const Address &addr);
  void SendPacket();
  void ChannelSwitch();
  void StateChange(uint32_t nodeId, Time start, Time duration,
                   WifiPhyState state);

private:
  NodeContainer m_apNode;
  NodeContainer m_staNode;
  NetDeviceContainer m_apDevice;
  NetDeviceContainer m_staDevice;
  uint8_t m_assocCount;
  uint8_t m_txCount;
  uint64_t m_rxBytes;
  uint32_t m_payloadSize;
  std::array<uint8_t, 2> m_channelSwitchCount{0, 0};
};

WifiChannelSwitchingTest::WifiChannelSwitchingTest()
    : TestCase("Test case for resuming data transmission when the recipient "
               "moves back"),
      m_assocCount(0), m_txCount(0), m_rxBytes(0), m_payloadSize(2000) {}

WifiChannelSwitchingTest::~WifiChannelSwitchingTest() {}

void WifiChannelSwitchingTest::Associated(Mac48Address bssid) {
  m_assocCount++;
}

void WifiChannelSwitchingTest::Transmit(WifiConstPsduMap psduMap,
                                        WifiTxVector txVector,
                                        double txPowerW) {
  for (const auto &psduPair : psduMap) {
    std::stringstream ss;
    ss << " " << psduPair.second->GetHeader(0).GetTypeString() << " seq "
       << psduPair.second->GetHeader(0).GetSequenceNumber() << " from "
       << psduPair.second->GetAddr2() << " to " << psduPair.second->GetAddr1();
    NS_LOG_INFO(ss.str());
  }
  NS_LOG_INFO(" TXVECTOR " << txVector << "\n");

  if (psduMap.begin()->second->GetHeader(0).IsQosData()) {
    m_txCount++;

    if (!psduMap.begin()->second->GetHeader(0).IsRetry()) {
      Time txDuration =
          WifiPhy::CalculateTxDuration(psduMap, txVector, WIFI_PHY_BAND_5GHZ);
      Simulator::Schedule(txDuration / 2,
                          &WifiChannelSwitchingTest::ChannelSwitch, this);
    }
  }
}

void WifiChannelSwitchingTest::L7Receive(Ptr<const Packet> p,
                                         const Address &addr) {
  if (p->GetSize() == m_payloadSize) {
    m_rxBytes += m_payloadSize;
  }
}

void WifiChannelSwitchingTest::SendPacket() {
  PacketSocketAddress socket;
  socket.SetSingleDevice(m_apDevice.Get(0)->GetIfIndex());
  socket.SetPhysicalAddress(m_staDevice.Get(0)->GetAddress());
  socket.SetProtocol(1);

  PacketSocketHelper packetSocket;
  packetSocket.Install(m_staNode);
  packetSocket.Install(m_apNode);

  Ptr<PacketSocketClient> client = CreateObject<PacketSocketClient>();
  client->SetAttribute("PacketSize", UintegerValue(m_payloadSize));
  client->SetAttribute("MaxPackets", UintegerValue(1));
  client->SetAttribute("Interval", TimeValue(MicroSeconds(0)));
  client->SetRemote(socket);
  m_apNode.Get(0)->AddApplication(client);
  client->SetStartTime(Seconds(0.5));
  client->SetStopTime(Seconds(1.0));

  Ptr<PacketSocketServer> server = CreateObject<PacketSocketServer>();
  server->SetLocal(socket);
  m_staNode.Get(0)->AddApplication(server);
  server->SetStartTime(Seconds(0.0));
  server->SetStopTime(Seconds(1.0));
}

void WifiChannelSwitchingTest::ChannelSwitch() {
  NS_LOG_INFO("CHANNEL SWITCH\n");
  Config::Set("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/$ns3::WifiPhy/"
              "ChannelSettings",
              StringValue("{1, 20, BAND_2_4GHZ, 0}"));
}

void WifiChannelSwitchingTest::StateChange(uint32_t nodeId, ns3::Time start,
                                           ns3::Time duration,
                                           WifiPhyState state) {
  if (state == WifiPhyState::SWITCHING) {
    m_channelSwitchCount[nodeId]++;
  }
}

void WifiChannelSwitchingTest::DoRun() {
  Time simulationTime(Seconds(6.0));

  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(40);
  int64_t streamNumber = 100;

  m_apNode.Create(1);
  m_staNode.Create(1);

  Ptr<MultiModelSpectrumChannel> spectrumChannel =
      CreateObject<MultiModelSpectrumChannel>();
  Ptr<FriisPropagationLossModel> lossModel =
      CreateObject<FriisPropagationLossModel>();
  spectrumChannel->AddPropagationLossModel(lossModel);
  Ptr<ConstantSpeedPropagationDelayModel> delayModel =
      CreateObject<ConstantSpeedPropagationDelayModel>();
  spectrumChannel->SetPropagationDelayModel(delayModel);

  SpectrumWifiPhyHelper phy;
  phy.SetChannel(spectrumChannel);
  phy.Set("ChannelSettings", StringValue("{36, 20, BAND_5GHZ, 0}"));

  WifiHelper wifi;
  wifi.SetStandard(WIFI_STANDARD_80211ax);
  wifi.SetRemoteStationManager("ns3::IdealWifiManager");

  WifiMacHelper mac;
  mac.SetType("ns3::StaWifiMac", "Ssid",
              SsidValue(Ssid("channel-switching-test")));

  m_staDevice = wifi.Install(phy, mac, m_staNode);

  mac.SetType("ns3::ApWifiMac", "Ssid",
              SsidValue(Ssid("channel-switching-test")), "EnableBeaconJitter",
              BooleanValue(false));

  m_apDevice = wifi.Install(phy, mac, m_apNode);

  wifi.AssignStreams(m_apDevice, streamNumber);

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc =
      CreateObject<ListPositionAllocator>();

  positionAlloc->Add(Vector(0.0, 0.0, 0.0));
  positionAlloc->Add(Vector(5.0, 0.0, 0.0));
  mobility.SetPositionAllocator(positionAlloc);

  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(m_apNode);
  mobility.Install(m_staNode);

  SendPacket();

  Config::ConnectWithoutContext(
      "/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/$ns3::StaWifiMac/Assoc",
      MakeCallback(&WifiChannelSwitchingTest::Associated, this));
  Config::ConnectWithoutContext(
      "/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyTxPsduBegin",
      MakeCallback(&WifiChannelSwitchingTest::Transmit, this));
  Config::ConnectWithoutContext(
      "/NodeList/*/ApplicationList/0/$ns3::PacketSocketServer/Rx",
      MakeCallback(&WifiChannelSwitchingTest::L7Receive, this));
  Config::ConnectWithoutContext(
      "/NodeList/0/DeviceList/*/$ns3::WifiNetDevice/Phy/State/State",
      MakeCallback(&WifiChannelSwitchingTest::StateChange, this).Bind(0));
  Config::ConnectWithoutContext(
      "/NodeList/1/DeviceList/*/$ns3::WifiNetDevice/Phy/State/State",
      MakeCallback(&WifiChannelSwitchingTest::StateChange, this).Bind(1));

  Simulator::Stop(Seconds(2));
  Simulator::Run();

  NS_TEST_EXPECT_MSG_EQ(+m_assocCount, 2, "STA did not associate twice");
  NS_TEST_EXPECT_MSG_EQ(
      +m_txCount, 2,
      "The QoS Data frame should have been transmitted twice by the AP");
  NS_TEST_EXPECT_MSG_EQ(
      m_rxBytes, m_payloadSize,
      "The QoS Data frame should have been received once by the STA");
  NS_TEST_EXPECT_MSG_EQ(+m_channelSwitchCount[0], 1,
                        "AP had to perform one channel switch");
  NS_TEST_EXPECT_MSG_EQ(+m_channelSwitchCount[1], 1,
                        "STA had to perform one channel switch");

  Simulator::Destroy();
}

class WifiChannelSwitchingTestSuite : public TestSuite {
public:
  WifiChannelSwitchingTestSuite();
};

WifiChannelSwitchingTestSuite::WifiChannelSwitchingTestSuite()
    : TestSuite("wifi-channel-switching", UNIT) {
  AddTestCase(new WifiChannelSwitchingTest, TestCase::QUICK);
}

static WifiChannelSwitchingTestSuite g_issue211TestSuite;
