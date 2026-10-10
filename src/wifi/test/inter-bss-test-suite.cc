
#include "ns3/config.h"
#include "ns3/constant-obss-pd-algorithm.h"
#include "ns3/double.h"
#include "ns3/he-configuration.h"
#include "ns3/log.h"
#include "ns3/mobility-helper.h"
#include "ns3/multi-model-spectrum-channel.h"
#include "ns3/pointer.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/spectrum-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/string.h"
#include "ns3/test.h"
#include "ns3/uinteger.h"
#include "ns3/wifi-net-device.h"
#include "ns3/wifi-utils.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("InterBssTestSuite");

static uint32_t ConvertContextToNodeId(std::string context) {
  std::string sub = context.substr(10);
  uint32_t pos = sub.find("/Device");
  uint32_t nodeId = std::stoi(sub.substr(0, pos));
  return nodeId;
}

class TestInterBssConstantObssPdAlgo : public TestCase {
public:
  TestInterBssConstantObssPdAlgo(WifiStandard standard);
  ~TestInterBssConstantObssPdAlgo() override;

  void DoRun() override;

private:
  void SendOnePacket(Ptr<WifiNetDevice> tx_dev, Ptr<WifiNetDevice> rx_dev,
                     uint32_t payloadSize);

  Ptr<ListPositionAllocator> AllocatePositions(double d1, double d2, double d3,
                                               double d4, double d5);

  void SetExpectedTxPower(double txPowerDbm);

  void SetupSimulation();

  void CheckResults();

  void ResetResults();

  void ClearDropReasons();

  void RunOne();

  void CheckPhyState(Ptr<WifiNetDevice> device, WifiPhyState expectedState);

  void
  CheckPhyDropReasons(Ptr<WifiNetDevice> device,
                      std::vector<WifiPhyRxfailureReason> expectedDropReasons);

  void NotifyPhyTxBegin(std::string context, Ptr<const Packet> p,
                        double txPowerW);

  void NotifyPhyRxEnd(std::string context, Ptr<const Packet> p);

  void NotifyPhyRxDrop(std::string context, Ptr<const Packet> p,
                       WifiPhyRxfailureReason reason);

  unsigned int m_numSta1PacketsSent;
  unsigned int m_numSta2PacketsSent;
  unsigned int m_numAp1PacketsSent;
  unsigned int m_numAp2PacketsSent;

  unsigned int m_numSta1PacketsReceived;
  unsigned int m_numSta2PacketsReceived;
  unsigned int m_numAp1PacketsReceived;
  unsigned int m_numAp2PacketsReceived;

  std::vector<WifiPhyRxfailureReason> m_dropReasonsSta1;
  std::vector<WifiPhyRxfailureReason> m_dropReasonsSta2;
  std::vector<WifiPhyRxfailureReason> m_dropReasonsAp1;
  std::vector<WifiPhyRxfailureReason> m_dropReasonsAp2;

  unsigned int m_payloadSize1;
  unsigned int m_payloadSize2;
  unsigned int m_payloadSize3;

  NetDeviceContainer m_staDevices;
  NetDeviceContainer m_apDevices;

  double m_txPowerDbm;
  double m_obssPdLevelDbm;
  double m_obssRxPowerDbm;
  double m_expectedTxPowerDbm;

  uint8_t m_bssColor1;
  uint8_t m_bssColor2;
  uint8_t m_bssColor3;

  WifiStandard m_standard;
};

TestInterBssConstantObssPdAlgo::TestInterBssConstantObssPdAlgo(
    WifiStandard standard)
    : TestCase("InterBssConstantObssPd"), m_numSta1PacketsSent(0),
      m_numSta2PacketsSent(0), m_numAp1PacketsSent(0), m_numAp2PacketsSent(0),
      m_numSta1PacketsReceived(0), m_numSta2PacketsReceived(0),
      m_numAp1PacketsReceived(0), m_numAp2PacketsReceived(0),
      m_payloadSize1(1000), m_payloadSize2(1500), m_payloadSize3(2000),
      m_txPowerDbm(15), m_obssPdLevelDbm(-72), m_obssRxPowerDbm(-82),
      m_expectedTxPowerDbm(15), m_bssColor1(1), m_bssColor2(2), m_bssColor3(3),
      m_standard(standard) {}

