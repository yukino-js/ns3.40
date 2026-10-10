
#include "ns3/boolean.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/he-phy.h"
#include "ns3/interference-helper.h"
#include "ns3/log.h"
#include "ns3/multi-model-spectrum-channel.h"
#include "ns3/nist-error-rate-model.h"
#include "ns3/ofdm-ppdu.h"
#include "ns3/pointer.h"
#include "ns3/spectrum-wifi-helper.h"
#include "ns3/spectrum-wifi-phy.h"
#include "ns3/string.h"
#include "ns3/test.h"
#include "ns3/wifi-mac-header.h"
#include "ns3/wifi-net-device.h"
#include "ns3/wifi-phy-listener.h"
#include "ns3/wifi-psdu.h"
#include "ns3/wifi-spectrum-phy-interface.h"
#include "ns3/wifi-spectrum-signal-parameters.h"
#include "ns3/wifi-spectrum-value-helper.h"
#include "ns3/wifi-utils.h"

#include <memory>
#include <tuple>
#include <vector>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SpectrumWifiPhyTest");

static const uint8_t CHANNEL_NUMBER = 36;
static const uint16_t CHANNEL_WIDTH = 20;
static const uint16_t GUARD_WIDTH = CHANNEL_WIDTH;

class ExtSpectrumWifiPhy : public SpectrumWifiPhy {
public:
  using SpectrumWifiPhy::SpectrumWifiPhy;
  using WifiPhy::GetBand;
};

class SpectrumWifiPhyBasicTest : public TestCase {
public:
  SpectrumWifiPhyBasicTest();
  SpectrumWifiPhyBasicTest(std::string name);
  ~SpectrumWifiPhyBasicTest() override;

protected:
  void DoSetup() override;
  void DoTeardown() override;
  Ptr<SpectrumWifiPhy> m_phy;
  Ptr<SpectrumSignalParameters>
  MakeSignal(double txPowerWatts, const WifiPhyOperatingChannel &channel);
  void SendSignal(double txPowerWatts);
  void SpectrumWifiPhyRxSuccess(Ptr<const WifiPsdu> psdu,
                                RxSignalInfo rxSignalInfo,
                                WifiTxVector txVector,
                                std::vector<bool> statusPerMpdu);
  void SpectrumWifiPhyRxFailure(Ptr<const WifiPsdu> psdu);
  uint32_t m_count;

private:
  void DoRun() override;

  uint64_t m_uid;
};

SpectrumWifiPhyBasicTest::SpectrumWifiPhyBasicTest()
    : SpectrumWifiPhyBasicTest(
          "SpectrumWifiPhy test case receives one packet") {}

SpectrumWifiPhyBasicTest::SpectrumWifiPhyBasicTest(std::string name)
    : TestCase(name), m_count(0), m_uid(0) {}

Ptr<SpectrumSignalParameters>
SpectrumWifiPhyBasicTest::MakeSignal(double txPowerWatts,
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

  Ptr<WifiPpdu> ppdu = Create<OfdmPpdu>(psdu, txVector, channel, m_uid++);

  Ptr<SpectrumValue> txPowerSpectrum =
      WifiSpectrumValueHelper::CreateOfdmTxPowerSpectralDensity(
          channel.GetPrimaryChannelCenterFrequency(CHANNEL_WIDTH),
          CHANNEL_WIDTH, txPowerWatts, GUARD_WIDTH);
  Ptr<WifiSpectrumSignalParameters> txParams =
      Create<WifiSpectrumSignalParameters>();
  txParams->psd = txPowerSpectrum;
  txParams->txPhy = nullptr;
  txParams->duration = txDuration;
  txParams->ppdu = ppdu;

  return txParams;
}

void SpectrumWifiPhyBasicTest::SendSignal(double txPowerWatts) {
  m_phy->StartRx(MakeSignal(txPowerWatts, m_phy->GetOperatingChannel()),
                 nullptr);
}

void SpectrumWifiPhyBasicTest::SpectrumWifiPhyRxSuccess(
    Ptr<const WifiPsdu> psdu, RxSignalInfo rxSignalInfo, WifiTxVector txVector,
    std::vector<bool> statusPerMpdu) {
  NS_LOG_FUNCTION(this << *psdu << rxSignalInfo << txVector);
  m_count++;
}

void SpectrumWifiPhyBasicTest::SpectrumWifiPhyRxFailure(
    Ptr<const WifiPsdu> psdu) {
  NS_LOG_FUNCTION(this << *psdu);
  m_count++;
}

SpectrumWifiPhyBasicTest::~SpectrumWifiPhyBasicTest() {}

void SpectrumWifiPhyBasicTest::DoSetup() {
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
  m_phy->ConfigureStandard(WIFI_STANDARD_80211n);
  m_phy->SetReceiveOkCallback(
      MakeCallback(&SpectrumWifiPhyBasicTest::SpectrumWifiPhyRxSuccess, this));
  m_phy->SetReceiveErrorCallback(
      MakeCallback(&SpectrumWifiPhyBasicTest::SpectrumWifiPhyRxFailure, this));
  dev->SetPhy(m_phy);
  node->AddDevice(dev);
}

void SpectrumWifiPhyBasicTest::DoTeardown() {
  m_phy->Dispose();
  m_phy = nullptr;
}

void SpectrumWifiPhyBasicTest::DoRun() {
  double txPowerWatts = 0.010;
  Simulator::Schedule(Seconds(1), &SpectrumWifiPhyBasicTest::SendSignal, this,
                      txPowerWatts);
  Simulator::Schedule(Seconds(2), &SpectrumWifiPhyBasicTest::SendSignal, this,
                      txPowerWatts);
  Simulator::Schedule(Seconds(3), &SpectrumWifiPhyBasicTest::SendSignal, this,
                      txPowerWatts);
  Simulator::Schedule(MicroSeconds(4000000),
                      &SpectrumWifiPhyBasicTest::SendSignal, this,
                      txPowerWatts);
  Simulator::Schedule(MicroSeconds(4000001),
                      &SpectrumWifiPhyBasicTest::SendSignal, this,
                      txPowerWatts);
  Simulator::Run();
  Simulator::Destroy();

  NS_TEST_ASSERT_MSG_EQ(m_count, 3, "Didn't receive right number of packets");
}

class TestPhyListener : public ns3::WifiPhyListener {
public:
  TestPhyListener() = default;
  ~TestPhyListener() override = default;

  void NotifyRxStart(Time duration) override {
    NS_LOG_FUNCTION(this << duration);
    ++m_notifyRxStart;
  }

  void NotifyRxEndOk() override {
    NS_LOG_FUNCTION(this);
    ++m_notifyRxEndOk;
  }

  void NotifyRxEndError() override {
    NS_LOG_FUNCTION(this);
    ++m_notifyRxEndError;
  }

  void NotifyTxStart(Time duration, double txPowerDbm) override {
    NS_LOG_FUNCTION(this << duration << txPowerDbm);
  }

