
#include "ns3/ap-wifi-mac.h"
#include "ns3/boolean.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/he-configuration.h"
#include "ns3/he-phy.h"
#include "ns3/interference-helper.h"
#include "ns3/log.h"
#include "ns3/mobility-helper.h"
#include "ns3/multi-model-spectrum-channel.h"
#include "ns3/nist-error-rate-model.h"
#include "ns3/node.h"
#include "ns3/non-communicating-net-device.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/simulator.h"
#include "ns3/spectrum-wifi-helper.h"
#include "ns3/spectrum-wifi-phy.h"
#include "ns3/sta-wifi-mac.h"
#include "ns3/test.h"
#include "ns3/waveform-generator.h"
#include "ns3/wifi-mac-header.h"
#include "ns3/wifi-net-device.h"
#include "ns3/wifi-psdu.h"
#include "ns3/wifi-spectrum-signal-parameters.h"
#include "ns3/wifi-spectrum-value-helper.h"
#include "ns3/wifi-utils.h"

#include <vector>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("WifiNonHtDuplicateTest");

constexpr uint32_t DEFAULT_FREQUENCY = 5180;

class MuRtsCtsHePhy : public HePhy {
public:
  MuRtsCtsHePhy();
  ~MuRtsCtsHePhy() override;

  void SetPreviousTxPpduUid(uint64_t uid);

  void SetMuRtsTxVector(const WifiTxVector &muRtsTxVector);
};

MuRtsCtsHePhy::MuRtsCtsHePhy() : HePhy() { NS_LOG_FUNCTION(this); }

MuRtsCtsHePhy::~MuRtsCtsHePhy() { NS_LOG_FUNCTION(this); }

void MuRtsCtsHePhy::SetPreviousTxPpduUid(uint64_t uid) {
  NS_LOG_FUNCTION(this << uid);
  m_previouslyTxPpduUid = uid;
}

void MuRtsCtsHePhy::SetMuRtsTxVector(const WifiTxVector &muRtsTxVector) {
  NS_LOG_FUNCTION(this << muRtsTxVector);
  m_currentTxVector = muRtsTxVector;
}

class MuRtsCtsSpectrumWifiPhy : public SpectrumWifiPhy {
public:
  static TypeId GetTypeId();

  MuRtsCtsSpectrumWifiPhy();
  ~MuRtsCtsSpectrumWifiPhy() override;

  void DoInitialize() override;
  void DoDispose() override;

  void SetPpduUid(uint64_t uid);

  void SetMuRtsTxVector(const WifiTxVector &muRtsTxVector);

private:
  Ptr<MuRtsCtsHePhy> m_muRtsCtsHePhy;
};

TypeId MuRtsCtsSpectrumWifiPhy::GetTypeId() {
  static TypeId tid = TypeId("ns3::MuRtsCtsSpectrumWifiPhy")
                          .SetParent<SpectrumWifiPhy>()
                          .SetGroupName("Wifi");
  return tid;
}

MuRtsCtsSpectrumWifiPhy::MuRtsCtsSpectrumWifiPhy() : SpectrumWifiPhy() {
  NS_LOG_FUNCTION(this);
  m_muRtsCtsHePhy = Create<MuRtsCtsHePhy>();
  m_muRtsCtsHePhy->SetOwner(this);
}

MuRtsCtsSpectrumWifiPhy::~MuRtsCtsSpectrumWifiPhy() { NS_LOG_FUNCTION(this); }

void MuRtsCtsSpectrumWifiPhy::DoInitialize() {
  m_phyEntities[WIFI_MOD_CLASS_HE] = m_muRtsCtsHePhy;
  SpectrumWifiPhy::DoInitialize();
}

void MuRtsCtsSpectrumWifiPhy::DoDispose() {
  m_muRtsCtsHePhy = nullptr;
  SpectrumWifiPhy::DoDispose();
}

void MuRtsCtsSpectrumWifiPhy::SetPpduUid(uint64_t uid) {
  NS_LOG_FUNCTION(this << uid);
  m_muRtsCtsHePhy->SetPreviousTxPpduUid(uid);
  m_previouslyRxPpduUid = uid;
}

void MuRtsCtsSpectrumWifiPhy::SetMuRtsTxVector(
    const WifiTxVector &muRtsTxVector) {
  NS_LOG_FUNCTION(this << muRtsTxVector);
  m_muRtsCtsHePhy->SetMuRtsTxVector(muRtsTxVector);
}

class TestNonHtDuplicatePhyReception : public TestCase {
public:
  using StasParams = std::vector<std::tuple<WifiStandard, uint16_t, uint8_t>>;

  TestNonHtDuplicatePhyReception(WifiStandard apStandard, uint16_t apFrequency,
                                 uint8_t apP20Index, StasParams stasParams,
                                 std::vector<bool> per20MhzInterference = {});

private:
  void DoSetup() override;
  void DoTeardown() override;
  void DoRun() override;

