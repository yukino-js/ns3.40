
#include "ns3/ap-wifi-mac.h"
#include "ns3/boolean.h"
#include "ns3/config.h"
#include "ns3/mobility-helper.h"
#include "ns3/packet-socket-client.h"
#include "ns3/packet-socket-helper.h"
#include "ns3/packet-socket-server.h"
#include "ns3/packet.h"
#include "ns3/pointer.h"
#include "ns3/qos-txop.h"
#include "ns3/qos-utils.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/single-model-spectrum-channel.h"
#include "ns3/spectrum-wifi-helper.h"
#include "ns3/string.h"
#include "ns3/test.h"
#include "ns3/wifi-mac-header.h"
#include "ns3/wifi-net-device.h"
#include "ns3/wifi-ppdu.h"
#include "ns3/wifi-psdu.h"

using namespace ns3;

class WifiTxopTest : public TestCase {
public:
  WifiTxopTest(bool pifsRecovery);
  ~WifiTxopTest() override;

  void L7Receive(std::string context, Ptr<const Packet> p, const Address &addr);
  void Transmit(std::string context, WifiConstPsduMap psduMap,
                WifiTxVector txVector, double txPowerW);
  void CheckResults();

private:
  void DoRun() override;

  struct FrameInfo {
    Time txStart;
    Time txDuration;
    WifiMacHeader header;
    WifiTxVector txVector;
  };

  uint16_t m_nStations;
  NetDeviceContainer m_staDevices;
  NetDeviceContainer m_apDevices;
  std::vector<FrameInfo> m_txPsdus;
  Time m_txopLimit;
  uint8_t m_aifsn;
  uint32_t m_cwMin;
  uint16_t m_received;
  bool m_pifsRecovery;
};

WifiTxopTest::WifiTxopTest(bool pifsRecovery)
    : TestCase("Check correct operation within TXOPs"), m_nStations(3),
      m_txopLimit(MicroSeconds(4768)), m_received(0),
      m_pifsRecovery(pifsRecovery) {}

WifiTxopTest::~WifiTxopTest() {}

void WifiTxopTest::L7Receive(std::string context, Ptr<const Packet> p,
                             const Address &addr) {
  if (p->GetSize() >= 500) {
    m_received++;
  }
}

void WifiTxopTest::Transmit(std::string context, WifiConstPsduMap psduMap,
                            WifiTxVector txVector, double txPowerW) {
  if (!psduMap.begin()->second->GetHeader(0).IsBeacon() &&
      Simulator::Now() > MilliSeconds(400)) {
    m_txPsdus.push_back(
        {Simulator::Now(),
         WifiPhy::CalculateTxDuration(psduMap, txVector, WIFI_PHY_BAND_5GHZ),
         psduMap[SU_STA_ID]->GetHeader(0), txVector});
  }

  std::cout << Simulator::Now() << " "
            << psduMap.begin()->second->GetHeader(0).GetTypeString() << " seq "
            << psduMap.begin()->second->GetHeader(0).GetSequenceNumber()
            << " to " << psduMap.begin()->second->GetAddr1() << " TX duration "
            << WifiPhy::CalculateTxDuration(psduMap, txVector,
                                            WIFI_PHY_BAND_5GHZ)
            << " duration/ID "
            << psduMap.begin()->second->GetHeader(0).GetDuration() << std::endl;
}