TestInterBssConstantObssPdAlgo::~TestInterBssConstantObssPdAlgo() {
  ClearDropReasons();
}

Ptr<ListPositionAllocator> TestInterBssConstantObssPdAlgo::AllocatePositions(
    double d1, double d2, double d3, double d4, double d5) {
  Ptr<ListPositionAllocator> positionAlloc =
      CreateObject<ListPositionAllocator>();
  positionAlloc->Add(Vector(0.0, 0.0, 0.0));
  positionAlloc->Add(Vector(d1 + d2, 0.0, 0.0));
  positionAlloc->Add(Vector(d1 + d2 + d3 + d4, 0.0, 0.0));
  positionAlloc->Add(Vector(d1, 0.0, 0.0));
  positionAlloc->Add(Vector(d1 + d2 + d3, 0.0, 0.0));
  positionAlloc->Add(Vector(d1 + d2 + d3 + d4 + d5, 0.0, 0.0));
  return positionAlloc;
}

void TestInterBssConstantObssPdAlgo::SetupSimulation() {
  Ptr<WifiNetDevice> ap_device1 =
      DynamicCast<WifiNetDevice>(m_apDevices.Get(0));
  Ptr<WifiNetDevice> ap_device2 =
      DynamicCast<WifiNetDevice>(m_apDevices.Get(1));
  Ptr<WifiNetDevice> ap_device3 =
      DynamicCast<WifiNetDevice>(m_apDevices.Get(2));
  Ptr<WifiNetDevice> sta_device1 =
      DynamicCast<WifiNetDevice>(m_staDevices.Get(0));
  Ptr<WifiNetDevice> sta_device2 =
      DynamicCast<WifiNetDevice>(m_staDevices.Get(1));
  Ptr<WifiNetDevice> sta_device3 =
      DynamicCast<WifiNetDevice>(m_staDevices.Get(2));

  bool expectFilter = (m_bssColor1 != 0) && (m_bssColor2 != 0);
  bool expectPhyReset = expectFilter && (m_obssPdLevelDbm >= m_obssRxPowerDbm);
  std::vector<WifiPhyRxfailureReason> dropReasons;
  WifiPhyState stateDuringPayloadNeighboringBss =
      expectFilter ? WifiPhyState::CCA_BUSY : WifiPhyState::RX;
  if (expectFilter) {
    dropReasons.push_back(FILTERED);
  }
  if (expectPhyReset) {
    dropReasons.push_back(OBSS_PD_CCA_RESET);
  }

  Simulator::Schedule(Seconds(0.25),
                      &TestInterBssConstantObssPdAlgo::SendOnePacket, this,
                      ap_device1, sta_device1, m_payloadSize1);
  Simulator::Schedule(Seconds(0.5),
                      &TestInterBssConstantObssPdAlgo::SendOnePacket, this,
                      sta_device1, ap_device1, m_payloadSize1);
  Simulator::Schedule(Seconds(0.75),
                      &TestInterBssConstantObssPdAlgo::SendOnePacket, this,
                      ap_device2, sta_device2, m_payloadSize2);
  Simulator::Schedule(Seconds(1),
                      &TestInterBssConstantObssPdAlgo::SendOnePacket, this,
                      sta_device2, ap_device2, m_payloadSize2);
  Simulator::Schedule(Seconds(1.25),
                      &TestInterBssConstantObssPdAlgo::SendOnePacket, this,
                      ap_device3, sta_device3, m_payloadSize3);
  Simulator::Schedule(Seconds(1.5),
                      &TestInterBssConstantObssPdAlgo::SendOnePacket, this,
                      sta_device3, ap_device3, m_payloadSize3);

  Simulator::Schedule(Seconds(2.0),
                      &TestInterBssConstantObssPdAlgo::ClearDropReasons, this);
  Simulator::Schedule(Seconds(2.0),
                      &TestInterBssConstantObssPdAlgo::SendOnePacket, this,
                      ap_device2, sta_device2, m_payloadSize2);
  Simulator::Schedule(Seconds(2.0) + MicroSeconds(10),
                      &TestInterBssConstantObssPdAlgo::CheckPhyState, this,
                      ap_device2, WifiPhyState::TX);
  Simulator::Schedule(Seconds(2.0) + MicroSeconds(13),
                      &TestInterBssConstantObssPdAlgo::CheckPhyState, this,
                      sta_device1, WifiPhyState::IDLE);
  Simulator::Schedule(Seconds(2.0) + MicroSeconds(13),
                      &TestInterBssConstantObssPdAlgo::CheckPhyState, this,
                      sta_device2, WifiPhyState::IDLE);
  Simulator::Schedule(Seconds(2.0) + MicroSeconds(13),
                      &TestInterBssConstantObssPdAlgo::CheckPhyState, this,
                      ap_device1, WifiPhyState::IDLE);
  Simulator::Schedule(Seconds(2.0) + MicroSeconds(14),
                      &TestInterBssConstantObssPdAlgo::CheckPhyState, this,
                      sta_device1, WifiPhyState::CCA_BUSY);
  Simulator::Schedule(Seconds(2.0) + MicroSeconds(14),
                      &TestInterBssConstantObssPdAlgo::CheckPhyState, this,
                      sta_device2, WifiPhyState::CCA_BUSY);
  Simulator::Schedule(Seconds(2.0) + MicroSeconds(14),
                      &TestInterBssConstantObssPdAlgo::CheckPhyState, this,
                      ap_device1, WifiPhyState::CCA_BUSY);
  Simulator::Schedule(Seconds(2.0) + MicroSeconds(43),
                      &TestInterBssConstantObssPdAlgo::CheckPhyDropReasons,
                      this, sta_device1, dropReasons);
  Simulator::Schedule(
      Seconds(2.0) + MicroSeconds(43),
      &TestInterBssConstantObssPdAlgo::CheckPhyState, this, sta_device1,
      expectPhyReset ? WifiPhyState::IDLE : WifiPhyState::CCA_BUSY);
  Simulator::Schedule(Seconds(2.0) + MicroSeconds(43),
                      &TestInterBssConstantObssPdAlgo::CheckPhyDropReasons,
                      this, ap_device1, dropReasons);
  Simulator::Schedule(
      Seconds(2.0) + MicroSeconds(43),
      &TestInterBssConstantObssPdAlgo::CheckPhyState, this, ap_device1,
      expectPhyReset ? WifiPhyState::IDLE : WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(2.0) + MicroSeconds(54),
      &TestInterBssConstantObssPdAlgo::CheckPhyState, this, sta_device1,
      expectPhyReset ? WifiPhyState::IDLE : stateDuringPayloadNeighboringBss);
  Simulator::Schedule(
      Seconds(2.0) + MicroSeconds(54),
      &TestInterBssConstantObssPdAlgo::CheckPhyState, this, ap_device1,
      expectPhyReset ? WifiPhyState::IDLE : stateDuringPayloadNeighboringBss);
  Simulator::Schedule(Seconds(2.0) + MicroSeconds(54),
                      &TestInterBssConstantObssPdAlgo::CheckPhyState, this,
                      sta_device2, WifiPhyState::RX);

  Simulator::Schedule(Seconds(2.1),
                      &TestInterBssConstantObssPdAlgo::ClearDropReasons, this);
  Simulator::Schedule(Seconds(2.1),
                      &TestInterBssConstantObssPdAlgo::SendOnePacket, this,
                      ap_device2, sta_device2, m_payloadSize2);
  Simulator::Schedule(Seconds(2.1) + MicroSeconds(42),
                      &TestInterBssConstantObssPdAlgo::SendOnePacket, this,
                      sta_device1, ap_device1, m_payloadSize1);
  if (expectPhyReset) {
    double expectedTxPower =
        std::min(m_txPowerDbm, 21 - (m_obssPdLevelDbm + 82));
    Simulator::Schedule(Seconds(2.1) + MicroSeconds(41),
                        &TestInterBssConstantObssPdAlgo::SetExpectedTxPower,
                        this, expectedTxPower);
  }
  Simulator::Schedule(Seconds(2.1) + MicroSeconds(100),
                      &TestInterBssConstantObssPdAlgo::CheckPhyState, this,
                      ap_device2, WifiPhyState::TX);
  Simulator::Schedule(Seconds(2.1) + MicroSeconds(100),
                      &TestInterBssConstantObssPdAlgo::CheckPhyDropReasons,
                      this, sta_device1, dropReasons);
  Simulator::Schedule(
      Seconds(2.1) + MicroSeconds(100),
      &TestInterBssConstantObssPdAlgo::CheckPhyState, this, sta_device1,
      expectPhyReset ? WifiPhyState::TX : stateDuringPayloadNeighboringBss);
  Simulator::Schedule(Seconds(2.1) + MicroSeconds(100),
                      &TestInterBssConstantObssPdAlgo::CheckPhyState, this,
                      sta_device2, WifiPhyState::RX);
  Simulator::Schedule(Seconds(2.1) + MicroSeconds(100),
                      &TestInterBssConstantObssPdAlgo::CheckPhyDropReasons,
                      this, ap_device1, dropReasons);
  Simulator::Schedule(Seconds(2.1) + MicroSeconds(100),
                      &TestInterBssConstantObssPdAlgo::CheckPhyState, this,
                      ap_device1, stateDuringPayloadNeighboringBss);
  Simulator::Schedule(
      Seconds(2.1) + MicroSeconds(142),
      &TestInterBssConstantObssPdAlgo::CheckPhyState, this, ap_device1,
      expectPhyReset ? WifiPhyState::RX : stateDuringPayloadNeighboringBss);

  Simulator::Schedule(Seconds(2.2),
                      &TestInterBssConstantObssPdAlgo::ClearDropReasons, this);
  Simulator::Schedule(Seconds(2.2),
                      &TestInterBssConstantObssPdAlgo::SetExpectedTxPower, this,
                      m_txPowerDbm);
  Simulator::Schedule(Seconds(2.2),
                      &TestInterBssConstantObssPdAlgo::SendOnePacket, this,
                      ap_device2, sta_device2, m_payloadSize2);
  Simulator::Schedule(Seconds(2.2) + MicroSeconds(90),
                      &TestInterBssConstantObssPdAlgo::SendOnePacket, this,
                      sta_device1, ap_device1, m_payloadSize1);
  if (expectPhyReset) {
    double expectedTxPower =
        std::min(m_txPowerDbm, 21 - (m_obssPdLevelDbm + 82));
    Simulator::Schedule(Seconds(2.2) + MicroSeconds(89),
                        &TestInterBssConstantObssPdAlgo::SetExpectedTxPower,
                        this, expectedTxPower);
  }
  Simulator::Schedule(Seconds(2.2) + MicroSeconds(105),
                      &TestInterBssConstantObssPdAlgo::CheckPhyState, this,
                      ap_device2, WifiPhyState::TX);
  Simulator::Schedule(Seconds(2.2) + MicroSeconds(105),
                      &TestInterBssConstantObssPdAlgo::CheckPhyDropReasons,
                      this, sta_device1, dropReasons);
  Simulator::Schedule(
      Seconds(2.2) + MicroSeconds(105),
      &TestInterBssConstantObssPdAlgo::CheckPhyState, this, sta_device1,
      expectPhyReset ? WifiPhyState::TX : stateDuringPayloadNeighboringBss);
  Simulator::Schedule(Seconds(2.2) + MicroSeconds(105),
                      &TestInterBssConstantObssPdAlgo::CheckPhyState, this,
                      sta_device2, WifiPhyState::RX);
  Simulator::Schedule(Seconds(2.2) + MicroSeconds(105),
                      &TestInterBssConstantObssPdAlgo::CheckPhyDropReasons,
                      this, ap_device1, dropReasons);
  Simulator::Schedule(Seconds(2.2) + MicroSeconds(105),
                      &TestInterBssConstantObssPdAlgo::CheckPhyState, this,
                      ap_device1, stateDuringPayloadNeighboringBss);
  Simulator::Schedule(
      Seconds(2.2) + MicroSeconds(195),
      &TestInterBssConstantObssPdAlgo::CheckPhyState, this, ap_device1,
      expectPhyReset ? WifiPhyState::RX : stateDuringPayloadNeighboringBss);

  Simulator::Schedule(Seconds(2.3),
                      &TestInterBssConstantObssPdAlgo::SetExpectedTxPower, this,
                      m_txPowerDbm);
  Simulator::Schedule(Seconds(2.3),
                      &TestInterBssConstantObssPdAlgo::SendOnePacket, this,
                      ap_device2, sta_device2, m_payloadSize2);
  Simulator::Schedule(Seconds(2.4),
                      &TestInterBssConstantObssPdAlgo::SendOnePacket, this,
                      sta_device1, ap_device1, m_payloadSize1);

  Simulator::Schedule(Seconds(2.5),
                      &TestInterBssConstantObssPdAlgo::SendOnePacket, this,
                      sta_device2, ap_device2, m_payloadSize2 / 10);
  Simulator::Schedule(Seconds(2.5) + MicroSeconds(15),
                      &TestInterBssConstantObssPdAlgo::SendOnePacket, this,
                      ap_device2, sta_device2, m_payloadSize2 / 10);
  Simulator::Schedule(Seconds(2.5) + MicroSeconds(270),
                      &TestInterBssConstantObssPdAlgo::SendOnePacket, this,
                      ap_device1, sta_device1, m_payloadSize1 / 10);
  Simulator::Schedule(Seconds(2.5) + MicroSeconds(300),
                      &TestInterBssConstantObssPdAlgo::SendOnePacket, this,
                      ap_device3, sta_device3, m_payloadSize3 / 10);
  if (expectPhyReset) {
    double expectedTxPower =
        std::min(m_txPowerDbm, 21 - (m_obssPdLevelDbm + 82));
    Simulator::Schedule(Seconds(2.5) + MicroSeconds(338),
                        &TestInterBssConstantObssPdAlgo::SetExpectedTxPower,
                        this, expectedTxPower);
  }

  Simulator::Stop(Seconds(2.6));
}

