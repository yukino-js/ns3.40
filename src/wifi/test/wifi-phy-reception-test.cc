
#include "ns3/ampdu-tag.h"
#include "ns3/ap-wifi-mac.h"
#include "ns3/boolean.h"
#include "ns3/config.h"
#include "ns3/double.h"
#include "ns3/he-phy.h"
#include "ns3/he-ppdu.h"
#include "ns3/interference-helper.h"
#include "ns3/log.h"
#include "ns3/mobility-helper.h"
#include "ns3/mpdu-aggregator.h"
#include "ns3/multi-model-spectrum-channel.h"
#include "ns3/nist-error-rate-model.h"
#include "ns3/packet-socket-address.h"
#include "ns3/packet-socket-client.h"
#include "ns3/packet-socket-helper.h"
#include "ns3/packet-socket-server.h"
#include "ns3/pointer.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/simple-frame-capture-model.h"
#include "ns3/single-model-spectrum-channel.h"
#include "ns3/spectrum-wifi-helper.h"
#include "ns3/spectrum-wifi-phy.h"
#include "ns3/test.h"
#include "ns3/threshold-preamble-detection-model.h"
#include "ns3/wifi-bandwidth-filter.h"
#include "ns3/wifi-mac-header.h"
#include "ns3/wifi-mpdu.h"
#include "ns3/wifi-net-device.h"
#include "ns3/wifi-psdu.h"
#include "ns3/wifi-spectrum-phy-interface.h"
#include "ns3/wifi-spectrum-signal-parameters.h"
#include "ns3/wifi-spectrum-value-helper.h"
#include "ns3/wifi-utils.h"

#include <optional>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("WifiPhyReceptionTest");

static const uint8_t CHANNEL_NUMBER = 36;
static const uint32_t FREQUENCY = 5180;
static const uint16_t CHANNEL_WIDTH = 20;
static const uint16_t GUARD_WIDTH = CHANNEL_WIDTH;

class WifiPhyReceptionTest : public TestCase {
public:
  WifiPhyReceptionTest(std::string test_name);
  ~WifiPhyReceptionTest() override = default;

protected:
  void DoSetup() override;
  void DoTeardown() override;

  void SendPacket(double rxPowerDbm, uint32_t packetSize, uint8_t mcs);

  void CheckPhyState(WifiPhyState expectedState);
  void DoCheckPhyState(WifiPhyState expectedState);

  Ptr<SpectrumWifiPhy> m_phy;
  uint64_t m_uid{0};
};

WifiPhyReceptionTest::WifiPhyReceptionTest(std::string test_name)
    : TestCase{test_name} {}

void WifiPhyReceptionTest::SendPacket(double rxPowerDbm, uint32_t packetSize,
                                      uint8_t mcs) {
  WifiTxVector txVector = WifiTxVector(
      HePhy::GetHeMcs(mcs), 0, WIFI_PREAMBLE_HE_SU, 800, 1, 1, 0, 20, false);

  Ptr<Packet> pkt = Create<Packet>(packetSize);
  WifiMacHeader hdr;

  hdr.SetType(WIFI_MAC_QOSDATA);
  hdr.SetQosTid(0);

  Ptr<WifiPsdu> psdu = Create<WifiPsdu>(pkt, hdr);
  Time txDuration = m_phy->CalculateTxDuration(psdu->GetSize(), txVector,
                                               m_phy->GetPhyBand());

  Ptr<WifiPpdu> ppdu = Create<HePpdu>(
      psdu, txVector, m_phy->GetOperatingChannel(), txDuration, m_uid++);

  Ptr<SpectrumValue> txPowerSpectrum =
      WifiSpectrumValueHelper::CreateHeOfdmTxPowerSpectralDensity(
          FREQUENCY, CHANNEL_WIDTH, DbmToW(rxPowerDbm), GUARD_WIDTH);

  Ptr<WifiSpectrumSignalParameters> txParams =
      Create<WifiSpectrumSignalParameters>();
  txParams->psd = txPowerSpectrum;
  txParams->txPhy = nullptr;
  txParams->duration = txDuration;
  txParams->ppdu = ppdu;

  m_phy->StartRx(txParams, nullptr);
}

void WifiPhyReceptionTest::CheckPhyState(WifiPhyState expectedState) {
  Simulator::ScheduleNow(&WifiPhyReceptionTest::DoCheckPhyState, this,
                         expectedState);
}

void WifiPhyReceptionTest::DoCheckPhyState(WifiPhyState expectedState) {
  WifiPhyState currentState;
  PointerValue ptr;
  m_phy->GetAttribute("State", ptr);
  Ptr<WifiPhyStateHelper> state =
      DynamicCast<WifiPhyStateHelper>(ptr.Get<WifiPhyStateHelper>());
  currentState = state->GetState();
  NS_LOG_FUNCTION(this << currentState);
  NS_TEST_ASSERT_MSG_EQ(currentState, expectedState,
                        "PHY State "
                            << currentState << " does not match expected state "
                            << expectedState << " at " << Simulator::Now());
}

void WifiPhyReceptionTest::DoSetup() {
  Ptr<MultiModelSpectrumChannel> spectrumChannel =
      CreateObject<MultiModelSpectrumChannel>();
  Ptr<Node> node = CreateObject<Node>();
  Ptr<WifiNetDevice> dev = CreateObject<WifiNetDevice>();
  m_phy = CreateObject<SpectrumWifiPhy>();
  Ptr<InterferenceHelper> interferenceHelper =
      CreateObject<InterferenceHelper>();
  m_phy->SetInterferenceHelper(interferenceHelper);
  Ptr<ErrorRateModel> error = CreateObject<NistErrorRateModel>();
  m_phy->SetErrorRateModel(error);
  m_phy->SetDevice(dev);
  m_phy->AddChannel(spectrumChannel);
  m_phy->SetOperatingChannel(
      WifiPhy::ChannelTuple{CHANNEL_NUMBER, 0, WIFI_PHY_BAND_5GHZ, 0});
  m_phy->ConfigureStandard(WIFI_STANDARD_80211ax);
  dev->SetPhy(m_phy);
  node->AddDevice(dev);
}

void WifiPhyReceptionTest::DoTeardown() {
  m_phy->Dispose();
  m_phy = nullptr;
}

class TestThresholdPreambleDetectionWithoutFrameCapture
    : public WifiPhyReceptionTest {
public:
  TestThresholdPreambleDetectionWithoutFrameCapture();

protected:
  void DoSetup() override;

  void RxSuccess(Ptr<const WifiPsdu> psdu, RxSignalInfo rxSignalInfo,
                 WifiTxVector txVector, std::vector<bool> statusPerMpdu);
  void RxFailure(Ptr<const WifiPsdu> psdu);
  uint32_t m_countRxSuccess{0};
  uint32_t m_countRxFailure{0};

private:
  void DoRun() override;

  void CheckRxPacketCount(uint32_t expectedSuccessCount,
                          uint32_t expectedFailureCount);
};

TestThresholdPreambleDetectionWithoutFrameCapture::
    TestThresholdPreambleDetectionWithoutFrameCapture()
    : WifiPhyReceptionTest("Threshold preamble detection model test when no "
                           "frame capture model is applied") {}

void TestThresholdPreambleDetectionWithoutFrameCapture::CheckRxPacketCount(
    uint32_t expectedSuccessCount, uint32_t expectedFailureCount) {
  NS_TEST_ASSERT_MSG_EQ(m_countRxSuccess, expectedSuccessCount,
                        "Didn't receive right number of successful packets");
  NS_TEST_ASSERT_MSG_EQ(m_countRxFailure, expectedFailureCount,
                        "Didn't receive right number of unsuccessful packets");
}

void TestThresholdPreambleDetectionWithoutFrameCapture::RxSuccess(
    Ptr<const WifiPsdu> psdu, RxSignalInfo rxSignalInfo, WifiTxVector txVector,
    std::vector<bool> statusPerMpdu) {
  NS_LOG_FUNCTION(this << *psdu << rxSignalInfo << txVector);
  m_countRxSuccess++;
}

void TestThresholdPreambleDetectionWithoutFrameCapture::RxFailure(
    Ptr<const WifiPsdu> psdu) {
  NS_LOG_FUNCTION(this << *psdu);
  m_countRxFailure++;
}

void TestThresholdPreambleDetectionWithoutFrameCapture::DoSetup() {
  WifiPhyReceptionTest::DoSetup();

  m_phy->SetReceiveOkCallback(MakeCallback(
      &TestThresholdPreambleDetectionWithoutFrameCapture::RxSuccess, this));
  m_phy->SetReceiveErrorCallback(MakeCallback(
      &TestThresholdPreambleDetectionWithoutFrameCapture::RxFailure, this));

  Ptr<ThresholdPreambleDetectionModel> preambleDetectionModel =
      CreateObject<ThresholdPreambleDetectionModel>();
  preambleDetectionModel->SetAttribute("Threshold", DoubleValue(4));
  preambleDetectionModel->SetAttribute("MinimumRssi", DoubleValue(-82));
  m_phy->SetPreambleDetectionModel(preambleDetectionModel);
}

void TestThresholdPreambleDetectionWithoutFrameCapture::DoRun() {
  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(1);
  int64_t streamNumber = 0;
  m_phy->AssignStreams(streamNumber);

  double rxPowerDbm = -50;

  Simulator::Schedule(
      Seconds(1.0),
      &TestThresholdPreambleDetectionWithoutFrameCapture::SendPacket, this,
      rxPowerDbm, 1000, 7);
  Simulator::Schedule(
      Seconds(1.0) + NanoSeconds(3999),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckPhyState, this,
      WifiPhyState::IDLE);
  Simulator::Schedule(
      Seconds(1.0) + NanoSeconds(4000),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(1.0) + NanoSeconds(43999),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(1.0) + NanoSeconds(44000),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckPhyState, this,
      WifiPhyState::RX);
  Simulator::Schedule(
      Seconds(1.0) + NanoSeconds(152799),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckPhyState, this,
      WifiPhyState::RX);
  Simulator::Schedule(
      Seconds(1.0) + NanoSeconds(152800),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckPhyState, this,
      WifiPhyState::IDLE);
  Simulator::Schedule(
      Seconds(1.1),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckRxPacketCount,
      this, 1, 0);

  Simulator::Schedule(
      Seconds(2.0),
      &TestThresholdPreambleDetectionWithoutFrameCapture::SendPacket, this,
      rxPowerDbm, 1000, 7);
  Simulator::Schedule(
      Seconds(2.0) + MicroSeconds(2.0),
      &TestThresholdPreambleDetectionWithoutFrameCapture::SendPacket, this,
      rxPowerDbm, 1000, 7);
  Simulator::Schedule(
      Seconds(2.0) + NanoSeconds(3999),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckPhyState, this,
      WifiPhyState::IDLE);
  Simulator::Schedule(
      Seconds(2.0) + NanoSeconds(4000),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(2.0) + NanoSeconds(154799),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(2.0) + NanoSeconds(154800),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckPhyState, this,
      WifiPhyState::IDLE);
  Simulator::Schedule(
      Seconds(2.1),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckRxPacketCount,
      this, 1, 0);

  Simulator::Schedule(
      Seconds(3.0),
      &TestThresholdPreambleDetectionWithoutFrameCapture::SendPacket, this,
      rxPowerDbm, 1000, 7);
  Simulator::Schedule(
      Seconds(3.0) + MicroSeconds(2.0),
      &TestThresholdPreambleDetectionWithoutFrameCapture::SendPacket, this,
      rxPowerDbm - 3, 1000, 7);
  Simulator::Schedule(
      Seconds(3.0) + NanoSeconds(3999),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckPhyState, this,
      WifiPhyState::IDLE);
  Simulator::Schedule(
      Seconds(3.0) + NanoSeconds(4000),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(3.0) + NanoSeconds(154799),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(3.0) + NanoSeconds(154800),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckPhyState, this,
      WifiPhyState::IDLE);
  Simulator::Schedule(
      Seconds(3.1),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckRxPacketCount,
      this, 1, 0);

  Simulator::Schedule(
      Seconds(4.0),
      &TestThresholdPreambleDetectionWithoutFrameCapture::SendPacket, this,
      rxPowerDbm, 1000, 7);
  Simulator::Schedule(
      Seconds(4.0) + MicroSeconds(2.0),
      &TestThresholdPreambleDetectionWithoutFrameCapture::SendPacket, this,
      rxPowerDbm - 6, 1000, 7);
  Simulator::Schedule(
      Seconds(4.0) + NanoSeconds(3999),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckPhyState, this,
      WifiPhyState::IDLE);
  Simulator::Schedule(
      Seconds(4.0) + NanoSeconds(4000),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(4.0) + NanoSeconds(43999),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(4.0) + NanoSeconds(44000),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckPhyState, this,
      WifiPhyState::RX);
  Simulator::Schedule(
      Seconds(4.0) + NanoSeconds(152799),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckPhyState, this,
      WifiPhyState::RX);
  Simulator::Schedule(
      Seconds(4.0) + NanoSeconds(152800),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(4.0) + NanoSeconds(154799),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(4.0) + NanoSeconds(154800),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckPhyState, this,
      WifiPhyState::IDLE);
  Simulator::Schedule(
      Seconds(4.1),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckRxPacketCount,
      this, 1, 1);

  Simulator::Schedule(
      Seconds(5.0),
      &TestThresholdPreambleDetectionWithoutFrameCapture::SendPacket, this,
      rxPowerDbm, 1000, 7);
  Simulator::Schedule(
      Seconds(5.0) + MicroSeconds(2.0),
      &TestThresholdPreambleDetectionWithoutFrameCapture::SendPacket, this,
      rxPowerDbm + 3, 1000, 7);
  Simulator::Schedule(
      Seconds(5.0) + NanoSeconds(5999),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckPhyState, this,
      WifiPhyState::IDLE);
  Simulator::Schedule(
      Seconds(5.0) + NanoSeconds(6000),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(5.0) + NanoSeconds(154799),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(5.0) + NanoSeconds(154800),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckPhyState, this,
      WifiPhyState::IDLE);
  Simulator::Schedule(
      Seconds(5.1),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckRxPacketCount,
      this, 1, 1);

  rxPowerDbm = -70;

  Simulator::Schedule(
      Seconds(6.0),
      &TestThresholdPreambleDetectionWithoutFrameCapture::SendPacket, this,
      rxPowerDbm, 1000, 7);
  Simulator::Schedule(
      Seconds(6.0) + NanoSeconds(3999),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckPhyState, this,
      WifiPhyState::IDLE);
  Simulator::Schedule(
      Seconds(6.0) + NanoSeconds(4000),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(6.0) + NanoSeconds(43999),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(6.0) + NanoSeconds(44000),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckPhyState, this,
      WifiPhyState::RX);
  Simulator::Schedule(
      Seconds(6.0) + NanoSeconds(152799),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckPhyState, this,
      WifiPhyState::RX);
  Simulator::Schedule(
      Seconds(6.0) + NanoSeconds(152800),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckPhyState, this,
      WifiPhyState::IDLE);
  Simulator::Schedule(
      Seconds(6.1),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckRxPacketCount,
      this, 2, 1);

  Simulator::Schedule(
      Seconds(7.0),
      &TestThresholdPreambleDetectionWithoutFrameCapture::SendPacket, this,
      rxPowerDbm, 1000, 7);
  Simulator::Schedule(
      Seconds(7.0) + MicroSeconds(2.0),
      &TestThresholdPreambleDetectionWithoutFrameCapture::SendPacket, this,
      rxPowerDbm, 1000, 7);
  Simulator::Schedule(
      Seconds(7.0) + MicroSeconds(4.0),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(7.1),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckRxPacketCount,
      this, 2, 1);

  Simulator::Schedule(
      Seconds(8.0),
      &TestThresholdPreambleDetectionWithoutFrameCapture::SendPacket, this,
      rxPowerDbm, 1000, 7);
  Simulator::Schedule(
      Seconds(8.0) + MicroSeconds(2.0),
      &TestThresholdPreambleDetectionWithoutFrameCapture::SendPacket, this,
      rxPowerDbm - 3, 1000, 7);
  Simulator::Schedule(
      Seconds(8.0) + MicroSeconds(4.0),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(8.1),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckRxPacketCount,
      this, 2, 1);

  Simulator::Schedule(
      Seconds(9.0),
      &TestThresholdPreambleDetectionWithoutFrameCapture::SendPacket, this,
      rxPowerDbm, 1000, 7);
  Simulator::Schedule(
      Seconds(9.0) + MicroSeconds(2.0),
      &TestThresholdPreambleDetectionWithoutFrameCapture::SendPacket, this,
      rxPowerDbm - 6, 1000, 7);
  Simulator::Schedule(
      Seconds(9.0) + NanoSeconds(3999),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckPhyState, this,
      WifiPhyState::IDLE);
  Simulator::Schedule(
      Seconds(9.0) + NanoSeconds(4000),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(9.0) + NanoSeconds(43999),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(9.0) + NanoSeconds(44000),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckPhyState, this,
      WifiPhyState::RX);
  Simulator::Schedule(
      Seconds(9.0) + NanoSeconds(152799),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckPhyState, this,
      WifiPhyState::RX);
  Simulator::Schedule(
      Seconds(9.0) + NanoSeconds(152800),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(9.1),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckRxPacketCount,
      this, 2, 2);

  Simulator::Schedule(
      Seconds(10.0),
      &TestThresholdPreambleDetectionWithoutFrameCapture::SendPacket, this,
      rxPowerDbm, 1000, 7);
  Simulator::Schedule(
      Seconds(10.0) + MicroSeconds(2.0),
      &TestThresholdPreambleDetectionWithoutFrameCapture::SendPacket, this,
      rxPowerDbm + 3, 1000, 7);
  Simulator::Schedule(
      Seconds(10.0) + MicroSeconds(4.0),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckPhyState, this,
      WifiPhyState::IDLE);
  Simulator::Schedule(
      Seconds(10.1),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckRxPacketCount,
      this, 2, 2);

  rxPowerDbm = -81;

  Simulator::Schedule(
      Seconds(11.0),
      &TestThresholdPreambleDetectionWithoutFrameCapture::SendPacket, this,
      rxPowerDbm, 1000, 7);
  Simulator::Schedule(
      Seconds(11.0) + NanoSeconds(3999),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckPhyState, this,
      WifiPhyState::IDLE);
  Simulator::Schedule(
      Seconds(11.0) + NanoSeconds(4000),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(11.0) + NanoSeconds(43999),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(11.0) + NanoSeconds(44000),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckPhyState, this,
      WifiPhyState::RX);
  Simulator::Schedule(
      Seconds(11.0) + NanoSeconds(152799),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckPhyState, this,
      WifiPhyState::RX);
  Simulator::Schedule(
      Seconds(11.0) + NanoSeconds(152800),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckPhyState, this,
      WifiPhyState::IDLE);

  rxPowerDbm = -83;

  Simulator::Schedule(
      Seconds(12.0),
      &TestThresholdPreambleDetectionWithoutFrameCapture::SendPacket, this,
      rxPowerDbm, 1000, 7);
  Simulator::Schedule(
      Seconds(12.0) + MicroSeconds(4.0),
      &TestThresholdPreambleDetectionWithoutFrameCapture::CheckPhyState, this,
      WifiPhyState::IDLE);

  Simulator::Run();
  Simulator::Destroy();
}