  void NotifyCcaBusyStart(Time duration, WifiChannelListType channelType,
                          const std::vector<Time> &) override {
    NS_LOG_FUNCTION(this << duration << channelType);
    if (duration.IsStrictlyPositive()) {
      ++m_notifyMaybeCcaBusyStart;
      if (!m_ccaBusyStart.IsStrictlyPositive()) {
        m_ccaBusyStart = Simulator::Now();
      }
      m_ccaBusyEnd = std::max(m_ccaBusyEnd, Simulator::Now() + duration);
    }
  }

  void NotifySwitchingStart(Time duration) override {}

  void NotifySleep() override {}

  void NotifyOff() override {}

  void NotifyWakeup() override {}

  void NotifyOn() override {}

  void Reset() {
    NS_LOG_FUNCTION(this);
    m_notifyRxStart = 0;
    m_notifyRxEndOk = 0;
    m_notifyRxEndError = 0;
    m_notifyMaybeCcaBusyStart = 0;
    m_ccaBusyStart = Seconds(0);
    m_ccaBusyEnd = Seconds(0);
  }

  uint32_t m_notifyRxStart{0};
  uint32_t m_notifyRxEndOk{0};
  uint32_t m_notifyRxEndError{0};
  uint32_t m_notifyMaybeCcaBusyStart{0};
  Time m_ccaBusyStart{0};
  Time m_ccaBusyEnd{0};
};

class SpectrumWifiPhyListenerTest : public SpectrumWifiPhyBasicTest {
public:
  SpectrumWifiPhyListenerTest();
  ~SpectrumWifiPhyListenerTest() override;

private:
  void DoSetup() override;
  void DoRun() override;
  TestPhyListener *m_listener;
};

SpectrumWifiPhyListenerTest::SpectrumWifiPhyListenerTest()
    : SpectrumWifiPhyBasicTest(
          "SpectrumWifiPhy test operation of WifiPhyListener") {}

SpectrumWifiPhyListenerTest::~SpectrumWifiPhyListenerTest() {}

void SpectrumWifiPhyListenerTest::DoSetup() {
  SpectrumWifiPhyBasicTest::DoSetup();
  m_listener = new TestPhyListener;
  m_phy->RegisterListener(m_listener);
}

void SpectrumWifiPhyListenerTest::DoRun() {
  double txPowerWatts = 0.010;
  Simulator::Schedule(Seconds(1), &SpectrumWifiPhyListenerTest::SendSignal,
                      this, txPowerWatts);
  Simulator::Run();

  NS_TEST_ASSERT_MSG_EQ(m_count, 1, "Didn't receive right number of packets");
  NS_TEST_ASSERT_MSG_EQ(
      m_listener->m_notifyMaybeCcaBusyStart, 2,
      "Didn't receive NotifyCcaBusyStart (once preamble is detected + "
      "prolonged by L-SIG "
      "reception, then switched to Rx by at the beginning of data)");
  NS_TEST_ASSERT_MSG_EQ(m_listener->m_notifyRxStart, 1,
                        "Didn't receive NotifyRxStart");
  NS_TEST_ASSERT_MSG_EQ(m_listener->m_notifyRxEndOk, 1,
                        "Didn't receive NotifyRxEnd");

  Simulator::Destroy();
  delete m_listener;
}

class SpectrumWifiPhyFilterTest : public TestCase {
public:
  SpectrumWifiPhyFilterTest();
  SpectrumWifiPhyFilterTest(std::string name);
  ~SpectrumWifiPhyFilterTest() override;

private:
  void DoSetup() override;
  void DoTeardown() override;
  void DoRun() override;

  void RunOne();

  void SendPpdu();

  void RxCallback(Ptr<const Packet> p, RxPowerWattPerChannelBand rxPowersW);

  Ptr<ExtSpectrumWifiPhy> m_txPhy;
  Ptr<ExtSpectrumWifiPhy> m_rxPhy;

  uint16_t m_txChannelWidth;
  uint16_t m_rxChannelWidth;

  std::set<WifiSpectrumBandIndices> m_ruBands;
};

SpectrumWifiPhyFilterTest::SpectrumWifiPhyFilterTest()
    : TestCase("SpectrumWifiPhy test RX filters"), m_txChannelWidth(20),
      m_rxChannelWidth(20) {}

SpectrumWifiPhyFilterTest::SpectrumWifiPhyFilterTest(std::string name)
    : TestCase(name) {}

void SpectrumWifiPhyFilterTest::SendPpdu() {
  WifiTxVector txVector =
      WifiTxVector(HePhy::GetHeMcs0(), 0, WIFI_PREAMBLE_HE_SU, 800, 1, 1, 0,
                   m_txChannelWidth, false, false);
  Ptr<Packet> pkt = Create<Packet>(1000);
  WifiMacHeader hdr;
  hdr.SetType(WIFI_MAC_QOSDATA);
  hdr.SetQosTid(0);
  hdr.SetAddr1(Mac48Address("00:00:00:00:00:01"));
  hdr.SetSequenceNumber(1);
  Ptr<WifiPsdu> psdu = Create<WifiPsdu>(pkt, hdr);
  m_txPhy->Send(WifiConstPsduMap({std::make_pair(SU_STA_ID, psdu)}), txVector);
}

SpectrumWifiPhyFilterTest::~SpectrumWifiPhyFilterTest() {
  m_txPhy = nullptr;
  m_rxPhy = nullptr;
}

void SpectrumWifiPhyFilterTest::RxCallback(
    Ptr<const Packet> p, RxPowerWattPerChannelBand rxPowersW) {
  for (const auto &pair : rxPowersW) {
    NS_LOG_INFO("band: (" << pair.first << ") -> powerW=" << pair.second << " ("
                          << WToDbm(pair.second) << " dBm)");
  }

  size_t numBands = rxPowersW.size();
  size_t expectedNumBands = std::max(1, (m_rxChannelWidth / 20));
  expectedNumBands += (m_rxChannelWidth / 40);
  expectedNumBands += (m_rxChannelWidth / 80);
  expectedNumBands += (m_rxChannelWidth / 160);
  expectedNumBands += m_ruBands.size();

  NS_TEST_ASSERT_MSG_EQ(
      numBands, expectedNumBands,
      "Total number of bands handled by the receiver is incorrect");

  uint16_t channelWidth = std::min(m_txChannelWidth, m_rxChannelWidth);
  auto band = m_rxPhy->GetBand(channelWidth, 0);
  auto it = rxPowersW.find(band);
  NS_LOG_INFO("powerW total band: " << it->second << " (" << WToDbm(it->second)
                                    << " dBm)");
  int totalRxPower = static_cast<int>(WToDbm(it->second) + 0.5);
  int expectedTotalRxPower;
  if (m_txChannelWidth <= m_rxChannelWidth) {
    expectedTotalRxPower = 16;
  } else {
    expectedTotalRxPower =
        16 - static_cast<int>(RatioToDb(m_txChannelWidth / m_rxChannelWidth));
  }
  NS_TEST_ASSERT_MSG_EQ(totalRxPower, expectedTotalRxPower,
                        "Total received power is not correct");

  if ((m_txChannelWidth <= m_rxChannelWidth) && (channelWidth >= 20)) {
    band = m_rxPhy->GetBand(20, 0);
    it = rxPowersW.find(band);
    NS_LOG_INFO("powerW in primary 20 MHz channel: "
                << it->second << " (" << WToDbm(it->second) << " dBm)");
    int rxPowerPrimaryChannel20 = static_cast<int>(WToDbm(it->second) + 0.5);
    int expectedRxPowerPrimaryChannel20 =
        16 - static_cast<int>(RatioToDb(channelWidth / 20));
    NS_TEST_ASSERT_MSG_EQ(
        rxPowerPrimaryChannel20, expectedRxPowerPrimaryChannel20,
        "Received power in the primary 20 MHz band is not correct");
  }
}