void TestInterBssConstantObssPdAlgo::ResetResults() {
  m_numSta1PacketsSent = 0;
  m_numSta2PacketsSent = 0;
  m_numAp1PacketsSent = 0;
  m_numAp2PacketsSent = 0;
  m_numSta1PacketsReceived = 0;
  m_numSta2PacketsReceived = 0;
  m_numAp1PacketsReceived = 0;
  m_numAp2PacketsReceived = 0;
  ClearDropReasons();
  m_expectedTxPowerDbm = m_txPowerDbm;
}

void TestInterBssConstantObssPdAlgo::ClearDropReasons() {
  m_dropReasonsSta1.clear();
  m_dropReasonsSta2.clear();
  m_dropReasonsAp1.clear();
  m_dropReasonsAp2.clear();
}

void TestInterBssConstantObssPdAlgo::CheckResults() {
  NS_TEST_ASSERT_MSG_EQ(m_numSta1PacketsSent, 4,
                        "The number of packets sent by STA1 is not correct!");
  NS_TEST_ASSERT_MSG_EQ(m_numSta2PacketsSent, 2,
                        "The number of packets sent by STA2 is not correct!");
  NS_TEST_ASSERT_MSG_EQ(m_numAp1PacketsSent, 2,
                        "The number of packets sent by AP1 is not correct!");
  NS_TEST_ASSERT_MSG_EQ(m_numAp2PacketsSent, 6,
                        "The number of packets sent by AP2 is not correct!");
  NS_TEST_ASSERT_MSG_EQ(
      m_numSta1PacketsReceived, 2,
      "The number of packets received by STA1 is not correct!");
  NS_TEST_ASSERT_MSG_EQ(
      m_numSta2PacketsReceived, 6,
      "The number of packets received by STA2 is not correct!");
  NS_TEST_ASSERT_MSG_EQ(
      m_numAp1PacketsReceived, 4,
      "The number of packets received by AP1 is not correct!");
  NS_TEST_ASSERT_MSG_EQ(
      m_numAp2PacketsReceived, 2,
      "The number of packets received by AP2 is not correct!");
}