  void RxSuccess(std::size_t index, Ptr<const WifiPsdu> psdu,
                 RxSignalInfo rxSignalInfo, WifiTxVector txVector,
                 std::vector<bool> statusPerMpdu);

  void RxFailure(std::size_t index, Ptr<const WifiPsdu> psdu);

  void CheckResults(std::size_t index, uint32_t expectedRxSuccess,
                    uint32_t expectedRxFailure);

  void ResetResults();

  void SendNonHtDuplicatePpdu(uint16_t channelWidth);

  void GenerateInterference(Ptr<WaveformGenerator> interferer,
                            Ptr<SpectrumValue> interferencePsd, Time duration);
  void StopInterference(Ptr<WaveformGenerator> interferer);

  WifiStandard m_apStandard;
  uint16_t m_apFrequency;
  uint8_t m_apP20Index;
  StasParams m_stasParams;
  std::vector<bool> m_per20MhzInterference;

  std::vector<uint32_t> m_countRxSuccessStas;
  std::vector<uint32_t> m_countRxFailureStas;

  Ptr<SpectrumWifiPhy> m_phyAp;
  std::vector<Ptr<SpectrumWifiPhy>> m_phyStas;

  std::vector<Ptr<WaveformGenerator>> m_phyInterferers;
};

TestNonHtDuplicatePhyReception::TestNonHtDuplicatePhyReception(
    WifiStandard apStandard, uint16_t apFrequency, uint8_t apP20Index,
    StasParams stasParams, std::vector<bool> per20MhzInterference)
    : TestCase{"non-HT duplicate PHY reception test"}, m_apStandard{apStandard},
      m_apFrequency{apFrequency}, m_apP20Index{apP20Index},
      m_stasParams{stasParams}, m_per20MhzInterference{per20MhzInterference},
      m_countRxSuccessStas{}, m_countRxFailureStas{}, m_phyStas{} {}

void TestNonHtDuplicatePhyReception::ResetResults() {
  for (auto &countRxSuccess : m_countRxSuccessStas) {
    countRxSuccess = 0;
  }
  for (auto &countRxFailure : m_countRxFailureStas) {
    countRxFailure = 0;
  }
}

void TestNonHtDuplicatePhyReception::SendNonHtDuplicatePpdu(
    uint16_t channelWidth) {
  NS_LOG_FUNCTION(this << channelWidth);
  WifiTxVector txVector =
      WifiTxVector(OfdmPhy::GetOfdmRate24Mbps(), 0, WIFI_PREAMBLE_LONG, 800, 1,
                   1, 0, channelWidth, false);

  Ptr<Packet> pkt = Create<Packet>(1000);
  WifiMacHeader hdr;

  hdr.SetType(WIFI_MAC_QOSDATA);
  hdr.SetQosTid(0);

  Ptr<WifiPsdu> psdu = Create<WifiPsdu>(pkt, hdr);
  Time txDuration = m_phyAp->CalculateTxDuration(psdu->GetSize(), txVector,
                                                 m_phyAp->GetPhyBand());

  m_phyAp->Send(WifiConstPsduMap({std::make_pair(SU_STA_ID, psdu)}), txVector);
}

void TestNonHtDuplicatePhyReception::GenerateInterference(
    Ptr<WaveformGenerator> interferer, Ptr<SpectrumValue> interferencePsd,
    Time duration) {
  NS_LOG_FUNCTION(this << interferer << duration);
  interferer->SetTxPowerSpectralDensity(interferencePsd);
  interferer->SetPeriod(duration);
  interferer->Start();
  Simulator::Schedule(duration,
                      &TestNonHtDuplicatePhyReception::StopInterference, this,
                      interferer);
}

void TestNonHtDuplicatePhyReception::StopInterference(
    Ptr<WaveformGenerator> interferer) {
  NS_LOG_FUNCTION(this << interferer);
  interferer->Stop();
}

void TestNonHtDuplicatePhyReception::RxSuccess(std::size_t index,
                                               Ptr<const WifiPsdu> psdu,
                                               RxSignalInfo rxSignalInfo,
                                               WifiTxVector txVector,
                                               std::vector<bool>) {
  NS_LOG_FUNCTION(this << index << *psdu << rxSignalInfo << txVector);
  const auto expectedWidth = std::min(m_phyAp->GetChannelWidth(),
                                      m_phyStas.at(index)->GetChannelWidth());
  NS_TEST_ASSERT_MSG_EQ(txVector.GetChannelWidth(), expectedWidth,
                        "Incorrect channel width in TXVECTOR");
  m_countRxSuccessStas.at(index)++;
}