void SpectrumWifiPhyFilterTest::DoSetup() {

  Ptr<MultiModelSpectrumChannel> spectrumChannel =
      CreateObject<MultiModelSpectrumChannel>();
  Ptr<FriisPropagationLossModel> lossModel =
      CreateObject<FriisPropagationLossModel>();
  lossModel->SetFrequency(5.180e9);
  spectrumChannel->AddPropagationLossModel(lossModel);
  Ptr<ConstantSpeedPropagationDelayModel> delayModel =
      CreateObject<ConstantSpeedPropagationDelayModel>();
  spectrumChannel->SetPropagationDelayModel(delayModel);

  Ptr<Node> txNode = CreateObject<Node>();
  Ptr<WifiNetDevice> txDev = CreateObject<WifiNetDevice>();
  m_txPhy = CreateObject<ExtSpectrumWifiPhy>();
  Ptr<InterferenceHelper> txInterferenceHelper =
      CreateObject<InterferenceHelper>();
  m_txPhy->SetInterferenceHelper(txInterferenceHelper);
  Ptr<ErrorRateModel> txErrorModel = CreateObject<NistErrorRateModel>();
  m_txPhy->SetErrorRateModel(txErrorModel);
  m_txPhy->SetDevice(txDev);
  m_txPhy->AddChannel(spectrumChannel);
  m_txPhy->ConfigureStandard(WIFI_STANDARD_80211ax);
  Ptr<ConstantPositionMobilityModel> apMobility =
      CreateObject<ConstantPositionMobilityModel>();
  m_txPhy->SetMobility(apMobility);
  txDev->SetPhy(m_txPhy);
  txNode->AggregateObject(apMobility);
  txNode->AddDevice(txDev);

  Ptr<Node> rxNode = CreateObject<Node>();
  Ptr<WifiNetDevice> rxDev = CreateObject<WifiNetDevice>();
  m_rxPhy = CreateObject<ExtSpectrumWifiPhy>();
  Ptr<InterferenceHelper> rxInterferenceHelper =
      CreateObject<InterferenceHelper>();
  m_rxPhy->SetInterferenceHelper(rxInterferenceHelper);
  Ptr<ErrorRateModel> rxErrorModel = CreateObject<NistErrorRateModel>();
  m_rxPhy->SetErrorRateModel(rxErrorModel);
  m_rxPhy->AddChannel(spectrumChannel);
  m_rxPhy->ConfigureStandard(WIFI_STANDARD_80211ax);
  Ptr<ConstantPositionMobilityModel> sta1Mobility =
      CreateObject<ConstantPositionMobilityModel>();
  m_rxPhy->SetMobility(sta1Mobility);
  rxDev->SetPhy(m_rxPhy);
  rxNode->AggregateObject(sta1Mobility);
  rxNode->AddDevice(rxDev);
  m_rxPhy->TraceConnectWithoutContext(
      "PhyRxBegin", MakeCallback(&SpectrumWifiPhyFilterTest::RxCallback, this));
}

void SpectrumWifiPhyFilterTest::DoTeardown() {
  m_txPhy->Dispose();
  m_txPhy = nullptr;
  m_rxPhy->Dispose();
  m_rxPhy = nullptr;
}

void SpectrumWifiPhyFilterTest::RunOne() {
  uint16_t txFrequency;
  switch (m_txChannelWidth) {
  case 20:
  default:
    txFrequency = 5180;
    break;
  case 40:
    txFrequency = 5190;
    break;
  case 80:
    txFrequency = 5210;
    break;
  case 160:
    txFrequency = 5250;
    break;
  }
  auto txChannelNum = std::get<0>(*WifiPhyOperatingChannel::FindFirst(
      0, txFrequency, m_txChannelWidth, WIFI_STANDARD_80211ax,
      WIFI_PHY_BAND_5GHZ));
  m_txPhy->SetOperatingChannel(WifiPhy::ChannelTuple{
      txChannelNum, m_txChannelWidth, (int)(WIFI_PHY_BAND_5GHZ), 0});

  uint16_t rxFrequency;
  switch (m_rxChannelWidth) {
  case 20:
  default:
    rxFrequency = 5180;
    break;
  case 40:
    rxFrequency = 5190;
    break;
  case 80:
    rxFrequency = 5210;
    break;
  case 160:
    rxFrequency = 5250;
    break;
  }
  auto rxChannelNum = std::get<0>(*WifiPhyOperatingChannel::FindFirst(
      0, rxFrequency, m_rxChannelWidth, WIFI_STANDARD_80211ax,
      WIFI_PHY_BAND_5GHZ));
  m_rxPhy->SetOperatingChannel(WifiPhy::ChannelTuple{
      rxChannelNum, m_rxChannelWidth, (int)(WIFI_PHY_BAND_5GHZ), 0});

  m_ruBands.clear();
  for (uint16_t bw = 160; bw >= 20; bw = bw / 2) {
    for (uint16_t i = 0; i < (m_rxChannelWidth / bw); ++i) {
      for (unsigned int type = 0; type < 7; type++) {
        auto ruType = static_cast<HeRu::RuType>(type);
        for (std::size_t index = 1; index <= HeRu::GetNRus(bw, ruType);
             index++) {
          HeRu::SubcarrierGroup subcarrierGroup =
              HeRu::GetSubcarrierGroup(bw, ruType, index);
          HeRu::SubcarrierRange subcarrierRange = std::make_pair(
              subcarrierGroup.front().first, subcarrierGroup.back().second);
          const auto band = HePhy::ConvertHeRuSubcarriers(
              bw, m_rxPhy->GetGuardBandwidth(m_rxChannelWidth),
              m_rxPhy->GetSubcarrierSpacing(), subcarrierRange, i);
          m_ruBands.insert(band);
        }
      }
    }
  }

  Simulator::Schedule(Seconds(1), &SpectrumWifiPhyFilterTest::SendPpdu, this);

  Simulator::Run();
}