void TestInterBssConstantObssPdAlgo::NotifyPhyTxBegin(std::string context,
                                                      Ptr<const Packet> p,
                                                      double txPowerW) {
  uint32_t idx = ConvertContextToNodeId(context);
  uint32_t pktSize = p->GetSize() - 38;
  if ((idx == 0) &&
      ((pktSize == m_payloadSize1) || (pktSize == (m_payloadSize1 / 10)))) {
    m_numSta1PacketsSent++;
    NS_TEST_EXPECT_MSG_EQ(
        TestDoubleIsEqual(WToDbm(txPowerW), m_expectedTxPowerDbm, 1e-12), true,
        "Tx power is not correct!");
  } else if ((idx == 1) && ((pktSize == m_payloadSize2) ||
                            (pktSize == (m_payloadSize2 / 10)))) {
    m_numSta2PacketsSent++;
    NS_TEST_EXPECT_MSG_EQ(
        TestDoubleIsEqual(WToDbm(txPowerW), m_expectedTxPowerDbm, 1e-12), true,
        "Tx power is not correct!");
  } else if ((idx == 3) && ((pktSize == m_payloadSize1) ||
                            (pktSize == (m_payloadSize1 / 10)))) {
    m_numAp1PacketsSent++;
    NS_TEST_EXPECT_MSG_EQ(
        TestDoubleIsEqual(WToDbm(txPowerW), m_expectedTxPowerDbm, 1e-12), true,
        "Tx power is not correct!");
  } else if ((idx == 4) && ((pktSize == m_payloadSize2) ||
                            (pktSize == (m_payloadSize2 / 10)))) {
    m_numAp2PacketsSent++;
    NS_TEST_EXPECT_MSG_EQ(
        TestDoubleIsEqual(WToDbm(txPowerW), m_expectedTxPowerDbm, 1e-12), true,
        "Tx power is not correct!");
  }
}