class TestThresholdPreambleDetectionWithFrameCapture
    : public WifiPhyReceptionTest {
public:
  TestThresholdPreambleDetectionWithFrameCapture();

protected:
  void DoSetup() override;

  void RxSuccess(Ptr<const WifiPsdu> psdu, RxSignalInfo rxSignalInfo,
                 WifiTxVector txVector, std::vector<bool> statusPerMpdu);
  void RxFailure(Ptr<const WifiPsdu> psdu);
  uint32_t m_countRxSuccess{0};
  uint32_t m_countRxFailure{0};

private:
  void DoRun() override;

  void CheckRxPacketCount(uint32_t expectedSuccessCount,
                          uint32_t expectedFailureCount);
};

TestThresholdPreambleDetectionWithFrameCapture::
    TestThresholdPreambleDetectionWithFrameCapture()
    : WifiPhyReceptionTest("Threshold preamble detection model test when "
                           "simple frame capture model is applied") {}

void TestThresholdPreambleDetectionWithFrameCapture::CheckRxPacketCount(
    uint32_t expectedSuccessCount, uint32_t expectedFailureCount) {
  NS_TEST_ASSERT_MSG_EQ(m_countRxSuccess, expectedSuccessCount,
                        "Didn't receive right number of successful packets");
  NS_TEST_ASSERT_MSG_EQ(m_countRxFailure, expectedFailureCount,
                        "Didn't receive right number of unsuccessful packets");
}

void TestThresholdPreambleDetectionWithFrameCapture::RxSuccess(
    Ptr<const WifiPsdu> psdu, RxSignalInfo rxSignalInfo, WifiTxVector txVector,
    std::vector<bool> statusPerMpdu) {
  NS_LOG_FUNCTION(this << *psdu << txVector);
  m_countRxSuccess++;
}

void TestThresholdPreambleDetectionWithFrameCapture::RxFailure(
    Ptr<const WifiPsdu> psdu) {
  NS_LOG_FUNCTION(this << *psdu);
  m_countRxFailure++;
}

void TestThresholdPreambleDetectionWithFrameCapture::DoSetup() {
  WifiPhyReceptionTest::DoSetup();

  m_phy->SetReceiveOkCallback(MakeCallback(
      &TestThresholdPreambleDetectionWithFrameCapture::RxSuccess, this));
  m_phy->SetReceiveErrorCallback(MakeCallback(
      &TestThresholdPreambleDetectionWithFrameCapture::RxFailure, this));

  Ptr<ThresholdPreambleDetectionModel> preambleDetectionModel =
      CreateObject<ThresholdPreambleDetectionModel>();
  preambleDetectionModel->SetAttribute("Threshold", DoubleValue(4));
  preambleDetectionModel->SetAttribute("MinimumRssi", DoubleValue(-82));
  m_phy->SetPreambleDetectionModel(preambleDetectionModel);

  Ptr<SimpleFrameCaptureModel> frameCaptureModel =
      CreateObject<SimpleFrameCaptureModel>();
  frameCaptureModel->SetAttribute("Margin", DoubleValue(5));
  frameCaptureModel->SetAttribute("CaptureWindow", TimeValue(MicroSeconds(16)));
  m_phy->SetFrameCaptureModel(frameCaptureModel);
}

void TestThresholdPreambleDetectionWithFrameCapture::DoRun() {
  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(1);
  int64_t streamNumber = 1;
  m_phy->AssignStreams(streamNumber);

  double rxPowerDbm = -50;

  Simulator::Schedule(
      Seconds(1.0), &TestThresholdPreambleDetectionWithFrameCapture::SendPacket,
      this, rxPowerDbm, 1000, 7);
  Simulator::Schedule(
      Seconds(1.0) + NanoSeconds(3999),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::IDLE);
  Simulator::Schedule(
      Seconds(1.0) + NanoSeconds(4000),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(1.0) + NanoSeconds(43999),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(1.0) + NanoSeconds(44000),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::RX);
  Simulator::Schedule(
      Seconds(1.0) + NanoSeconds(152799),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::RX);
  Simulator::Schedule(
      Seconds(1.0) + NanoSeconds(152800),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::IDLE);
  Simulator::Schedule(
      Seconds(1.1),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckRxPacketCount, this,
      1, 0);

  Simulator::Schedule(
      Seconds(2.0), &TestThresholdPreambleDetectionWithFrameCapture::SendPacket,
      this, rxPowerDbm, 1000, 7);
  Simulator::Schedule(
      Seconds(2.0) + MicroSeconds(2.0),
      &TestThresholdPreambleDetectionWithFrameCapture::SendPacket, this,
      rxPowerDbm, 1000, 7);
  Simulator::Schedule(
      Seconds(2.0) + NanoSeconds(3999),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::IDLE);
  Simulator::Schedule(
      Seconds(2.0) + NanoSeconds(4000),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(2.0) + NanoSeconds(154799),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(2.0) + NanoSeconds(154800),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::IDLE);
  Simulator::Schedule(
      Seconds(2.1),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckRxPacketCount, this,
      1, 0);

  Simulator::Schedule(
      Seconds(3.0), &TestThresholdPreambleDetectionWithFrameCapture::SendPacket,
      this, rxPowerDbm, 1000, 7);
  Simulator::Schedule(
      Seconds(3.0) + MicroSeconds(2.0),
      &TestThresholdPreambleDetectionWithFrameCapture::SendPacket, this,
      rxPowerDbm - 3, 1000, 7);
  Simulator::Schedule(
      Seconds(3.0) + NanoSeconds(3999),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::IDLE);
  Simulator::Schedule(
      Seconds(3.0) + NanoSeconds(4000),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(3.0) + NanoSeconds(154799),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(3.0) + NanoSeconds(154800),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::IDLE);
  Simulator::Schedule(
      Seconds(3.1),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckRxPacketCount, this,
      1, 0);

  Simulator::Schedule(
      Seconds(4.0), &TestThresholdPreambleDetectionWithFrameCapture::SendPacket,
      this, rxPowerDbm, 1000, 7);
  Simulator::Schedule(
      Seconds(4.0) + MicroSeconds(2.0),
      &TestThresholdPreambleDetectionWithFrameCapture::SendPacket, this,
      rxPowerDbm - 6, 1000, 7);
  Simulator::Schedule(
      Seconds(4.0) + NanoSeconds(3999),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::IDLE);
  Simulator::Schedule(
      Seconds(4.0) + NanoSeconds(4000),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(4.0) + NanoSeconds(43999),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(4.0) + NanoSeconds(44000),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::RX);
  Simulator::Schedule(
      Seconds(4.0) + NanoSeconds(152799),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::RX);
  Simulator::Schedule(
      Seconds(4.0) + NanoSeconds(152800),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(4.0) + NanoSeconds(154799),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(4.0) + NanoSeconds(154800),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::IDLE);
  Simulator::Schedule(
      Seconds(4.1),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckRxPacketCount, this,
      1, 1);

  Simulator::Schedule(
      Seconds(5.0), &TestThresholdPreambleDetectionWithFrameCapture::SendPacket,
      this, rxPowerDbm, 1000, 7);
  Simulator::Schedule(
      Seconds(5.0) + MicroSeconds(2.0),
      &TestThresholdPreambleDetectionWithFrameCapture::SendPacket, this,
      rxPowerDbm + 3, 1000, 7);
  Simulator::Schedule(
      Seconds(5.0) + MicroSeconds(4.0),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::IDLE);
  Simulator::Schedule(
      Seconds(5.0) + NanoSeconds(5999),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::IDLE);
  Simulator::Schedule(
      Seconds(5.0) + NanoSeconds(6000),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(5.0) + NanoSeconds(154799),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(5.0) + NanoSeconds(154800),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::IDLE);
  Simulator::Schedule(
      Seconds(5.1),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckRxPacketCount, this,
      1, 1);

  Simulator::Schedule(
      Seconds(6.0), &TestThresholdPreambleDetectionWithFrameCapture::SendPacket,
      this, rxPowerDbm, 1000, 7);
  Simulator::Schedule(
      Seconds(6.0) + MicroSeconds(2.0),
      &TestThresholdPreambleDetectionWithFrameCapture::SendPacket, this,
      rxPowerDbm + 6, 1000, 7);
  Simulator::Schedule(
      Seconds(6.0) + MicroSeconds(4.0),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::IDLE);
  Simulator::Schedule(
      Seconds(6.0) + NanoSeconds(5999),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::IDLE);
  Simulator::Schedule(
      Seconds(6.0) + NanoSeconds(6000),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(6.0) + NanoSeconds(45999),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(6.0) + NanoSeconds(46000),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::RX);
  Simulator::Schedule(
      Seconds(6.0) + NanoSeconds(154799),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::RX);
  Simulator::Schedule(
      Seconds(6.0) + NanoSeconds(154800),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::IDLE);
  Simulator::Schedule(
      Seconds(6.1),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckRxPacketCount, this,
      1, 2);

  Simulator::Schedule(
      Seconds(7.0), &TestThresholdPreambleDetectionWithFrameCapture::SendPacket,
      this, rxPowerDbm, 1000, 7);
  Simulator::Schedule(
      Seconds(7.0), &TestThresholdPreambleDetectionWithFrameCapture::SendPacket,
      this, rxPowerDbm, 1000, 7);
  Simulator::Schedule(
      Seconds(7.0) + NanoSeconds(3999),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::IDLE);
  Simulator::Schedule(
      Seconds(7.0) + NanoSeconds(4000),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(7.0) + NanoSeconds(152799),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(7.0) + NanoSeconds(152800),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::IDLE);
  Simulator::Schedule(
      Seconds(7.1),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckRxPacketCount, this,
      1, 2);

  Simulator::Schedule(
      Seconds(8.0), &TestThresholdPreambleDetectionWithFrameCapture::SendPacket,
      this, rxPowerDbm, 1000, 7);
  Simulator::Schedule(
      Seconds(8.0), &TestThresholdPreambleDetectionWithFrameCapture::SendPacket,
      this, rxPowerDbm - 3, 1000, 7);
  Simulator::Schedule(
      Seconds(8.0) + NanoSeconds(3999),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::IDLE);
  Simulator::Schedule(
      Seconds(8.0) + NanoSeconds(4000),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(8.0) + NanoSeconds(152799),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(8.0) + NanoSeconds(152800),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::IDLE);
  Simulator::Schedule(
      Seconds(8.1),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckRxPacketCount, this,
      1, 2);

  Simulator::Schedule(
      Seconds(9.0), &TestThresholdPreambleDetectionWithFrameCapture::SendPacket,
      this, rxPowerDbm, 1000, 7);
  Simulator::Schedule(
      Seconds(9.0), &TestThresholdPreambleDetectionWithFrameCapture::SendPacket,
      this, rxPowerDbm - 6, 1000, 7);
  Simulator::Schedule(
      Seconds(9.0) + NanoSeconds(3999),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::IDLE);
  Simulator::Schedule(
      Seconds(9.0) + NanoSeconds(4000),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(9.0) + NanoSeconds(43999),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(9.0) + NanoSeconds(44000),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::RX);
  Simulator::Schedule(
      Seconds(9.0) + NanoSeconds(152799),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::RX);
  Simulator::Schedule(
      Seconds(9.0) + NanoSeconds(152800),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::IDLE);
  Simulator::Schedule(
      Seconds(9.1),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckRxPacketCount, this,
      1, 3);

  Simulator::Schedule(
      Seconds(10.0),
      &TestThresholdPreambleDetectionWithFrameCapture::SendPacket, this,
      rxPowerDbm, 1000, 7);
  Simulator::Schedule(
      Seconds(10.0),
      &TestThresholdPreambleDetectionWithFrameCapture::SendPacket, this,
      rxPowerDbm + 3, 1000, 7);
  Simulator::Schedule(
      Seconds(10.0) + NanoSeconds(3999),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::IDLE);
  Simulator::Schedule(
      Seconds(10.0) + NanoSeconds(4000),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(10.0) + NanoSeconds(152799),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(10.0) + NanoSeconds(152800),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::IDLE);
  Simulator::Schedule(
      Seconds(10.1),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckRxPacketCount, this,
      1, 3);

  Simulator::Schedule(
      Seconds(11.0),
      &TestThresholdPreambleDetectionWithFrameCapture::SendPacket, this,
      rxPowerDbm, 1000, 7);
  Simulator::Schedule(
      Seconds(11.0),
      &TestThresholdPreambleDetectionWithFrameCapture::SendPacket, this,
      rxPowerDbm + 6, 1000, 7);
  Simulator::Schedule(
      Seconds(11.0) + NanoSeconds(3999),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::IDLE);
  Simulator::Schedule(
      Seconds(11.0) + NanoSeconds(4000),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(11.0) + NanoSeconds(43999),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(11.0) + NanoSeconds(44000),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::RX);
  Simulator::Schedule(
      Seconds(11.0) + NanoSeconds(152799),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::RX);
  Simulator::Schedule(
      Seconds(11.0) + NanoSeconds(152800),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::IDLE);
  Simulator::Schedule(
      Seconds(11.1),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckRxPacketCount, this,
      1, 4);

  rxPowerDbm = -70;

  Simulator::Schedule(
      Seconds(12.0),
      &TestThresholdPreambleDetectionWithFrameCapture::SendPacket, this,
      rxPowerDbm, 1000, 7);
  Simulator::Schedule(
      Seconds(12.0) + NanoSeconds(3999),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::IDLE);
  Simulator::Schedule(
      Seconds(12.0) + NanoSeconds(4000),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(12.0) + NanoSeconds(43999),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(12.0) + NanoSeconds(44000),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::RX);
  Simulator::Schedule(
      Seconds(12.0) + NanoSeconds(152799),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::RX);
  Simulator::Schedule(
      Seconds(12.0) + NanoSeconds(152800),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::IDLE);
  Simulator::Schedule(
      Seconds(12.1),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckRxPacketCount, this,
      2, 4);

  Simulator::Schedule(
      Seconds(13.0),
      &TestThresholdPreambleDetectionWithFrameCapture::SendPacket, this,
      rxPowerDbm, 1000, 7);
  Simulator::Schedule(
      Seconds(13.0) + MicroSeconds(2.0),
      &TestThresholdPreambleDetectionWithFrameCapture::SendPacket, this,
      rxPowerDbm, 1000, 7);
  Simulator::Schedule(
      Seconds(13.0) + MicroSeconds(4.0),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(13.1),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckRxPacketCount, this,
      2, 4);

  Simulator::Schedule(
      Seconds(14.0),
      &TestThresholdPreambleDetectionWithFrameCapture::SendPacket, this,
      rxPowerDbm, 1000, 7);
  Simulator::Schedule(
      Seconds(14.0) + MicroSeconds(2.0),
      &TestThresholdPreambleDetectionWithFrameCapture::SendPacket, this,
      rxPowerDbm - 3, 1000, 7);
  Simulator::Schedule(
      Seconds(14.0) + MicroSeconds(4.0),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(14.1),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckRxPacketCount, this,
      2, 4);

  Simulator::Schedule(
      Seconds(15.0),
      &TestThresholdPreambleDetectionWithFrameCapture::SendPacket, this,
      rxPowerDbm, 1000, 7);
  Simulator::Schedule(
      Seconds(15.0) + MicroSeconds(2.0),
      &TestThresholdPreambleDetectionWithFrameCapture::SendPacket, this,
      rxPowerDbm - 6, 1000, 7);
  Simulator::Schedule(
      Seconds(15.0) + NanoSeconds(3999),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::IDLE);
  Simulator::Schedule(
      Seconds(15.0) + NanoSeconds(4000),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(15.0) + NanoSeconds(43999),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(15.0) + NanoSeconds(44000),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::RX);
  Simulator::Schedule(
      Seconds(15.0) + NanoSeconds(152799),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::RX);
  Simulator::Schedule(
      Seconds(15.0) + NanoSeconds(152800),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(15.1),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckRxPacketCount, this,
      2, 5);

  Simulator::Schedule(
      Seconds(16.0),
      &TestThresholdPreambleDetectionWithFrameCapture::SendPacket, this,
      rxPowerDbm, 1000, 7);
  Simulator::Schedule(
      Seconds(16.0) + MicroSeconds(2.0),
      &TestThresholdPreambleDetectionWithFrameCapture::SendPacket, this,
      rxPowerDbm + 3, 1000, 7);
  Simulator::Schedule(
      Seconds(16.0) + MicroSeconds(4.0),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::IDLE);
  Simulator::Schedule(
      Seconds(16.0) + MicroSeconds(6.0),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(16.1),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckRxPacketCount, this,
      2, 5);

  Simulator::Schedule(
      Seconds(17.0),
      &TestThresholdPreambleDetectionWithFrameCapture::SendPacket, this,
      rxPowerDbm, 1000, 7);
  Simulator::Schedule(
      Seconds(17.0) + MicroSeconds(2.0),
      &TestThresholdPreambleDetectionWithFrameCapture::SendPacket, this,
      rxPowerDbm + 6, 1000, 7);
  Simulator::Schedule(
      Seconds(17.0) + MicroSeconds(4.0),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::IDLE);
  Simulator::Schedule(
      Seconds(17.0) + NanoSeconds(5999),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::IDLE);
  Simulator::Schedule(
      Seconds(17.0) + NanoSeconds(6000),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(17.0) + NanoSeconds(45999),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(17.0) + NanoSeconds(46000),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::RX);
  Simulator::Schedule(
      Seconds(17.0) + NanoSeconds(154799),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::RX);
  Simulator::Schedule(
      Seconds(17.0) + NanoSeconds(154800),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckPhyState, this,
      WifiPhyState::IDLE);
  Simulator::Schedule(
      Seconds(17.1),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckRxPacketCount, this,
      2, 6);

  rxPowerDbm = -50;

  Simulator::Schedule(
      Seconds(18.0),
      &TestThresholdPreambleDetectionWithFrameCapture::SendPacket, this,
      rxPowerDbm, 1000, 7);
  Simulator::Schedule(
      Seconds(18.0) + MicroSeconds(2.0),
      &TestThresholdPreambleDetectionWithFrameCapture::SendPacket, this,
      rxPowerDbm + 50, 1000, 7);
  Simulator::Schedule(
      Seconds(18.1),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckRxPacketCount, this,
      3, 6);

  Simulator::Schedule(
      Seconds(19.0),
      &TestThresholdPreambleDetectionWithFrameCapture::SendPacket, this,
      rxPowerDbm, 1000, 7);
  Simulator::Schedule(
      Seconds(19.0) + MicroSeconds(2.0),
      &TestThresholdPreambleDetectionWithFrameCapture::SendPacket, this,
      rxPowerDbm + 10, 1000, 7);
  Simulator::Schedule(
      Seconds(19.1),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckRxPacketCount, this,
      3, 7);

  Simulator::Schedule(
      Seconds(20.0),
      &TestThresholdPreambleDetectionWithFrameCapture::SendPacket, this,
      rxPowerDbm, 1000, 7);
  Simulator::Schedule(
      Seconds(20.0),
      &TestThresholdPreambleDetectionWithFrameCapture::SendPacket, this,
      rxPowerDbm + 50, 1000, 7);
  Simulator::Schedule(
      Seconds(20.1),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckRxPacketCount, this,
      4, 7);

  Simulator::Schedule(
      Seconds(21.0),
      &TestThresholdPreambleDetectionWithFrameCapture::SendPacket, this,
      rxPowerDbm, 1000, 7);
  Simulator::Schedule(
      Seconds(21.0),
      &TestThresholdPreambleDetectionWithFrameCapture::SendPacket, this,
      rxPowerDbm + 10, 1000, 7);
  Simulator::Schedule(
      Seconds(21.1),
      &TestThresholdPreambleDetectionWithFrameCapture::CheckRxPacketCount, this,
      4, 8);

  Simulator::Run();
  Simulator::Destroy();
}