void SpectrumWifiPhyFilterTest::DoRun() {
  m_txChannelWidth = 20;
  m_rxChannelWidth = 20;
  RunOne();

  m_txChannelWidth = 40;
  m_rxChannelWidth = 40;
  RunOne();

  m_txChannelWidth = 80;
  m_rxChannelWidth = 80;
  RunOne();

  m_txChannelWidth = 160;
  m_rxChannelWidth = 160;
  RunOne();

  m_txChannelWidth = 20;
  m_rxChannelWidth = 40;
  RunOne();

  m_txChannelWidth = 20;
  m_rxChannelWidth = 80;
  RunOne();

  m_txChannelWidth = 40;
  m_rxChannelWidth = 80;
  RunOne();

  m_txChannelWidth = 20;
  m_rxChannelWidth = 160;
  RunOne();

  m_txChannelWidth = 40;
  m_rxChannelWidth = 160;
  RunOne();

  m_txChannelWidth = 80;
  m_rxChannelWidth = 160;
  RunOne();

  m_txChannelWidth = 40;
  m_rxChannelWidth = 20;
  RunOne();

  m_txChannelWidth = 80;
  m_rxChannelWidth = 20;
  RunOne();

  m_txChannelWidth = 80;
  m_rxChannelWidth = 40;
  RunOne();

  m_txChannelWidth = 160;
  m_rxChannelWidth = 20;
  RunOne();

  m_txChannelWidth = 160;
  m_rxChannelWidth = 40;
  RunOne();

  m_txChannelWidth = 160;
  m_rxChannelWidth = 80;
  RunOne();

  Simulator::Destroy();
}

class SpectrumWifiPhyMultipleInterfacesTest : public TestCase {
public:
  SpectrumWifiPhyMultipleInterfacesTest(bool trackSignalsInactiveInterfaces);

private:
  void DoSetup() override;
  void DoTeardown() override;
  void DoRun() override;

  void SwitchChannel(std::size_t index, WifiPhyBand band, uint8_t channelNumber,
                     uint16_t channelWidth);

  void SendPpdu(Ptr<SpectrumWifiPhy> phy, double txPowerDbm);

  void RxCallback(std::size_t index, Ptr<const Packet> packet,
                  RxPowerWattPerChannelBand rxPowersW);

  void CheckInterferences(Ptr<SpectrumWifiPhy> phy,
                          const FrequencyRange &freqRange,
                          const WifiSpectrumBandInfo &band,
                          bool interferencesExpected);

  void DoCheckInterferences(Ptr<SpectrumWifiPhy> phy,
                            const WifiSpectrumBandInfo &band,
                            bool interferencesExpected);

  void
  CheckResults(std::size_t index, uint32_t expectedNumRx,
               FrequencyRange expectedFrequencyRangeActiveRfInterface,
               const std::vector<std::size_t> &expectedConnectedPhysPerChannel);

  void CheckCcaIndication(std::size_t index, bool expectedCcaBusyIndication,
                          Time switchingDelay);

  void Reset();

  bool m_trackSignalsInactiveInterfaces;

  std::vector<Ptr<MultiModelSpectrumChannel>> m_spectrumChannels;
  std::vector<Ptr<SpectrumWifiPhy>> m_txPhys{};
  std::vector<Ptr<SpectrumWifiPhy>> m_rxPhys{};
  std::vector<std::unique_ptr<TestPhyListener>> m_listeners{};

  std::vector<uint32_t> m_counts{0};

  Time m_lastTxStart{0};
  Time m_lastTxEnd{0};
};

SpectrumWifiPhyMultipleInterfacesTest::SpectrumWifiPhyMultipleInterfacesTest(
    bool trackSignalsInactiveInterfaces)
    : TestCase{"SpectrumWifiPhy test operation with multiple RF interfaces"},
      m_trackSignalsInactiveInterfaces{trackSignalsInactiveInterfaces} {}

void SpectrumWifiPhyMultipleInterfacesTest::SwitchChannel(
    std::size_t index, WifiPhyBand band, uint8_t channelNumber,
    uint16_t channelWidth) {
  NS_LOG_FUNCTION(this << index << band << +channelNumber << channelWidth);
  auto &listener = m_listeners.at(index);
  listener->m_notifyMaybeCcaBusyStart = 0;
  listener->m_ccaBusyStart = Seconds(0);
  listener->m_ccaBusyEnd = Seconds(0);
  auto phy = m_rxPhys.at(index);
  phy->SetOperatingChannel(
      WifiPhy::ChannelTuple{channelNumber, channelWidth, band, 0});
}

void SpectrumWifiPhyMultipleInterfacesTest::SendPpdu(Ptr<SpectrumWifiPhy> phy,
                                                     double txPowerDbm) {
  NS_LOG_FUNCTION(this << phy << txPowerDbm << phy->GetCurrentFrequencyRange()
                       << phy->GetChannelWidth() << phy->GetChannelNumber());

  WifiTxVector txVector =
      WifiTxVector(HePhy::GetHeMcs0(), 0, WIFI_PREAMBLE_HE_SU, 800, 1, 1, 0, 20,
                   false, false);
  Ptr<Packet> pkt = Create<Packet>(1000);
  WifiMacHeader hdr;
  hdr.SetType(WIFI_MAC_QOSDATA);
  hdr.SetQosTid(0);
  hdr.SetAddr1(Mac48Address("00:00:00:00:00:01"));
  hdr.SetSequenceNumber(1);
  Ptr<WifiPsdu> psdu = Create<WifiPsdu>(pkt, hdr);

  m_lastTxStart = Simulator::Now();
  m_lastTxEnd = m_lastTxStart +
                WifiPhy::CalculateTxDuration({std::make_pair(SU_STA_ID, psdu)},
                                             txVector, phy->GetPhyBand());
  phy->SetTxPowerStart(txPowerDbm);
  phy->SetTxPowerEnd(txPowerDbm);
  phy->Send(WifiConstPsduMap({std::make_pair(SU_STA_ID, psdu)}), txVector);
}

void SpectrumWifiPhyMultipleInterfacesTest::RxCallback(
    std::size_t index, Ptr<const Packet>, RxPowerWattPerChannelBand) {
  auto phy = m_rxPhys.at(index);
  NS_LOG_FUNCTION(this << index << phy->GetCurrentFrequencyRange()
                       << phy->GetChannelWidth() << phy->GetChannelNumber());
  m_counts.at(index)++;
}

void SpectrumWifiPhyMultipleInterfacesTest::CheckInterferences(
    Ptr<SpectrumWifiPhy> phy, const FrequencyRange &freqRange,
    const WifiSpectrumBandInfo &band, bool interferencesExpected) {
  if ((!m_trackSignalsInactiveInterfaces) &&
      (phy->GetCurrentFrequencyRange() != freqRange)) {
    return;
  }
  Simulator::ScheduleNow(
      &SpectrumWifiPhyMultipleInterfacesTest::DoCheckInterferences, this, phy,
      band, interferencesExpected);
}

void SpectrumWifiPhyMultipleInterfacesTest::DoCheckInterferences(
    Ptr<SpectrumWifiPhy> phy, const WifiSpectrumBandInfo &band,
    bool interferencesExpected) {
  NS_LOG_FUNCTION(this << phy << band << interferencesExpected);
  PointerValue ptr;
  phy->GetAttribute("InterferenceHelper", ptr);
  auto interferenceHelper =
      DynamicCast<InterferenceHelper>(ptr.Get<InterferenceHelper>());
  NS_ASSERT(interferenceHelper);
  const auto energyDuration = interferenceHelper->GetEnergyDuration(0, band);
  NS_TEST_ASSERT_MSG_EQ(energyDuration.IsStrictlyPositive(),
                        interferencesExpected,
                        "Incorrect interferences detection");
}