void TestInterBssConstantObssPdAlgo::NotifyPhyRxEnd(std::string context,
                                                    Ptr<const Packet> p) {
  uint32_t idx = ConvertContextToNodeId(context);
  uint32_t pktSize = p->GetSize() - 38;
  if ((idx == 0) &&
      ((pktSize == m_payloadSize1) || (pktSize == (m_payloadSize1 / 10)))) {
    m_numSta1PacketsReceived++;
  } else if ((idx == 1) && ((pktSize == m_payloadSize2) ||
                            (pktSize == (m_payloadSize2 / 10)))) {
    m_numSta2PacketsReceived++;
  } else if ((idx == 3) && ((pktSize == m_payloadSize1) ||
                            (pktSize == (m_payloadSize1 / 10)))) {
    m_numAp1PacketsReceived++;
  } else if ((idx == 4) && ((pktSize == m_payloadSize2) ||
                            (pktSize == (m_payloadSize2 / 10)))) {
    m_numAp2PacketsReceived++;
  }
}

void TestInterBssConstantObssPdAlgo::NotifyPhyRxDrop(
    std::string context, Ptr<const Packet> p, WifiPhyRxfailureReason reason) {
  uint32_t idx = ConvertContextToNodeId(context);
  uint32_t pktSize = p->GetSize() - 38;
  if ((idx == 0) &&
      ((pktSize != m_payloadSize1) && (pktSize != (m_payloadSize1 / 10)))) {
    m_dropReasonsSta1.push_back(reason);
  } else if ((idx == 1) && ((pktSize != m_payloadSize2) &&
                            (pktSize != (m_payloadSize2 / 10)))) {
    m_dropReasonsSta2.push_back(reason);
  } else if ((idx == 3) && ((pktSize != m_payloadSize1) &&
                            (pktSize != (m_payloadSize1 / 10)))) {
    m_dropReasonsAp1.push_back(reason);
  } else if ((idx == 4) && ((pktSize != m_payloadSize2) &&
                            (pktSize != (m_payloadSize2 / 10)))) {
    m_dropReasonsAp2.push_back(reason);
  }
}

