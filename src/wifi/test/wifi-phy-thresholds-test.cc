
#include "ns3/interference-helper.h"
#include "ns3/log.h"
#include "ns3/multi-model-spectrum-channel.h"
#include "ns3/nist-error-rate-model.h"
#include "ns3/ofdm-phy.h"
#include "ns3/ofdm-ppdu.h"
#include "ns3/spectrum-phy.h"
#include "ns3/spectrum-wifi-helper.h"
#include "ns3/spectrum-wifi-phy.h"
#include "ns3/test.h"
#include "ns3/wifi-mac-header.h"
#include "ns3/wifi-net-device.h"
#include "ns3/wifi-psdu.h"
#include "ns3/wifi-spectrum-phy-interface.h"
#include "ns3/wifi-spectrum-signal-parameters.h"
#include "ns3/wifi-spectrum-value-helper.h"
#include "ns3/wifi-utils.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("WifiPhyThresholdsTest");

static const uint8_t CHANNEL_NUMBER = 36;
static const uint32_t FREQUENCY = 5180;
static const uint16_t CHANNEL_WIDTH = 20;

class WifiPhyThresholdsTest : public TestCase {
public:
  WifiPhyThresholdsTest(std::string test_name);
  ~WifiPhyThresholdsTest() override;

protected:
  virtual Ptr<SpectrumSignalParameters>
  MakeWifiSignal(double txPowerWatts, const WifiPhyOperatingChannel &channel);
  virtual Ptr<SpectrumSignalParameters> MakeForeignSignal(double txPowerWatts);
  virtual void SendSignal(double txPowerWatts, bool wifiSignal);
  virtual void RxSuccess(Ptr<const WifiPsdu> psdu, RxSignalInfo rxSignalInfo,
                         WifiTxVector txVector,
                         std::vector<bool> statusPerMpdu);
  virtual void RxFailure(Ptr<const WifiPsdu> psdu);
  void RxDropped(Ptr<const Packet> p, WifiPhyRxfailureReason reason);
  virtual void PhyStateChanged(Time start, Time duration,
                               WifiPhyState newState);

  Ptr<SpectrumWifiPhy> m_phy;
  uint32_t m_rxSuccess;
  uint32_t m_rxFailure;
  uint32_t m_rxDropped;
  uint32_t m_stateChanged;
  uint32_t m_rxStateCount;
  uint32_t m_idleStateCount;
  uint32_t m_ccabusyStateCount;

private:
  void DoSetup() override;
  void DoTeardown() override;
};

WifiPhyThresholdsTest::WifiPhyThresholdsTest(std::string test_name)
    : TestCase(test_name), m_rxSuccess(0), m_rxFailure(0), m_rxDropped(0),
      m_stateChanged(0), m_rxStateCount(0), m_idleStateCount(0),
      m_ccabusyStateCount(0) {}

WifiPhyThresholdsTest::~WifiPhyThresholdsTest() {}

Ptr<SpectrumSignalParameters>
WifiPhyThresholdsTest::MakeWifiSignal(double txPowerWatts,
                                      const WifiPhyOperatingChannel &channel) {
  WifiTxVector txVector =
      WifiTxVector(OfdmPhy::GetOfdmRate6Mbps(), 0, WIFI_PREAMBLE_LONG, 800, 1,
                   1, 0, CHANNEL_WIDTH, false);

  Ptr<Packet> pkt = Create<Packet>(1000);
  WifiMacHeader hdr;

  hdr.SetType(WIFI_MAC_QOSDATA);
  hdr.SetQosTid(0);

  Ptr<WifiPsdu> psdu = Create<WifiPsdu>(pkt, hdr);
  Time txDuration = m_phy->CalculateTxDuration(psdu->GetSize(), txVector,
                                               m_phy->GetPhyBand());

  Ptr<WifiPpdu> ppdu = Create<OfdmPpdu>(psdu, txVector, channel, 0);

  Ptr<SpectrumValue> txPowerSpectrum =
      WifiSpectrumValueHelper::CreateHeOfdmTxPowerSpectralDensity(
          channel.GetPrimaryChannelCenterFrequency(CHANNEL_WIDTH),
          CHANNEL_WIDTH, txPowerWatts, CHANNEL_WIDTH);
  Ptr<WifiSpectrumSignalParameters> txParams =
      Create<WifiSpectrumSignalParameters>();
  txParams->psd = txPowerSpectrum;
  txParams->txPhy = nullptr;
  txParams->duration = txDuration;
  txParams->ppdu = ppdu;
  return txParams;
}