void TestNonHtDuplicatePhyReception::RxFailure(std::size_t index,
                                               Ptr<const WifiPsdu> psdu) {
  NS_LOG_FUNCTION(this << index << *psdu);
  m_countRxFailureStas.at(index)++;
}

void TestNonHtDuplicatePhyReception::CheckResults(std::size_t index,
                                                  uint32_t expectedRxSuccess,
                                                  uint32_t expectedRxFailure) {
  NS_LOG_FUNCTION(this << index << expectedRxSuccess << expectedRxFailure);
  NS_TEST_ASSERT_MSG_EQ(m_countRxSuccessStas.at(index), expectedRxSuccess,
                        "The number of successfully received packets by STA "
                            << index << " is not correct!");
  NS_TEST_ASSERT_MSG_EQ(m_countRxFailureStas.at(index), expectedRxFailure,
                        "The number of unsuccessfully received packets by STA "
                            << index << " is not correct!");
}

void TestNonHtDuplicatePhyReception::DoSetup() {
  auto spectrumChannel = CreateObject<MultiModelSpectrumChannel>();
  auto lossModel = CreateObject<FriisPropagationLossModel>();
  lossModel->SetFrequency(m_apFrequency * 1e6);
  spectrumChannel->AddPropagationLossModel(lossModel);
  auto delayModel = CreateObject<ConstantSpeedPropagationDelayModel>();
  spectrumChannel->SetPropagationDelayModel(delayModel);

  auto apNode = CreateObject<Node>();
  auto apDev = CreateObject<WifiNetDevice>();
  m_phyAp = CreateObject<SpectrumWifiPhy>();
  auto apInterferenceHelper = CreateObject<InterferenceHelper>();
  m_phyAp->SetInterferenceHelper(apInterferenceHelper);
  auto apErrorModel = CreateObject<NistErrorRateModel>();
  m_phyAp->SetErrorRateModel(apErrorModel);
  m_phyAp->SetDevice(apDev);
  m_phyAp->AddChannel(spectrumChannel);
  m_phyAp->ConfigureStandard(WIFI_STANDARD_80211ax);
  auto apMobility = CreateObject<ConstantPositionMobilityModel>();
  m_phyAp->SetMobility(apMobility);
  apDev->SetPhy(m_phyAp);
  apNode->AggregateObject(apMobility);
  apNode->AddDevice(apDev);

  for (const auto &staParams : m_stasParams) {
    auto staNode = CreateObject<Node>();
    auto staDev = CreateObject<WifiNetDevice>();
    auto staPhy = CreateObject<SpectrumWifiPhy>();
    auto sta1InterferenceHelper = CreateObject<InterferenceHelper>();
    staPhy->SetInterferenceHelper(sta1InterferenceHelper);
    auto sta1ErrorModel = CreateObject<NistErrorRateModel>();
    staPhy->SetErrorRateModel(sta1ErrorModel);
    staPhy->SetDevice(staDev);
    staPhy->AddChannel(spectrumChannel);
    staPhy->ConfigureStandard(std::get<0>(staParams));
    staPhy->SetReceiveOkCallback(
        MakeCallback(&TestNonHtDuplicatePhyReception::RxSuccess, this)
            .Bind(m_phyStas.size()));
    staPhy->SetReceiveErrorCallback(
        MakeCallback(&TestNonHtDuplicatePhyReception::RxFailure, this)
            .Bind(m_phyStas.size()));
    auto staMobility = CreateObject<ConstantPositionMobilityModel>();
    staPhy->SetMobility(staMobility);
    staDev->SetPhy(staPhy);
    staNode->AggregateObject(staMobility);
    staNode->AddDevice(staDev);
    m_phyStas.push_back(staPhy);
    m_countRxSuccessStas.push_back(0);
    m_countRxFailureStas.push_back(0);
  }

  if (!m_per20MhzInterference.empty()) {
    [[maybe_unused]] auto [channelNum, centerFreq, apChannelWidth, type,
                           phyBand] =
        (*WifiPhyOperatingChannel::FindFirst(0, m_apFrequency, 0, m_apStandard,
                                             WIFI_PHY_BAND_5GHZ));
    NS_ASSERT(m_per20MhzInterference.size() == (apChannelWidth / 20));
    for (std::size_t i = 0; i < m_per20MhzInterference.size(); ++i) {
      auto interfererNode = CreateObject<Node>();
      auto interfererDev = CreateObject<NonCommunicatingNetDevice>();
      auto phyInterferer = CreateObject<WaveformGenerator>();
      phyInterferer->SetDevice(interfererDev);
      phyInterferer->SetChannel(spectrumChannel);
      phyInterferer->SetDutyCycle(1);
      interfererNode->AddDevice(interfererDev);
      m_phyInterferers.push_back(phyInterferer);
    }
  }
}