void TestInterBssConstantObssPdAlgo::SendOnePacket(Ptr<WifiNetDevice> tx_dev,
                                                   Ptr<WifiNetDevice> rx_dev,
                                                   uint32_t payloadSize) {
  Ptr<Packet> p = Create<Packet>(payloadSize);
  tx_dev->Send(p, rx_dev->GetAddress(), 1);
}

void TestInterBssConstantObssPdAlgo::SetExpectedTxPower(double txPowerDbm) {
  m_expectedTxPowerDbm = txPowerDbm;
}

void TestInterBssConstantObssPdAlgo::CheckPhyState(Ptr<WifiNetDevice> device,
                                                   WifiPhyState expectedState) {
  WifiPhyState currentState;
  PointerValue ptr;
  Ptr<WifiPhy> phy = DynamicCast<WifiPhy>(device->GetPhy());
  phy->GetAttribute("State", ptr);
  Ptr<WifiPhyStateHelper> state =
      DynamicCast<WifiPhyStateHelper>(ptr.Get<WifiPhyStateHelper>());
  currentState = state->GetState();
  NS_TEST_ASSERT_MSG_EQ(currentState, expectedState,
                        "PHY State "
                            << currentState << " does not match expected state "
                            << expectedState << " at " << Simulator::Now());
}