void SpectrumWifiPhyMultipleInterfacesTest::CheckResults(
    std::size_t index, uint32_t expectedNumRx,
    FrequencyRange expectedFrequencyRangeActiveRfInterface,
    const std::vector<std::size_t> &expectedConnectedPhysPerChannel) {
  NS_LOG_FUNCTION(this << index << expectedNumRx
                       << expectedFrequencyRangeActiveRfInterface);
  const auto phy = m_rxPhys.at(index);
  std::size_t numActiveInterfaces = 0;
  for (const auto &[freqRange, interface] : phy->GetSpectrumPhyInterfaces()) {
    const auto expectedActive =
        (freqRange == expectedFrequencyRangeActiveRfInterface);
    const auto isActive = (interface == phy->GetCurrentInterface());
    NS_TEST_ASSERT_MSG_EQ(isActive, expectedActive,
                          "Incorrect active interface");
    if (isActive) {
      numActiveInterfaces++;
    }
  }
  NS_TEST_ASSERT_MSG_EQ(numActiveInterfaces, 1,
                        "There should always be one active interface");
  NS_ASSERT(expectedConnectedPhysPerChannel.size() ==
            m_spectrumChannels.size());
  for (std::size_t i = 0; i < m_spectrumChannels.size(); ++i) {
    NS_TEST_ASSERT_MSG_EQ(
        m_spectrumChannels.at(i)->GetNDevices(),
        expectedConnectedPhysPerChannel.at(i),
        "Incorrect number of PHYs attached to the spectrum channel");
  }
  NS_TEST_ASSERT_MSG_EQ(m_counts.at(index), expectedNumRx,
                        "Didn't receive right number of packets");
  NS_TEST_ASSERT_MSG_EQ(m_listeners.at(index)->m_notifyRxStart, expectedNumRx,
                        "Didn't receive NotifyRxStart");
}

void SpectrumWifiPhyMultipleInterfacesTest::CheckCcaIndication(
    std::size_t index, bool expectedCcaBusyIndication, Time switchingDelay) {
  const auto expectedCcaBusyStart =
      expectedCcaBusyIndication ? m_lastTxStart + switchingDelay : Seconds(0);
  const auto expectedCcaBusyEnd =
      expectedCcaBusyIndication ? m_lastTxEnd : Seconds(0);
  NS_LOG_FUNCTION(this << index << expectedCcaBusyIndication
                       << expectedCcaBusyStart << expectedCcaBusyEnd);
  auto &listener = m_listeners.at(index);
  const auto ccaBusyIndication = (listener->m_notifyMaybeCcaBusyStart > 0);
  const auto ccaBusyStart = listener->m_ccaBusyStart;
  const auto ccaBusyEnd = listener->m_ccaBusyEnd;
  NS_TEST_ASSERT_MSG_EQ(ccaBusyIndication, expectedCcaBusyIndication,
                        "CCA busy indication check failed");
  NS_TEST_ASSERT_MSG_EQ(ccaBusyStart, expectedCcaBusyStart,
                        "CCA busy start mismatch");
  NS_TEST_ASSERT_MSG_EQ(ccaBusyEnd, expectedCcaBusyEnd,
                        "CCA busy end mismatch");
}

void SpectrumWifiPhyMultipleInterfacesTest::Reset() {
  NS_LOG_FUNCTION(this);
  for (auto &count : m_counts) {
    count = 0;
  }
  for (auto &listener : m_listeners) {
    listener->Reset();
  }
  for (std::size_t rxPhyIndex = 0; rxPhyIndex < m_rxPhys.size(); ++rxPhyIndex) {
    auto txPhy = m_txPhys.at(rxPhyIndex);
    SwitchChannel(rxPhyIndex, txPhy->GetPhyBand(), txPhy->GetChannelNumber(),
                  txPhy->GetChannelWidth());
  }
}

void SpectrumWifiPhyMultipleInterfacesTest::DoSetup() {
  NS_LOG_FUNCTION(this);

  NodeContainer wifiApNode(1);
  NodeContainer wifiStaNode(1);

  WifiHelper wifi;
  wifi.SetStandard(WIFI_STANDARD_80211be);

  SpectrumWifiPhyHelper phyHelper(4);
  phyHelper.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);

  struct SpectrumPhyInterfaceInfo {
    FrequencyRange range;
    uint8_t number;
    WifiPhyBand band;
    std::string bandName;
  };

  const FrequencyRange WIFI_SPECTRUM_5_GHZ_LOW{
      WIFI_SPECTRUM_5_GHZ.minFrequency,
      WIFI_SPECTRUM_5_GHZ.minFrequency + ((WIFI_SPECTRUM_5_GHZ.maxFrequency -
                                           WIFI_SPECTRUM_5_GHZ.minFrequency) /
                                          2)};
  const FrequencyRange WIFI_SPECTRUM_5_GHZ_HIGH{
      WIFI_SPECTRUM_5_GHZ.minFrequency + ((WIFI_SPECTRUM_5_GHZ.maxFrequency -
                                           WIFI_SPECTRUM_5_GHZ.minFrequency) /
                                          2),
      WIFI_SPECTRUM_5_GHZ.maxFrequency};

  const std::vector<SpectrumPhyInterfaceInfo> interfaces{
      {WIFI_SPECTRUM_2_4_GHZ, 2, WIFI_PHY_BAND_2_4GHZ, "BAND_2_4GHZ"},
      {WIFI_SPECTRUM_5_GHZ_LOW, 42, WIFI_PHY_BAND_5GHZ, "BAND_5GHZ"},
      {WIFI_SPECTRUM_5_GHZ_HIGH, 163, WIFI_PHY_BAND_5GHZ, "BAND_5GHZ"},
      {WIFI_SPECTRUM_6_GHZ, 215, WIFI_PHY_BAND_6GHZ, "BAND_6GHZ"}};

  for (std::size_t i = 0; i < interfaces.size(); ++i) {
    auto spectrumChannel = CreateObject<MultiModelSpectrumChannel>();
    [[maybe_unused]] const auto [channel, frequency, channelWidth, type, band] =
        (*WifiPhyOperatingChannel::FindFirst(interfaces.at(i).number, 0, 0,
                                             WIFI_STANDARD_80211be,
                                             interfaces.at(i).band));

    std::ostringstream oss;
    oss << "{" << +interfaces.at(i).number << ", 0, "
        << interfaces.at(i).bandName << ", 0}";
    phyHelper.Set(i, "ChannelSettings", StringValue(oss.str()));
    phyHelper.AddChannel(spectrumChannel, interfaces.at(i).range);

    m_spectrumChannels.emplace_back(spectrumChannel);
  }

  WifiMacHelper mac;
  mac.SetType("ns3::ApWifiMac", "BeaconGeneration", BooleanValue(false));
  phyHelper.Set("TrackSignalsFromInactiveInterfaces", BooleanValue(false));
  auto apDevice = wifi.Install(phyHelper, mac, wifiApNode.Get(0));

  mac.SetType("ns3::StaWifiMac", "ActiveProbing", BooleanValue(false));
  phyHelper.Set("TrackSignalsFromInactiveInterfaces",
                BooleanValue(m_trackSignalsInactiveInterfaces));
  auto staDevice = wifi.Install(phyHelper, mac, wifiStaNode.Get(0));

  for (std::size_t i = 0; i < interfaces.size(); ++i) {
    auto txPhy = DynamicCast<SpectrumWifiPhy>(
        DynamicCast<WifiNetDevice>(apDevice.Get(0))->GetPhy(i));
    m_txPhys.push_back(txPhy);

    const auto index = m_rxPhys.size();
    auto rxPhy = DynamicCast<SpectrumWifiPhy>(
        DynamicCast<WifiNetDevice>(staDevice.Get(0))->GetPhy(i));
    rxPhy->TraceConnectWithoutContext(
        "PhyRxBegin",
        MakeCallback(&SpectrumWifiPhyMultipleInterfacesTest::RxCallback, this)
            .Bind(index));

    auto listener = std::make_unique<TestPhyListener>();
    rxPhy->RegisterListener(listener.get());
    m_listeners.push_back(std::move(listener));

    m_rxPhys.push_back(rxPhy);
    m_counts.push_back(0);
  }
}