void TestNonHtDuplicatePhyReception::DoTeardown() {
  m_phyAp->Dispose();
  m_phyAp = nullptr;
  for (auto phySta : m_phyStas) {
    phySta->Dispose();
    phySta = nullptr;
  }
  for (auto phyInterferer : m_phyInterferers) {
    phyInterferer->Dispose();
    phyInterferer = nullptr;
  }
}

void TestNonHtDuplicatePhyReception::DoRun() {
  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(1);
  int64_t streamNumber = 0;
  m_phyAp->AssignStreams(streamNumber);
  for (auto phySta : m_phyStas) {
    phySta->AssignStreams(streamNumber);
  }

  [[maybe_unused]] auto [apChannelNum, centerFreq, apChannelWidth, type,
                         phyBand] =
      (*WifiPhyOperatingChannel::FindFirst(0, m_apFrequency, 0, m_apStandard,
                                           WIFI_PHY_BAND_5GHZ));
  m_phyAp->SetOperatingChannel(WifiPhy::ChannelTuple{
      apChannelNum, apChannelWidth, WIFI_PHY_BAND_5GHZ, m_apP20Index});

  auto index = 0;
  for (const auto &[staStandard, staFrequency, staP20Index] : m_stasParams) {
    [[maybe_unused]] auto [staChannelNum, centerFreq, staChannelWidth, type,
                           phyBand] =
        (*WifiPhyOperatingChannel::FindFirst(0, staFrequency, 0, staStandard,
                                             WIFI_PHY_BAND_5GHZ));
    m_phyStas.at(index++)->SetOperatingChannel(WifiPhy::ChannelTuple{
        staChannelNum, staChannelWidth, WIFI_PHY_BAND_5GHZ, staP20Index});
  }

  index = 0;
  const auto minApCenterFrequency =
      m_phyAp->GetFrequency() - (m_phyAp->GetChannelWidth() / 2) + (20 / 2);
  for (auto channelWidth = 20; channelWidth <= apChannelWidth;
       channelWidth *= 2, ++index) {
    if (!m_phyInterferers.empty()) {
      for (std::size_t i = 0; i < m_phyInterferers.size(); ++i) {
        if (!m_per20MhzInterference.at(i)) {
          continue;
        }
        BandInfo bandInfo;
        bandInfo.fc = (minApCenterFrequency + (i * 20)) * 1e6;
        bandInfo.fl = bandInfo.fc - (5 * 1e6);
        bandInfo.fh = bandInfo.fc + (5 * 1e6);
        Bands bands;
        bands.push_back(bandInfo);
        auto spectrumInterference = Create<SpectrumModel>(bands);
        auto interferencePsd = Create<SpectrumValue>(spectrumInterference);
        auto interferencePower = 0.005;
        *interferencePsd = interferencePower / 10e6;
        Simulator::Schedule(
            Seconds(index),
            &TestNonHtDuplicatePhyReception::GenerateInterference, this,
            m_phyInterferers.at(i), interferencePsd, Seconds(0.5));
      }
    }
    const auto apCenterFreq =
        m_phyAp->GetOperatingChannel().GetPrimaryChannelCenterFrequency(
            channelWidth);
    const auto apMinFreq = apCenterFreq - (channelWidth / 2);
    const auto apMaxFreq = apCenterFreq + (channelWidth / 2);
    Simulator::Schedule(Seconds(index + 0.1),
                        &TestNonHtDuplicatePhyReception::SendNonHtDuplicatePpdu,
                        this, channelWidth);
    for (std::size_t i = 0; i < m_stasParams.size(); ++i) {
      const auto p20Width = 20;
      const auto staP20Freq = m_phyStas.at(i)
                                  ->GetOperatingChannel()
                                  .GetPrimaryChannelCenterFrequency(p20Width);
      const auto staP20MinFreq = staP20Freq - (p20Width / 2);
      const auto staP20MaxFreq = staP20Freq + (p20Width / 2);
      bool expectRx =
          (staP20MinFreq >= apMinFreq && staP20MaxFreq <= apMaxFreq);
      bool expectSuccess = true;
      if (!m_per20MhzInterference.empty()) {
        const auto index20MhzSubBand =
            ((staP20Freq - minApCenterFrequency) / 20);
        expectSuccess = !m_per20MhzInterference.at(index20MhzSubBand);
      }
      Simulator::Schedule(
          Seconds(index + 0.5), &TestNonHtDuplicatePhyReception::CheckResults,
          this, i, expectRx ? expectSuccess : 0, expectRx ? !expectSuccess : 0);
    }
    Simulator::Schedule(Seconds(index + 0.5),
                        &TestNonHtDuplicatePhyReception::ResetResults, this);
  }

  Simulator::Run();
  Simulator::Destroy();
}