void TestInterBssConstantObssPdAlgo::CheckPhyDropReasons(
    Ptr<WifiNetDevice> device,
    std::vector<WifiPhyRxfailureReason> expectedDropReasons) {
  std::vector<WifiPhyRxfailureReason> currentDropReasons;
  uint32_t nodeId = device->GetNode()->GetId();
  switch (nodeId) {
  case 0:
    currentDropReasons = m_dropReasonsSta1;
    break;
  case 1:
    currentDropReasons = m_dropReasonsSta2;
    break;
  case 3:
    currentDropReasons = m_dropReasonsAp1;
    break;
  case 4:
    currentDropReasons = m_dropReasonsAp2;
    break;
  default:
    return;
  }
  NS_TEST_ASSERT_MSG_EQ(
      currentDropReasons.size(), expectedDropReasons.size(),
      "Number of drop reasons "
          << currentDropReasons.size() << " does not match expected one "
          << expectedDropReasons.size() << " at " << Simulator::Now());
  for (std::size_t i = 0; i < currentDropReasons.size(); ++i) {
    NS_TEST_ASSERT_MSG_EQ(currentDropReasons[i], expectedDropReasons[i],
                          "Drop reason " << i << ": " << currentDropReasons[i]
                                         << " does not match expected reason "
                                         << expectedDropReasons[i] << " at "
                                         << Simulator::Now());
  }
}