class TestSimpleFrameCaptureModel : public WifiPhyReceptionTest {
public:
  TestSimpleFrameCaptureModel();

private:
  void DoSetup() override;
  void DoRun() override;

  void Reset();
  void RxSuccess(Ptr<const WifiPsdu> psdu, RxSignalInfo rxSignalInfo,
                 WifiTxVector txVector, std::vector<bool> statusPerMpdu);
  void RxDropped(Ptr<const Packet> p, WifiPhyRxfailureReason reason);

  void Expect1000BPacketReceived();
  void Expect1500BPacketReceived();
  void Expect1000BPacketDropped();
  void Expect1500BPacketDropped();

  bool m_rxSuccess1000B{false};
  bool m_rxSuccess1500B{false};
  bool m_rxDropped1000B{false};
  bool m_rxDropped1500B{false};
};

TestSimpleFrameCaptureModel::TestSimpleFrameCaptureModel()
    : WifiPhyReceptionTest("Simple frame capture model test") {}

void TestSimpleFrameCaptureModel::RxSuccess(Ptr<const WifiPsdu> psdu,
                                            RxSignalInfo rxSignalInfo,
                                            WifiTxVector txVector,
                                            std::vector<bool> statusPerMpdu) {
  NS_LOG_FUNCTION(this << *psdu << rxSignalInfo << txVector);
  NS_ASSERT(!psdu->IsAggregate() || psdu->IsSingle());
  if (psdu->GetSize() == 1030) {
    m_rxSuccess1000B = true;
  } else if (psdu->GetSize() == 1530) {
    m_rxSuccess1500B = true;
  }
}

void TestSimpleFrameCaptureModel::RxDropped(Ptr<const Packet> p,
                                            WifiPhyRxfailureReason reason) {
  NS_LOG_FUNCTION(this << p << reason);
  if (p->GetSize() == 1030) {
    m_rxDropped1000B = true;
  } else if (p->GetSize() == 1530) {
    m_rxDropped1500B = true;
  }
}

void TestSimpleFrameCaptureModel::Reset() {
  m_rxSuccess1000B = false;
  m_rxSuccess1500B = false;
  m_rxDropped1000B = false;
  m_rxDropped1500B = false;
}

void TestSimpleFrameCaptureModel::Expect1000BPacketReceived() {
  NS_TEST_ASSERT_MSG_EQ(m_rxSuccess1000B, true, "Didn't receive 1000B packet");
}

void TestSimpleFrameCaptureModel::Expect1500BPacketReceived() {
  NS_TEST_ASSERT_MSG_EQ(m_rxSuccess1500B, true, "Didn't receive 1500B packet");
}

void TestSimpleFrameCaptureModel::Expect1000BPacketDropped() {
  NS_TEST_ASSERT_MSG_EQ(m_rxDropped1000B, true, "Didn't drop 1000B packet");
}

void TestSimpleFrameCaptureModel::Expect1500BPacketDropped() {
  NS_TEST_ASSERT_MSG_EQ(m_rxDropped1500B, true, "Didn't drop 1500B packet");
}

void TestSimpleFrameCaptureModel::DoSetup() {
  WifiPhyReceptionTest::DoSetup();

  m_phy->SetReceiveOkCallback(
      MakeCallback(&TestSimpleFrameCaptureModel::RxSuccess, this));
  m_phy->TraceConnectWithoutContext(
      "PhyRxDrop", MakeCallback(&TestSimpleFrameCaptureModel::RxDropped, this));

  Ptr<ThresholdPreambleDetectionModel> preambleDetectionModel =
      CreateObject<ThresholdPreambleDetectionModel>();
  preambleDetectionModel->SetAttribute("Threshold", DoubleValue(2));
  m_phy->SetPreambleDetectionModel(preambleDetectionModel);

  Ptr<SimpleFrameCaptureModel> frameCaptureModel =
      CreateObject<SimpleFrameCaptureModel>();
  frameCaptureModel->SetAttribute("Margin", DoubleValue(5));
  frameCaptureModel->SetAttribute("CaptureWindow", TimeValue(MicroSeconds(16)));
  m_phy->SetFrameCaptureModel(frameCaptureModel);
}

void TestSimpleFrameCaptureModel::DoRun() {
  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(1);
  int64_t streamNumber = 2;
  double rxPowerDbm = -30;
  m_phy->AssignStreams(streamNumber);

  Simulator::Schedule(Seconds(1.0), &TestSimpleFrameCaptureModel::SendPacket,
                      this, rxPowerDbm, 1000, 0);
  Simulator::Schedule(Seconds(1.0) + MicroSeconds(10.0),
                      &TestSimpleFrameCaptureModel::SendPacket, this,
                      rxPowerDbm, 1500, 0);
  Simulator::Schedule(Seconds(1.1),
                      &TestSimpleFrameCaptureModel::Expect1500BPacketDropped,
                      this);
  Simulator::Schedule(Seconds(1.2), &TestSimpleFrameCaptureModel::Reset, this);

  Simulator::Schedule(Seconds(2.0), &TestSimpleFrameCaptureModel::SendPacket,
                      this, rxPowerDbm, 1000, 0);
  Simulator::Schedule(Seconds(2.0) + MicroSeconds(10.0),
                      &TestSimpleFrameCaptureModel::SendPacket, this,
                      rxPowerDbm - 6, 1500, 0);
  Simulator::Schedule(Seconds(2.1),
                      &TestSimpleFrameCaptureModel::Expect1000BPacketReceived,
                      this);
  Simulator::Schedule(Seconds(2.1),
                      &TestSimpleFrameCaptureModel::Expect1500BPacketDropped,
                      this);
  Simulator::Schedule(Seconds(2.2), &TestSimpleFrameCaptureModel::Reset, this);

  Simulator::Schedule(Seconds(3.0), &TestSimpleFrameCaptureModel::SendPacket,
                      this, rxPowerDbm, 1000, 0);
  Simulator::Schedule(Seconds(3.0) + MicroSeconds(10.0),
                      &TestSimpleFrameCaptureModel::SendPacket, this,
                      rxPowerDbm + 6, 1500, 0);
  Simulator::Schedule(Seconds(3.1),
                      &TestSimpleFrameCaptureModel::Expect1000BPacketDropped,
                      this);
  Simulator::Schedule(Seconds(3.1),
                      &TestSimpleFrameCaptureModel::Expect1500BPacketReceived,
                      this);
  Simulator::Schedule(Seconds(3.2), &TestSimpleFrameCaptureModel::Reset, this);

  Simulator::Schedule(Seconds(4.0), &TestSimpleFrameCaptureModel::SendPacket,
                      this, rxPowerDbm, 1000, 0);
  Simulator::Schedule(Seconds(4.0) + MicroSeconds(25.0),
                      &TestSimpleFrameCaptureModel::SendPacket, this,
                      rxPowerDbm + 6, 1500, 0);
  Simulator::Schedule(Seconds(4.1),
                      &TestSimpleFrameCaptureModel::Expect1500BPacketDropped,
                      this);
  Simulator::Schedule(Seconds(4.2), &TestSimpleFrameCaptureModel::Reset, this);

  Simulator::Run();
  Simulator::Destroy();
}

class TestPhyHeadersReception : public WifiPhyReceptionTest {
public:
  TestPhyHeadersReception();

private:
  void DoRun() override;
};

TestPhyHeadersReception::TestPhyHeadersReception()
    : WifiPhyReceptionTest("PHY headers reception test") {}