void WifiTxopTest::DoRun() {
  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(40);
  int64_t streamNumber = 100;

  NodeContainer wifiApNode;
  wifiApNode.Create(1);

  NodeContainer wifiStaNodes;
  wifiStaNodes.Create(m_nStations);

  Ptr<SingleModelSpectrumChannel> spectrumChannel =
      CreateObject<SingleModelSpectrumChannel>();
  Ptr<FriisPropagationLossModel> lossModel =
      CreateObject<FriisPropagationLossModel>();
  spectrumChannel->AddPropagationLossModel(lossModel);
  Ptr<ConstantSpeedPropagationDelayModel> delayModel =
      CreateObject<ConstantSpeedPropagationDelayModel>();
  spectrumChannel->SetPropagationDelayModel(delayModel);

  SpectrumWifiPhyHelper phy;
  phy.SetChannel(spectrumChannel);

  Config::SetDefault("ns3::QosFrameExchangeManager::PifsRecovery",
                     BooleanValue(m_pifsRecovery));
  Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold",
                     UintegerValue(1900));

  WifiHelper wifi;
  wifi.SetStandard(WIFI_STANDARD_80211a);
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode",
                               StringValue("OfdmRate12Mbps"), "ControlMode",
                               StringValue("OfdmRate6Mbps"));

  WifiMacHelper mac;
  mac.SetType("ns3::StaWifiMac", "QosSupported", BooleanValue(true), "Ssid",
              SsidValue(Ssid("non-existent-ssid")));

  m_staDevices = wifi.Install(phy, mac, wifiStaNodes);

  mac.SetType("ns3::ApWifiMac", "QosSupported", BooleanValue(true), "Ssid",
              SsidValue(Ssid("wifi-txop-ssid")), "BeaconInterval",
              TimeValue(MicroSeconds(102400)), "EnableBeaconJitter",
              BooleanValue(false));

  m_apDevices = wifi.Install(phy, mac, wifiApNode);

  Ptr<WifiNetDevice> dev = DynamicCast<WifiNetDevice>(m_staDevices.Get(0));
  dev->GetMac()->SetSsid(Ssid("wifi-txop-ssid"));

  for (uint16_t i = 1; i < m_nStations; i++) {
    dev = DynamicCast<WifiNetDevice>(m_staDevices.Get(i));
    Simulator::Schedule(i * MicroSeconds(102400), &WifiMac::SetSsid,
                        dev->GetMac(), Ssid("wifi-txop-ssid"));
  }

  wifi.AssignStreams(m_apDevices, streamNumber);

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc =
      CreateObject<ListPositionAllocator>();

  positionAlloc->Add(Vector(0.0, 0.0, 0.0));
  positionAlloc->Add(Vector(1.0, 0.0, 0.0));
  positionAlloc->Add(Vector(0.0, 1.0, 0.0));
  positionAlloc->Add(Vector(-1.0, 0.0, 0.0));
  mobility.SetPositionAllocator(positionAlloc);

  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(wifiApNode);
  mobility.Install(wifiStaNodes);

  dev = DynamicCast<WifiNetDevice>(m_apDevices.Get(0));
  PointerValue ptr;
  dev->GetMac()->GetAttribute("BE_Txop", ptr);
  ptr.Get<QosTxop>()->SetTxopLimit(m_txopLimit);
  m_aifsn = ptr.Get<QosTxop>()->Txop::GetAifsn();
  m_cwMin = ptr.Get<QosTxop>()->Txop::GetMinCw();

  PacketSocketHelper packetSocket;
  packetSocket.Install(wifiApNode);
  packetSocket.Install(wifiStaNodes);

  for (uint16_t i = 0; i < m_nStations; i++) {
    PacketSocketAddress socket;
    socket.SetSingleDevice(m_apDevices.Get(0)->GetIfIndex());
    socket.SetPhysicalAddress(m_staDevices.Get(i)->GetAddress());
    socket.SetProtocol(1);

    Ptr<PacketSocketClient> client1 = CreateObject<PacketSocketClient>();
    client1->SetAttribute("PacketSize", UintegerValue(500));
    client1->SetAttribute("MaxPackets", UintegerValue(1));
    client1->SetAttribute("Interval", TimeValue(MicroSeconds(1)));
    client1->SetRemote(socket);
    wifiApNode.Get(0)->AddApplication(client1);
    client1->SetStartTime(MilliSeconds(410));
    client1->SetStopTime(Seconds(1.0));

    Ptr<PacketSocketClient> client2 = CreateObject<PacketSocketClient>();
    client2->SetAttribute("PacketSize", UintegerValue(2000));
    client2->SetAttribute("MaxPackets", UintegerValue(1));
    client2->SetAttribute("Interval", TimeValue(MicroSeconds(1)));
    client2->SetRemote(socket);
    wifiApNode.Get(0)->AddApplication(client2);
    client2->SetStartTime(MilliSeconds(520));
    client2->SetStopTime(Seconds(1.0));

    Ptr<PacketSocketServer> server = CreateObject<PacketSocketServer>();
    server->SetLocal(socket);
    wifiStaNodes.Get(i)->AddApplication(server);
    server->SetStartTime(Seconds(0.0));
    server->SetStopTime(Seconds(1.0));
  }

  Ptr<ReceiveListErrorModel> apPem = CreateObject<ReceiveListErrorModel>();
  apPem->SetList({9});
  dev = DynamicCast<WifiNetDevice>(m_apDevices.Get(0));
  dev->GetMac()->GetWifiPhy()->SetPostReceptionErrorModel(apPem);

  Ptr<ReceiveListErrorModel> sta2Pem = CreateObject<ReceiveListErrorModel>();
  sta2Pem->SetList({24});
  dev = DynamicCast<WifiNetDevice>(m_staDevices.Get(1));
  dev->GetMac()->GetWifiPhy()->SetPostReceptionErrorModel(sta2Pem);

  {
    PacketSocketAddress socket;
    socket.SetSingleDevice(m_staDevices.Get(0)->GetIfIndex());
    socket.SetPhysicalAddress(m_apDevices.Get(0)->GetAddress());
    socket.SetProtocol(1);

    Ptr<PacketSocketClient> client = CreateObject<PacketSocketClient>();
    client->SetAttribute("PacketSize", UintegerValue(1500));
    client->SetAttribute("MaxPackets", UintegerValue(1));
    client->SetAttribute("Interval", TimeValue(MicroSeconds(0)));
    client->SetRemote(socket);
    wifiStaNodes.Get(0)->AddApplication(client);
    client->SetStartTime(MilliSeconds(412));
    client->SetStopTime(Seconds(1.0));

    Ptr<PacketSocketServer> server = CreateObject<PacketSocketServer>();
    server->SetLocal(socket);
    wifiApNode.Get(0)->AddApplication(server);
    server->SetStartTime(Seconds(0.0));
    server->SetStopTime(Seconds(1.0));
  }

  Config::Connect("/NodeList/*/ApplicationList/*/$ns3::PacketSocketServer/Rx",
                  MakeCallback(&WifiTxopTest::L7Receive, this));
  Config::Connect(
      "/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyTxPsduBegin",
      MakeCallback(&WifiTxopTest::Transmit, this));

  Simulator::Stop(Seconds(1));
  Simulator::Run();

  CheckResults();

  Simulator::Destroy();
}