class TestMultipleCtsResponsesFromMuRts : public TestCase {
public:
  struct CtsTxInfos {
    uint16_t bw{20};
    bool discard{false};
  };

  TestMultipleCtsResponsesFromMuRts(
      const std::vector<CtsTxInfos> &ctsTxInfosPerSta);

private:
  void DoSetup() override;
  void DoTeardown() override;
  void DoRun() override;

  void FakePreviousMuRts();

  void TxNonHtDuplicateCts(std::size_t phyIndex);

  void RxCtsSuccess(std::size_t phyIndex, Ptr<const WifiPsdu> psdu,
                    RxSignalInfo rxSignalInfo, WifiTxVector txVector,
                    std::vector<bool> statusPerMpdu);

  void RxCtsFailure(std::size_t phyIndex, Ptr<const WifiPsdu> psdu);

  void CheckResults();

  Ptr<MuRtsCtsSpectrumWifiPhy> m_phyAp;
  std::vector<Ptr<MuRtsCtsSpectrumWifiPhy>> m_phyStas;

  std::vector<CtsTxInfos> m_ctsTxInfosPerSta;

  std::size_t m_countApRxCtsSuccess;
  std::size_t m_countApRxCtsFailure;
  std::size_t m_countStaRxCtsSuccess;
  std::size_t m_countStaRxCtsFailure;

  double m_stasTxPowerDbm;
};

TestMultipleCtsResponsesFromMuRts::TestMultipleCtsResponsesFromMuRts(
    const std::vector<CtsTxInfos> &ctsTxInfosPerSta)
    : TestCase{"test PHY reception of multiple CTS frames following a MU-RTS "
               "frame"},
      m_ctsTxInfosPerSta{ctsTxInfosPerSta}, m_countApRxCtsSuccess{0},
      m_countApRxCtsFailure{0}, m_countStaRxCtsSuccess{0},
      m_countStaRxCtsFailure{0}, m_stasTxPowerDbm(10.0) {}

void TestMultipleCtsResponsesFromMuRts::FakePreviousMuRts() {
  NS_LOG_FUNCTION(this);

  const auto bw =
      std::max_element(
          m_ctsTxInfosPerSta.cbegin(), m_ctsTxInfosPerSta.cend(),
          [](const auto &lhs, const auto &rhs) { return lhs.bw < rhs.bw; })
          ->bw;
  WifiTxVector txVector;
  txVector.SetChannelWidth(bw);

  m_phyAp->SetMuRtsTxVector(txVector);
  m_phyAp->SetPpduUid(0);

  for (auto &phySta : m_phyStas) {
    phySta->SetPpduUid(0);
  }
}

void TestMultipleCtsResponsesFromMuRts::TxNonHtDuplicateCts(
    std::size_t phyIndex) {
  const auto bw = m_ctsTxInfosPerSta.at(phyIndex).bw;
  const auto discarded = m_ctsTxInfosPerSta.at(phyIndex).discard;
  NS_LOG_FUNCTION(this << phyIndex << bw << discarded);

  if (discarded) {
    return;
  }

  WifiTxVector txVector =
      WifiTxVector(OfdmPhy::GetOfdmRate54Mbps(), 0, WIFI_PREAMBLE_LONG, 800, 1,
                   1, 0, bw, false, false);
  txVector.SetTriggerResponding(true);

  WifiMacHeader hdr;
  hdr.SetType(WIFI_MAC_CTL_CTS);
  hdr.SetDsNotFrom();
  hdr.SetDsNotTo();
  hdr.SetNoMoreFragments();
  hdr.SetNoRetry();

  auto pkt = Create<Packet>();
  auto mpdu = Create<WifiMpdu>(pkt, hdr);
  auto psdu = Create<WifiPsdu>(mpdu, false);

  m_phyStas.at(phyIndex)->Send(psdu, txVector);
}

void TestMultipleCtsResponsesFromMuRts::RxCtsSuccess(std::size_t phyIndex,
                                                     Ptr<const WifiPsdu> psdu,
                                                     RxSignalInfo rxSignalInfo,
                                                     WifiTxVector txVector,
                                                     std::vector<bool>) {
  NS_LOG_FUNCTION(this << phyIndex << *psdu << rxSignalInfo << txVector);
  std::vector<CtsTxInfos> successfulCtsInfos{};
  std::copy_if(m_ctsTxInfosPerSta.cbegin(), m_ctsTxInfosPerSta.cend(),
               std::back_inserter(successfulCtsInfos),
               [](const auto &info) { return !info.discard; });
  const auto isAp = (phyIndex == 0);
  if (isAp) {
    NS_TEST_EXPECT_MSG_EQ_TOL(
        rxSignalInfo.rssi,
        WToDbm(DbmToW(m_stasTxPowerDbm) * successfulCtsInfos.size()), 0.1,
        "RX power is not correct!");
  }
  auto expectedWidth =
      std::max_element(
          successfulCtsInfos.cbegin(), successfulCtsInfos.cend(),
          [](const auto &lhs, const auto &rhs) { return lhs.bw < rhs.bw; })
          ->bw;
  if (!isAp) {
    expectedWidth =
        std::min(m_ctsTxInfosPerSta.at(phyIndex - 1).bw, expectedWidth);
  }
  NS_TEST_ASSERT_MSG_EQ(txVector.GetChannelWidth(), expectedWidth,
                        "Incorrect channel width in TXVECTOR");
  if (isAp) {
    m_countApRxCtsSuccess++;
  } else {
    m_countStaRxCtsSuccess++;
  }
}