Ptr<SpectrumSignalParameters>
WifiPhyThresholdsTest::MakeForeignSignal(double txPowerWatts) {
  Ptr<SpectrumValue> txPowerSpectrum =
      WifiSpectrumValueHelper::CreateHeOfdmTxPowerSpectralDensity(
          FREQUENCY, CHANNEL_WIDTH, txPowerWatts, CHANNEL_WIDTH);
  Ptr<SpectrumSignalParameters> txParams = Create<SpectrumSignalParameters>();
  txParams->psd = txPowerSpectrum;
  txParams->txPhy = nullptr;
  txParams->duration = Seconds(0.5);
  return txParams;
}

void WifiPhyThresholdsTest::SendSignal(double txPowerWatts, bool wifiSignal) {
  if (wifiSignal) {
    m_phy->StartRx(MakeWifiSignal(txPowerWatts, m_phy->GetOperatingChannel()),
                   nullptr);
  } else {
    m_phy->StartRx(MakeForeignSignal(txPowerWatts), nullptr);
  }
}

void WifiPhyThresholdsTest::RxSuccess(Ptr<const WifiPsdu> psdu,
                                      RxSignalInfo rxSignalInfo,
                                      WifiTxVector txVector,
                                      std::vector<bool> statusPerMpdu) {
  NS_LOG_FUNCTION(this << *psdu << rxSignalInfo << txVector);
  m_rxSuccess++;
}

void WifiPhyThresholdsTest::RxFailure(Ptr<const WifiPsdu> psdu) {
  NS_LOG_FUNCTION(this << *psdu);
  m_rxFailure++;
}

void WifiPhyThresholdsTest::RxDropped(Ptr<const Packet> p,
                                      WifiPhyRxfailureReason reason) {
  NS_LOG_FUNCTION(this << p << reason);
  m_rxDropped++;
}

void WifiPhyThresholdsTest::PhyStateChanged(Time start, Time duration,
                                            WifiPhyState newState) {
  NS_LOG_FUNCTION(this << start << duration << newState);
  m_stateChanged++;
  if (newState == WifiPhyState::IDLE) {
    m_idleStateCount++;
  } else if (newState == WifiPhyState::RX) {
    m_rxStateCount++;
  } else if (newState == WifiPhyState::CCA_BUSY) {
    m_ccabusyStateCount++;
  }
}

void WifiPhyThresholdsTest::DoSetup() {
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
  m_phy->SetReceiveOkCallback(
      MakeCallback(&WifiPhyThresholdsTest::RxSuccess, this));
  m_phy->SetReceiveErrorCallback(
      MakeCallback(&WifiPhyThresholdsTest::RxFailure, this));
  m_phy->TraceConnectWithoutContext(
      "PhyRxDrop", MakeCallback(&WifiPhyThresholdsTest::RxDropped, this));
  m_phy->GetState()->TraceConnectWithoutContext(
      "State", MakeCallback(&WifiPhyThresholdsTest::PhyStateChanged, this));
  dev->SetPhy(m_phy);
  node->AddDevice(dev);
}

void WifiPhyThresholdsTest::DoTeardown() {
  m_phy->Dispose();
  m_phy = nullptr;
}

class WifiPhyThresholdsWeakWifiSignalTest : public WifiPhyThresholdsTest {
public:
  WifiPhyThresholdsWeakWifiSignalTest();
  ~WifiPhyThresholdsWeakWifiSignalTest() override;
  void DoRun() override;
};

WifiPhyThresholdsWeakWifiSignalTest::WifiPhyThresholdsWeakWifiSignalTest()
    : WifiPhyThresholdsTest(
          "WifiPhy reception thresholds: test weak wifi signal reception") {}

WifiPhyThresholdsWeakWifiSignalTest::~WifiPhyThresholdsWeakWifiSignalTest() {}

void WifiPhyThresholdsWeakWifiSignalTest::DoRun() {
  double txPowerWatts = DbmToW(-110);

  Simulator::Schedule(Seconds(1),
                      &WifiPhyThresholdsWeakWifiSignalTest::SendSignal, this,
                      txPowerWatts, true);

  Simulator::Run();
  Simulator::Destroy();

  NS_TEST_ASSERT_MSG_EQ(
      m_rxDropped + m_rxSuccess + m_rxFailure, 0,
      "Reception should not have been triggered if packet is weaker than "
      "RxSensitivity threshold");
  NS_TEST_ASSERT_MSG_EQ(
      m_stateChanged, 0,
      "State should stay idle if reception involves a signal weaker than "
      "RxSensitivity threshold");
}

class WifiPhyThresholdsWeakForeignSignalTest : public WifiPhyThresholdsTest {
public:
  WifiPhyThresholdsWeakForeignSignalTest();
  ~WifiPhyThresholdsWeakForeignSignalTest() override;
  void DoRun() override;
};

WifiPhyThresholdsWeakForeignSignalTest::WifiPhyThresholdsWeakForeignSignalTest()
    : WifiPhyThresholdsTest(
          "WifiPhy reception thresholds: test weak foreign signal reception") {}