void WifiTxopTest::CheckResults() {
  Time tEnd;
  Time tStart;
  Time txopStart;
  Time tolerance = NanoSeconds(50);
  Time sifs =
      DynamicCast<WifiNetDevice>(m_apDevices.Get(0))->GetPhy()->GetSifs();
  Time slot =
      DynamicCast<WifiNetDevice>(m_apDevices.Get(0))->GetPhy()->GetSlot();
  Time navEnd;

  auto RoundDurationId = [](Time t) {
    return MicroSeconds(ceil(static_cast<double>(t.GetNanoSeconds()) / 1000));
  };

  NS_TEST_ASSERT_MSG_EQ(m_txPsdus.size(), 25, "Expected 25 transmitted frames");

  txopStart = m_txPsdus[0].txStart;

  NS_TEST_ASSERT_MSG_EQ(m_txPsdus[0].header.IsQosData(), true,
                        "Expected a QoS data frame");
  NS_TEST_ASSERT_MSG_EQ(
      m_txPsdus[0].header.GetAddr1(),
      DynamicCast<WifiNetDevice>(m_staDevices.Get(0))->GetMac()->GetAddress(),
      "Expected a frame sent by the AP to the first station");
  NS_TEST_ASSERT_MSG_EQ(
      m_txPsdus[0].header.GetDuration(),
      RoundDurationId(m_txopLimit - m_txPsdus[0].txDuration),
      "Duration/ID of the first frame must cover the whole TXOP");

  tEnd = m_txPsdus[0].txStart + m_txPsdus[0].txDuration;
  tStart = m_txPsdus[1].txStart;

  NS_TEST_ASSERT_MSG_LT(tEnd + sifs, tStart,
                        "Ack in response to the first frame sent too early");
  NS_TEST_ASSERT_MSG_LT(tStart, tEnd + sifs + tolerance,
                        "Ack in response to the first frame sent too late");
  NS_TEST_ASSERT_MSG_EQ(m_txPsdus[1].header.IsAck(), true,
                        "Expected a Normal Ack");
  NS_TEST_ASSERT_MSG_EQ(
      m_txPsdus[1].header.GetAddr1(),
      DynamicCast<WifiNetDevice>(m_apDevices.Get(0))->GetMac()->GetAddress(),
      "Expected a Normal Ack sent to the AP");
  NS_TEST_ASSERT_MSG_EQ(
      m_txPsdus[1].header.GetDuration(),
      RoundDurationId(m_txPsdus[0].header.GetDuration() - sifs -
                      m_txPsdus[1].txDuration),
      "Duration/ID of the Ack must be derived from that of the first frame");

  txopStart = m_txPsdus[2].txStart;

  tEnd = m_txPsdus[1].txStart + m_txPsdus[1].txDuration;
  tStart = m_txPsdus[2].txStart;

  NS_TEST_ASSERT_MSG_GT_OR_EQ(
      tStart - tEnd, sifs + m_aifsn * slot,
      "Less than AIFS elapsed between AckTimeout and the next TXOP start");
  NS_TEST_ASSERT_MSG_LT_OR_EQ(
      tStart - tEnd, sifs + m_aifsn * slot + (2 * (m_cwMin + 1) - 1) * slot,
      "More than AIFS+BO elapsed between AckTimeout and the next TXOP start");
  NS_TEST_ASSERT_MSG_EQ(m_txPsdus[2].header.IsQosData(), true,
                        "Expected to retransmit a QoS data frame");
  NS_TEST_ASSERT_MSG_EQ(
      m_txPsdus[2].header.GetAddr1(),
      DynamicCast<WifiNetDevice>(m_staDevices.Get(0))->GetMac()->GetAddress(),
      "Expected to retransmit a frame to the first station");
  NS_TEST_ASSERT_MSG_EQ(
      m_txPsdus[2].header.GetDuration(),
      RoundDurationId(m_txopLimit - m_txPsdus[2].txDuration),
      "Duration/ID of the retransmitted frame must cover the whole TXOP");

  tEnd = m_txPsdus[2].txStart + m_txPsdus[2].txDuration;
  tStart = m_txPsdus[3].txStart;

  NS_TEST_ASSERT_MSG_LT(tEnd + sifs, tStart,
                        "Ack in response to the first frame sent too early");
  NS_TEST_ASSERT_MSG_LT(tStart, tEnd + sifs + tolerance,
                        "Ack in response to the first frame sent too late");
  NS_TEST_ASSERT_MSG_EQ(m_txPsdus[3].header.IsAck(), true,
                        "Expected a Normal Ack");
  NS_TEST_ASSERT_MSG_EQ(
      m_txPsdus[3].header.GetAddr1(),
      DynamicCast<WifiNetDevice>(m_apDevices.Get(0))->GetMac()->GetAddress(),
      "Expected a Normal Ack sent to the AP");
  NS_TEST_ASSERT_MSG_EQ(
      m_txPsdus[3].header.GetDuration(),
      RoundDurationId(m_txPsdus[2].header.GetDuration() - sifs -
                      m_txPsdus[3].txDuration),
      "Duration/ID of the Ack must be derived from that of the previous frame");

  tEnd = m_txPsdus[3].txStart + m_txPsdus[3].txDuration;
  tStart = m_txPsdus[4].txStart;

  NS_TEST_ASSERT_MSG_LT(tEnd + sifs, tStart, "Second frame sent too early");
  NS_TEST_ASSERT_MSG_LT(tStart, tEnd + sifs + tolerance,
                        "Second frame sent too late");
  NS_TEST_ASSERT_MSG_EQ(m_txPsdus[4].header.IsQosData(), true,
                        "Expected a QoS data frame");
  NS_TEST_ASSERT_MSG_EQ(
      m_txPsdus[4].header.GetAddr1(),
      DynamicCast<WifiNetDevice>(m_staDevices.Get(1))->GetMac()->GetAddress(),
      "Expected a frame sent by the AP to the second station");
  NS_TEST_ASSERT_MSG_EQ(
      m_txPsdus[4].header.GetDuration(),
      RoundDurationId(m_txopLimit - (m_txPsdus[4].txStart - txopStart) -
                      m_txPsdus[4].txDuration),
      "Duration/ID of the second frame does not cover the remaining TXOP");

  tEnd = m_txPsdus[4].txStart + m_txPsdus[4].txDuration + sifs + slot +
         WifiPhy::CalculatePhyPreambleAndHeaderDuration(m_txPsdus[4].txVector);
  tStart = m_txPsdus[5].txStart;

  if (m_pifsRecovery) {
    NS_TEST_ASSERT_MSG_EQ(tEnd + sifs + slot, tStart,
                          "Second frame must have been sent after a PIFS");
  } else {
    NS_TEST_ASSERT_MSG_GT_OR_EQ(
        tStart - tEnd, sifs + m_aifsn * slot,
        "Less than AIFS elapsed between AckTimeout and the next transmission");
    NS_TEST_ASSERT_MSG_LT_OR_EQ(
        tStart - tEnd, sifs + m_aifsn * slot + (2 * (m_cwMin + 1) - 1) * slot,
        "More than AIFS+BO elapsed between AckTimeout and the next TXOP start");
  }
  NS_TEST_ASSERT_MSG_EQ(m_txPsdus[5].header.IsQosData(), true,
                        "Expected a QoS data frame");
  NS_TEST_ASSERT_MSG_EQ(
      m_txPsdus[5].header.GetAddr1(),
      DynamicCast<WifiNetDevice>(m_staDevices.Get(1))->GetMac()->GetAddress(),
      "Expected a frame sent by the AP to the second station");
  NS_TEST_ASSERT_MSG_EQ(
      m_txPsdus[5].header.GetDuration(),
      RoundDurationId(m_txopLimit - (m_txPsdus[5].txStart - txopStart) -
                      m_txPsdus[5].txDuration),
      "Duration/ID of the second frame does not cover the remaining TXOP");

  tEnd = m_txPsdus[5].txStart + m_txPsdus[5].txDuration;
  tStart = m_txPsdus[6].txStart;

  NS_TEST_ASSERT_MSG_LT(tEnd + sifs, tStart,
                        "Ack in response to the second frame sent too early");
  NS_TEST_ASSERT_MSG_LT(tStart, tEnd + sifs + tolerance,
                        "Ack in response to the second frame sent too late");
  NS_TEST_ASSERT_MSG_EQ(m_txPsdus[6].header.IsAck(), true,
                        "Expected a Normal Ack");
  NS_TEST_ASSERT_MSG_EQ(
      m_txPsdus[6].header.GetAddr1(),
      DynamicCast<WifiNetDevice>(m_apDevices.Get(0))->GetMac()->GetAddress(),
      "Expected a Normal Ack sent to the AP");
  NS_TEST_ASSERT_MSG_EQ(
      m_txPsdus[6].header.GetDuration(),
      RoundDurationId(m_txPsdus[5].header.GetDuration() - sifs -
                      m_txPsdus[6].txDuration),
      "Duration/ID of the Ack must be derived from that of the previous frame");

  tEnd = m_txPsdus[6].txStart + m_txPsdus[6].txDuration;
  tStart = m_txPsdus[7].txStart;

  NS_TEST_ASSERT_MSG_LT(tEnd + sifs, tStart, "Third frame sent too early");
  NS_TEST_ASSERT_MSG_LT(tStart, tEnd + sifs + tolerance,
                        "Third frame sent too late");
  NS_TEST_ASSERT_MSG_EQ(m_txPsdus[7].header.IsQosData(), true,
                        "Expected a QoS data frame");
  NS_TEST_ASSERT_MSG_EQ(
      m_txPsdus[7].header.GetAddr1(),
      DynamicCast<WifiNetDevice>(m_staDevices.Get(2))->GetMac()->GetAddress(),
      "Expected a frame sent by the AP to the third station");
  NS_TEST_ASSERT_MSG_EQ(
      m_txPsdus[7].header.GetDuration(),
      RoundDurationId(m_txopLimit - (m_txPsdus[7].txStart - txopStart) -
                      m_txPsdus[7].txDuration),
      "Duration/ID of the third frame does not cover the remaining TXOP");

  tEnd = m_txPsdus[7].txStart + m_txPsdus[7].txDuration;
  tStart = m_txPsdus[8].txStart;

  NS_TEST_ASSERT_MSG_LT(tEnd + sifs, tStart,
                        "Ack in response to the third frame sent too early");
  NS_TEST_ASSERT_MSG_LT(tStart, tEnd + sifs + tolerance,
                        "Ack in response to the third frame sent too late");
  NS_TEST_ASSERT_MSG_EQ(m_txPsdus[8].header.IsAck(), true,
                        "Expected a Normal Ack");
  NS_TEST_ASSERT_MSG_EQ(
      m_txPsdus[8].header.GetAddr1(),
      DynamicCast<WifiNetDevice>(m_apDevices.Get(0))->GetMac()->GetAddress(),
      "Expected a Normal Ack sent to the AP");
  NS_TEST_ASSERT_MSG_EQ(
      m_txPsdus[8].header.GetDuration(),
      RoundDurationId(m_txPsdus[7].header.GetDuration() - sifs -
                      m_txPsdus[8].txDuration),
      "Duration/ID of the Ack must be derived from that of the previous frame");

  tEnd = m_txPsdus[8].txStart + m_txPsdus[8].txDuration;
  tStart = m_txPsdus[9].txStart;

  NS_TEST_ASSERT_MSG_LT(tEnd + sifs, tStart, "CF-End sent too early");
  NS_TEST_ASSERT_MSG_LT(tStart, tEnd + sifs + tolerance,
                        "CF-End sent too late");
  NS_TEST_ASSERT_MSG_EQ(m_txPsdus[9].header.IsCfEnd(), true,
                        "Expected a CF-End frame");
  NS_TEST_ASSERT_MSG_EQ(m_txPsdus[9].header.GetDuration(), Seconds(0),
                        "Duration/ID must be set to 0 for CF-End frames");

  tEnd = m_txPsdus[9].txStart + m_txPsdus[9].txDuration;
  tStart = m_txPsdus[10].txStart;

  NS_TEST_ASSERT_MSG_GT_OR_EQ(tStart - tEnd, sifs + m_aifsn * slot,
                              "Less than AIFS elapsed between two TXOPs");
  NS_TEST_ASSERT_MSG_LT_OR_EQ(
      tStart - tEnd, sifs + m_aifsn * slot + m_cwMin * slot + tolerance,
      "More than AIFS+BO elapsed between two TXOPs");
  NS_TEST_ASSERT_MSG_EQ(m_txPsdus[10].header.IsQosData(), true,
                        "Expected a QoS data frame");
  NS_TEST_ASSERT_MSG_EQ(
      m_txPsdus[10].header.GetAddr1(),
      DynamicCast<WifiNetDevice>(m_apDevices.Get(0))->GetMac()->GetAddress(),
      "Expected a frame sent by the first station to the AP");
  NS_TEST_ASSERT_MSG_EQ(m_txPsdus[10].header.GetDuration(),
                        RoundDurationId(m_txopLimit - m_txPsdus[10].txDuration),
                        "Duration/ID of the frame sent by the first station "
                        "does not cover the remaining TXOP");

  tEnd = m_txPsdus[10].txStart + m_txPsdus[10].txDuration;
  tStart = m_txPsdus[11].txStart;

  NS_TEST_ASSERT_MSG_LT(tEnd + sifs, tStart, "Ack sent too early");
  NS_TEST_ASSERT_MSG_LT(tStart, tEnd + sifs + tolerance, "Ack sent too late");
  NS_TEST_ASSERT_MSG_EQ(m_txPsdus[11].header.IsAck(), true,
                        "Expected a Normal Ack");
  NS_TEST_ASSERT_MSG_EQ(
      m_txPsdus[11].header.GetAddr1(),
      DynamicCast<WifiNetDevice>(m_staDevices.Get(0))->GetMac()->GetAddress(),
      "Expected a Normal Ack sent to the first station");
  NS_TEST_ASSERT_MSG_EQ(
      m_txPsdus[11].header.GetDuration(),
      RoundDurationId(m_txPsdus[10].header.GetDuration() - sifs -
                      m_txPsdus[11].txDuration),
      "Duration/ID of the Ack must be derived from that of the previous frame");

  tEnd = m_txPsdus[11].txStart + m_txPsdus[11].txDuration;
  tStart = m_txPsdus[12].txStart;

  NS_TEST_ASSERT_MSG_LT(tEnd + sifs, tStart, "CF-End sent too early");
  NS_TEST_ASSERT_MSG_LT(tStart, tEnd + sifs + tolerance,
                        "CF-End sent too late");
  NS_TEST_ASSERT_MSG_EQ(m_txPsdus[12].header.IsCfEnd(), true,
                        "Expected a CF-End frame");
  NS_TEST_ASSERT_MSG_EQ(m_txPsdus[12].header.GetDuration(), Seconds(0),
                        "Duration/ID must be set to 0 for CF-End frames");

  txopStart = m_txPsdus[13].txStart;

  NS_TEST_ASSERT_MSG_EQ(m_txPsdus[13].header.IsRts(), true,
                        "Expected an RTS frame");
  NS_TEST_ASSERT_MSG_EQ(
      m_txPsdus[13].header.GetAddr1(),
      DynamicCast<WifiNetDevice>(m_staDevices.Get(0))->GetMac()->GetAddress(),
      "Expected an RTS frame sent by the AP to the first station");
  NS_TEST_ASSERT_MSG_EQ(
      m_txPsdus[13].header.GetDuration(),
      RoundDurationId(m_txopLimit - m_txPsdus[13].txDuration),
      "Duration/ID of the first RTS frame must cover the whole TXOP");

  tEnd = m_txPsdus[13].txStart + m_txPsdus[13].txDuration;
  tStart = m_txPsdus[14].txStart;

  NS_TEST_ASSERT_MSG_LT(
      tEnd + sifs, tStart,
      "CTS in response to the first RTS frame sent too early");
  NS_TEST_ASSERT_MSG_LT(tStart, tEnd + sifs + tolerance,
                        "CTS in response to the first RTS frame sent too late");
  NS_TEST_ASSERT_MSG_EQ(m_txPsdus[14].header.IsCts(), true, "Expected a CTS");
  NS_TEST_ASSERT_MSG_EQ(
      m_txPsdus[14].header.GetAddr1(),
      DynamicCast<WifiNetDevice>(m_apDevices.Get(0))->GetMac()->GetAddress(),
      "Expected a CTS frame sent to the AP");
  NS_TEST_ASSERT_MSG_EQ(m_txPsdus[14].header.GetDuration(),
                        RoundDurationId(m_txPsdus[13].header.GetDuration() -
                                        sifs - m_txPsdus[14].txDuration),
                        "Duration/ID of the CTS frame must be derived from "
                        "that of the RTS frame");

  tEnd = m_txPsdus[14].txStart + m_txPsdus[14].txDuration;
  tStart = m_txPsdus[15].txStart;

  NS_TEST_ASSERT_MSG_LT(tEnd + sifs, tStart,
                        "First QoS data frame sent too early");
  NS_TEST_ASSERT_MSG_LT(tStart, tEnd + sifs + tolerance,
                        "First QoS data frame sent too late");
  NS_TEST_ASSERT_MSG_EQ(m_txPsdus[15].header.IsQosData(), true,
                        "Expected a QoS data frame");
  NS_TEST_ASSERT_MSG_EQ(
      m_txPsdus[15].header.GetAddr1(),
      DynamicCast<WifiNetDevice>(m_staDevices.Get(0))->GetMac()->GetAddress(),
      "Expected a frame sent by the AP to the first station");
  NS_TEST_ASSERT_MSG_EQ(m_txPsdus[15].header.GetDuration(),
                        RoundDurationId(m_txopLimit -
                                        (m_txPsdus[15].txStart - txopStart) -
                                        m_txPsdus[15].txDuration),
                        "Duration/ID of the first QoS data frame does not "
                        "cover the remaining TXOP");

  tEnd = m_txPsdus[15].txStart + m_txPsdus[15].txDuration;
  tStart = m_txPsdus[16].txStart;

  NS_TEST_ASSERT_MSG_LT(
      tEnd + sifs, tStart,
      "Ack in response to the first QoS data frame sent too early");
  NS_TEST_ASSERT_MSG_LT(
      tStart, tEnd + sifs + tolerance,
      "Ack in response to the first QoS data frame sent too late");
  NS_TEST_ASSERT_MSG_EQ(m_txPsdus[16].header.IsAck(), true,
                        "Expected a Normal Ack");
  NS_TEST_ASSERT_MSG_EQ(
      m_txPsdus[16].header.GetAddr1(),
      DynamicCast<WifiNetDevice>(m_apDevices.Get(0))->GetMac()->GetAddress(),
      "Expected a Normal Ack sent to the AP");
  NS_TEST_ASSERT_MSG_EQ(
      m_txPsdus[16].header.GetDuration(),
      RoundDurationId(m_txPsdus[15].header.GetDuration() - sifs -
                      m_txPsdus[16].txDuration),
      "Duration/ID of the Ack must be derived from that of the previous frame");

  tEnd = m_txPsdus[16].txStart + m_txPsdus[16].txDuration;
  tStart = m_txPsdus[17].txStart;

  NS_TEST_ASSERT_MSG_LT(tEnd + sifs, tStart, "Second RTS frame sent too early");
  NS_TEST_ASSERT_MSG_LT(tStart, tEnd + sifs + tolerance,
                        "Second RTS frame sent too late");
  NS_TEST_ASSERT_MSG_EQ(m_txPsdus[17].header.IsRts(), true,
                        "Expected an RTS frame");
  NS_TEST_ASSERT_MSG_EQ(
      m_txPsdus[17].header.GetAddr1(),
      DynamicCast<WifiNetDevice>(m_staDevices.Get(1))->GetMac()->GetAddress(),
      "Expected an RTS frame sent by the AP to the second station");
  NS_TEST_ASSERT_MSG_EQ(
      m_txPsdus[17].header.GetDuration(),
      RoundDurationId(m_txopLimit - (m_txPsdus[17].txStart - txopStart) -
                      m_txPsdus[17].txDuration),
      "Duration/ID of the second RTS frame must cover the whole TXOP");

  tEnd = m_txPsdus[17].txStart + m_txPsdus[17].txDuration;
  tStart = m_txPsdus[18].txStart;

  NS_TEST_ASSERT_MSG_LT(
      tEnd + sifs, tStart,
      "CTS in response to the second RTS frame sent too early");
  NS_TEST_ASSERT_MSG_LT(
      tStart, tEnd + sifs + tolerance,
      "CTS in response to the second RTS frame sent too late");
  NS_TEST_ASSERT_MSG_EQ(m_txPsdus[18].header.IsCts(), true, "Expected a CTS");
  NS_TEST_ASSERT_MSG_EQ(
      m_txPsdus[18].header.GetAddr1(),
      DynamicCast<WifiNetDevice>(m_apDevices.Get(0))->GetMac()->GetAddress(),
      "Expected a CTS frame sent to the AP");
  NS_TEST_ASSERT_MSG_EQ(m_txPsdus[18].header.GetDuration(),
                        RoundDurationId(m_txPsdus[17].header.GetDuration() -
                                        sifs - m_txPsdus[18].txDuration),
                        "Duration/ID of the CTS frame must be derived from "
                        "that of the RTS frame");

  tEnd = m_txPsdus[18].txStart + m_txPsdus[18].txDuration;
  tStart = m_txPsdus[19].txStart;

  NS_TEST_ASSERT_MSG_LT(tEnd + sifs, tStart,
                        "Second QoS data frame sent too early");
  NS_TEST_ASSERT_MSG_LT(tStart, tEnd + sifs + tolerance,
                        "Second QoS data frame sent too late");
  NS_TEST_ASSERT_MSG_EQ(m_txPsdus[19].header.IsQosData(), true,
                        "Expected a QoS data frame");
  NS_TEST_ASSERT_MSG_EQ(
      m_txPsdus[19].header.GetAddr1(),
      DynamicCast<WifiNetDevice>(m_staDevices.Get(1))->GetMac()->GetAddress(),
      "Expected a frame sent by the AP to the second station");
  NS_TEST_ASSERT_MSG_EQ(m_txPsdus[19].header.GetDuration(),
                        RoundDurationId(m_txopLimit -
                                        (m_txPsdus[19].txStart - txopStart) -
                                        m_txPsdus[19].txDuration),
                        "Duration/ID of the second QoS data frame does not "
                        "cover the remaining TXOP");

  tEnd = m_txPsdus[19].txStart + m_txPsdus[19].txDuration;
  tStart = m_txPsdus[20].txStart;

  NS_TEST_ASSERT_MSG_LT(
      tEnd + sifs, tStart,
      "Ack in response to the second QoS data frame sent too early");
  NS_TEST_ASSERT_MSG_LT(
      tStart, tEnd + sifs + tolerance,
      "Ack in response to the second QoS data frame sent too late");
  NS_TEST_ASSERT_MSG_EQ(m_txPsdus[20].header.IsAck(), true,
                        "Expected a Normal Ack");
  NS_TEST_ASSERT_MSG_EQ(
      m_txPsdus[20].header.GetAddr1(),
      DynamicCast<WifiNetDevice>(m_apDevices.Get(0))->GetMac()->GetAddress(),
      "Expected a Normal Ack sent to the AP");
  NS_TEST_ASSERT_MSG_EQ(
      m_txPsdus[20].header.GetDuration(),
      RoundDurationId(m_txPsdus[19].header.GetDuration() - sifs -
                      m_txPsdus[20].txDuration),
      "Duration/ID of the Ack must be derived from that of the previous frame");

  tEnd = m_txPsdus[20].txStart + m_txPsdus[20].txDuration;
  tStart = m_txPsdus[21].txStart;

  NS_TEST_ASSERT_MSG_LT(tEnd + sifs, tStart, "Third RTS frame sent too early");
  NS_TEST_ASSERT_MSG_LT(tStart, tEnd + sifs + tolerance,
                        "Third RTS frame sent too late");
  NS_TEST_ASSERT_MSG_EQ(m_txPsdus[21].header.IsRts(), true,
                        "Expected an RTS frame");
  NS_TEST_ASSERT_MSG_EQ(
      m_txPsdus[21].header.GetAddr1(),
      DynamicCast<WifiNetDevice>(m_staDevices.Get(2))->GetMac()->GetAddress(),
      "Expected an RTS frame sent by the AP to the third station");
  NS_TEST_ASSERT_MSG_EQ(
      m_txPsdus[21].header.GetDuration(),
      RoundDurationId(m_txopLimit - (m_txPsdus[21].txStart - txopStart) -
                      m_txPsdus[21].txDuration),
      "Duration/ID of the third RTS frame must cover the whole TXOP");

  tEnd = m_txPsdus[21].txStart + m_txPsdus[21].txDuration;
  tStart = m_txPsdus[22].txStart;

  NS_TEST_ASSERT_MSG_LT(
      tEnd + sifs, tStart,
      "CTS in response to the third RTS frame sent too early");
  NS_TEST_ASSERT_MSG_LT(tStart, tEnd + sifs + tolerance,
                        "CTS in response to the third RTS frame sent too late");
  NS_TEST_ASSERT_MSG_EQ(m_txPsdus[22].header.IsCts(), true, "Expected a CTS");
  NS_TEST_ASSERT_MSG_EQ(
      m_txPsdus[22].header.GetAddr1(),
      DynamicCast<WifiNetDevice>(m_apDevices.Get(0))->GetMac()->GetAddress(),
      "Expected a CTS frame sent to the AP");
  NS_TEST_ASSERT_MSG_EQ(m_txPsdus[22].header.GetDuration(),
                        RoundDurationId(m_txPsdus[21].header.GetDuration() -
                                        sifs - m_txPsdus[22].txDuration),
                        "Duration/ID of the CTS frame must be derived from "
                        "that of the RTS frame");

  tEnd = m_txPsdus[22].txStart + m_txPsdus[22].txDuration;
  tStart = m_txPsdus[23].txStart;

  NS_TEST_ASSERT_MSG_LT(tEnd + sifs, tStart,
                        "Third QoS data frame sent too early");
  NS_TEST_ASSERT_MSG_LT(tStart, tEnd + sifs + tolerance,
                        "Third QoS data frame sent too late");
  NS_TEST_ASSERT_MSG_EQ(m_txPsdus[23].header.IsQosData(), true,
                        "Expected a QoS data frame");
  NS_TEST_ASSERT_MSG_EQ(
      m_txPsdus[23].header.GetAddr1(),
      DynamicCast<WifiNetDevice>(m_staDevices.Get(2))->GetMac()->GetAddress(),
      "Expected a frame sent by the AP to the third station");
  NS_TEST_ASSERT_MSG_EQ(m_txPsdus[23].header.GetDuration(),
                        RoundDurationId(m_txopLimit -
                                        (m_txPsdus[23].txStart - txopStart) -
                                        m_txPsdus[23].txDuration),
                        "Duration/ID of the third QoS data frame does not "
                        "cover the remaining TXOP");

  tEnd = m_txPsdus[23].txStart + m_txPsdus[23].txDuration;
  tStart = m_txPsdus[24].txStart;

  NS_TEST_ASSERT_MSG_LT(
      tEnd + sifs, tStart,
      "Ack in response to the third QoS data frame sent too early");
  NS_TEST_ASSERT_MSG_LT(
      tStart, tEnd + sifs + tolerance,
      "Ack in response to the third QoS data frame sent too late");
  NS_TEST_ASSERT_MSG_EQ(m_txPsdus[24].header.IsAck(), true,
                        "Expected a Normal Ack");
  NS_TEST_ASSERT_MSG_EQ(
      m_txPsdus[24].header.GetAddr1(),
      DynamicCast<WifiNetDevice>(m_apDevices.Get(0))->GetMac()->GetAddress(),
      "Expected a Normal Ack sent to the AP");
  NS_TEST_ASSERT_MSG_EQ(
      m_txPsdus[24].header.GetDuration(),
      RoundDurationId(m_txPsdus[23].header.GetDuration() - sifs -
                      m_txPsdus[24].txDuration),
      "Duration/ID of the Ack must be derived from that of the previous frame");

  NS_TEST_ASSERT_MSG_EQ(m_received, 7, "Unexpected number of packets received");
}

class WifiTxopTestSuite : public TestSuite {
public:
  WifiTxopTestSuite();
};

WifiTxopTestSuite::WifiTxopTestSuite() : TestSuite("wifi-txop", UNIT) {
  AddTestCase(new WifiTxopTest(true), TestCase::QUICK);
  AddTestCase(new WifiTxopTest(false), TestCase::QUICK);
}

static WifiTxopTestSuite g_wifiTxopTestSuite;