void SpectrumWifiPhyMultipleInterfacesTest::DoTeardown() {
  NS_LOG_FUNCTION(this);
  for (auto &phy : m_txPhys) {
    phy->Dispose();
    phy = nullptr;
  }
  for (auto &phy : m_rxPhys) {
    phy->Dispose();
    phy = nullptr;
  }
  Simulator::Destroy();
}

void SpectrumWifiPhyMultipleInterfacesTest::DoRun() {
  NS_LOG_FUNCTION(this);

  const auto ccaEdThresholdDbm = -62.0;
  const auto txAfterChannelSwitchDelay = Seconds(0.25);
  const auto checkResultsDelay = Seconds(0.5);
  const auto flushResultsDelay = Seconds(0.9);
  const auto txOngoingAfterTxStartedDelay = MicroSeconds(50);

  Time delay{0};

  std::vector<std::size_t> expectedConnectedPhysPerChannel =
      m_trackSignalsInactiveInterfaces ? std::vector<std::size_t>{5, 5, 5, 5}
                                       : std::vector<std::size_t>{2, 2, 2, 2};
  for (std::size_t i = 0; i < 4; ++i) {
    auto txPpduPhy = m_txPhys.at(i);
    delay += Seconds(1);
    Simulator::Schedule(delay, &SpectrumWifiPhyMultipleInterfacesTest::SendPpdu,
                        this, txPpduPhy, 0);
    for (std::size_t j = 0; j < 4; ++j) {
      auto txPhy = m_txPhys.at(j);
      auto rxPhy = m_rxPhys.at(j);
      const auto &expectedFreqRange = txPhy->GetCurrentFrequencyRange();
      Simulator::Schedule(
          delay + txOngoingAfterTxStartedDelay,
          &SpectrumWifiPhyMultipleInterfacesTest::CheckInterferences, this,
          rxPhy, txPpduPhy->GetCurrentFrequencyRange(),
          txPpduPhy->GetBand(txPpduPhy->GetChannelWidth(), 0), true);
      Simulator::Schedule(delay + checkResultsDelay,
                          &SpectrumWifiPhyMultipleInterfacesTest::CheckResults,
                          this, j, (i == j) ? 1 : 0, expectedFreqRange,
                          expectedConnectedPhysPerChannel);
    }
    Simulator::Schedule(delay + flushResultsDelay,
                        &SpectrumWifiPhyMultipleInterfacesTest::Reset, this);
  }

  for (std::size_t i = 0; i < 4; ++i) {
    delay += Seconds(1);
    auto txPpduPhy = m_txPhys.at(i);
    Simulator::Schedule(delay + txAfterChannelSwitchDelay,
                        &SpectrumWifiPhyMultipleInterfacesTest::SendPpdu, this,
                        txPpduPhy, 0);
    const auto &expectedFreqRange = txPpduPhy->GetCurrentFrequencyRange();
    for (std::size_t j = 0; j < 4; ++j) {
      if (!m_trackSignalsInactiveInterfaces) {
        for (std::size_t k = 0; k < expectedConnectedPhysPerChannel.size();
             ++k) {
          expectedConnectedPhysPerChannel.at(k) = (k == i) ? 5 : 1;
        }
      }
      auto rxPhy = m_rxPhys.at(j);
      Simulator::Schedule(
          delay, &SpectrumWifiPhyMultipleInterfacesTest::SwitchChannel, this, j,
          txPpduPhy->GetPhyBand(), txPpduPhy->GetChannelNumber(),
          txPpduPhy->GetChannelWidth());
      Simulator::Schedule(
          delay + txAfterChannelSwitchDelay + txOngoingAfterTxStartedDelay,
          &SpectrumWifiPhyMultipleInterfacesTest::CheckInterferences, this,
          rxPhy, txPpduPhy->GetCurrentFrequencyRange(),
          txPpduPhy->GetBand(txPpduPhy->GetChannelWidth(), 0), true);
      Simulator::Schedule(delay + checkResultsDelay,
                          &SpectrumWifiPhyMultipleInterfacesTest::CheckResults,
                          this, j, 1, expectedFreqRange,
                          expectedConnectedPhysPerChannel);
    }
    Simulator::Schedule(delay + flushResultsDelay,
                        &SpectrumWifiPhyMultipleInterfacesTest::Reset, this);
  }

  const auto secondSpectrumChannelIndex = 1;
  auto channel36TxPhy = m_txPhys.at(secondSpectrumChannelIndex);
  const auto &expectedFreqRange = channel36TxPhy->GetCurrentFrequencyRange();
  for (std::size_t i = 0; i < 4; ++i) {
    delay += Seconds(1);
    auto txPpduPhy = m_txPhys.at(i);
    Simulator::Schedule(delay + txAfterChannelSwitchDelay,
                        &SpectrumWifiPhyMultipleInterfacesTest::SendPpdu, this,
                        txPpduPhy, 0);
    for (std::size_t j = 0; j < 4; ++j) {
      if (!m_trackSignalsInactiveInterfaces) {
        for (std::size_t k = 0; k < expectedConnectedPhysPerChannel.size();
             ++k) {
          expectedConnectedPhysPerChannel.at(k) =
              (k == secondSpectrumChannelIndex) ? 5 : 1;
        }
      }
      Simulator::Schedule(
          delay, &SpectrumWifiPhyMultipleInterfacesTest::SwitchChannel, this, j,
          WIFI_PHY_BAND_5GHZ, CHANNEL_NUMBER, CHANNEL_WIDTH);
      Simulator::Schedule(delay + checkResultsDelay,
                          &SpectrumWifiPhyMultipleInterfacesTest::CheckResults,
                          this, j, (i == secondSpectrumChannelIndex) ? 1 : 0,
                          expectedFreqRange, expectedConnectedPhysPerChannel);
    }
    Simulator::Schedule(delay + flushResultsDelay,
                        &SpectrumWifiPhyMultipleInterfacesTest::Reset, this);
  }

  for (const auto txPowerDbm : {-60.0, -70.0}) {
    for (std::size_t i = 0; i < 4; ++i) {
      for (std::size_t j = 0; j < 4; ++j) {
        auto txPpduPhy = m_txPhys.at(i);
        const auto startChannel = WifiPhyOperatingChannel::FindFirst(
            txPpduPhy->GetPrimaryChannelNumber(20), 0, 20,
            WIFI_STANDARD_80211ax, txPpduPhy->GetPhyBand());
        for (uint16_t bw = txPpduPhy->GetChannelWidth(); bw >= 20; bw /= 2) {
          [[maybe_unused]] const auto [channel, frequency, channelWidth, type,
                                       band] =
              (*WifiPhyOperatingChannel::FindFirst(
                  0, 0, bw, WIFI_STANDARD_80211ax, txPpduPhy->GetPhyBand(),
                  startChannel));
          delay += Seconds(1);
          Simulator::Schedule(delay,
                              &SpectrumWifiPhyMultipleInterfacesTest::SendPpdu,
                              this, txPpduPhy, txPowerDbm);
          Simulator::Schedule(
              delay + txOngoingAfterTxStartedDelay,
              &SpectrumWifiPhyMultipleInterfacesTest::SwitchChannel, this, j,
              band, channel, channelWidth);
          for (std::size_t k = 0; k < 4; ++k) {
            if ((i != j) && (k == i)) {
              continue;
            }
            const auto expectCcaBusyIndication =
                (k == i) ? (txPowerDbm >= ccaEdThresholdDbm)
                         : (m_trackSignalsInactiveInterfaces
                                ? ((txPowerDbm >= ccaEdThresholdDbm) ? (j == k)
                                                                     : false)
                                : false);
            Simulator::Schedule(
                delay + checkResultsDelay,
                &SpectrumWifiPhyMultipleInterfacesTest::CheckCcaIndication,
                this, k, expectCcaBusyIndication, txOngoingAfterTxStartedDelay);
          }
          Simulator::Schedule(delay + flushResultsDelay,
                              &SpectrumWifiPhyMultipleInterfacesTest::Reset,
                              this);
        }
      }
    }
  }

  Simulator::Stop(Seconds(30));
  Simulator::Run();
}