void TestMultipleCtsResponsesFromMuRts::RxCtsFailure(std::size_t phyIndex,
                                                     Ptr<const WifiPsdu> psdu) {
  NS_LOG_FUNCTION(this << phyIndex << *psdu);
  const auto isAp = (phyIndex == 0);
  if (isAp) {
    m_countApRxCtsFailure++;
  } else {
    m_countStaRxCtsFailure++;
  }
}

void TestMultipleCtsResponsesFromMuRts::CheckResults() {
  NS_TEST_ASSERT_MSG_EQ(
      m_countApRxCtsSuccess, 1,
      "The number of successfully received CTS frames by AP is not correct!");
  NS_TEST_ASSERT_MSG_EQ(m_countStaRxCtsSuccess, m_ctsTxInfosPerSta.size(),
                        "The number of successfully received CTS frames by "
                        "non-participating STAs is not correct!");
  NS_TEST_ASSERT_MSG_EQ(
      m_countApRxCtsFailure, 0,
      "The number of unsuccessfully received CTS frames by AP is not correct!");
  NS_TEST_ASSERT_MSG_EQ(
      m_countStaRxCtsFailure, 0,
      "The number of unsuccessfully received CTS frames by non-participating "
      "STAs is not correct!");
}

void TestMultipleCtsResponsesFromMuRts::DoSetup() {
  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(1);
  int64_t streamNumber = 0;

  auto spectrumChannel = CreateObject<MultiModelSpectrumChannel>();
  auto lossModel = CreateObject<FriisPropagationLossModel>();
  lossModel->SetFrequency(DEFAULT_FREQUENCY * 1e6);
  spectrumChannel->AddPropagationLossModel(lossModel);
  auto delayModel = CreateObject<ConstantSpeedPropagationDelayModel>();
  spectrumChannel->SetPropagationDelayModel(delayModel);

  auto apNode = CreateObject<Node>();
  auto apDev = CreateObject<WifiNetDevice>();
  auto apMac = CreateObject<ApWifiMac>();
  apMac->SetAttribute("BeaconGeneration", BooleanValue(false));
  apDev->SetMac(apMac);
  m_phyAp = CreateObject<MuRtsCtsSpectrumWifiPhy>();
  apDev->SetHeConfiguration(CreateObject<HeConfiguration>());
  auto apInterferenceHelper = CreateObject<InterferenceHelper>();
  m_phyAp->SetInterferenceHelper(apInterferenceHelper);
  auto apErrorModel = CreateObject<NistErrorRateModel>();
  m_phyAp->SetErrorRateModel(apErrorModel);
  m_phyAp->SetDevice(apDev);
  m_phyAp->AddChannel(spectrumChannel);
  m_phyAp->ConfigureStandard(WIFI_STANDARD_80211ax);
  m_phyAp->AssignStreams(streamNumber);

  m_phyAp->SetReceiveOkCallback(
      MakeCallback(&TestMultipleCtsResponsesFromMuRts::RxCtsSuccess, this)
          .Bind(0));
  m_phyAp->SetReceiveErrorCallback(
      MakeCallback(&TestMultipleCtsResponsesFromMuRts::RxCtsFailure, this)
          .Bind(0));

  const auto apBw =
      std::max_element(
          m_ctsTxInfosPerSta.cbegin(), m_ctsTxInfosPerSta.cend(),
          [](const auto &lhs, const auto &rhs) { return lhs.bw < rhs.bw; })
          ->bw;
  auto apChannelNum = std::get<0>(*WifiPhyOperatingChannel::FindFirst(
      0, 0, apBw, WIFI_STANDARD_80211ac, WIFI_PHY_BAND_5GHZ));

  m_phyAp->SetOperatingChannel(
      WifiPhy::ChannelTuple{apChannelNum, apBw, WIFI_PHY_BAND_5GHZ, 0});

  auto apMobility = CreateObject<ConstantPositionMobilityModel>();
  m_phyAp->SetMobility(apMobility);
  apDev->SetPhy(m_phyAp);
  apDev->SetStandard(WIFI_STANDARD_80211ax);
  apDev->SetHeConfiguration(CreateObject<HeConfiguration>());
  apMac->SetWifiPhys({m_phyAp});
  apNode->AggregateObject(apMobility);
  apNode->AddDevice(apDev);

  for (std::size_t i = 0; i < m_ctsTxInfosPerSta.size(); ++i) {
    auto staNode = CreateObject<Node>();
    auto staDev = CreateObject<WifiNetDevice>();
    auto phySta = CreateObject<MuRtsCtsSpectrumWifiPhy>();
    auto staInterferenceHelper = CreateObject<InterferenceHelper>();
    phySta->SetInterferenceHelper(staInterferenceHelper);
    auto staErrorModel = CreateObject<NistErrorRateModel>();
    phySta->SetErrorRateModel(staErrorModel);
    phySta->SetDevice(staDev);
    phySta->AddChannel(spectrumChannel);
    phySta->ConfigureStandard(WIFI_STANDARD_80211ax);
    phySta->AssignStreams(streamNumber);
    phySta->SetTxPowerStart(m_stasTxPowerDbm);
    phySta->SetTxPowerEnd(m_stasTxPowerDbm);

    auto channelNum = std::get<0>(*WifiPhyOperatingChannel::FindFirst(
        0, 0, m_ctsTxInfosPerSta.at(i).bw, WIFI_STANDARD_80211ac,
        WIFI_PHY_BAND_5GHZ));

    phySta->SetOperatingChannel(WifiPhy::ChannelTuple{
        channelNum, m_ctsTxInfosPerSta.at(i).bw, WIFI_PHY_BAND_5GHZ, 0});

    auto staMobility = CreateObject<ConstantPositionMobilityModel>();
    phySta->SetMobility(staMobility);
    staDev->SetPhy(phySta);
    staDev->SetStandard(WIFI_STANDARD_80211ax);
    staDev->SetHeConfiguration(CreateObject<HeConfiguration>());
    staNode->AggregateObject(staMobility);
    staNode->AddDevice(staDev);
    m_phyStas.push_back(phySta);

    auto nonParticipatingHeStaNode = CreateObject<Node>();
    auto nonParticipatingHeStaDev = CreateObject<WifiNetDevice>();
    auto nonParticipatingHePhySta = CreateObject<SpectrumWifiPhy>();
    auto nonParticipatingHeStaInterferenceHelper =
        CreateObject<InterferenceHelper>();
    nonParticipatingHePhySta->SetInterferenceHelper(
        nonParticipatingHeStaInterferenceHelper);
    auto nonParticipatingHeStaErrorModel = CreateObject<NistErrorRateModel>();
    nonParticipatingHePhySta->SetErrorRateModel(
        nonParticipatingHeStaErrorModel);
    nonParticipatingHePhySta->SetDevice(nonParticipatingHeStaDev);
    nonParticipatingHePhySta->AddChannel(spectrumChannel);
    nonParticipatingHePhySta->ConfigureStandard(WIFI_STANDARD_80211ax);

    nonParticipatingHePhySta->SetOperatingChannel(WifiPhy::ChannelTuple{
        channelNum, m_ctsTxInfosPerSta.at(i).bw, WIFI_PHY_BAND_5GHZ, 0});

    auto nonParticipatingHeStaMobility =
        CreateObject<ConstantPositionMobilityModel>();
    nonParticipatingHePhySta->SetMobility(nonParticipatingHeStaMobility);
    nonParticipatingHeStaDev->SetPhy(nonParticipatingHePhySta);
    nonParticipatingHeStaDev->SetStandard(WIFI_STANDARD_80211ax);
    nonParticipatingHeStaDev->SetHeConfiguration(
        CreateObject<HeConfiguration>());
    nonParticipatingHePhySta->AssignStreams(streamNumber);
    nonParticipatingHeStaNode->AggregateObject(nonParticipatingHeStaMobility);
    nonParticipatingHeStaNode->AddDevice(nonParticipatingHeStaDev);

    nonParticipatingHePhySta->SetReceiveOkCallback(
        MakeCallback(&TestMultipleCtsResponsesFromMuRts::RxCtsSuccess, this)
            .Bind(i + 1));
    nonParticipatingHePhySta->SetReceiveErrorCallback(
        MakeCallback(&TestMultipleCtsResponsesFromMuRts::RxCtsFailure, this)
            .Bind(i + 1));
  }

  auto nonHeStaNode = CreateObject<Node>();
  auto nonHeStaDev = CreateObject<WifiNetDevice>();
  auto nonHePhySta = CreateObject<SpectrumWifiPhy>();
  auto nonHeStaInterferenceHelper = CreateObject<InterferenceHelper>();
  nonHePhySta->SetInterferenceHelper(nonHeStaInterferenceHelper);
  auto nonHeStaErrorModel = CreateObject<NistErrorRateModel>();
  nonHePhySta->SetErrorRateModel(nonHeStaErrorModel);
  nonHePhySta->SetDevice(nonHeStaDev);
  nonHePhySta->AddChannel(spectrumChannel);
  nonHePhySta->ConfigureStandard(WIFI_STANDARD_80211ac);
  nonHePhySta->SetOperatingChannel(
      WifiPhy::ChannelTuple{apChannelNum, apBw, WIFI_PHY_BAND_5GHZ, 0});
  auto nonHeStaMobility = CreateObject<ConstantPositionMobilityModel>();
  nonHePhySta->SetMobility(nonHeStaMobility);
  nonHeStaDev->SetPhy(nonHePhySta);
  nonHeStaDev->SetStandard(WIFI_STANDARD_80211ac);
  nonHePhySta->AssignStreams(streamNumber);
  nonHeStaNode->AggregateObject(nonHeStaMobility);
  nonHeStaNode->AddDevice(nonHeStaDev);
}