void TestPhyHeadersReception::DoRun() {
  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(1);
  int64_t streamNumber = 0;
  m_phy->AssignStreams(streamNumber);

  double rxPowerDbm = -50;

  Simulator::Schedule(Seconds(1.0), &TestPhyHeadersReception::SendPacket, this,
                      rxPowerDbm, 1000, 7);
  Simulator::Schedule(Seconds(1.0) + MicroSeconds(10),
                      &TestPhyHeadersReception::SendPacket, this, rxPowerDbm,
                      1000, 7);
  Simulator::Schedule(Seconds(1.0) + MicroSeconds(10.0),
                      &TestPhyHeadersReception::CheckPhyState, this,
                      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(Seconds(1.0) + NanoSeconds(44000),
                      &TestPhyHeadersReception::CheckPhyState, this,
                      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(Seconds(1.0) + NanoSeconds(162799),
                      &TestPhyHeadersReception::CheckPhyState, this,
                      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(Seconds(1.0) + NanoSeconds(162800),
                      &TestPhyHeadersReception::CheckPhyState, this,
                      WifiPhyState::IDLE);

  Simulator::Schedule(Seconds(2.0), &TestPhyHeadersReception::SendPacket, this,
                      rxPowerDbm, 1000, 7);
  Simulator::Schedule(Seconds(2.0) + MicroSeconds(10),
                      &TestPhyHeadersReception::SendPacket, this,
                      rxPowerDbm - 3, 1000, 7);
  Simulator::Schedule(Seconds(2.0) + MicroSeconds(10.0),
                      &TestPhyHeadersReception::CheckPhyState, this,
                      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(Seconds(2.0) + NanoSeconds(43999),
                      &TestPhyHeadersReception::CheckPhyState, this,
                      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(Seconds(2.0) + NanoSeconds(44000),
                      &TestPhyHeadersReception::CheckPhyState, this,
                      WifiPhyState::RX);
  Simulator::Schedule(Seconds(2.0) + NanoSeconds(152799),
                      &TestPhyHeadersReception::CheckPhyState, this,
                      WifiPhyState::RX);
  Simulator::Schedule(Seconds(2.0) + NanoSeconds(152800),
                      &TestPhyHeadersReception::CheckPhyState, this,
                      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(Seconds(2.0) + NanoSeconds(162799),
                      &TestPhyHeadersReception::CheckPhyState, this,
                      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(Seconds(2.0) + NanoSeconds(162800),
                      &TestPhyHeadersReception::CheckPhyState, this,
                      WifiPhyState::IDLE);

  Simulator::Schedule(Seconds(3.0), &TestPhyHeadersReception::SendPacket, this,
                      rxPowerDbm, 1000, 7);
  Simulator::Schedule(Seconds(3.0) + MicroSeconds(25),
                      &TestPhyHeadersReception::SendPacket, this, rxPowerDbm,
                      1000, 7);
  Simulator::Schedule(Seconds(3.0) + MicroSeconds(44.0),
                      &TestPhyHeadersReception::CheckPhyState, this,
                      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(Seconds(3.0) + NanoSeconds(152799),
                      &TestPhyHeadersReception::CheckPhyState, this,
                      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(Seconds(3.0) + NanoSeconds(152800),
                      &TestPhyHeadersReception::CheckPhyState, this,
                      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(Seconds(3.0) + NanoSeconds(177799),
                      &TestPhyHeadersReception::CheckPhyState, this,
                      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(Seconds(3.0) + NanoSeconds(177800),
                      &TestPhyHeadersReception::CheckPhyState, this,
                      WifiPhyState::IDLE);

  Simulator::Schedule(Seconds(4.0), &TestPhyHeadersReception::SendPacket, this,
                      rxPowerDbm, 1000, 7);
  Simulator::Schedule(Seconds(4.0) + MicroSeconds(25),
                      &TestPhyHeadersReception::SendPacket, this,
                      rxPowerDbm - 3, 1000, 7);
  Simulator::Schedule(Seconds(4.0) + MicroSeconds(10.0),
                      &TestPhyHeadersReception::CheckPhyState, this,
                      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(Seconds(4.0) + NanoSeconds(43999),
                      &TestPhyHeadersReception::CheckPhyState, this,
                      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(Seconds(4.0) + NanoSeconds(44000),
                      &TestPhyHeadersReception::CheckPhyState, this,
                      WifiPhyState::RX);
  Simulator::Schedule(Seconds(4.0) + NanoSeconds(152799),
                      &TestPhyHeadersReception::CheckPhyState, this,
                      WifiPhyState::RX);
  Simulator::Schedule(Seconds(4.0) + NanoSeconds(152800),
                      &TestPhyHeadersReception::CheckPhyState, this,
                      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(Seconds(4.0) + NanoSeconds(177799),
                      &TestPhyHeadersReception::CheckPhyState, this,
                      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(Seconds(4.0) + NanoSeconds(177800),
                      &TestPhyHeadersReception::CheckPhyState, this,
                      WifiPhyState::IDLE);

  rxPowerDbm = -70;

  Simulator::Schedule(Seconds(5.0), &TestPhyHeadersReception::SendPacket, this,
                      rxPowerDbm, 1000, 7);
  Simulator::Schedule(Seconds(5.0) + MicroSeconds(10),
                      &TestPhyHeadersReception::SendPacket, this, rxPowerDbm,
                      1000, 7);
  Simulator::Schedule(Seconds(5.0) + MicroSeconds(10.0),
                      &TestPhyHeadersReception::CheckPhyState, this,
                      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(Seconds(5.0) + NanoSeconds(24000),
                      &TestPhyHeadersReception::CheckPhyState, this,
                      WifiPhyState::CCA_BUSY);

  Simulator::Schedule(Seconds(6.0), &TestPhyHeadersReception::SendPacket, this,
                      rxPowerDbm, 1000, 7);
  Simulator::Schedule(Seconds(6.0) + MicroSeconds(10),
                      &TestPhyHeadersReception::SendPacket, this,
                      rxPowerDbm - 3, 1000, 7);
  Simulator::Schedule(Seconds(6.0) + MicroSeconds(10.0),
                      &TestPhyHeadersReception::CheckPhyState, this,
                      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(Seconds(6.0) + MicroSeconds(24.0),
                      &TestPhyHeadersReception::CheckPhyState, this,
                      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(Seconds(6.0) + NanoSeconds(43999),
                      &TestPhyHeadersReception::CheckPhyState, this,
                      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(Seconds(6.0) + NanoSeconds(44000),
                      &TestPhyHeadersReception::CheckPhyState, this,
                      WifiPhyState::RX);
  Simulator::Schedule(Seconds(6.0) + NanoSeconds(152799),
                      &TestPhyHeadersReception::CheckPhyState, this,
                      WifiPhyState::RX);
  Simulator::Schedule(Seconds(6.0) + NanoSeconds(152800),
                      &TestPhyHeadersReception::CheckPhyState, this,
                      WifiPhyState::CCA_BUSY);

  Simulator::Schedule(Seconds(7.0), &TestPhyHeadersReception::SendPacket, this,
                      rxPowerDbm, 1000, 7);
  Simulator::Schedule(Seconds(7.0) + MicroSeconds(25),
                      &TestPhyHeadersReception::SendPacket, this, rxPowerDbm,
                      1000, 7);
  Simulator::Schedule(Seconds(7.0) + MicroSeconds(10.0),
                      &TestPhyHeadersReception::CheckPhyState, this,
                      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(Seconds(7.0) + MicroSeconds(24.0),
                      &TestPhyHeadersReception::CheckPhyState, this,
                      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(Seconds(7.0) + MicroSeconds(44.0),
                      &TestPhyHeadersReception::CheckPhyState, this,
                      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(Seconds(7.0) + NanoSeconds(152800),
                      &TestPhyHeadersReception::CheckPhyState, this,
                      WifiPhyState::CCA_BUSY);

  Simulator::Schedule(Seconds(8.0), &TestPhyHeadersReception::SendPacket, this,
                      rxPowerDbm, 1000, 7);
  Simulator::Schedule(Seconds(8.0) + MicroSeconds(25),
                      &TestPhyHeadersReception::SendPacket, this,
                      rxPowerDbm - 3, 1000, 7);
  Simulator::Schedule(Seconds(8.0) + MicroSeconds(10.0),
                      &TestPhyHeadersReception::CheckPhyState, this,
                      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(Seconds(8.0) + MicroSeconds(24.0),
                      &TestPhyHeadersReception::CheckPhyState, this,
                      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(Seconds(8.0) + NanoSeconds(43999),
                      &TestPhyHeadersReception::CheckPhyState, this,
                      WifiPhyState::CCA_BUSY);
  Simulator::Schedule(Seconds(8.0) + NanoSeconds(44000),
                      &TestPhyHeadersReception::CheckPhyState, this,
                      WifiPhyState::RX);
  Simulator::Schedule(Seconds(8.0) + NanoSeconds(152799),
                      &TestPhyHeadersReception::CheckPhyState, this,
                      WifiPhyState::RX);
  Simulator::Schedule(Seconds(8.0) + NanoSeconds(152800),
                      &TestPhyHeadersReception::CheckPhyState, this,
                      WifiPhyState::CCA_BUSY);

  Simulator::Run();
  Simulator::Destroy();
}

class TestAmpduReception : public WifiPhyReceptionTest {
public:
  TestAmpduReception();

private:
  void DoSetup() override;
  void DoRun() override;

  void RxSuccess(Ptr<const WifiPsdu> psdu, RxSignalInfo rxSignalInfo,
                 WifiTxVector txVector, std::vector<bool> statusPerMpdu);
  void RxFailure(Ptr<const WifiPsdu> psdu);
  void RxDropped(Ptr<const Packet> p, WifiPhyRxfailureReason reason);
  void IncrementSuccessBitmap(uint32_t size);
  void IncrementFailureBitmap(uint32_t size);

  void ResetBitmaps();

  void SendAmpduWithThreeMpdus(double rxPowerDbm, uint32_t referencePacketSize);

  void CheckRxSuccessBitmapAmpdu1(uint8_t expected);
  void CheckRxSuccessBitmapAmpdu2(uint8_t expected);
  void CheckRxFailureBitmapAmpdu1(uint8_t expected);
  void CheckRxFailureBitmapAmpdu2(uint8_t expected);
  void CheckRxDroppedBitmapAmpdu1(uint8_t expected);
  void CheckRxDroppedBitmapAmpdu2(uint8_t expected);

  void CheckPhyState(WifiPhyState expectedState);

  uint8_t m_rxSuccessBitmapAmpdu1{0};
  uint8_t m_rxSuccessBitmapAmpdu2{0};

  uint8_t m_rxFailureBitmapAmpdu1{0};
  uint8_t m_rxFailureBitmapAmpdu2{0};

  uint8_t m_rxDroppedBitmapAmpdu1{0};
  uint8_t m_rxDroppedBitmapAmpdu2{0};
};

TestAmpduReception::TestAmpduReception()
    : WifiPhyReceptionTest("A-MPDU reception test") {}

void TestAmpduReception::ResetBitmaps() {
  m_rxSuccessBitmapAmpdu1 = 0;
  m_rxSuccessBitmapAmpdu2 = 0;
  m_rxFailureBitmapAmpdu1 = 0;
  m_rxFailureBitmapAmpdu2 = 0;
  m_rxDroppedBitmapAmpdu1 = 0;
  m_rxDroppedBitmapAmpdu2 = 0;
}

void TestAmpduReception::RxSuccess(Ptr<const WifiPsdu> psdu,
                                   RxSignalInfo rxSignalInfo,
                                   WifiTxVector txVector,
                                   std::vector<bool> statusPerMpdu) {
  NS_LOG_FUNCTION(this << *psdu << rxSignalInfo << txVector);
  if (statusPerMpdu.empty()) {
    return;
  }
  NS_ABORT_MSG_IF(psdu->GetNMpdus() != statusPerMpdu.size(),
                  "Should have one receive status per MPDU");
  auto rxOkForMpdu = statusPerMpdu.begin();
  for (auto mpdu = psdu->begin(); mpdu != psdu->end(); ++mpdu) {
    if (*rxOkForMpdu) {
      IncrementSuccessBitmap((*mpdu)->GetSize());
    } else {
      IncrementFailureBitmap((*mpdu)->GetSize());
    }
    ++rxOkForMpdu;
  }
}

void TestAmpduReception::IncrementSuccessBitmap(uint32_t size) {
  if (size == 1030) {
    m_rxSuccessBitmapAmpdu1 |= 1;
  } else if (size == 1130) {
    m_rxSuccessBitmapAmpdu1 |= (1 << 1);
  } else if (size == 1230) {
    m_rxSuccessBitmapAmpdu1 |= (1 << 2);
  } else if (size == 1330) {
    m_rxSuccessBitmapAmpdu2 |= 1;
  } else if (size == 1430) {
    m_rxSuccessBitmapAmpdu2 |= (1 << 1);
  } else if (size == 1530) {
    m_rxSuccessBitmapAmpdu2 |= (1 << 2);
  }
}

void TestAmpduReception::RxFailure(Ptr<const WifiPsdu> psdu) {
  NS_LOG_FUNCTION(this << *psdu);
  for (auto mpdu = psdu->begin(); mpdu != psdu->end(); ++mpdu) {
    IncrementFailureBitmap((*mpdu)->GetSize());
  }
}

void TestAmpduReception::IncrementFailureBitmap(uint32_t size) {
  if (size == 1030) {
    m_rxFailureBitmapAmpdu1 |= 1;
  } else if (size == 1130) {
    m_rxFailureBitmapAmpdu1 |= (1 << 1);
  } else if (size == 1230) {
    m_rxFailureBitmapAmpdu1 |= (1 << 2);
  } else if (size == 1330) {
    m_rxFailureBitmapAmpdu2 |= 1;
  } else if (size == 1430) {
    m_rxFailureBitmapAmpdu2 |= (1 << 1);
  } else if (size == 1530) {
    m_rxFailureBitmapAmpdu2 |= (1 << 2);
  }
}

void TestAmpduReception::RxDropped(Ptr<const Packet> p,
                                   WifiPhyRxfailureReason reason) {
  NS_LOG_FUNCTION(this << p << reason);
  if (p->GetSize() == 1030) {
    m_rxDroppedBitmapAmpdu1 |= 1;
  } else if (p->GetSize() == 1130) {
    m_rxDroppedBitmapAmpdu1 |= (1 << 1);
  } else if (p->GetSize() == 1230) {
    m_rxDroppedBitmapAmpdu1 |= (1 << 2);
  } else if (p->GetSize() == 1330) {
    m_rxDroppedBitmapAmpdu2 |= 1;
  } else if (p->GetSize() == 1430) {
    m_rxDroppedBitmapAmpdu2 |= (1 << 1);
  } else if (p->GetSize() == 1530) {
    m_rxDroppedBitmapAmpdu2 |= (1 << 2);
  }
}

void TestAmpduReception::CheckRxSuccessBitmapAmpdu1(uint8_t expected) {
  NS_TEST_ASSERT_MSG_EQ(m_rxSuccessBitmapAmpdu1, expected,
                        "RX success bitmap for A-MPDU 1 is not as expected");
}

void TestAmpduReception::CheckRxSuccessBitmapAmpdu2(uint8_t expected) {
  NS_TEST_ASSERT_MSG_EQ(m_rxSuccessBitmapAmpdu2, expected,
                        "RX success bitmap for A-MPDU 2 is not as expected");
}

void TestAmpduReception::CheckRxFailureBitmapAmpdu1(uint8_t expected) {
  NS_TEST_ASSERT_MSG_EQ(m_rxFailureBitmapAmpdu1, expected,
                        "RX failure bitmap for A-MPDU 1 is not as expected");
}

void TestAmpduReception::CheckRxFailureBitmapAmpdu2(uint8_t expected) {
  NS_TEST_ASSERT_MSG_EQ(m_rxFailureBitmapAmpdu2, expected,
                        "RX failure bitmap for A-MPDU 2 is not as expected");
}

void TestAmpduReception::CheckRxDroppedBitmapAmpdu1(uint8_t expected) {
  NS_TEST_ASSERT_MSG_EQ(m_rxDroppedBitmapAmpdu1, expected,
                        "RX dropped bitmap for A-MPDU 1 is not as expected");
}

void TestAmpduReception::CheckRxDroppedBitmapAmpdu2(uint8_t expected) {
  NS_TEST_ASSERT_MSG_EQ(m_rxDroppedBitmapAmpdu2, expected,
                        "RX dropped bitmap for A-MPDU 2 is not as expected");
}

void TestAmpduReception::CheckPhyState(WifiPhyState expectedState) {
  WifiPhyState currentState;
  PointerValue ptr;
  m_phy->GetAttribute("State", ptr);
  Ptr<WifiPhyStateHelper> state =
      DynamicCast<WifiPhyStateHelper>(ptr.Get<WifiPhyStateHelper>());
  currentState = state->GetState();
  NS_TEST_ASSERT_MSG_EQ(currentState, expectedState,
                        "PHY State "
                            << currentState << " does not match expected state "
                            << expectedState << " at " << Simulator::Now());
}

void TestAmpduReception::SendAmpduWithThreeMpdus(double rxPowerDbm,
                                                 uint32_t referencePacketSize) {
  WifiTxVector txVector = WifiTxVector(
      HePhy::GetHeMcs0(), 0, WIFI_PREAMBLE_HE_SU, 800, 1, 1, 0, 20, true);

  WifiMacHeader hdr;
  hdr.SetType(WIFI_MAC_QOSDATA);
  hdr.SetQosTid(0);

  std::vector<Ptr<WifiMpdu>> mpduList;
  for (size_t i = 0; i < 3; ++i) {
    Ptr<Packet> p = Create<Packet>(referencePacketSize + i * 100);
    mpduList.push_back(Create<WifiMpdu>(p, hdr));
  }
  Ptr<WifiPsdu> psdu = Create<WifiPsdu>(mpduList);

  Time txDuration = m_phy->CalculateTxDuration(psdu->GetSize(), txVector,
                                               m_phy->GetPhyBand());

  Ptr<WifiPpdu> ppdu = Create<HePpdu>(
      psdu, txVector, m_phy->GetOperatingChannel(), txDuration, m_uid++);

  Ptr<SpectrumValue> txPowerSpectrum =
      WifiSpectrumValueHelper::CreateHeOfdmTxPowerSpectralDensity(
          FREQUENCY, CHANNEL_WIDTH, DbmToW(rxPowerDbm), GUARD_WIDTH);

  Ptr<WifiSpectrumSignalParameters> txParams =
      Create<WifiSpectrumSignalParameters>();
  txParams->psd = txPowerSpectrum;
  txParams->txPhy = nullptr;
  txParams->duration = txDuration;
  txParams->ppdu = ppdu;

  m_phy->StartRx(txParams, nullptr);
}

void TestAmpduReception::DoSetup() {
  WifiPhyReceptionTest::DoSetup();

  m_phy->SetReceiveOkCallback(
      MakeCallback(&TestAmpduReception::RxSuccess, this));
  m_phy->SetReceiveErrorCallback(
      MakeCallback(&TestAmpduReception::RxFailure, this));
  m_phy->TraceConnectWithoutContext(
      "PhyRxDrop", MakeCallback(&TestAmpduReception::RxDropped, this));

  Ptr<ThresholdPreambleDetectionModel> preambleDetectionModel =
      CreateObject<ThresholdPreambleDetectionModel>();
  preambleDetectionModel->SetAttribute("Threshold", DoubleValue(2));
  m_phy->SetPreambleDetectionModel(preambleDetectionModel);

  Ptr<SimpleFrameCaptureModel> frameCaptureModel =
      CreateObject<SimpleFrameCaptureModel>();
  frameCaptureModel->SetAttribute("Margin", DoubleValue(5));
  frameCaptureModel->SetAttribute("CaptureWindow", TimeValue(MicroSeconds(16)));
  m_phy->SetFrameCaptureModel(frameCaptureModel);
}

void TestAmpduReception::DoRun() {
  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(2);
  int64_t streamNumber = 1;
  double rxPowerDbm = -30;
  m_phy->AssignStreams(streamNumber);

  Simulator::Schedule(Seconds(1.0),
                      &TestAmpduReception::SendAmpduWithThreeMpdus, this,
                      rxPowerDbm - 100, 1000);

  Simulator::Schedule(Seconds(1.0) + MicroSeconds(2),
                      &TestAmpduReception::SendAmpduWithThreeMpdus, this,
                      rxPowerDbm, 1300);

  Simulator::Schedule(Seconds(1.1),
                      &TestAmpduReception::CheckRxSuccessBitmapAmpdu1, this,
                      0b00000000);
  Simulator::Schedule(Seconds(1.1),
                      &TestAmpduReception::CheckRxFailureBitmapAmpdu1, this,
                      0b00000000);
  Simulator::Schedule(Seconds(1.1),
                      &TestAmpduReception::CheckRxDroppedBitmapAmpdu1, this,
                      0b00000000);

  Simulator::Schedule(Seconds(1.1),
                      &TestAmpduReception::CheckRxSuccessBitmapAmpdu2, this,
                      0b00000111);
  Simulator::Schedule(Seconds(1.1),
                      &TestAmpduReception::CheckRxFailureBitmapAmpdu2, this,
                      0b00000000);
  Simulator::Schedule(Seconds(1.1),
                      &TestAmpduReception::CheckRxDroppedBitmapAmpdu2, this,
                      0b00000000);

  Simulator::Schedule(Seconds(1.2), &TestAmpduReception::ResetBitmaps, this);

  Simulator::Schedule(Seconds(2.0),
                      &TestAmpduReception::SendAmpduWithThreeMpdus, this,
                      rxPowerDbm, 1000);

  Simulator::Schedule(Seconds(2.0) + MicroSeconds(2),
                      &TestAmpduReception::SendAmpduWithThreeMpdus, this,
                      rxPowerDbm - 100, 1300);

  Simulator::Schedule(Seconds(2.1),
                      &TestAmpduReception::CheckRxSuccessBitmapAmpdu1, this,
                      0b00000111);
  Simulator::Schedule(Seconds(2.1),
                      &TestAmpduReception::CheckRxFailureBitmapAmpdu1, this,
                      0b00000000);
  Simulator::Schedule(Seconds(2.1),
                      &TestAmpduReception::CheckRxDroppedBitmapAmpdu1, this,
                      0b00000000);

  Simulator::Schedule(Seconds(2.1),
                      &TestAmpduReception::CheckRxSuccessBitmapAmpdu2, this,
                      0b00000000);
  Simulator::Schedule(Seconds(2.1),
                      &TestAmpduReception::CheckRxFailureBitmapAmpdu2, this,
                      0b00000000);
  Simulator::Schedule(Seconds(2.1),
                      &TestAmpduReception::CheckRxDroppedBitmapAmpdu2, this,
                      0b00000000);

  Simulator::Schedule(Seconds(2.2), &TestAmpduReception::ResetBitmaps, this);

  Simulator::Schedule(Seconds(3.0),
                      &TestAmpduReception::SendAmpduWithThreeMpdus, this,
                      rxPowerDbm - 100, 1000);

  Simulator::Schedule(Seconds(3.0) + MicroSeconds(10),
                      &TestAmpduReception::SendAmpduWithThreeMpdus, this,
                      rxPowerDbm, 1300);

  Simulator::Schedule(Seconds(3.1),
                      &TestAmpduReception::CheckRxSuccessBitmapAmpdu1, this,
                      0b00000000);
  Simulator::Schedule(Seconds(3.1),
                      &TestAmpduReception::CheckRxFailureBitmapAmpdu1, this,
                      0b00000000);
  Simulator::Schedule(Seconds(3.1),
                      &TestAmpduReception::CheckRxDroppedBitmapAmpdu1, this,
                      0b00000000);

  Simulator::Schedule(Seconds(3.1),
                      &TestAmpduReception::CheckRxSuccessBitmapAmpdu2, this,
                      0b00000111);
  Simulator::Schedule(Seconds(3.1),
                      &TestAmpduReception::CheckRxFailureBitmapAmpdu2, this,
                      0b00000000);
  Simulator::Schedule(Seconds(3.1),
                      &TestAmpduReception::CheckRxDroppedBitmapAmpdu2, this,
                      0b00000000);

  Simulator::Schedule(Seconds(3.2), &TestAmpduReception::ResetBitmaps, this);

  Simulator::Schedule(Seconds(4.0),
                      &TestAmpduReception::SendAmpduWithThreeMpdus, this,
                      rxPowerDbm, 1000);

  Simulator::Schedule(Seconds(4.0) + MicroSeconds(10),
                      &TestAmpduReception::SendAmpduWithThreeMpdus, this,
                      rxPowerDbm - 100, 1300);

  Simulator::Schedule(Seconds(4.1),
                      &TestAmpduReception::CheckRxSuccessBitmapAmpdu1, this,
                      0b00000111);
  Simulator::Schedule(Seconds(4.1),
                      &TestAmpduReception::CheckRxFailureBitmapAmpdu1, this,
                      0b00000000);
  Simulator::Schedule(Seconds(4.1),
                      &TestAmpduReception::CheckRxDroppedBitmapAmpdu1, this,
                      0b00000000);

  Simulator::Schedule(Seconds(4.1),
                      &TestAmpduReception::CheckRxSuccessBitmapAmpdu2, this,
                      0b00000000);
  Simulator::Schedule(Seconds(4.1),
                      &TestAmpduReception::CheckRxFailureBitmapAmpdu2, this,
                      0b00000000);
  Simulator::Schedule(Seconds(4.1),
                      &TestAmpduReception::CheckRxDroppedBitmapAmpdu2, this,
                      0b00000000);

  Simulator::Schedule(Seconds(4.2), &TestAmpduReception::ResetBitmaps, this);

  Simulator::Schedule(Seconds(5.0),
                      &TestAmpduReception::SendAmpduWithThreeMpdus, this,
                      rxPowerDbm - 100, 1000);

  Simulator::Schedule(Seconds(5.0) + MicroSeconds(100),
                      &TestAmpduReception::SendAmpduWithThreeMpdus, this,
                      rxPowerDbm, 1300);

  Simulator::Schedule(Seconds(5.1),
                      &TestAmpduReception::CheckRxSuccessBitmapAmpdu1, this,
                      0b00000000);
  Simulator::Schedule(Seconds(5.1),
                      &TestAmpduReception::CheckRxFailureBitmapAmpdu1, this,
                      0b00000000);
  Simulator::Schedule(Seconds(5.1),
                      &TestAmpduReception::CheckRxDroppedBitmapAmpdu1, this,
                      0b00000000);

  Simulator::Schedule(Seconds(5.1),
                      &TestAmpduReception::CheckRxSuccessBitmapAmpdu2, this,
                      0b00000111);
  Simulator::Schedule(Seconds(5.1),
                      &TestAmpduReception::CheckRxFailureBitmapAmpdu2, this,
                      0b00000000);
  Simulator::Schedule(Seconds(5.1),
                      &TestAmpduReception::CheckRxDroppedBitmapAmpdu2, this,
                      0b00000000);

  Simulator::Schedule(Seconds(5.2), &TestAmpduReception::ResetBitmaps, this);

  Simulator::Schedule(Seconds(6.0),
                      &TestAmpduReception::SendAmpduWithThreeMpdus, this,
                      rxPowerDbm, 1000);

  Simulator::Schedule(Seconds(6.0) + MicroSeconds(100),
                      &TestAmpduReception::SendAmpduWithThreeMpdus, this,
                      rxPowerDbm - 100, 1300);

  Simulator::Schedule(Seconds(6.1),
                      &TestAmpduReception::CheckRxSuccessBitmapAmpdu1, this,
                      0b00000111);
  Simulator::Schedule(Seconds(6.1),
                      &TestAmpduReception::CheckRxFailureBitmapAmpdu1, this,
                      0b00000000);
  Simulator::Schedule(Seconds(6.1),
                      &TestAmpduReception::CheckRxDroppedBitmapAmpdu1, this,
                      0b00000000);

  Simulator::Schedule(Seconds(6.1),
                      &TestAmpduReception::CheckRxSuccessBitmapAmpdu2, this,
                      0b00000000);
  Simulator::Schedule(Seconds(6.1),
                      &TestAmpduReception::CheckRxFailureBitmapAmpdu2, this,
                      0b00000000);
  Simulator::Schedule(Seconds(6.1),
                      &TestAmpduReception::CheckRxDroppedBitmapAmpdu2, this,
                      0b00000000);

  Simulator::Schedule(Seconds(6.2), &TestAmpduReception::ResetBitmaps, this);

  Simulator::Schedule(Seconds(7.0),
                      &TestAmpduReception::SendAmpduWithThreeMpdus, this,
                      rxPowerDbm - 100, 1000);

  Simulator::Schedule(Seconds(7.0) + NanoSeconds(1100000),
                      &TestAmpduReception::SendAmpduWithThreeMpdus, this,
                      rxPowerDbm, 1300);

  Simulator::Schedule(Seconds(7.1),
                      &TestAmpduReception::CheckRxSuccessBitmapAmpdu1, this,
                      0b00000000);
  Simulator::Schedule(Seconds(7.1),
                      &TestAmpduReception::CheckRxFailureBitmapAmpdu1, this,
                      0b00000000);
  Simulator::Schedule(Seconds(7.1),
                      &TestAmpduReception::CheckRxDroppedBitmapAmpdu1, this,
                      0b00000000);

  Simulator::Schedule(Seconds(7.1),
                      &TestAmpduReception::CheckRxSuccessBitmapAmpdu2, this,
                      0b00000111);
  Simulator::Schedule(Seconds(7.1),
                      &TestAmpduReception::CheckRxFailureBitmapAmpdu2, this,
                      0b00000000);
  Simulator::Schedule(Seconds(7.1),
                      &TestAmpduReception::CheckRxDroppedBitmapAmpdu2, this,
                      0b00000000);

  Simulator::Schedule(Seconds(7.2), &TestAmpduReception::ResetBitmaps, this);

  Simulator::Schedule(Seconds(8.0),
                      &TestAmpduReception::SendAmpduWithThreeMpdus, this,
                      rxPowerDbm, 1000);

  Simulator::Schedule(Seconds(8.0) + NanoSeconds(1100000),
                      &TestAmpduReception::SendAmpduWithThreeMpdus, this,
                      rxPowerDbm - 100, 1300);

  Simulator::Schedule(Seconds(8.1),
                      &TestAmpduReception::CheckRxSuccessBitmapAmpdu1, this,
                      0b00000111);
  Simulator::Schedule(Seconds(8.1),
                      &TestAmpduReception::CheckRxFailureBitmapAmpdu1, this,
                      0b00000000);
  Simulator::Schedule(Seconds(8.1),
                      &TestAmpduReception::CheckRxDroppedBitmapAmpdu1, this,
                      0b00000000);

  Simulator::Schedule(Seconds(8.1),
                      &TestAmpduReception::CheckRxSuccessBitmapAmpdu2, this,
                      0b00000000);
  Simulator::Schedule(Seconds(8.1),
                      &TestAmpduReception::CheckRxFailureBitmapAmpdu2, this,
                      0b00000000);
  Simulator::Schedule(Seconds(8.1),
                      &TestAmpduReception::CheckRxDroppedBitmapAmpdu2, this,
                      0b00000000);

  Simulator::Schedule(Seconds(8.2), &TestAmpduReception::ResetBitmaps, this);

  Simulator::Schedule(Seconds(9.0),
                      &TestAmpduReception::SendAmpduWithThreeMpdus, this,
                      rxPowerDbm, 1000);

  Simulator::Schedule(Seconds(9.0) + MicroSeconds(2),
                      &TestAmpduReception::SendAmpduWithThreeMpdus, this,
                      rxPowerDbm + 3, 1300);

  Simulator::Schedule(Seconds(9.1),
                      &TestAmpduReception::CheckRxSuccessBitmapAmpdu1, this,
                      0b00000000);
  Simulator::Schedule(Seconds(9.1),
                      &TestAmpduReception::CheckRxFailureBitmapAmpdu1, this,
                      0b00000000);
  Simulator::Schedule(Seconds(9.1),
                      &TestAmpduReception::CheckRxDroppedBitmapAmpdu1, this,
                      0b00000111);

  Simulator::Schedule(Seconds(9.1),
                      &TestAmpduReception::CheckRxSuccessBitmapAmpdu2, this,
                      0b00000000);
  Simulator::Schedule(Seconds(9.1),
                      &TestAmpduReception::CheckRxFailureBitmapAmpdu2, this,
                      0b00000111);
  Simulator::Schedule(Seconds(9.1),
                      &TestAmpduReception::CheckRxDroppedBitmapAmpdu2, this,
                      0b00000000);

  Simulator::Schedule(Seconds(9.2), &TestAmpduReception::ResetBitmaps, this);

  Simulator::Schedule(Seconds(10.0),
                      &TestAmpduReception::SendAmpduWithThreeMpdus, this,
                      rxPowerDbm, 1000);

  Simulator::Schedule(Seconds(10.0) + MicroSeconds(2),
                      &TestAmpduReception::SendAmpduWithThreeMpdus, this,
                      rxPowerDbm, 1300);

  Simulator::Schedule(Seconds(10.1),
                      &TestAmpduReception::CheckRxSuccessBitmapAmpdu1, this,
                      0b00000000);
  Simulator::Schedule(Seconds(10.1),
                      &TestAmpduReception::CheckRxFailureBitmapAmpdu1, this,
                      0b00000000);
  Simulator::Schedule(Seconds(10.1),
                      &TestAmpduReception::CheckRxDroppedBitmapAmpdu1, this,
                      0b00000111);

  Simulator::Schedule(Seconds(10.1),
                      &TestAmpduReception::CheckRxSuccessBitmapAmpdu2, this,
                      0b00000000);
  Simulator::Schedule(Seconds(10.1),
                      &TestAmpduReception::CheckRxFailureBitmapAmpdu2, this,
                      0b00000000);
  Simulator::Schedule(Seconds(10.1),
                      &TestAmpduReception::CheckRxDroppedBitmapAmpdu2, this,
                      0b00000111);

  Simulator::Schedule(Seconds(10.2), &TestAmpduReception::ResetBitmaps, this);

  Simulator::Schedule(Seconds(11.0),
                      &TestAmpduReception::SendAmpduWithThreeMpdus, this,
                      rxPowerDbm + 3, 1000);

  Simulator::Schedule(Seconds(11.0) + MicroSeconds(2),
                      &TestAmpduReception::SendAmpduWithThreeMpdus, this,
                      rxPowerDbm, 1300);

  Simulator::Schedule(Seconds(11.1),
                      &TestAmpduReception::CheckRxSuccessBitmapAmpdu1, this,
                      0b00000000);
  Simulator::Schedule(Seconds(11.1),
                      &TestAmpduReception::CheckRxFailureBitmapAmpdu1, this,
                      0b00000111);
  Simulator::Schedule(Seconds(11.1),
                      &TestAmpduReception::CheckRxDroppedBitmapAmpdu1, this,
                      0b00000000);

  Simulator::Schedule(Seconds(11.1),
                      &TestAmpduReception::CheckRxSuccessBitmapAmpdu2, this,
                      0b00000000);
  Simulator::Schedule(Seconds(11.1),
                      &TestAmpduReception::CheckRxFailureBitmapAmpdu2, this,
                      0b00000000);
  Simulator::Schedule(Seconds(11.1),
                      &TestAmpduReception::CheckRxDroppedBitmapAmpdu2, this,
                      0b00000111);

  Simulator::Schedule(Seconds(11.2), &TestAmpduReception::ResetBitmaps, this);

  Simulator::Schedule(Seconds(12.0),
                      &TestAmpduReception::SendAmpduWithThreeMpdus, this,
                      rxPowerDbm, 1000);

  Simulator::Schedule(Seconds(12.0) + MicroSeconds(10),
                      &TestAmpduReception::SendAmpduWithThreeMpdus, this,
                      rxPowerDbm + 3, 1300);

  Simulator::Schedule(Seconds(12.1),
                      &TestAmpduReception::CheckRxSuccessBitmapAmpdu1, this,
                      0b00000000);
  Simulator::Schedule(Seconds(12.1),
                      &TestAmpduReception::CheckRxFailureBitmapAmpdu1, this,
                      0b00000000);
  Simulator::Schedule(Seconds(12.1),
                      &TestAmpduReception::CheckRxDroppedBitmapAmpdu1, this,
                      0b00000111);

  Simulator::Schedule(Seconds(12.1),
                      &TestAmpduReception::CheckRxSuccessBitmapAmpdu2, this,
                      0b00000000);
  Simulator::Schedule(Seconds(12.1),
                      &TestAmpduReception::CheckRxFailureBitmapAmpdu2, this,
                      0b00000000);
  Simulator::Schedule(Seconds(12.1),
                      &TestAmpduReception::CheckRxDroppedBitmapAmpdu2, this,
                      0b00000111);

  Simulator::Schedule(Seconds(12.2), &TestAmpduReception::ResetBitmaps, this);

  Simulator::Schedule(Seconds(13.0),
                      &TestAmpduReception::SendAmpduWithThreeMpdus, this,
                      rxPowerDbm, 1000);

  Simulator::Schedule(Seconds(13.0) + MicroSeconds(10),
                      &TestAmpduReception::SendAmpduWithThreeMpdus, this,
                      rxPowerDbm, 1300);

  Simulator::Schedule(Seconds(13.1),
                      &TestAmpduReception::CheckRxSuccessBitmapAmpdu1, this,
                      0b00000000);
  Simulator::Schedule(Seconds(13.1),
                      &TestAmpduReception::CheckRxFailureBitmapAmpdu1, this,
                      0b00000000);
  Simulator::Schedule(Seconds(13.1),
                      &TestAmpduReception::CheckRxDroppedBitmapAmpdu1, this,
                      0b00000111);

  Simulator::Schedule(Seconds(13.1),
                      &TestAmpduReception::CheckRxSuccessBitmapAmpdu2, this,
                      0b00000000);
  Simulator::Schedule(Seconds(13.1),
                      &TestAmpduReception::CheckRxFailureBitmapAmpdu2, this,
                      0b00000000);
  Simulator::Schedule(Seconds(13.1),
                      &TestAmpduReception::CheckRxDroppedBitmapAmpdu2, this,
                      0b00000111);

  Simulator::Schedule(Seconds(13.2), &TestAmpduReception::ResetBitmaps, this);

  Simulator::Schedule(Seconds(14.0),
                      &TestAmpduReception::SendAmpduWithThreeMpdus, this,
                      rxPowerDbm + 3, 1000);

  Simulator::Schedule(Seconds(14.0) + MicroSeconds(10),
                      &TestAmpduReception::SendAmpduWithThreeMpdus, this,
                      rxPowerDbm, 1300);

  Simulator::Schedule(Seconds(14.1),
                      &TestAmpduReception::CheckRxSuccessBitmapAmpdu1, this,
                      0b00000000);
  Simulator::Schedule(Seconds(14.1),
                      &TestAmpduReception::CheckRxFailureBitmapAmpdu1, this,
                      0b00000111);
  Simulator::Schedule(Seconds(14.1),
                      &TestAmpduReception::CheckRxDroppedBitmapAmpdu1, this,
                      0b00000000);

  Simulator::Schedule(Seconds(14.1),
                      &TestAmpduReception::CheckRxSuccessBitmapAmpdu2, this,
                      0b00000000);
  Simulator::Schedule(Seconds(14.1),
                      &TestAmpduReception::CheckRxFailureBitmapAmpdu2, this,
                      0b00000000);
  Simulator::Schedule(Seconds(14.1),
                      &TestAmpduReception::CheckRxDroppedBitmapAmpdu2, this,
                      0b00000111);

  Simulator::Schedule(Seconds(14.2), &TestAmpduReception::ResetBitmaps, this);

  Simulator::Schedule(Seconds(15.0),
                      &TestAmpduReception::SendAmpduWithThreeMpdus, this,
                      rxPowerDbm, 1000);

  Simulator::Schedule(Seconds(15.0) + MicroSeconds(10),
                      &TestAmpduReception::SendAmpduWithThreeMpdus, this,
                      rxPowerDbm + 6, 1300);

  Simulator::Schedule(Seconds(15.1),
                      &TestAmpduReception::CheckRxSuccessBitmapAmpdu1, this,
                      0b00000000);
  Simulator::Schedule(Seconds(15.1),
                      &TestAmpduReception::CheckRxFailureBitmapAmpdu1, this,
                      0b00000000);
  Simulator::Schedule(Seconds(15.1),
                      &TestAmpduReception::CheckRxDroppedBitmapAmpdu1, this,
                      0b00000111);

  Simulator::Schedule(Seconds(15.1),
                      &TestAmpduReception::CheckRxSuccessBitmapAmpdu2, this,
                      0b00000111);
  Simulator::Schedule(Seconds(15.1),
                      &TestAmpduReception::CheckRxFailureBitmapAmpdu2, this,
                      0b00000000);
  Simulator::Schedule(Seconds(15.1),
                      &TestAmpduReception::CheckRxDroppedBitmapAmpdu2, this,
                      0b00000000);

  Simulator::Schedule(Seconds(15.2), &TestAmpduReception::ResetBitmaps, this);

  Simulator::Schedule(Seconds(16.0),
                      &TestAmpduReception::SendAmpduWithThreeMpdus, this,
                      rxPowerDbm + 6, 1000);

  Simulator::Schedule(Seconds(16.0) + MicroSeconds(10),
                      &TestAmpduReception::SendAmpduWithThreeMpdus, this,
                      rxPowerDbm, 1300);

  Simulator::Schedule(Seconds(16.1),
                      &TestAmpduReception::CheckRxSuccessBitmapAmpdu1, this,
                      0b00000111);
  Simulator::Schedule(Seconds(16.1),
                      &TestAmpduReception::CheckRxFailureBitmapAmpdu1, this,
                      0b00000000);
  Simulator::Schedule(Seconds(16.1),
                      &TestAmpduReception::CheckRxDroppedBitmapAmpdu1, this,
                      0b00000000);

  Simulator::Schedule(Seconds(16.1),
                      &TestAmpduReception::CheckRxSuccessBitmapAmpdu2, this,
                      0b00000000);
  Simulator::Schedule(Seconds(16.1),
                      &TestAmpduReception::CheckRxFailureBitmapAmpdu2, this,
                      0b00000000);
  Simulator::Schedule(Seconds(16.1),
                      &TestAmpduReception::CheckRxDroppedBitmapAmpdu2, this,
                      0b00000111);

  Simulator::Schedule(Seconds(16.2), &TestAmpduReception::ResetBitmaps, this);

  Simulator::Schedule(Seconds(17.0),
                      &TestAmpduReception::SendAmpduWithThreeMpdus, this,
                      rxPowerDbm, 1000);

  Simulator::Schedule(Seconds(17.0) + MicroSeconds(25),
                      &TestAmpduReception::SendAmpduWithThreeMpdus, this,
                      rxPowerDbm + 6, 1300);

  Simulator::Schedule(Seconds(17.1),
                      &TestAmpduReception::CheckRxSuccessBitmapAmpdu1, this,
                      0b00000000);
  Simulator::Schedule(Seconds(17.1),
                      &TestAmpduReception::CheckRxFailureBitmapAmpdu1, this,
                      0b00000000);
  Simulator::Schedule(Seconds(17.1),
                      &TestAmpduReception::CheckRxDroppedBitmapAmpdu1, this,
                      0b00000111);

  Simulator::Schedule(Seconds(17.1),
                      &TestAmpduReception::CheckRxSuccessBitmapAmpdu2, this,
                      0b00000000);
  Simulator::Schedule(Seconds(17.1),
                      &TestAmpduReception::CheckRxFailureBitmapAmpdu2, this,
                      0b00000000);
  Simulator::Schedule(Seconds(17.1),
                      &TestAmpduReception::CheckRxDroppedBitmapAmpdu2, this,
                      0b00000111);

  Simulator::Schedule(Seconds(17.2), &TestAmpduReception::ResetBitmaps, this);

  Simulator::Schedule(Seconds(18.0),
                      &TestAmpduReception::SendAmpduWithThreeMpdus, this,
                      rxPowerDbm + 6, 1000);

  Simulator::Schedule(Seconds(18.0) + MicroSeconds(25),
                      &TestAmpduReception::SendAmpduWithThreeMpdus, this,
                      rxPowerDbm, 1300);

  Simulator::Schedule(Seconds(18.1),
                      &TestAmpduReception::CheckRxSuccessBitmapAmpdu1, this,
                      0b00000111);
  Simulator::Schedule(Seconds(18.1),
                      &TestAmpduReception::CheckRxFailureBitmapAmpdu1, this,
                      0b00000000);
  Simulator::Schedule(Seconds(18.1),
                      &TestAmpduReception::CheckRxDroppedBitmapAmpdu1, this,
                      0b00000000);

  Simulator::Schedule(Seconds(18.1),
                      &TestAmpduReception::CheckRxSuccessBitmapAmpdu2, this,
                      0b00000000);
  Simulator::Schedule(Seconds(18.1),
                      &TestAmpduReception::CheckRxFailureBitmapAmpdu2, this,
                      0b00000000);
  Simulator::Schedule(Seconds(18.1),
                      &TestAmpduReception::CheckRxDroppedBitmapAmpdu2, this,
                      0b00000111);

  Simulator::Schedule(Seconds(18.2), &TestAmpduReception::ResetBitmaps, this);

  Simulator::Schedule(Seconds(19.0),
                      &TestAmpduReception::SendAmpduWithThreeMpdus, this,
                      rxPowerDbm, 1000);

  Simulator::Schedule(Seconds(19.0) + MicroSeconds(25),
                      &TestAmpduReception::SendAmpduWithThreeMpdus, this,
                      rxPowerDbm, 1300);

  Simulator::Schedule(Seconds(19.1),
                      &TestAmpduReception::CheckRxSuccessBitmapAmpdu1, this,
                      0b00000000);
  Simulator::Schedule(Seconds(19.1),
                      &TestAmpduReception::CheckRxFailureBitmapAmpdu1, this,
                      0b00000000);
  Simulator::Schedule(Seconds(19.1),
                      &TestAmpduReception::CheckRxDroppedBitmapAmpdu1, this,
                      0b00000111);

  Simulator::Schedule(Seconds(19.1),
                      &TestAmpduReception::CheckRxSuccessBitmapAmpdu2, this,
                      0b00000000);
  Simulator::Schedule(Seconds(19.1),
                      &TestAmpduReception::CheckRxFailureBitmapAmpdu2, this,
                      0b00000000);
  Simulator::Schedule(Seconds(19.1),
                      &TestAmpduReception::CheckRxDroppedBitmapAmpdu2, this,
                      0b00000111);

  Simulator::Schedule(Seconds(19.2), &TestAmpduReception::ResetBitmaps, this);

  Simulator::Schedule(Seconds(20.0),
                      &TestAmpduReception::SendAmpduWithThreeMpdus, this,
                      rxPowerDbm, 1000);

  Simulator::Schedule(Seconds(20.0) + MicroSeconds(100),
                      &TestAmpduReception::SendAmpduWithThreeMpdus, this,
                      rxPowerDbm + 6, 1300);

  Simulator::Schedule(Seconds(20.1),
                      &TestAmpduReception::CheckRxSuccessBitmapAmpdu1, this,
                      0b00000000);
  Simulator::Schedule(Seconds(20.1),
                      &TestAmpduReception::CheckRxFailureBitmapAmpdu1, this,
                      0b00000111);
  Simulator::Schedule(Seconds(20.1),
                      &TestAmpduReception::CheckRxDroppedBitmapAmpdu1, this,
                      0b00000000);

  Simulator::Schedule(Seconds(20.1),
                      &TestAmpduReception::CheckRxSuccessBitmapAmpdu2, this,
                      0b00000000);
  Simulator::Schedule(Seconds(20.1),
                      &TestAmpduReception::CheckRxFailureBitmapAmpdu2, this,
                      0b00000000);
  Simulator::Schedule(Seconds(20.1),
                      &TestAmpduReception::CheckRxDroppedBitmapAmpdu2, this,
                      0b00000111);

  Simulator::Schedule(Seconds(20.2), &TestAmpduReception::ResetBitmaps, this);

  Simulator::Schedule(Seconds(21.0),
                      &TestAmpduReception::SendAmpduWithThreeMpdus, this,
                      rxPowerDbm + 6, 1000);

  Simulator::Schedule(Seconds(21.0) + MicroSeconds(100),
                      &TestAmpduReception::SendAmpduWithThreeMpdus, this,
                      rxPowerDbm, 1300);

  Simulator::Schedule(Seconds(21.1),
                      &TestAmpduReception::CheckRxSuccessBitmapAmpdu1, this,
                      0b00000111);
  Simulator::Schedule(Seconds(21.1),
                      &TestAmpduReception::CheckRxFailureBitmapAmpdu1, this,
                      0b00000000);
  Simulator::Schedule(Seconds(21.1),
                      &TestAmpduReception::CheckRxDroppedBitmapAmpdu1, this,
                      0b00000000);

  Simulator::Schedule(Seconds(21.1),
                      &TestAmpduReception::CheckRxSuccessBitmapAmpdu2, this,
                      0b00000000);
  Simulator::Schedule(Seconds(21.1),
                      &TestAmpduReception::CheckRxFailureBitmapAmpdu2, this,
                      0b00000000);
  Simulator::Schedule(Seconds(21.1),
                      &TestAmpduReception::CheckRxDroppedBitmapAmpdu2, this,
                      0b00000111);

  Simulator::Schedule(Seconds(21.2), &TestAmpduReception::ResetBitmaps, this);

  Simulator::Schedule(Seconds(22.0),
                      &TestAmpduReception::SendAmpduWithThreeMpdus, this,
                      rxPowerDbm, 1000);

  Simulator::Schedule(Seconds(22.0) + MicroSeconds(100),
                      &TestAmpduReception::SendAmpduWithThreeMpdus, this,
                      rxPowerDbm, 1300);

  Simulator::Schedule(Seconds(22.1),
                      &TestAmpduReception::CheckRxSuccessBitmapAmpdu1, this,
                      0b00000000);
  Simulator::Schedule(Seconds(22.1),
                      &TestAmpduReception::CheckRxFailureBitmapAmpdu1, this,
                      0b00000111);
  Simulator::Schedule(Seconds(22.1),
                      &TestAmpduReception::CheckRxDroppedBitmapAmpdu1, this,
                      0b00000000);

  Simulator::Schedule(Seconds(22.1),
                      &TestAmpduReception::CheckRxSuccessBitmapAmpdu2, this,
                      0b00000000);
  Simulator::Schedule(Seconds(22.1),
                      &TestAmpduReception::CheckRxFailureBitmapAmpdu2, this,
                      0b00000000);
  Simulator::Schedule(Seconds(22.1),
                      &TestAmpduReception::CheckRxDroppedBitmapAmpdu2, this,
                      0b00000111);

  Simulator::Schedule(Seconds(22.2), &TestAmpduReception::ResetBitmaps, this);

  Simulator::Schedule(Seconds(23.0),
                      &TestAmpduReception::SendAmpduWithThreeMpdus, this,
                      rxPowerDbm, 1000);

  Simulator::Schedule(Seconds(23.0) + NanoSeconds(1100000),
                      &TestAmpduReception::SendAmpduWithThreeMpdus, this,
                      rxPowerDbm, 1300);

  Simulator::Schedule(Seconds(23.1),
                      &TestAmpduReception::CheckRxSuccessBitmapAmpdu1, this,
                      0b00000001);
  Simulator::Schedule(Seconds(23.1),
                      &TestAmpduReception::CheckRxFailureBitmapAmpdu1, this,
                      0b00000110);
  Simulator::Schedule(Seconds(23.1),
                      &TestAmpduReception::CheckRxDroppedBitmapAmpdu1, this,
                      0b00000000);

  Simulator::Schedule(Seconds(23.1),
                      &TestAmpduReception::CheckRxSuccessBitmapAmpdu2, this,
                      0b00000000);
  Simulator::Schedule(Seconds(23.1),
                      &TestAmpduReception::CheckRxFailureBitmapAmpdu2, this,
                      0b00000000);
  Simulator::Schedule(Seconds(23.1),
                      &TestAmpduReception::CheckRxDroppedBitmapAmpdu2, this,
                      0b00000111);

  Simulator::Schedule(Seconds(23.2), &TestAmpduReception::ResetBitmaps, this);

  Simulator::Run();
  Simulator::Destroy();
}

class TestUnsupportedModulationReception : public TestCase {
public:
  TestUnsupportedModulationReception();
  ~TestUnsupportedModulationReception() override = default;

private:
  void DoRun() override;

  void Dropped(std::string context, Ptr<const Packet> packet,
               WifiPhyRxfailureReason reason);
  void CheckResults();

  uint16_t m_dropped{0};
};

TestUnsupportedModulationReception::TestUnsupportedModulationReception()
    : TestCase("Check correct behavior when a STA is receiving a transmission "
               "using an unsupported "
               "modulation") {}

void TestUnsupportedModulationReception::Dropped(
    std::string context, Ptr<const Packet> packet,
    WifiPhyRxfailureReason reason) {
  if (reason == RXING) {
    std::cout << "Dropped a packet because already receiving" << std::endl;
    m_dropped++;
  }
}

void TestUnsupportedModulationReception::DoRun() {
  uint16_t m_nStations = 2;
  NetDeviceContainer m_staDevices;
  NetDeviceContainer m_apDevices;

  int64_t streamNumber = 100;

  NodeContainer wifiApNode;
  wifiApNode.Create(1);

  NodeContainer wifiStaNodes;
  wifiStaNodes.Create(m_nStations);

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

  Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold",
                     UintegerValue(65535));

  WifiHelper wifi;
  wifi.SetRemoteStationManager("ns3::IdealWifiManager");

  WifiMacHelper mac;
  mac.SetType("ns3::StaWifiMac", "QosSupported", BooleanValue(true), "Ssid",
              SsidValue(Ssid("non-existent-ssid")));

  wifi.SetStandard(WIFI_STANDARD_80211ax);
  m_staDevices.Add(wifi.Install(phy, mac, wifiStaNodes.Get(0)));
  wifi.SetStandard(WIFI_STANDARD_80211ac);
  m_staDevices.Add(wifi.Install(phy, mac, wifiStaNodes.Get(1)));

  wifi.SetStandard(WIFI_STANDARD_80211ax);
  mac.SetType("ns3::ApWifiMac", "QosSupported", BooleanValue(true), "Ssid",
              SsidValue(Ssid("wifi-backoff-ssid")), "BeaconInterval",
              TimeValue(MicroSeconds(102400)), "EnableBeaconJitter",
              BooleanValue(false));

  m_apDevices = wifi.Install(phy, mac, wifiApNode);

  Time init = MilliSeconds(100);
  Ptr<WifiNetDevice> dev;

  for (uint16_t i = 0; i < m_nStations; i++) {
    dev = DynamicCast<WifiNetDevice>(m_staDevices.Get(i));
    Simulator::Schedule(init + i * MicroSeconds(102400), &WifiMac::SetSsid,
                        dev->GetMac(), Ssid("wifi-backoff-ssid"));
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

  PacketSocketHelper packetSocket;
  packetSocket.Install(wifiApNode);
  packetSocket.Install(wifiStaNodes);

  for (uint16_t i = 0; i < m_nStations; i++) {
    PacketSocketAddress socket;
    socket.SetSingleDevice(m_staDevices.Get(0)->GetIfIndex());
    socket.SetPhysicalAddress(m_apDevices.Get(0)->GetAddress());
    socket.SetProtocol(1);
    Ptr<PacketSocketClient> client = CreateObject<PacketSocketClient>();
    client->SetAttribute("PacketSize", UintegerValue(1500));
    client->SetAttribute("MaxPackets", UintegerValue(200));
    client->SetAttribute("Interval", TimeValue(MicroSeconds(0)));
    client->SetRemote(socket);
    wifiStaNodes.Get(i)->AddApplication(client);
    client->SetStartTime(MicroSeconds(400000));
    client->SetStopTime(Seconds(1.0));
    Ptr<PacketSocketClient> legacyStaClient =
        CreateObject<PacketSocketClient>();
    legacyStaClient->SetAttribute("PacketSize", UintegerValue(1500));
    legacyStaClient->SetAttribute("MaxPackets", UintegerValue(200));
    legacyStaClient->SetAttribute("Interval", TimeValue(MicroSeconds(0)));
    legacyStaClient->SetRemote(socket);
    wifiStaNodes.Get(i)->AddApplication(legacyStaClient);
    legacyStaClient->SetStartTime(MicroSeconds(400000));
    legacyStaClient->SetStopTime(Seconds(1.0));
    Ptr<PacketSocketServer> server = CreateObject<PacketSocketServer>();
    server->SetLocal(socket);
    wifiApNode.Get(0)->AddApplication(server);
    server->SetStartTime(Seconds(0.0));
    server->SetStopTime(Seconds(1.0));
  }

  Config::Connect(
      "/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyRxDrop",
      MakeCallback(&TestUnsupportedModulationReception::Dropped, this));

  Simulator::Stop(Seconds(1));
  Simulator::Run();

  CheckResults();

  Simulator::Destroy();
}

void TestUnsupportedModulationReception::CheckResults() {
  NS_TEST_EXPECT_MSG_EQ(m_dropped, 0, "Dropped some packets unexpectedly");
}

class TestUnsupportedBandwidthReception : public TestCase {
public:
  TestUnsupportedBandwidthReception();

private:
  void DoSetup() override;
  void DoTeardown() override;
  void DoRun() override;

  void SendPpdu(uint16_t centerFreqMhz, uint16_t bandwidthMhz);

  void RxSuccess(Ptr<const WifiPsdu> psdu, RxSignalInfo rxSignalInfo,
                 WifiTxVector txVector, std::vector<bool> statusPerMpdu);

  void RxFailure(Ptr<const WifiPsdu> psdu);

  void RxDropped(Ptr<const Packet> packet, WifiPhyRxfailureReason reason);

  void CheckRx(uint32_t expectedCountRxSuccess, uint32_t expectedCountRxFailure,
               uint32_t expectedCountRxDropped,
               std::optional<Time> expectedLastRxSucceeded,
               std::optional<Time> expectedLastRxFailed,
               std::optional<Time> expectedLastRxDropped);

  uint32_t m_countRxSuccess;
  uint32_t m_countRxFailure;
  uint32_t m_countRxDropped;

  std::optional<Time> m_lastRxSucceeded;
  std::optional<Time> m_lastRxFailed;
  std::optional<Time> m_lastRxDropped;

  Ptr<SpectrumWifiPhy> m_rxPhy;
  Ptr<SpectrumWifiPhy> m_txPhy;
};

TestUnsupportedBandwidthReception::TestUnsupportedBandwidthReception()
    : TestCase("Check correct behavior when a STA is receiving a transmission "
               "using an unsupported "
               "bandwidth"),
      m_countRxSuccess(0), m_countRxFailure(0), m_countRxDropped(0),
      m_lastRxSucceeded(std::nullopt), m_lastRxFailed(std::nullopt),
      m_lastRxDropped(std::nullopt) {}

void TestUnsupportedBandwidthReception::SendPpdu(uint16_t centerFreqMhz,
                                                 uint16_t bandwidthMhz) {
  auto txVector = WifiTxVector(HePhy::GetHeMcs0(), 0, WIFI_PREAMBLE_HE_SU, 800,
                               1, 1, 0, bandwidthMhz, false);

  auto pkt = Create<Packet>(1000);
  WifiMacHeader hdr;

  hdr.SetType(WIFI_MAC_QOSDATA);
  hdr.SetQosTid(0);

  Ptr<WifiPsdu> psdu = Create<WifiPsdu>(pkt, hdr);
  Time txDuration = m_rxPhy->CalculateTxDuration(psdu->GetSize(), txVector,
                                                 m_rxPhy->GetPhyBand());

  auto ppdu = Create<HePpdu>(psdu, txVector, m_txPhy->GetOperatingChannel(),
                             txDuration, 0);

  auto txPowerSpectrum =
      WifiSpectrumValueHelper::CreateHeOfdmTxPowerSpectralDensity(
          centerFreqMhz, bandwidthMhz, DbmToW(-50), bandwidthMhz);

  auto txParams = Create<WifiSpectrumSignalParameters>();
  txParams->psd = txPowerSpectrum;
  txParams->txPhy = nullptr;
  txParams->duration = txDuration;
  txParams->ppdu = ppdu;

  m_rxPhy->StartRx(txParams, nullptr);
}

void TestUnsupportedBandwidthReception::RxSuccess(
    Ptr<const WifiPsdu> psdu, RxSignalInfo rxSignalInfo, WifiTxVector txVector,
    std::vector<bool> statusPerMpdu) {
  NS_LOG_FUNCTION(this << *psdu << rxSignalInfo << txVector);
  m_countRxSuccess++;
  m_lastRxSucceeded = Simulator::Now();
}

void TestUnsupportedBandwidthReception::RxFailure(Ptr<const WifiPsdu> psdu) {
  NS_LOG_FUNCTION(this << *psdu);
  m_countRxFailure++;
  m_lastRxFailed = Simulator::Now();
}

void TestUnsupportedBandwidthReception::RxDropped(
    Ptr<const Packet> p, WifiPhyRxfailureReason reason) {
  NS_LOG_FUNCTION(this << p << reason);
  NS_ASSERT(reason == UNSUPPORTED_SETTINGS);
  m_countRxDropped++;
  m_lastRxDropped = Simulator::Now();
}

void TestUnsupportedBandwidthReception::CheckRx(
    uint32_t expectedCountRxSuccess, uint32_t expectedCountRxFailure,
    uint32_t expectedCountRxDropped,
    std::optional<Time> expectedLastRxSucceeded,
    std::optional<Time> expectedLastRxFailed,
    std::optional<Time> expectedLastRxDropped) {
  NS_TEST_ASSERT_MSG_EQ(m_countRxSuccess, expectedCountRxSuccess,
                        "Didn't receive right number of successful packets");

  NS_TEST_ASSERT_MSG_EQ(m_countRxFailure, expectedCountRxFailure,
                        "Didn't receive right number of unsuccessful packets");

  NS_TEST_ASSERT_MSG_EQ(m_countRxDropped, expectedCountRxDropped,
                        "Didn't receive right number of dropped packets");

  if (expectedCountRxSuccess > 0) {
    NS_ASSERT(m_lastRxSucceeded.has_value());
    NS_ASSERT(expectedLastRxSucceeded.has_value());
    NS_TEST_ASSERT_MSG_EQ(
        m_lastRxSucceeded.value(), expectedLastRxSucceeded.value(),
        "Didn't receive the last successful packet at the expected time");
  }

  if (expectedCountRxFailure > 0) {
    NS_ASSERT(m_lastRxFailed.has_value());
    NS_ASSERT(expectedLastRxFailed.has_value());
    NS_TEST_ASSERT_MSG_EQ(
        m_lastRxFailed.value(), expectedLastRxFailed.value(),
        "Didn't receive the last unsuccessful packet at the expected time");
  }

  if (expectedCountRxDropped > 0) {
    NS_ASSERT(m_lastRxDropped.has_value());
    NS_ASSERT(expectedLastRxDropped.has_value());
    NS_TEST_ASSERT_MSG_EQ(
        m_lastRxDropped.value(), expectedLastRxDropped.value(),
        "Didn't drop the last filtered packet at the expected time");
  }
}

void TestUnsupportedBandwidthReception::DoSetup() {
  Ptr<MultiModelSpectrumChannel> spectrumChannel =
      CreateObject<MultiModelSpectrumChannel>();
  Ptr<Node> node = CreateObject<Node>();
  Ptr<WifiNetDevice> dev = CreateObject<WifiNetDevice>();
  m_rxPhy = CreateObject<SpectrumWifiPhy>();
  auto rxInterferenceHelper = CreateObject<InterferenceHelper>();
  m_rxPhy->SetInterferenceHelper(rxInterferenceHelper);
  auto rxErrorRateModel = CreateObject<NistErrorRateModel>();
  m_rxPhy->SetErrorRateModel(rxErrorRateModel);
  m_rxPhy->SetDevice(dev);
  m_rxPhy->AddChannel(spectrumChannel);
  m_rxPhy->ConfigureStandard(WIFI_STANDARD_80211ax);
  dev->SetPhy(m_rxPhy);
  node->AddDevice(dev);

  m_rxPhy->SetReceiveOkCallback(
      MakeCallback(&TestUnsupportedBandwidthReception::RxSuccess, this));
  m_rxPhy->SetReceiveErrorCallback(
      MakeCallback(&TestUnsupportedBandwidthReception::RxFailure, this));
  m_rxPhy->TraceConnectWithoutContext(
      "PhyRxDrop",
      MakeCallback(&TestUnsupportedBandwidthReception::RxDropped, this));

  m_txPhy = CreateObject<SpectrumWifiPhy>();
  auto txInterferenceHelper = CreateObject<InterferenceHelper>();
  m_txPhy->SetInterferenceHelper(txInterferenceHelper);
  auto txErrorRateModel = CreateObject<NistErrorRateModel>();
  m_txPhy->SetErrorRateModel(txErrorRateModel);
  m_txPhy->AddChannel(spectrumChannel);
  m_txPhy->ConfigureStandard(WIFI_STANDARD_80211ax);
}

void TestUnsupportedBandwidthReception::DoTeardown() {
  m_rxPhy->Dispose();
  m_rxPhy = nullptr;

  m_txPhy->Dispose();
  m_txPhy = nullptr;
}

void TestUnsupportedBandwidthReception::DoRun() {
  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(1);

  int64_t streamNumber = 0;
  m_rxPhy->AssignStreams(streamNumber);

  m_txPhy->SetOperatingChannel(
      WifiPhy::ChannelTuple{38, 40, WIFI_PHY_BAND_5GHZ, 0});
  m_rxPhy->SetOperatingChannel(
      WifiPhy::ChannelTuple{36, 20, WIFI_PHY_BAND_5GHZ, 0});

  Simulator::Schedule(Seconds(1.0),
                      &TestUnsupportedBandwidthReception::SendPpdu, this, 5190,
                      40);

  auto heSigAExpectedRxTime = Seconds(1.0) + MicroSeconds(32);
  Simulator::Schedule(Seconds(1.5), &TestUnsupportedBandwidthReception::CheckRx,
                      this, 0, 0, 1, std::nullopt, std::nullopt,
                      heSigAExpectedRxTime);

  Simulator::Run();
  Simulator::Destroy();
}

class TestPrimary20CoveredByPpdu : public TestCase {
public:
  TestPrimary20CoveredByPpdu();

private:
  void DoSetup() override;
  void DoRun() override;
  void DoTeardown() override;

  Ptr<HePpdu> CreatePpdu(uint16_t ppduCenterFreqMhz);

  void RunOne(WifiPhyBand band, uint16_t phyCenterFreqMhz, uint8_t p20Index,
              uint16_t ppduCenterFreqMhz, bool expectedP20Overlap,
              bool expectedP20Covered);

  Ptr<SpectrumWifiPhy> m_rxPhy;
  Ptr<SpectrumWifiPhy> m_txPhy;
};

TestPrimary20CoveredByPpdu::TestPrimary20CoveredByPpdu()
    : TestCase("Check correct detection of whether P20 is fully covered (hence "
               "it can be received) "
               "or overlaps with the bandwidth of an incoming PPDU") {}

Ptr<HePpdu> TestPrimary20CoveredByPpdu::CreatePpdu(uint16_t ppduCenterFreqMhz) {
  [[maybe_unused]] auto [channelNumber, centerFreq, channelWidth, type,
                         phyBand] =
      (*WifiPhyOperatingChannel::FindFirst(0, ppduCenterFreqMhz, 0,
                                           WIFI_STANDARD_80211ax,
                                           m_rxPhy->GetPhyBand()));
  m_txPhy->SetOperatingChannel(
      WifiPhy::ChannelTuple{channelNumber, channelWidth, phyBand, 0});
  auto txVector = WifiTxVector(HePhy::GetHeMcs7(), 0, WIFI_PREAMBLE_HE_SU, 800,
                               1, 1, 0, channelWidth, false);

  auto pkt = Create<Packet>(1000);
  WifiMacHeader hdr(WIFI_MAC_QOSDATA);

  auto psdu = Create<WifiPsdu>(pkt, hdr);
  auto txDuration =
      m_txPhy->CalculateTxDuration(psdu->GetSize(), txVector, phyBand);

  return Create<HePpdu>(psdu, txVector, m_txPhy->GetOperatingChannel(),
                        txDuration, 0);
}

void TestPrimary20CoveredByPpdu::DoSetup() {
  m_rxPhy = CreateObject<SpectrumWifiPhy>();
  auto rxInterferenceHelper = CreateObject<InterferenceHelper>();
  m_rxPhy->SetInterferenceHelper(rxInterferenceHelper);
  auto rxErrorRateModel = CreateObject<NistErrorRateModel>();
  m_rxPhy->SetErrorRateModel(rxErrorRateModel);
  m_rxPhy->AddChannel(CreateObject<MultiModelSpectrumChannel>());
  m_rxPhy->ConfigureStandard(WIFI_STANDARD_80211ax);

  m_txPhy = CreateObject<SpectrumWifiPhy>();
  auto txInterferenceHelper = CreateObject<InterferenceHelper>();
  m_txPhy->SetInterferenceHelper(txInterferenceHelper);
  auto txErrorRateModel = CreateObject<NistErrorRateModel>();
  m_txPhy->SetErrorRateModel(txErrorRateModel);
  m_txPhy->AddChannel(CreateObject<MultiModelSpectrumChannel>());
  m_txPhy->ConfigureStandard(WIFI_STANDARD_80211ax);
}

void TestPrimary20CoveredByPpdu::DoTeardown() {
  m_rxPhy->Dispose();
  m_rxPhy = nullptr;
  m_txPhy->Dispose();
  m_txPhy = nullptr;
}

void TestPrimary20CoveredByPpdu::RunOne(WifiPhyBand band,
                                        uint16_t phyCenterFreqMhz,
                                        uint8_t p20Index,
                                        uint16_t ppduCenterFreqMhz,
                                        bool expectedP20Overlap,
                                        bool expectedP20Covered) {
  [[maybe_unused]] const auto [channelNumber, centerFreq, channelWidth, type,
                               phyBand] =
      (*WifiPhyOperatingChannel::FindFirst(0, phyCenterFreqMhz, 0,
                                           WIFI_STANDARD_80211ax, band));

  m_rxPhy->SetOperatingChannel(
      WifiPhy::ChannelTuple{channelNumber, channelWidth, band, p20Index});
  auto p20CenterFreq =
      m_rxPhy->GetOperatingChannel().GetPrimaryChannelCenterFrequency(20);
  auto p20MinFreq = p20CenterFreq - 10;
  auto p20MaxFreq = p20CenterFreq + 10;

  auto ppdu = CreatePpdu(ppduCenterFreqMhz);

  auto p20Overlap = ppdu->DoesOverlapChannel(p20MinFreq, p20MaxFreq);
  NS_TEST_ASSERT_MSG_EQ(
      p20Overlap, expectedP20Overlap,
      "PPDU is " << (expectedP20Overlap ? "expected" : "not expected")
                 << " to overlap with the P20");

  auto p20Covered =
      m_rxPhy->GetPhyEntity(WIFI_STANDARD_80211ax)->CanStartRx(ppdu);
  NS_TEST_ASSERT_MSG_EQ(
      p20Covered, expectedP20Covered,
      "PPDU is " << (expectedP20Covered ? "expected" : "not expected")
                 << " to cover the whole P20");
}

void TestPrimary20CoveredByPpdu::DoRun() {
  RunOne(WIFI_PHY_BAND_2_4GHZ, 2427, 0, 2427, true, true);

  RunOne(WIFI_PHY_BAND_2_4GHZ, 2427, 0, 2437, true, false);

  RunOne(WIFI_PHY_BAND_5GHZ, 5180, 0, 5190, true, true);

  RunOne(WIFI_PHY_BAND_5GHZ, 5180, 0, 5200, false, false);

  RunOne(WIFI_PHY_BAND_5GHZ, 5190, 0, 5180, true, true);

  RunOne(WIFI_PHY_BAND_5GHZ, 5190, 1, 5180, false, false);

  Simulator::Destroy();
}

class TestSpectrumChannelWithBandwidthFilter : public TestCase {
public:
  TestSpectrumChannelWithBandwidthFilter(uint16_t channel,
                                         uint16_t expectedValue);

protected:
  void DoSetup() override;
  void DoTeardown() override;

private:
  void RxBegin(bool signalType, uint32_t senderNodeId, double rxPower,
               Time duration);

  void Send() const;

  void CheckRxPacketCount(uint16_t expectedValue);

  void DoRun() override;

  Ptr<SpectrumWifiPhy> m_tx{nullptr};
  Ptr<SpectrumWifiPhy> m_rx{nullptr};
  uint32_t m_countRxBegin{0};
  uint16_t m_channel{36};
  uint16_t m_expectedValue{0};
};

TestSpectrumChannelWithBandwidthFilter::TestSpectrumChannelWithBandwidthFilter(
    uint16_t channel, uint16_t expectedValue)
    : TestCase("Test for early discard of signal in "
               "single-model-spectrum-channel::StartTx()"),
      m_channel(channel), m_expectedValue(expectedValue) {}

void TestSpectrumChannelWithBandwidthFilter::Send() const {
  WifiTxVector txVector = WifiTxVector(
      HePhy::GetHeMcs7(), 0, WIFI_PREAMBLE_HE_SU, 800, 1, 1, 0, 20, false);

  Ptr<Packet> pkt = Create<Packet>(1000);
  WifiMacHeader hdr;

  hdr.SetType(WIFI_MAC_QOSDATA);
  hdr.SetQosTid(0);

  Ptr<WifiPsdu> psdu = Create<WifiPsdu>(pkt, hdr);
  m_tx->Send(psdu, txVector);
}

void TestSpectrumChannelWithBandwidthFilter::CheckRxPacketCount(
    uint16_t expectedValue) {
  NS_TEST_ASSERT_MSG_EQ(
      m_countRxBegin, expectedValue,
      "Received a different amount of packets than expected.");
}

void TestSpectrumChannelWithBandwidthFilter::RxBegin(
    bool signalType [[maybe_unused]], uint32_t senderNodeId [[maybe_unused]],
    double rxPower [[maybe_unused]], Time duration [[maybe_unused]]) {
  NS_LOG_FUNCTION(this << signalType << senderNodeId << rxPower << duration);
  m_countRxBegin++;
}

void TestSpectrumChannelWithBandwidthFilter::DoSetup() {
  NS_LOG_FUNCTION(this);
  Ptr<SingleModelSpectrumChannel> channel =
      CreateObject<SingleModelSpectrumChannel>();

  Ptr<WifiBandwidthFilter> wifiFilter = CreateObject<WifiBandwidthFilter>();
  channel->AddSpectrumTransmitFilter(wifiFilter);

  Ptr<Node> node = CreateObject<Node>();
  Ptr<WifiNetDevice> dev = CreateObject<WifiNetDevice>();
  m_tx = CreateObject<SpectrumWifiPhy>();
  m_tx->SetDevice(dev);
  m_tx->SetTxPowerStart(20);
  m_tx->SetTxPowerEnd(20);

  Ptr<Node> nodeRx = CreateObject<Node>();
  Ptr<WifiNetDevice> devRx = CreateObject<WifiNetDevice>();
  m_rx = CreateObject<SpectrumWifiPhy>();
  m_rx->SetDevice(devRx);

  Ptr<InterferenceHelper> interferenceTx = CreateObject<InterferenceHelper>();
  m_tx->SetInterferenceHelper(interferenceTx);
  Ptr<ErrorRateModel> errorTx = CreateObject<NistErrorRateModel>();
  m_tx->SetErrorRateModel(errorTx);

  Ptr<InterferenceHelper> interferenceRx = CreateObject<InterferenceHelper>();
  m_rx->SetInterferenceHelper(interferenceRx);
  Ptr<ErrorRateModel> errorRx = CreateObject<NistErrorRateModel>();
  m_rx->SetErrorRateModel(errorRx);

  m_tx->AddChannel(channel);
  m_rx->AddChannel(channel);

  m_tx->ConfigureStandard(WIFI_STANDARD_80211ax);
  m_rx->ConfigureStandard(WIFI_STANDARD_80211ax);

  dev->SetPhy(m_tx);
  node->AddDevice(dev);
  devRx->SetPhy(m_rx);
  nodeRx->AddDevice(devRx);

  m_rx->TraceConnectWithoutContext(
      "SignalArrival",
      MakeCallback(&TestSpectrumChannelWithBandwidthFilter::RxBegin, this));
}

void TestSpectrumChannelWithBandwidthFilter::DoTeardown() {
  m_tx->Dispose();
  m_rx->Dispose();
}

void TestSpectrumChannelWithBandwidthFilter::DoRun() {
  NS_LOG_FUNCTION(this);
  m_tx->SetOperatingChannel(
      WifiPhy::ChannelTuple{m_channel, 0, WIFI_PHY_BAND_5GHZ, 0});
  m_rx->SetOperatingChannel(
      WifiPhy::ChannelTuple{36, 0, WIFI_PHY_BAND_5GHZ, 0});

  Simulator::Schedule(MilliSeconds(100),
                      &TestSpectrumChannelWithBandwidthFilter::Send, this);
  Simulator::Schedule(
      MilliSeconds(101),
      &TestSpectrumChannelWithBandwidthFilter::CheckRxPacketCount, this,
      m_expectedValue);

  Simulator::Run();
  Simulator::Destroy();
}

class WifiPhyReceptionTestSuite : public TestSuite {
public:
  WifiPhyReceptionTestSuite();
};

WifiPhyReceptionTestSuite::WifiPhyReceptionTestSuite()
    : TestSuite("wifi-phy-reception", UNIT) {
  AddTestCase(new TestThresholdPreambleDetectionWithoutFrameCapture,
              TestCase::QUICK);
  AddTestCase(new TestThresholdPreambleDetectionWithFrameCapture,
              TestCase::QUICK);
  AddTestCase(new TestSimpleFrameCaptureModel, TestCase::QUICK);
  AddTestCase(new TestPhyHeadersReception, TestCase::QUICK);
  AddTestCase(new TestAmpduReception, TestCase::QUICK);
  AddTestCase(new TestUnsupportedModulationReception(), TestCase::QUICK);
  AddTestCase(new TestUnsupportedBandwidthReception(), TestCase::QUICK);
  AddTestCase(new TestPrimary20CoveredByPpdu(), TestCase::QUICK);
  AddTestCase(new TestSpectrumChannelWithBandwidthFilter(36, 1),
              TestCase::QUICK);
  AddTestCase(new TestSpectrumChannelWithBandwidthFilter(40, 1),
              TestCase::QUICK);
  AddTestCase(new TestSpectrumChannelWithBandwidthFilter(44, 0),
              TestCase::QUICK);
}

static WifiPhyReceptionTestSuite wifiPhyReceptionTestSuite;