class SpectrumWifiPhyInterfacesHelperTest : public TestCase {
public:
  SpectrumWifiPhyInterfacesHelperTest();
  ~SpectrumWifiPhyInterfacesHelperTest() override = default;

private:
  void DoRun() override;
};

SpectrumWifiPhyInterfacesHelperTest::SpectrumWifiPhyInterfacesHelperTest()
    : TestCase("Check PHY interfaces added to PHY instances using helper") {}

void SpectrumWifiPhyInterfacesHelperTest::DoRun() {
  WifiHelper wifiHelper;
  wifiHelper.SetStandard(WIFI_STANDARD_80211be);

  SpectrumWifiPhyHelper phyHelper(3);
  phyHelper.Set(0, "ChannelSettings", StringValue("{2, 0, BAND_2_4GHZ, 0}"));
  phyHelper.Set(1, "ChannelSettings", StringValue("{36, 0, BAND_5GHZ, 0}"));
  phyHelper.Set(2, "ChannelSettings", StringValue("{1, 0, BAND_6GHZ, 0}"));

  phyHelper.AddChannel(CreateObject<MultiModelSpectrumChannel>(),
                       WIFI_SPECTRUM_2_4_GHZ);
  phyHelper.AddChannel(CreateObject<MultiModelSpectrumChannel>(),
                       WIFI_SPECTRUM_5_GHZ);
  phyHelper.AddChannel(CreateObject<MultiModelSpectrumChannel>(),
                       WIFI_SPECTRUM_6_GHZ);

  WifiMacHelper macHelper;
  NodeContainer nodes(4);

  auto device = wifiHelper.Install(phyHelper, macHelper, nodes.Get(0));

  auto phyLink0 = DynamicCast<SpectrumWifiPhy>(
      DynamicCast<WifiNetDevice>(device.Get(0))->GetPhy(0));
  NS_ASSERT(phyLink0);
  NS_TEST_ASSERT_MSG_EQ(
      phyLink0->GetSpectrumPhyInterfaces().size(), 3,
      "Incorrect number of PHY interfaces added to PHY link ID 0");

  auto phyLink1 = DynamicCast<SpectrumWifiPhy>(
      DynamicCast<WifiNetDevice>(device.Get(0))->GetPhy(1));
  NS_ASSERT(phyLink1);
  NS_TEST_ASSERT_MSG_EQ(
      phyLink1->GetSpectrumPhyInterfaces().size(), 3,
      "Incorrect number of PHY interfaces added to PHY link ID 1");

  auto phyLink2 = DynamicCast<SpectrumWifiPhy>(
      DynamicCast<WifiNetDevice>(device.Get(0))->GetPhy(2));
  NS_ASSERT(phyLink2);
  NS_TEST_ASSERT_MSG_EQ(
      phyLink1->GetSpectrumPhyInterfaces().size(), 3,
      "Incorrect number of PHY interfaces added to PHY link ID 2");

  phyHelper.AddPhyToFreqRangeMapping(0, WIFI_SPECTRUM_2_4_GHZ);
  phyHelper.AddPhyToFreqRangeMapping(1, WIFI_SPECTRUM_5_GHZ);
  phyHelper.AddPhyToFreqRangeMapping(2, WIFI_SPECTRUM_6_GHZ);
  device = wifiHelper.Install(phyHelper, macHelper, nodes.Get(1));

  phyLink0 = DynamicCast<SpectrumWifiPhy>(
      DynamicCast<WifiNetDevice>(device.Get(0))->GetPhy(0));
  NS_ASSERT(phyLink0);
  NS_TEST_ASSERT_MSG_EQ(
      phyLink0->GetSpectrumPhyInterfaces().size(), 1,
      "Incorrect number of PHY interfaces added to PHY link ID 0");
  NS_TEST_ASSERT_MSG_EQ(
      phyLink0->GetSpectrumPhyInterfaces().count(WIFI_SPECTRUM_2_4_GHZ), 1,
      "Incorrect PHY interfaces added to PHY link ID 0");

  phyLink1 = DynamicCast<SpectrumWifiPhy>(
      DynamicCast<WifiNetDevice>(device.Get(0))->GetPhy(1));
  NS_ASSERT(phyLink1);
  NS_TEST_ASSERT_MSG_EQ(
      phyLink1->GetSpectrumPhyInterfaces().size(), 1,
      "Incorrect number of PHY interfaces added to PHY link ID 1");
  NS_TEST_ASSERT_MSG_EQ(
      phyLink1->GetSpectrumPhyInterfaces().count(WIFI_SPECTRUM_5_GHZ), 1,
      "Incorrect PHY interfaces added to PHY link ID 1");

  phyLink2 = DynamicCast<SpectrumWifiPhy>(
      DynamicCast<WifiNetDevice>(device.Get(0))->GetPhy(2));
  NS_ASSERT(phyLink2);
  NS_TEST_ASSERT_MSG_EQ(
      phyLink2->GetSpectrumPhyInterfaces().size(), 1,
      "Incorrect number of PHY interfaces added to PHY link ID 2");
  NS_TEST_ASSERT_MSG_EQ(
      phyLink2->GetSpectrumPhyInterfaces().count(WIFI_SPECTRUM_6_GHZ), 1,
      "Incorrect PHY interfaces added to PHY link ID 2");

  phyHelper.AddPhyToFreqRangeMapping(0, WIFI_SPECTRUM_5_GHZ);
  device = wifiHelper.Install(phyHelper, macHelper, nodes.Get(2));

  phyLink0 = DynamicCast<SpectrumWifiPhy>(
      DynamicCast<WifiNetDevice>(device.Get(0))->GetPhy(0));
  NS_ASSERT(phyLink0);
  NS_TEST_ASSERT_MSG_EQ(
      phyLink0->GetSpectrumPhyInterfaces().size(), 2,
      "Incorrect number of PHY interfaces added to PHY link ID 0");
  NS_TEST_ASSERT_MSG_EQ(
      phyLink0->GetSpectrumPhyInterfaces().count(WIFI_SPECTRUM_2_4_GHZ), 1,
      "Incorrect PHY interfaces added to PHY link ID 0");
  NS_TEST_ASSERT_MSG_EQ(
      phyLink0->GetSpectrumPhyInterfaces().count(WIFI_SPECTRUM_5_GHZ), 1,
      "Incorrect PHY interfaces added to PHY link ID 0");

  phyLink1 = DynamicCast<SpectrumWifiPhy>(
      DynamicCast<WifiNetDevice>(device.Get(0))->GetPhy(1));
  NS_ASSERT(phyLink1);
  NS_TEST_ASSERT_MSG_EQ(
      phyLink1->GetSpectrumPhyInterfaces().size(), 1,
      "Incorrect number of PHY interfaces added to PHY link ID 1");
  NS_TEST_ASSERT_MSG_EQ(
      phyLink1->GetSpectrumPhyInterfaces().count(WIFI_SPECTRUM_5_GHZ), 1,
      "Incorrect PHY interfaces added to PHY link ID 1");

  phyLink2 = DynamicCast<SpectrumWifiPhy>(
      DynamicCast<WifiNetDevice>(device.Get(0))->GetPhy(2));
  NS_ASSERT(phyLink2);
  NS_TEST_ASSERT_MSG_EQ(
      phyLink2->GetSpectrumPhyInterfaces().size(), 1,
      "Incorrect number of PHY interfaces added to PHY link ID 2");
  NS_TEST_ASSERT_MSG_EQ(
      phyLink2->GetSpectrumPhyInterfaces().count(WIFI_SPECTRUM_6_GHZ), 1,
      "Incorrect PHY interfaces added to PHY link ID 2");

  phyHelper.ResetPhyToFreqRangeMapping();
  device = wifiHelper.Install(phyHelper, macHelper, nodes.Get(3));

  phyLink0 = DynamicCast<SpectrumWifiPhy>(
      DynamicCast<WifiNetDevice>(device.Get(0))->GetPhy(0));
  NS_ASSERT(phyLink0);
  NS_TEST_ASSERT_MSG_EQ(
      phyLink0->GetSpectrumPhyInterfaces().size(), 3,
      "Incorrect number of PHY interfaces added to PHY link ID 0");
  NS_TEST_ASSERT_MSG_EQ(
      phyLink0->GetSpectrumPhyInterfaces().count(WIFI_SPECTRUM_2_4_GHZ), 1,
      "Incorrect PHY interfaces added to PHY link ID 0");
  NS_TEST_ASSERT_MSG_EQ(
      phyLink0->GetSpectrumPhyInterfaces().count(WIFI_SPECTRUM_5_GHZ), 1,
      "Incorrect PHY interfaces added to PHY link ID 0");
  NS_TEST_ASSERT_MSG_EQ(
      phyLink0->GetSpectrumPhyInterfaces().count(WIFI_SPECTRUM_6_GHZ), 1,
      "Incorrect PHY interfaces added to PHY link ID 0");

  phyLink1 = DynamicCast<SpectrumWifiPhy>(
      DynamicCast<WifiNetDevice>(device.Get(0))->GetPhy(1));
  NS_ASSERT(phyLink1);
  NS_TEST_ASSERT_MSG_EQ(
      phyLink1->GetSpectrumPhyInterfaces().size(), 3,
      "Incorrect number of PHY interfaces added to PHY link ID 1");
  NS_TEST_ASSERT_MSG_EQ(
      phyLink1->GetSpectrumPhyInterfaces().count(WIFI_SPECTRUM_2_4_GHZ), 1,
      "Incorrect PHY interfaces added to PHY link ID 0");
  NS_TEST_ASSERT_MSG_EQ(
      phyLink1->GetSpectrumPhyInterfaces().count(WIFI_SPECTRUM_5_GHZ), 1,
      "Incorrect PHY interfaces added to PHY link ID 0");
  NS_TEST_ASSERT_MSG_EQ(
      phyLink1->GetSpectrumPhyInterfaces().count(WIFI_SPECTRUM_6_GHZ), 1,
      "Incorrect PHY interfaces added to PHY link ID 0");

  phyLink2 = DynamicCast<SpectrumWifiPhy>(
      DynamicCast<WifiNetDevice>(device.Get(0))->GetPhy(2));
  NS_ASSERT(phyLink2);
  NS_TEST_ASSERT_MSG_EQ(
      phyLink2->GetSpectrumPhyInterfaces().size(), 3,
      "Incorrect number of PHY interfaces added to PHY link ID 2");
  NS_TEST_ASSERT_MSG_EQ(
      phyLink2->GetSpectrumPhyInterfaces().count(WIFI_SPECTRUM_2_4_GHZ), 1,
      "Incorrect PHY interfaces added to PHY link ID 0");
  NS_TEST_ASSERT_MSG_EQ(
      phyLink2->GetSpectrumPhyInterfaces().count(WIFI_SPECTRUM_5_GHZ), 1,
      "Incorrect PHY interfaces added to PHY link ID 0");
  NS_TEST_ASSERT_MSG_EQ(
      phyLink2->GetSpectrumPhyInterfaces().count(WIFI_SPECTRUM_6_GHZ), 1,
      "Incorrect PHY interfaces added to PHY link ID 0");

  Simulator::Destroy();
}

class SpectrumWifiPhyTestSuite : public TestSuite {
public:
  SpectrumWifiPhyTestSuite();
};

SpectrumWifiPhyTestSuite::SpectrumWifiPhyTestSuite()
    : TestSuite("wifi-spectrum-wifi-phy", UNIT) {
  AddTestCase(new SpectrumWifiPhyBasicTest, TestCase::QUICK);
  AddTestCase(new SpectrumWifiPhyListenerTest, TestCase::QUICK);
  AddTestCase(new SpectrumWifiPhyFilterTest, TestCase::QUICK);
  AddTestCase(new SpectrumWifiPhyMultipleInterfacesTest(false),
              TestCase::QUICK);
  AddTestCase(new SpectrumWifiPhyMultipleInterfacesTest(true), TestCase::QUICK);
  AddTestCase(new SpectrumWifiPhyInterfacesHelperTest, TestCase::QUICK);
}

static SpectrumWifiPhyTestSuite spectrumWifiPhyTestSuite;