WifiPhyThresholdsWeakForeignSignalTest::
    ~WifiPhyThresholdsWeakForeignSignalTest() {}

void WifiPhyThresholdsWeakForeignSignalTest::DoRun() {
  double txPowerWatts = DbmToW(-90);

  Simulator::Schedule(Seconds(1),
                      &WifiPhyThresholdsWeakForeignSignalTest::SendSignal, this,
                      txPowerWatts, false);

  Simulator::Run();
  Simulator::Destroy();

  NS_TEST_ASSERT_MSG_EQ(m_rxDropped + m_rxSuccess + m_rxFailure, 0,
                        "Reception of non-wifi packet should not be triggered");
  NS_TEST_ASSERT_MSG_EQ(
      m_stateChanged, 0,
      "State should stay idle if reception involves a signal weaker than "
      "RxSensitivity threshold");
}

class WifiPhyThresholdsStrongWifiSignalTest : public WifiPhyThresholdsTest {
public:
  WifiPhyThresholdsStrongWifiSignalTest();
  ~WifiPhyThresholdsStrongWifiSignalTest() override;
  void DoRun() override;
};

WifiPhyThresholdsStrongWifiSignalTest::WifiPhyThresholdsStrongWifiSignalTest()
    : WifiPhyThresholdsTest(
          "WifiPhy reception thresholds: test strong wifi signal reception") {}

WifiPhyThresholdsStrongWifiSignalTest::
    ~WifiPhyThresholdsStrongWifiSignalTest() {}

void WifiPhyThresholdsStrongWifiSignalTest::DoRun() {
  double txPowerWatts = DbmToW(-60);

  Simulator::Schedule(Seconds(1),
                      &WifiPhyThresholdsStrongWifiSignalTest::SendSignal, this,
                      txPowerWatts, true);

  Simulator::Run();
  Simulator::Destroy();

  NS_TEST_ASSERT_MSG_EQ(m_rxDropped + m_rxFailure, 0,
                        "Packet reception should have been successful");
  NS_TEST_ASSERT_MSG_EQ(m_rxSuccess, 1,
                        "Packet should have been successfully received");
  NS_TEST_ASSERT_MSG_EQ(m_ccabusyStateCount, 2,
                        "State should have moved to CCA_BUSY once");
  NS_TEST_ASSERT_MSG_EQ(m_stateChanged, 4,
                        "State should have moved to CCA_BUSY, then to RX and "
                        "finally back to IDLE");
  NS_TEST_ASSERT_MSG_EQ(m_rxStateCount, 1,
                        "State should have moved to RX once");
  NS_TEST_ASSERT_MSG_EQ(m_idleStateCount, 1,
                        "State should have moved to IDLE once");
}

class WifiPhyThresholdsStrongForeignSignalTest : public WifiPhyThresholdsTest {
public:
  WifiPhyThresholdsStrongForeignSignalTest();
  ~WifiPhyThresholdsStrongForeignSignalTest() override;
  void DoRun() override;
};

WifiPhyThresholdsStrongForeignSignalTest::
    WifiPhyThresholdsStrongForeignSignalTest()
    : WifiPhyThresholdsTest(
          "WifiPhy reception thresholds: test weak foreign signal reception") {}

WifiPhyThresholdsStrongForeignSignalTest::
    ~WifiPhyThresholdsStrongForeignSignalTest() {}

void WifiPhyThresholdsStrongForeignSignalTest::DoRun() {
  double txPowerWatts = DbmToW(-60);

  Simulator::Schedule(Seconds(1),
                      &WifiPhyThresholdsStrongForeignSignalTest::SendSignal,
                      this, txPowerWatts, false);

  Simulator::Run();
  Simulator::Destroy();

  NS_TEST_ASSERT_MSG_EQ(m_rxDropped + m_rxSuccess + m_rxFailure, 0,
                        "Reception of non-wifi packet should not be triggered");
  NS_TEST_ASSERT_MSG_EQ(
      m_idleStateCount, 1,
      "State should have moved to CCA-BUSY then back to IDLE");
}

class WifiPhyThresholdsTestSuite : public TestSuite {
public:
  WifiPhyThresholdsTestSuite();
};

WifiPhyThresholdsTestSuite::WifiPhyThresholdsTestSuite()
    : TestSuite("wifi-phy-thresholds", UNIT) {
  AddTestCase(new WifiPhyThresholdsWeakWifiSignalTest, TestCase::QUICK);
  AddTestCase(new WifiPhyThresholdsWeakForeignSignalTest, TestCase::QUICK);
  AddTestCase(new WifiPhyThresholdsStrongWifiSignalTest, TestCase::QUICK);
  AddTestCase(new WifiPhyThresholdsStrongForeignSignalTest, TestCase::QUICK);
}

static WifiPhyThresholdsTestSuite wifiPhyThresholdsTestSuite;