void TestMultipleCtsResponsesFromMuRts::DoTeardown() {
  m_phyAp->Dispose();
  m_phyAp = nullptr;
  for (auto &phySta : m_phyStas) {
    phySta->Dispose();
    phySta = nullptr;
  }
  m_phyStas.clear();
}

void TestMultipleCtsResponsesFromMuRts::DoRun() {
  Simulator::Schedule(Seconds(0.0),
                      &TestMultipleCtsResponsesFromMuRts::FakePreviousMuRts,
                      this);

  for (std::size_t index = 0; index < m_phyStas.size(); ++index) {
    const auto delay = (index + 1) * NanoSeconds(1.0);
    Simulator::Schedule(delay,
                        &TestMultipleCtsResponsesFromMuRts::TxNonHtDuplicateCts,
                        this, index);
  }

  Simulator::Schedule(Seconds(1.0),
                      &TestMultipleCtsResponsesFromMuRts::CheckResults, this);

  Simulator::Run();
  Simulator::Destroy();
}

class WifiNonHtDuplicateTestSuite : public TestSuite {
public:
  WifiNonHtDuplicateTestSuite();
};

WifiNonHtDuplicateTestSuite::WifiNonHtDuplicateTestSuite()
    : TestSuite("wifi-non-ht-dup", UNIT) {
  AddTestCase(
      new TestNonHtDuplicatePhyReception(WIFI_STANDARD_80211ax, 5210, 0,
                                         {{WIFI_STANDARD_80211a, 5180, 0},
                                          {WIFI_STANDARD_80211n, 5200, 0},
                                          {WIFI_STANDARD_80211ac, 5230, 0}}),
      TestCase::QUICK);
  AddTestCase(
      new TestNonHtDuplicatePhyReception(WIFI_STANDARD_80211ax, 5210, 0,
                                         {{WIFI_STANDARD_80211a, 5180, 0},
                                          {WIFI_STANDARD_80211n, 5200, 0},
                                          {WIFI_STANDARD_80211ac, 5230, 0}},
                                         {false, true, false, false}),
      TestCase::QUICK);
  AddTestCase(new TestMultipleCtsResponsesFromMuRts({{20}, {20}, {20}, {20}}),
              TestCase::QUICK);
  AddTestCase(new TestMultipleCtsResponsesFromMuRts({{40}, {40}, {40}, {40}}),
              TestCase::QUICK);
  AddTestCase(new TestMultipleCtsResponsesFromMuRts({{80}, {80}, {80}, {80}}),
              TestCase::QUICK);
  AddTestCase(
      new TestMultipleCtsResponsesFromMuRts({{160}, {160}, {160}, {160}}),
      TestCase::QUICK);
  AddTestCase(new TestMultipleCtsResponsesFromMuRts({{160}, {80}, {40}, {20}}),
              TestCase::QUICK);
  AddTestCase(new TestMultipleCtsResponsesFromMuRts({{20}, {40}, {80}, {160}}),
              TestCase::QUICK);
  AddTestCase(new TestMultipleCtsResponsesFromMuRts({{80, true}, {40, false}}),
              TestCase::QUICK);
  AddTestCase(new TestMultipleCtsResponsesFromMuRts({{80, false}, {40, true}}),
              TestCase::QUICK);
  AddTestCase(new TestMultipleCtsResponsesFromMuRts({{40, true}, {80, false}}),
              TestCase::QUICK);
  AddTestCase(new TestMultipleCtsResponsesFromMuRts({{40, false}, {80, true}}),
              TestCase::QUICK);
}

static WifiNonHtDuplicateTestSuite wifiNonHtDuplicateTestSuite;