void TestInterBssConstantObssPdAlgo::RunOne() {
  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(3);
  int64_t streamNumber = 50;

  Config::Set(
      "/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/BE_MaxAmpduSize",
      UintegerValue(0));

  ResetResults();

  NodeContainer wifiStaNodes;
  wifiStaNodes.Create(3);

  NodeContainer wifiApNodes;
  wifiApNodes.Create(3);

  Ptr<MatrixPropagationLossModel> lossModel =
      CreateObject<MatrixPropagationLossModel>();
  lossModel->SetDefaultLoss(m_txPowerDbm - m_obssRxPowerDbm);

  SpectrumWifiPhyHelper phy;
  phy.DisablePreambleDetectionModel();
  phy.SetFrameCaptureModel("ns3::SimpleFrameCaptureModel");
  Ptr<MultiModelSpectrumChannel> channel =
      CreateObject<MultiModelSpectrumChannel>();
  channel->SetPropagationDelayModel(
      CreateObject<ConstantSpeedPropagationDelayModel>());
  channel->AddPropagationLossModel(lossModel);
  phy.SetChannel(channel);
  phy.Set("TxPowerStart", DoubleValue(m_txPowerDbm));
  phy.Set("TxPowerEnd", DoubleValue(m_txPowerDbm));
  phy.Set("ChannelSettings", StringValue("{36, 20, BAND_5GHZ, 0}"));

  WifiHelper wifi;
  wifi.SetStandard(m_standard);
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode",
                               StringValue("HeMcs5"), "ControlMode",
                               StringValue("HeMcs0"));

  wifi.SetObssPdAlgorithm("ns3::ConstantObssPdAlgorithm", "ObssPdLevel",
                          DoubleValue(m_obssPdLevelDbm));

  WifiMacHelper mac;
  Ssid ssid = Ssid("ns-3-ssid");
  mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid));
  m_staDevices = wifi.Install(phy, mac, wifiStaNodes);

  wifi.AssignStreams(m_staDevices, streamNumber);

  mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
  m_apDevices = wifi.Install(phy, mac, wifiApNodes);

  wifi.AssignStreams(m_apDevices, streamNumber);

  for (uint32_t i = 0; i < m_apDevices.GetN(); i++) {
    Ptr<WifiNetDevice> device = DynamicCast<WifiNetDevice>(m_apDevices.Get(i));
    Ptr<HeConfiguration> heConfiguration = device->GetHeConfiguration();
    if (i == 0) {
      heConfiguration->SetAttribute("BssColor", UintegerValue(m_bssColor1));
    } else if (i == 1) {
      heConfiguration->SetAttribute("BssColor", UintegerValue(m_bssColor2));
    } else {
      heConfiguration->SetAttribute("BssColor", UintegerValue(m_bssColor3));
    }
  }

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc =
      AllocatePositions(10, 50, 10, 50, 10);
  mobility.SetPositionAllocator(positionAlloc);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(wifiApNodes);
  mobility.Install(wifiStaNodes);

  lossModel->SetLoss(wifiStaNodes.Get(0)->GetObject<MobilityModel>(),
                     wifiApNodes.Get(0)->GetObject<MobilityModel>(),
                     m_txPowerDbm + 30);
  lossModel->SetLoss(wifiStaNodes.Get(1)->GetObject<MobilityModel>(),
                     wifiApNodes.Get(1)->GetObject<MobilityModel>(),
                     m_txPowerDbm + 30);
  lossModel->SetLoss(wifiStaNodes.Get(2)->GetObject<MobilityModel>(),
                     wifiApNodes.Get(2)->GetObject<MobilityModel>(),
                     m_txPowerDbm + 30);

  Config::Connect(
      "/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyTxBegin",
      MakeCallback(&TestInterBssConstantObssPdAlgo::NotifyPhyTxBegin, this));
  Config::Connect(
      "/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyRxEnd",
      MakeCallback(&TestInterBssConstantObssPdAlgo::NotifyPhyRxEnd, this));
  Config::Connect(
      "/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyRxDrop",
      MakeCallback(&TestInterBssConstantObssPdAlgo::NotifyPhyRxDrop, this));

  SetupSimulation();

  Simulator::Run();
  Simulator::Destroy();

  CheckResults();
}

void TestInterBssConstantObssPdAlgo::DoRun() {
  m_obssPdLevelDbm = -72;
  m_obssRxPowerDbm = -82;
  m_bssColor1 = 1;
  m_bssColor2 = 2;
  m_bssColor3 = 3;
  RunOne();

  m_obssPdLevelDbm = -72;
  m_obssRxPowerDbm = -62;
  m_bssColor1 = 1;
  m_bssColor2 = 2;
  m_bssColor3 = 3;
  RunOne();

  m_obssPdLevelDbm = -72;
  m_obssRxPowerDbm = -72;
  m_bssColor1 = 1;
  m_bssColor2 = 2;
  m_bssColor3 = 3;
  RunOne();

  m_obssPdLevelDbm = -72;
  m_obssRxPowerDbm = -82;
  m_bssColor1 = 1;
  m_bssColor2 = 0;
  m_bssColor3 = 0;
  RunOne();

  m_obssPdLevelDbm = -72;
  m_obssRxPowerDbm = -82;
  m_bssColor1 = 0;
  m_bssColor2 = 2;
  m_bssColor3 = 3;
  RunOne();
}

class InterBssTestSuite : public TestSuite {
public:
  InterBssTestSuite();
};

InterBssTestSuite::InterBssTestSuite() : TestSuite("wifi-inter-bss", UNIT) {
  AddTestCase(new TestInterBssConstantObssPdAlgo(WIFI_STANDARD_80211ax),
              TestCase::QUICK);
  AddTestCase(new TestInterBssConstantObssPdAlgo(WIFI_STANDARD_80211be),
              TestCase::QUICK);
}

static InterBssTestSuite interBssTestSuite;
