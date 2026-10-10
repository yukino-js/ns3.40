
#include "ns3/ap-wifi-mac.h"
#include "ns3/boolean.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/ctrl-headers.h"
#include "ns3/double.h"
#include "ns3/he-configuration.h"
#include "ns3/he-phy.h"
#include "ns3/he-ppdu.h"
#include "ns3/interference-helper.h"
#include "ns3/log.h"
#include "ns3/mobility-helper.h"
#include "ns3/multi-model-spectrum-channel.h"
#include "ns3/nist-error-rate-model.h"
#include "ns3/node.h"
#include "ns3/non-communicating-net-device.h"
#include "ns3/pointer.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/simulator.h"
#include "ns3/spectrum-wifi-helper.h"
#include "ns3/spectrum-wifi-phy.h"
#include "ns3/sta-wifi-mac.h"
#include "ns3/string.h"
#include "ns3/test.h"
#include "ns3/threshold-preamble-detection-model.h"
#include "ns3/waveform-generator.h"
#include "ns3/wifi-mac-header.h"
#include "ns3/wifi-net-device.h"
#include "ns3/wifi-phy-listener.h"
#include "ns3/wifi-psdu.h"
#include "ns3/wifi-spectrum-phy-interface.h"
#include "ns3/wifi-spectrum-signal-parameters.h"
#include "ns3/wifi-spectrum-value-helper.h"
#include "ns3/wifi-utils.h"

#include <algorithm>
#include <iterator>
#include <memory>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("WifiPhyOfdmaTest");

static const uint8_t DEFAULT_CHANNEL_NUMBER = 36;
static const uint32_t DEFAULT_FREQUENCY = 5180;
static const WifiPhyBand DEFAULT_WIFI_BAND = WIFI_PHY_BAND_5GHZ;
static const uint16_t DEFAULT_CHANNEL_WIDTH = 20;
static const uint16_t DEFAULT_GUARD_WIDTH = DEFAULT_CHANNEL_WIDTH;

class OfdmaTestHePhy : public HePhy {
public:
  OfdmaTestHePhy(uint16_t staId);
  ~OfdmaTestHePhy() override;

  uint16_t GetStaId(const Ptr<const WifiPpdu> ppdu) const override;

  void SetGlobalPpduUid(uint64_t uid);

private:
  uint16_t m_staId;
};

OfdmaTestHePhy::OfdmaTestHePhy(uint16_t staId) : HePhy(), m_staId(staId) {}

OfdmaTestHePhy::~OfdmaTestHePhy() {}

uint16_t OfdmaTestHePhy::GetStaId(const Ptr<const WifiPpdu> ppdu) const {
  if (ppdu->GetType() == WIFI_PPDU_TYPE_DL_MU) {
    return m_staId;
  }
  return HePhy::GetStaId(ppdu);
}

void OfdmaTestHePhy::SetGlobalPpduUid(uint64_t uid) { m_globalPpduUid = uid; }

class OfdmaSpectrumWifiPhy : public SpectrumWifiPhy {
public:
  static TypeId GetTypeId();
  OfdmaSpectrumWifiPhy(uint16_t staId);
  ~OfdmaSpectrumWifiPhy() override;

  void DoInitialize() override;
  void DoDispose() override;

  using WifiPhy::Reset;
  void StartTx(Ptr<const WifiPpdu> ppdu) override;

  typedef void (*TxPpduUidCallback)(uint64_t uid);

  void SetPpduUid(uint64_t uid);

  void SetTriggerFrameUid(uint64_t uid);

  std::map<std::pair<uint64_t, WifiPreamble>, Ptr<Event>> &
  GetCurrentPreambleEvents();
  Ptr<Event> GetCurrentEvent();

  Time GetEnergyDuration(double energyW, WifiSpectrumBandInfo band);

  Ptr<const HePhy> GetHePhy() const;

private:
  Ptr<OfdmaTestHePhy> m_ofdmTestHePhy;
  TracedCallback<uint64_t> m_phyTxPpduUidTrace;
};

TypeId OfdmaSpectrumWifiPhy::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::OfdmaSpectrumWifiPhy")
          .SetParent<SpectrumWifiPhy>()
          .SetGroupName("Wifi")
          .AddTraceSource("TxPpduUid", "UID of the PPDU to be transmitted",
                          MakeTraceSourceAccessor(
                              &OfdmaSpectrumWifiPhy::m_phyTxPpduUidTrace),
                          "ns3::OfdmaSpectrumWifiPhy::TxPpduUidCallback");
  return tid;
}

OfdmaSpectrumWifiPhy::OfdmaSpectrumWifiPhy(uint16_t staId) : SpectrumWifiPhy() {
  m_ofdmTestHePhy = Create<OfdmaTestHePhy>(staId);
  m_ofdmTestHePhy->SetOwner(this);
}

OfdmaSpectrumWifiPhy::~OfdmaSpectrumWifiPhy() {}

void OfdmaSpectrumWifiPhy::DoInitialize() {
  m_phyEntities[WIFI_MOD_CLASS_HE] = m_ofdmTestHePhy;
  SpectrumWifiPhy::DoInitialize();
}

void OfdmaSpectrumWifiPhy::DoDispose() {
  m_ofdmTestHePhy = nullptr;
  SpectrumWifiPhy::DoDispose();
}

void OfdmaSpectrumWifiPhy::SetPpduUid(uint64_t uid) {
  m_ofdmTestHePhy->SetGlobalPpduUid(uid);
  m_previouslyRxPpduUid = uid;
}

void OfdmaSpectrumWifiPhy::SetTriggerFrameUid(uint64_t uid) {
  m_previouslyRxPpduUid = uid;
}

void OfdmaSpectrumWifiPhy::StartTx(Ptr<const WifiPpdu> ppdu) {
  m_phyTxPpduUidTrace(ppdu->GetUid());
  SpectrumWifiPhy::StartTx(ppdu);
}

std::map<std::pair<uint64_t, WifiPreamble>, Ptr<Event>> &
OfdmaSpectrumWifiPhy::GetCurrentPreambleEvents() {
  return m_currentPreambleEvents;
}

Ptr<Event> OfdmaSpectrumWifiPhy::GetCurrentEvent() { return m_currentEvent; }

Time OfdmaSpectrumWifiPhy::GetEnergyDuration(double energyW,
                                             WifiSpectrumBandInfo band) {
  return m_interference->GetEnergyDuration(energyW, band);
}

Ptr<const HePhy> OfdmaSpectrumWifiPhy::GetHePhy() const {
  return DynamicCast<const HePhy>(GetLatestPhyEntity());
}

class TestDlOfdmaPhyTransmission : public TestCase {
public:
  TestDlOfdmaPhyTransmission();
  ~TestDlOfdmaPhyTransmission() override;

private:
  void DoSetup() override;
  void DoTeardown() override;
  void DoRun() override;

  void RxSuccessSta1(Ptr<const WifiPsdu> psdu, RxSignalInfo rxSignalInfo,
                     WifiTxVector txVector, std::vector<bool> statusPerMpdu);
  void RxSuccessSta2(Ptr<const WifiPsdu> psdu, RxSignalInfo rxSignalInfo,
                     WifiTxVector txVector, std::vector<bool> statusPerMpdu);
  void RxSuccessSta3(Ptr<const WifiPsdu> psdu, RxSignalInfo rxSignalInfo,
                     WifiTxVector txVector, std::vector<bool> statusPerMpdu);

  void RxFailureSta1(Ptr<const WifiPsdu> psdu);
  void RxFailureSta2(Ptr<const WifiPsdu> psdu);
  void RxFailureSta3(Ptr<const WifiPsdu> psdu);

  void CheckResultsSta1(uint32_t expectedRxSuccess, uint32_t expectedRxFailure,
                        uint32_t expectedRxBytes);
  void CheckResultsSta2(uint32_t expectedRxSuccess, uint32_t expectedRxFailure,
                        uint32_t expectedRxBytes);
  void CheckResultsSta3(uint32_t expectedRxSuccess, uint32_t expectedRxFailure,
                        uint32_t expectedRxBytes);

  void ResetResults();

  void SendMuPpdu(uint16_t rxStaId1, uint16_t rxStaId2);

  void GenerateInterference(Ptr<SpectrumValue> interferencePsd, Time duration);
  void StopInterference();

  void RunOne();

  void CheckPhyState(Ptr<OfdmaSpectrumWifiPhy> phy, WifiPhyState expectedState);
  void DoCheckPhyState(Ptr<OfdmaSpectrumWifiPhy> phy,
                       WifiPhyState expectedState);

  uint32_t m_countRxSuccessSta1;
  uint32_t m_countRxSuccessSta2;
  uint32_t m_countRxSuccessSta3;
  uint32_t m_countRxFailureSta1;
  uint32_t m_countRxFailureSta2;
  uint32_t m_countRxFailureSta3;
  uint32_t m_countRxBytesSta1;
  uint32_t m_countRxBytesSta2;
  uint32_t m_countRxBytesSta3;

  Ptr<SpectrumWifiPhy> m_phyAp;
  Ptr<OfdmaSpectrumWifiPhy> m_phySta1;
  Ptr<OfdmaSpectrumWifiPhy> m_phySta2;
  Ptr<OfdmaSpectrumWifiPhy> m_phySta3;
  Ptr<WaveformGenerator> m_phyInterferer;

  uint16_t m_frequency;
  uint16_t m_channelWidth;
  Time m_expectedPpduDuration;
};

TestDlOfdmaPhyTransmission::TestDlOfdmaPhyTransmission()
    : TestCase("DL-OFDMA PHY test"), m_countRxSuccessSta1(0),
      m_countRxSuccessSta2(0), m_countRxSuccessSta3(0), m_countRxFailureSta1(0),
      m_countRxFailureSta2(0), m_countRxFailureSta3(0), m_countRxBytesSta1(0),
      m_countRxBytesSta2(0), m_countRxBytesSta3(0),
      m_frequency(DEFAULT_FREQUENCY), m_channelWidth(DEFAULT_CHANNEL_WIDTH),
      m_expectedPpduDuration(NanoSeconds(306400)) {}

void TestDlOfdmaPhyTransmission::ResetResults() {
  m_countRxSuccessSta1 = 0;
  m_countRxSuccessSta2 = 0;
  m_countRxSuccessSta3 = 0;
  m_countRxFailureSta1 = 0;
  m_countRxFailureSta2 = 0;
  m_countRxFailureSta3 = 0;
  m_countRxBytesSta1 = 0;
  m_countRxBytesSta2 = 0;
  m_countRxBytesSta3 = 0;
}

void TestDlOfdmaPhyTransmission::SendMuPpdu(uint16_t rxStaId1,
                                            uint16_t rxStaId2) {
  NS_LOG_FUNCTION(this << rxStaId1 << rxStaId2);
  WifiConstPsduMap psdus;
  WifiTxVector txVector =
      WifiTxVector(HePhy::GetHeMcs7(), 0, WIFI_PREAMBLE_HE_MU, 800, 1, 1, 0,
                   m_channelWidth, false, false);
  HeRu::RuType ruType = HeRu::RU_106_TONE;
  if (m_channelWidth == 20) {
    ruType = HeRu::RU_106_TONE;
    txVector.SetRuAllocation({96}, 0);
  } else if (m_channelWidth == 40) {
    ruType = HeRu::RU_242_TONE;
    txVector.SetRuAllocation({192, 192}, 0);
  } else if (m_channelWidth == 80) {
    ruType = HeRu::RU_484_TONE;
    txVector.SetRuAllocation({200, 200, 200, 200}, 0);
  } else if (m_channelWidth == 160) {
    ruType = HeRu::RU_996_TONE;
    txVector.SetRuAllocation({208, 208, 208, 208, 208, 208, 208, 208}, 0);
  } else {
    NS_ASSERT_MSG(false, "Unsupported channel width");
  }

  txVector.SetSigBMode(VhtPhy::GetVhtMcs5());

  HeRu::RuSpec ru1(ruType, 1, true);
  txVector.SetRu(ru1, rxStaId1);
  txVector.SetMode(HePhy::GetHeMcs7(), rxStaId1);
  txVector.SetNss(1, rxStaId1);

  HeRu::RuSpec ru2(ruType, (m_channelWidth == 160 ? 1 : 2),
                   m_channelWidth != 160);
  txVector.SetRu(ru2, rxStaId2);
  txVector.SetMode(HePhy::GetHeMcs9(), rxStaId2);
  txVector.SetNss(1, rxStaId2);

  Ptr<Packet> pkt1 = Create<Packet>(1000);
  WifiMacHeader hdr1;
  hdr1.SetType(WIFI_MAC_QOSDATA);
  hdr1.SetQosTid(0);
  hdr1.SetAddr1(Mac48Address("00:00:00:00:00:01"));
  hdr1.SetSequenceNumber(1);
  Ptr<WifiPsdu> psdu1 = Create<WifiPsdu>(pkt1, hdr1);
  psdus.insert(std::make_pair(rxStaId1, psdu1));

  Ptr<Packet> pkt2 = Create<Packet>(1500);
  WifiMacHeader hdr2;
  hdr2.SetType(WIFI_MAC_QOSDATA);
  hdr2.SetQosTid(0);
  hdr2.SetAddr1(Mac48Address("00:00:00:00:00:02"));
  hdr2.SetSequenceNumber(2);
  Ptr<WifiPsdu> psdu2 = Create<WifiPsdu>(pkt2, hdr2);
  psdus.insert(std::make_pair(rxStaId2, psdu2));

  m_phyAp->Send(psdus, txVector);
}

void TestDlOfdmaPhyTransmission::GenerateInterference(
    Ptr<SpectrumValue> interferencePsd, Time duration) {
  m_phyInterferer->SetTxPowerSpectralDensity(interferencePsd);
  m_phyInterferer->SetPeriod(duration);
  m_phyInterferer->Start();
  Simulator::Schedule(duration, &TestDlOfdmaPhyTransmission::StopInterference,
                      this);
}

void TestDlOfdmaPhyTransmission::StopInterference() { m_phyInterferer->Stop(); }

TestDlOfdmaPhyTransmission::~TestDlOfdmaPhyTransmission() {}

void TestDlOfdmaPhyTransmission::RxSuccessSta1(Ptr<const WifiPsdu> psdu,
                                               RxSignalInfo rxSignalInfo,
                                               WifiTxVector txVector,
                                               std::vector<bool>) {
  NS_LOG_FUNCTION(this << *psdu << rxSignalInfo << txVector);
  m_countRxSuccessSta1++;
  m_countRxBytesSta1 += (psdu->GetSize() - 30);
}

void TestDlOfdmaPhyTransmission::RxSuccessSta2(Ptr<const WifiPsdu> psdu,
                                               RxSignalInfo rxSignalInfo,
                                               WifiTxVector txVector,
                                               std::vector<bool>) {
  NS_LOG_FUNCTION(this << *psdu << rxSignalInfo << txVector);
  m_countRxSuccessSta2++;
  m_countRxBytesSta2 += (psdu->GetSize() - 30);
}

void TestDlOfdmaPhyTransmission::RxSuccessSta3(Ptr<const WifiPsdu> psdu,
                                               RxSignalInfo rxSignalInfo,
                                               WifiTxVector txVector,
                                               std::vector<bool>) {
  NS_LOG_FUNCTION(this << *psdu << rxSignalInfo << txVector);
  m_countRxSuccessSta3++;
  m_countRxBytesSta3 += (psdu->GetSize() - 30);
}

void TestDlOfdmaPhyTransmission::RxFailureSta1(Ptr<const WifiPsdu> psdu) {
  NS_LOG_FUNCTION(this << *psdu);
  m_countRxFailureSta1++;
}

void TestDlOfdmaPhyTransmission::RxFailureSta2(Ptr<const WifiPsdu> psdu) {
  NS_LOG_FUNCTION(this << *psdu);
  m_countRxFailureSta2++;
}

void TestDlOfdmaPhyTransmission::RxFailureSta3(Ptr<const WifiPsdu> psdu) {
  NS_LOG_FUNCTION(this << *psdu);
  m_countRxFailureSta3++;
}

void TestDlOfdmaPhyTransmission::CheckResultsSta1(uint32_t expectedRxSuccess,
                                                  uint32_t expectedRxFailure,
                                                  uint32_t expectedRxBytes) {
  NS_TEST_ASSERT_MSG_EQ(
      m_countRxSuccessSta1, expectedRxSuccess,
      "The number of successfully received packets by STA 1 is not correct!");
  NS_TEST_ASSERT_MSG_EQ(
      m_countRxFailureSta1, expectedRxFailure,
      "The number of unsuccessfuly received packets by STA 1 is not correct!");
  NS_TEST_ASSERT_MSG_EQ(
      m_countRxBytesSta1, expectedRxBytes,
      "The number of bytes received by STA 1 is not correct!");
}

void TestDlOfdmaPhyTransmission::CheckResultsSta2(uint32_t expectedRxSuccess,
                                                  uint32_t expectedRxFailure,
                                                  uint32_t expectedRxBytes) {
  NS_TEST_ASSERT_MSG_EQ(
      m_countRxSuccessSta2, expectedRxSuccess,
      "The number of successfully received packets by STA 2 is not correct!");
  NS_TEST_ASSERT_MSG_EQ(
      m_countRxFailureSta2, expectedRxFailure,
      "The number of unsuccessfuly received packets by STA 2 is not correct!");
  NS_TEST_ASSERT_MSG_EQ(
      m_countRxBytesSta2, expectedRxBytes,
      "The number of bytes received by STA 2 is not correct!");
}

void TestDlOfdmaPhyTransmission::CheckResultsSta3(uint32_t expectedRxSuccess,
                                                  uint32_t expectedRxFailure,
                                                  uint32_t expectedRxBytes) {
  NS_TEST_ASSERT_MSG_EQ(
      m_countRxSuccessSta3, expectedRxSuccess,
      "The number of successfully received packets by STA 3 is not correct!");
  NS_TEST_ASSERT_MSG_EQ(
      m_countRxFailureSta3, expectedRxFailure,
      "The number of unsuccessfuly received packets by STA 3 is not correct!");
  NS_TEST_ASSERT_MSG_EQ(
      m_countRxBytesSta3, expectedRxBytes,
      "The number of bytes received by STA 3 is not correct!");
}

void TestDlOfdmaPhyTransmission::CheckPhyState(Ptr<OfdmaSpectrumWifiPhy> phy,
                                               WifiPhyState expectedState) {
  Simulator::ScheduleNow(&TestDlOfdmaPhyTransmission::DoCheckPhyState, this,
                         phy, expectedState);
}

void TestDlOfdmaPhyTransmission::DoCheckPhyState(Ptr<OfdmaSpectrumWifiPhy> phy,
                                                 WifiPhyState expectedState) {
  WifiPhyState currentState;
  PointerValue ptr;
  phy->GetAttribute("State", ptr);
  Ptr<WifiPhyStateHelper> state =
      DynamicCast<WifiPhyStateHelper>(ptr.Get<WifiPhyStateHelper>());
  currentState = state->GetState();
  NS_LOG_FUNCTION(this << currentState);
  NS_TEST_ASSERT_MSG_EQ(currentState, expectedState,
                        "PHY State "
                            << currentState << " does not match expected state "
                            << expectedState << " at " << Simulator::Now());
}

void TestDlOfdmaPhyTransmission::DoSetup() {
  Ptr<MultiModelSpectrumChannel> spectrumChannel =
      CreateObject<MultiModelSpectrumChannel>();
  Ptr<FriisPropagationLossModel> lossModel =
      CreateObject<FriisPropagationLossModel>();
  lossModel->SetFrequency(m_frequency * 1e6);
  spectrumChannel->AddPropagationLossModel(lossModel);
  Ptr<ConstantSpeedPropagationDelayModel> delayModel =
      CreateObject<ConstantSpeedPropagationDelayModel>();
  spectrumChannel->SetPropagationDelayModel(delayModel);

  Ptr<Node> apNode = CreateObject<Node>();
  Ptr<WifiNetDevice> apDev = CreateObject<WifiNetDevice>();
  m_phyAp = CreateObject<SpectrumWifiPhy>();
  Ptr<InterferenceHelper> apInterferenceHelper =
      CreateObject<InterferenceHelper>();
  m_phyAp->SetInterferenceHelper(apInterferenceHelper);
  Ptr<ErrorRateModel> apErrorModel = CreateObject<NistErrorRateModel>();
  m_phyAp->SetErrorRateModel(apErrorModel);
  m_phyAp->SetDevice(apDev);
  m_phyAp->AddChannel(spectrumChannel);
  m_phyAp->ConfigureStandard(WIFI_STANDARD_80211ax);
  Ptr<ConstantPositionMobilityModel> apMobility =
      CreateObject<ConstantPositionMobilityModel>();
  m_phyAp->SetMobility(apMobility);
  apDev->SetPhy(m_phyAp);
  apNode->AggregateObject(apMobility);
  apNode->AddDevice(apDev);

  Ptr<Node> sta1Node = CreateObject<Node>();
  Ptr<WifiNetDevice> sta1Dev = CreateObject<WifiNetDevice>();
  m_phySta1 = CreateObject<OfdmaSpectrumWifiPhy>(1);
  Ptr<InterferenceHelper> sta1InterferenceHelper =
      CreateObject<InterferenceHelper>();
  m_phySta1->SetInterferenceHelper(sta1InterferenceHelper);
  Ptr<ErrorRateModel> sta1ErrorModel = CreateObject<NistErrorRateModel>();
  m_phySta1->SetErrorRateModel(sta1ErrorModel);
  m_phySta1->SetDevice(sta1Dev);
  m_phySta1->AddChannel(spectrumChannel);
  m_phySta1->ConfigureStandard(WIFI_STANDARD_80211ax);
  m_phySta1->SetReceiveOkCallback(
      MakeCallback(&TestDlOfdmaPhyTransmission::RxSuccessSta1, this));
  m_phySta1->SetReceiveErrorCallback(
      MakeCallback(&TestDlOfdmaPhyTransmission::RxFailureSta1, this));
  Ptr<ConstantPositionMobilityModel> sta1Mobility =
      CreateObject<ConstantPositionMobilityModel>();
  m_phySta1->SetMobility(sta1Mobility);
  sta1Dev->SetPhy(m_phySta1);
  sta1Node->AggregateObject(sta1Mobility);
  sta1Node->AddDevice(sta1Dev);

  Ptr<Node> sta2Node = CreateObject<Node>();
  Ptr<WifiNetDevice> sta2Dev = CreateObject<WifiNetDevice>();
  m_phySta2 = CreateObject<OfdmaSpectrumWifiPhy>(2);
  Ptr<InterferenceHelper> sta2InterferenceHelper =
      CreateObject<InterferenceHelper>();
  m_phySta2->SetInterferenceHelper(sta2InterferenceHelper);
  Ptr<ErrorRateModel> sta2ErrorModel = CreateObject<NistErrorRateModel>();
  m_phySta2->SetErrorRateModel(sta2ErrorModel);
  m_phySta2->SetDevice(sta2Dev);
  m_phySta2->AddChannel(spectrumChannel);
  m_phySta2->ConfigureStandard(WIFI_STANDARD_80211ax);
  m_phySta2->SetReceiveOkCallback(
      MakeCallback(&TestDlOfdmaPhyTransmission::RxSuccessSta2, this));
  m_phySta2->SetReceiveErrorCallback(
      MakeCallback(&TestDlOfdmaPhyTransmission::RxFailureSta2, this));
  Ptr<ConstantPositionMobilityModel> sta2Mobility =
      CreateObject<ConstantPositionMobilityModel>();
  m_phySta2->SetMobility(sta2Mobility);
  sta2Dev->SetPhy(m_phySta2);
  sta2Node->AggregateObject(sta2Mobility);
  sta2Node->AddDevice(sta2Dev);

  Ptr<Node> sta3Node = CreateObject<Node>();
  Ptr<WifiNetDevice> sta3Dev = CreateObject<WifiNetDevice>();
  m_phySta3 = CreateObject<OfdmaSpectrumWifiPhy>(3);
  Ptr<InterferenceHelper> sta3InterferenceHelper =
      CreateObject<InterferenceHelper>();
  m_phySta3->SetInterferenceHelper(sta3InterferenceHelper);
  Ptr<ErrorRateModel> sta3ErrorModel = CreateObject<NistErrorRateModel>();
  m_phySta3->SetErrorRateModel(sta3ErrorModel);
  m_phySta3->SetDevice(sta3Dev);
  m_phySta3->AddChannel(spectrumChannel);
  m_phySta3->ConfigureStandard(WIFI_STANDARD_80211ax);
  m_phySta3->SetReceiveOkCallback(
      MakeCallback(&TestDlOfdmaPhyTransmission::RxSuccessSta3, this));
  m_phySta3->SetReceiveErrorCallback(
      MakeCallback(&TestDlOfdmaPhyTransmission::RxFailureSta3, this));
  Ptr<ConstantPositionMobilityModel> sta3Mobility =
      CreateObject<ConstantPositionMobilityModel>();
  m_phySta3->SetMobility(sta3Mobility);
  sta3Dev->SetPhy(m_phySta3);
  sta3Node->AggregateObject(sta3Mobility);
  sta3Node->AddDevice(sta3Dev);

  Ptr<Node> interfererNode = CreateObject<Node>();
  Ptr<NonCommunicatingNetDevice> interfererDev =
      CreateObject<NonCommunicatingNetDevice>();
  m_phyInterferer = CreateObject<WaveformGenerator>();
  m_phyInterferer->SetDevice(interfererDev);
  m_phyInterferer->SetChannel(spectrumChannel);
  m_phyInterferer->SetDutyCycle(1);
  interfererNode->AddDevice(interfererDev);
}

void TestDlOfdmaPhyTransmission::DoTeardown() {
  m_phyAp->Dispose();
  m_phyAp = nullptr;
  m_phySta1->Dispose();
  m_phySta1 = nullptr;
  m_phySta2->Dispose();
  m_phySta2 = nullptr;
  m_phySta3->Dispose();
  m_phySta3 = nullptr;
  m_phyInterferer->Dispose();
  m_phyInterferer = nullptr;
}

void TestDlOfdmaPhyTransmission::RunOne() {
  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(1);
  int64_t streamNumber = 0;
  m_phyAp->AssignStreams(streamNumber);
  m_phySta1->AssignStreams(streamNumber);
  m_phySta2->AssignStreams(streamNumber);
  m_phySta3->AssignStreams(streamNumber);

  auto channelNum = std::get<0>(*WifiPhyOperatingChannel::FindFirst(
      0, m_frequency, m_channelWidth, WIFI_STANDARD_80211ax,
      WIFI_PHY_BAND_5GHZ));

  m_phyAp->SetOperatingChannel(WifiPhy::ChannelTuple{
      channelNum, m_channelWidth, (int)(WIFI_PHY_BAND_5GHZ), 0});
  m_phySta1->SetOperatingChannel(WifiPhy::ChannelTuple{
      channelNum, m_channelWidth, (int)(WIFI_PHY_BAND_5GHZ), 0});
  m_phySta2->SetOperatingChannel(WifiPhy::ChannelTuple{
      channelNum, m_channelWidth, (int)(WIFI_PHY_BAND_5GHZ), 0});
  m_phySta3->SetOperatingChannel(WifiPhy::ChannelTuple{
      channelNum, m_channelWidth, (int)(WIFI_PHY_BAND_5GHZ), 0});

  Simulator::Schedule(Seconds(0.5), &TestDlOfdmaPhyTransmission::ResetResults,
                      this);

  Simulator::Schedule(Seconds(1.0), &TestDlOfdmaPhyTransmission::SendMuPpdu,
                      this, 1, 2);

  Simulator::Schedule(Seconds(1.0) + m_expectedPpduDuration - NanoSeconds(1),
                      &TestDlOfdmaPhyTransmission::CheckPhyState, this,
                      m_phySta1, WifiPhyState::RX);
  Simulator::Schedule(Seconds(1.0) + m_expectedPpduDuration - NanoSeconds(1),
                      &TestDlOfdmaPhyTransmission::CheckPhyState, this,
                      m_phySta2, WifiPhyState::RX);
  Simulator::Schedule(Seconds(1.0) + m_expectedPpduDuration - NanoSeconds(1),
                      &TestDlOfdmaPhyTransmission::CheckPhyState, this,
                      m_phySta3, WifiPhyState::CCA_BUSY);
  Simulator::Schedule(Seconds(1.0) + m_expectedPpduDuration,
                      &TestDlOfdmaPhyTransmission::CheckPhyState, this,
                      m_phySta1, WifiPhyState::IDLE);
  Simulator::Schedule(Seconds(1.0) + m_expectedPpduDuration,
                      &TestDlOfdmaPhyTransmission::CheckPhyState, this,
                      m_phySta2, WifiPhyState::IDLE);
  Simulator::Schedule(Seconds(1.0) + m_expectedPpduDuration,
                      &TestDlOfdmaPhyTransmission::CheckPhyState, this,
                      m_phySta3, WifiPhyState::IDLE);

  Simulator::Schedule(Seconds(1.1),
                      &TestDlOfdmaPhyTransmission::CheckResultsSta1, this, 1, 0,
                      1000);
  Simulator::Schedule(Seconds(1.1),
                      &TestDlOfdmaPhyTransmission::CheckResultsSta2, this, 1, 0,
                      1500);
  Simulator::Schedule(Seconds(1.1),
                      &TestDlOfdmaPhyTransmission::CheckResultsSta3, this, 0, 0,
                      0);

  Simulator::Schedule(Seconds(1.5), &TestDlOfdmaPhyTransmission::ResetResults,
                      this);

  Simulator::Schedule(Seconds(2.0), &TestDlOfdmaPhyTransmission::SendMuPpdu,
                      this, 1, 3);

  Simulator::Schedule(Seconds(2.0) + m_expectedPpduDuration - NanoSeconds(1),
                      &TestDlOfdmaPhyTransmission::CheckPhyState, this,
                      m_phySta1, WifiPhyState::RX);
  Simulator::Schedule(Seconds(2.0) + m_expectedPpduDuration - NanoSeconds(1),
                      &TestDlOfdmaPhyTransmission::CheckPhyState, this,
                      m_phySta2, WifiPhyState::CCA_BUSY);
  Simulator::Schedule(Seconds(2.0) + m_expectedPpduDuration - NanoSeconds(1),
                      &TestDlOfdmaPhyTransmission::CheckPhyState, this,
                      m_phySta3, WifiPhyState::RX);
  Simulator::Schedule(Seconds(2.0) + m_expectedPpduDuration,
                      &TestDlOfdmaPhyTransmission::CheckPhyState, this,
                      m_phySta1, WifiPhyState::IDLE);
  Simulator::Schedule(Seconds(2.0) + m_expectedPpduDuration,
                      &TestDlOfdmaPhyTransmission::CheckPhyState, this,
                      m_phySta2, WifiPhyState::IDLE);
  Simulator::Schedule(Seconds(2.0) + m_expectedPpduDuration,
                      &TestDlOfdmaPhyTransmission::CheckPhyState, this,
                      m_phySta3, WifiPhyState::IDLE);

  Simulator::Schedule(Seconds(2.1),
                      &TestDlOfdmaPhyTransmission::CheckResultsSta1, this, 1, 0,
                      1000);
  Simulator::Schedule(Seconds(2.1),
                      &TestDlOfdmaPhyTransmission::CheckResultsSta2, this, 0, 0,
                      0);
  Simulator::Schedule(Seconds(2.1),
                      &TestDlOfdmaPhyTransmission::CheckResultsSta3, this, 1, 0,
                      1500);

  Simulator::Schedule(Seconds(2.5), &TestDlOfdmaPhyTransmission::ResetResults,
                      this);

  Simulator::Schedule(Seconds(3.0), &TestDlOfdmaPhyTransmission::SendMuPpdu,
                      this, 1, 2);

  BandInfo bandInfo;
  bandInfo.fc = (m_frequency - (m_channelWidth / 4)) * 1e6;
  bandInfo.fl = bandInfo.fc - ((m_channelWidth / 4) * 1e6);
  bandInfo.fh = bandInfo.fc + ((m_channelWidth / 4) * 1e6);
  Bands bands;
  bands.push_back(bandInfo);

  Ptr<SpectrumModel> SpectrumInterferenceRu1 = Create<SpectrumModel>(bands);
  Ptr<SpectrumValue> interferencePsdRu1 =
      Create<SpectrumValue>(SpectrumInterferenceRu1);
  double interferencePower = 0.1;
  *interferencePsdRu1 = interferencePower / ((m_channelWidth / 2) * 20e6);

  Simulator::Schedule(Seconds(3.0) + MicroSeconds(50),
                      &TestDlOfdmaPhyTransmission::GenerateInterference, this,
                      interferencePsdRu1, MilliSeconds(100));

  Simulator::Schedule(Seconds(3.0) + m_expectedPpduDuration - NanoSeconds(1),
                      &TestDlOfdmaPhyTransmission::CheckPhyState, this,
                      m_phySta1, WifiPhyState::RX);
  Simulator::Schedule(Seconds(3.0) + m_expectedPpduDuration - NanoSeconds(1),
                      &TestDlOfdmaPhyTransmission::CheckPhyState, this,
                      m_phySta2, WifiPhyState::RX);
  Simulator::Schedule(Seconds(3.0) + m_expectedPpduDuration - NanoSeconds(1),
                      &TestDlOfdmaPhyTransmission::CheckPhyState, this,
                      m_phySta3, WifiPhyState::CCA_BUSY);
  Simulator::Schedule(Seconds(3.0) + m_expectedPpduDuration,
                      &TestDlOfdmaPhyTransmission::CheckPhyState, this,
                      m_phySta1, WifiPhyState::CCA_BUSY);
  Simulator::Schedule(Seconds(3.0) + m_expectedPpduDuration,
                      &TestDlOfdmaPhyTransmission::CheckPhyState, this,
                      m_phySta2, WifiPhyState::CCA_BUSY);
  Simulator::Schedule(Seconds(3.0) + m_expectedPpduDuration,
                      &TestDlOfdmaPhyTransmission::CheckPhyState, this,
                      m_phySta3, WifiPhyState::CCA_BUSY);

  Simulator::Schedule(Seconds(3.1),
                      &TestDlOfdmaPhyTransmission::CheckResultsSta1, this, 0, 1,
                      0);
  Simulator::Schedule(Seconds(3.1),
                      &TestDlOfdmaPhyTransmission::CheckResultsSta2, this, 1, 0,
                      1500);
  Simulator::Schedule(Seconds(3.1),
                      &TestDlOfdmaPhyTransmission::CheckResultsSta3, this, 0, 0,
                      0);

  Simulator::Schedule(Seconds(3.5), &TestDlOfdmaPhyTransmission::ResetResults,
                      this);

  Simulator::Schedule(Seconds(4.0), &TestDlOfdmaPhyTransmission::SendMuPpdu,
                      this, 1, 2);

  bandInfo.fc = (m_frequency + (m_channelWidth / 4)) * 1e6;
  bandInfo.fl = bandInfo.fc - ((m_channelWidth / 4) * 1e6);
  bandInfo.fh = bandInfo.fc + ((m_channelWidth / 4) * 1e6);
  bands.clear();
  bands.push_back(bandInfo);

  Ptr<SpectrumModel> SpectrumInterferenceRu2 = Create<SpectrumModel>(bands);
  Ptr<SpectrumValue> interferencePsdRu2 =
      Create<SpectrumValue>(SpectrumInterferenceRu2);
  *interferencePsdRu2 = interferencePower / ((m_channelWidth / 2) * 20e6);

  Simulator::Schedule(Seconds(4.0) + MicroSeconds(50),
                      &TestDlOfdmaPhyTransmission::GenerateInterference, this,
                      interferencePsdRu2, MilliSeconds(100));

  Simulator::Schedule(Seconds(4.0) + m_expectedPpduDuration - NanoSeconds(1),
                      &TestDlOfdmaPhyTransmission::CheckPhyState, this,
                      m_phySta1, WifiPhyState::RX);
  Simulator::Schedule(Seconds(4.0) + m_expectedPpduDuration - NanoSeconds(1),
                      &TestDlOfdmaPhyTransmission::CheckPhyState, this,
                      m_phySta2, WifiPhyState::RX);
  Simulator::Schedule(Seconds(4.0) + m_expectedPpduDuration - NanoSeconds(1),
                      &TestDlOfdmaPhyTransmission::CheckPhyState, this,
                      m_phySta3, WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(4.0) + m_expectedPpduDuration,
      &TestDlOfdmaPhyTransmission::CheckPhyState, this, m_phySta1,
      (m_channelWidth >= 40) ? WifiPhyState::IDLE : WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(4.0) + m_expectedPpduDuration,
      &TestDlOfdmaPhyTransmission::CheckPhyState, this, m_phySta2,
      (m_channelWidth >= 40) ? WifiPhyState::IDLE : WifiPhyState::CCA_BUSY);
  Simulator::Schedule(
      Seconds(4.0) + m_expectedPpduDuration,
      &TestDlOfdmaPhyTransmission::CheckPhyState, this, m_phySta3,
      (m_channelWidth >= 40) ? WifiPhyState::IDLE : WifiPhyState::CCA_BUSY);

  Simulator::Schedule(Seconds(4.1),
                      &TestDlOfdmaPhyTransmission::CheckResultsSta1, this, 1, 0,
                      1000);
  Simulator::Schedule(Seconds(4.1),
                      &TestDlOfdmaPhyTransmission::CheckResultsSta2, this, 0, 1,
                      0);
  Simulator::Schedule(Seconds(4.1),
                      &TestDlOfdmaPhyTransmission::CheckResultsSta3, this, 0, 0,
                      0);

  Simulator::Schedule(Seconds(4.5), &TestDlOfdmaPhyTransmission::ResetResults,
                      this);

  Simulator::Schedule(Seconds(5.0), &TestDlOfdmaPhyTransmission::SendMuPpdu,
                      this, 1, 2);

  bandInfo.fc = m_frequency * 1e6;
  bandInfo.fl = bandInfo.fc - ((m_channelWidth / 2) * 1e6);
  bandInfo.fh = bandInfo.fc + ((m_channelWidth / 2) * 1e6);
  bands.clear();
  bands.push_back(bandInfo);

  Ptr<SpectrumModel> SpectrumInterferenceAll = Create<SpectrumModel>(bands);
  Ptr<SpectrumValue> interferencePsdAll =
      Create<SpectrumValue>(SpectrumInterferenceAll);
  *interferencePsdAll = interferencePower / (m_channelWidth * 20e6);

  Simulator::Schedule(Seconds(5.0) + MicroSeconds(50),
                      &TestDlOfdmaPhyTransmission::GenerateInterference, this,
                      interferencePsdAll, MilliSeconds(100));

  Simulator::Schedule(Seconds(5.0) + m_expectedPpduDuration - NanoSeconds(1),
                      &TestDlOfdmaPhyTransmission::CheckPhyState, this,
                      m_phySta1, WifiPhyState::RX);
  Simulator::Schedule(Seconds(5.0) + m_expectedPpduDuration - NanoSeconds(1),
                      &TestDlOfdmaPhyTransmission::CheckPhyState, this,
                      m_phySta2, WifiPhyState::RX);
  Simulator::Schedule(Seconds(5.0) + m_expectedPpduDuration - NanoSeconds(1),
                      &TestDlOfdmaPhyTransmission::CheckPhyState, this,
                      m_phySta3, WifiPhyState::CCA_BUSY);
  Simulator::Schedule(Seconds(5.0) + m_expectedPpduDuration,
                      &TestDlOfdmaPhyTransmission::CheckPhyState, this,
                      m_phySta1, WifiPhyState::CCA_BUSY);
  Simulator::Schedule(Seconds(5.0) + m_expectedPpduDuration,
                      &TestDlOfdmaPhyTransmission::CheckPhyState, this,
                      m_phySta2, WifiPhyState::CCA_BUSY);
  Simulator::Schedule(Seconds(5.0) + m_expectedPpduDuration,
                      &TestDlOfdmaPhyTransmission::CheckPhyState, this,
                      m_phySta3, WifiPhyState::CCA_BUSY);

  Simulator::Schedule(Seconds(5.1),
                      &TestDlOfdmaPhyTransmission::CheckResultsSta1, this, 0, 1,
                      0);
  Simulator::Schedule(Seconds(5.1),
                      &TestDlOfdmaPhyTransmission::CheckResultsSta2, this, 0, 1,
                      0);
  Simulator::Schedule(Seconds(5.1),
                      &TestDlOfdmaPhyTransmission::CheckResultsSta3, this, 0, 0,
                      0);

  Simulator::Schedule(Seconds(5.5), &TestDlOfdmaPhyTransmission::ResetResults,
                      this);

  Simulator::Run();
}

void TestDlOfdmaPhyTransmission::DoRun() {
  m_frequency = 5180;
  m_channelWidth = 20;
  m_expectedPpduDuration = NanoSeconds(306400);
  RunOne();

  m_frequency = 5190;
  m_channelWidth = 40;
  m_expectedPpduDuration = NanoSeconds(156800);
  RunOne();

  m_frequency = 5210;
  m_channelWidth = 80;
  m_expectedPpduDuration = NanoSeconds(102400);
  RunOne();

  m_frequency = 5250;
  m_channelWidth = 160;
  m_expectedPpduDuration = NanoSeconds(75200);
  RunOne();

  Simulator::Destroy();
}

class TestDlOfdmaPhyPuncturing : public TestCase {
public:
  TestDlOfdmaPhyPuncturing();

private:
  void DoSetup() override;
  void DoTeardown() override;
  void DoRun() override;

  void RxSuccessSta1(Ptr<const WifiPsdu> psdu, RxSignalInfo rxSignalInfo,
                     WifiTxVector txVector,
                     const std::vector<bool> statusPerMpdu);

  void RxSuccessSta2(Ptr<const WifiPsdu> psdu, RxSignalInfo rxSignalInfo,
                     WifiTxVector txVector, std::vector<bool> statusPerMpdu);

  void RxFailureSta1(Ptr<const WifiPsdu> psdu);

  void RxFailureSta2(Ptr<const WifiPsdu> psdu);

  void CheckResultsSta1(uint32_t expectedRxSuccess, uint32_t expectedRxFailure,
                        uint32_t expectedRxBytes);

  void CheckResultsSta2(uint32_t expectedRxSuccess, uint32_t expectedRxFailure,
                        uint32_t expectedRxBytes);

  void ResetResults();

  void SendMuPpdu(uint16_t rxStaId1, uint16_t rxStaId2,
                  const std::vector<bool> &puncturedSubchannels);

  void GenerateInterference(Ptr<SpectrumValue> interferencePsd, Time duration);

  void StopInterference();

  void RunOne();

  void CheckPhyState(Ptr<OfdmaSpectrumWifiPhy> phy, WifiPhyState expectedState);

  void DoCheckPhyState(Ptr<OfdmaSpectrumWifiPhy> phy,
                       WifiPhyState expectedState);

  uint32_t m_countRxSuccessSta1;
  uint32_t m_countRxSuccessSta2;
  uint32_t m_countRxFailureSta1;
  uint32_t m_countRxFailureSta2;
  uint32_t m_countRxBytesSta1;
  uint32_t m_countRxBytesSta2;

  Ptr<SpectrumWifiPhy> m_phyAp;
  Ptr<OfdmaSpectrumWifiPhy> m_phySta1;
  Ptr<OfdmaSpectrumWifiPhy> m_phySta2;
  Ptr<WaveformGenerator> m_phyInterferer;

  uint16_t m_frequency;
  uint16_t m_channelWidth;

  uint8_t m_indexSubchannel;

  Time m_expectedPpduDuration20Mhz;
  Time m_expectedPpduDuration40Mhz;
};

TestDlOfdmaPhyPuncturing::TestDlOfdmaPhyPuncturing()
    : TestCase("DL-OFDMA PHY puncturing test"), m_countRxSuccessSta1(0),
      m_countRxSuccessSta2(0), m_countRxFailureSta1(0), m_countRxFailureSta2(0),
      m_countRxBytesSta1(0), m_countRxBytesSta2(0), m_frequency(5210),
      m_channelWidth(80), m_indexSubchannel(0),
      m_expectedPpduDuration20Mhz(NanoSeconds(156800)),
      m_expectedPpduDuration40Mhz(NanoSeconds(102400)) {}

void TestDlOfdmaPhyPuncturing::ResetResults() {
  m_countRxSuccessSta1 = 0;
  m_countRxSuccessSta2 = 0;
  m_countRxFailureSta1 = 0;
  m_countRxFailureSta2 = 0;
  m_countRxBytesSta1 = 0;
  m_countRxBytesSta2 = 0;
}

void TestDlOfdmaPhyPuncturing::SendMuPpdu(
    uint16_t rxStaId1, uint16_t rxStaId2,
    const std::vector<bool> &puncturedSubchannels) {
  NS_LOG_FUNCTION(this << rxStaId1 << rxStaId2);
  WifiConstPsduMap psdus;
  WifiTxVector txVector =
      WifiTxVector(HePhy::GetHeMcs7(), 0, WIFI_PREAMBLE_HE_MU, 800, 1, 1, 0,
                   m_channelWidth, false, false);

  HeRu::RuType ruType = puncturedSubchannels.empty()
                            ? HeRu::RU_484_TONE
                            : (puncturedSubchannels.at(1) ? HeRu::RU_242_TONE
                                                          : HeRu::RU_484_TONE);
  HeRu::RuSpec ru1(ruType, 1, true);
  txVector.SetRu(ru1, rxStaId1);
  txVector.SetMode(HePhy::GetHeMcs7(), rxStaId1);
  txVector.SetNss(1, rxStaId1);

  ruType = puncturedSubchannels.empty()
               ? HeRu::RU_484_TONE
               : (puncturedSubchannels.at(1) ? HeRu::RU_484_TONE
                                             : HeRu::RU_242_TONE);
  HeRu::RuSpec ru2(
      ruType,
      ruType == HeRu::RU_484_TONE ? 2 : (puncturedSubchannels.at(3) ? 3 : 4),
      true);
  txVector.SetRu(ru2, rxStaId2);
  txVector.SetMode(HePhy::GetHeMcs9(), rxStaId2);
  txVector.SetNss(1, rxStaId2);

  std::vector<uint8_t> ruAlloc;
  if (puncturedSubchannels.empty()) {
    std::fill_n(std::back_inserter(ruAlloc), 4, 200);
  } else {
    ruAlloc.push_back(puncturedSubchannels.at(1) ? 192 : 200);
    ruAlloc.push_back(puncturedSubchannels.at(1) ? 113 : 200);
    ruAlloc.push_back(puncturedSubchannels.at(2)
                          ? 113
                          : (puncturedSubchannels.at(3) ? 192 : 200));
    ruAlloc.push_back(puncturedSubchannels.at(2)
                          ? 192
                          : (puncturedSubchannels.at(3) ? 113 : 200));
  }

  txVector.SetRuAllocation(ruAlloc, 0);
  txVector.SetSigBMode(VhtPhy::GetVhtMcs5());

  Ptr<Packet> pkt1 = Create<Packet>(1000);
  WifiMacHeader hdr1;
  hdr1.SetType(WIFI_MAC_QOSDATA);
  hdr1.SetQosTid(0);
  hdr1.SetAddr1(Mac48Address("00:00:00:00:00:01"));
  hdr1.SetSequenceNumber(1);
  Ptr<WifiPsdu> psdu1 = Create<WifiPsdu>(pkt1, hdr1);
  psdus.insert(std::make_pair(rxStaId1, psdu1));

  Ptr<Packet> pkt2 = Create<Packet>(1500);
  WifiMacHeader hdr2;
  hdr2.SetType(WIFI_MAC_QOSDATA);
  hdr2.SetQosTid(0);
  hdr2.SetAddr1(Mac48Address("00:00:00:00:00:02"));
  hdr2.SetSequenceNumber(2);
  Ptr<WifiPsdu> psdu2 = Create<WifiPsdu>(pkt2, hdr2);
  psdus.insert(std::make_pair(rxStaId2, psdu2));

  if (!puncturedSubchannels.empty()) {
    txVector.SetInactiveSubchannels(puncturedSubchannels);
  }

  m_phyAp->Send(psdus, txVector);
}

void TestDlOfdmaPhyPuncturing::GenerateInterference(
    Ptr<SpectrumValue> interferencePsd, Time duration) {
  NS_LOG_FUNCTION(this << duration);
  m_phyInterferer->SetTxPowerSpectralDensity(interferencePsd);
  m_phyInterferer->SetPeriod(duration);
  m_phyInterferer->Start();
  Simulator::Schedule(duration, &TestDlOfdmaPhyPuncturing::StopInterference,
                      this);
}

void TestDlOfdmaPhyPuncturing::StopInterference() {
  NS_LOG_FUNCTION(this);
  m_phyInterferer->Stop();
}

void TestDlOfdmaPhyPuncturing::RxSuccessSta1(Ptr<const WifiPsdu> psdu,
                                             RxSignalInfo rxSignalInfo,
                                             WifiTxVector txVector,
                                             std::vector<bool>) {
  NS_LOG_FUNCTION(this << *psdu << rxSignalInfo << txVector);
  m_countRxSuccessSta1++;
  m_countRxBytesSta1 += (psdu->GetSize() - 30);
}

void TestDlOfdmaPhyPuncturing::RxSuccessSta2(Ptr<const WifiPsdu> psdu,
                                             RxSignalInfo rxSignalInfo,
                                             WifiTxVector txVector,
                                             std::vector<bool>) {
  NS_LOG_FUNCTION(this << *psdu << rxSignalInfo << txVector);
  m_countRxSuccessSta2++;
  m_countRxBytesSta2 += (psdu->GetSize() - 30);
}

void TestDlOfdmaPhyPuncturing::RxFailureSta1(Ptr<const WifiPsdu> psdu) {
  NS_LOG_FUNCTION(this << *psdu);
  m_countRxFailureSta1++;
}

void TestDlOfdmaPhyPuncturing::RxFailureSta2(Ptr<const WifiPsdu> psdu) {
  NS_LOG_FUNCTION(this << *psdu);
  m_countRxFailureSta2++;
}

void TestDlOfdmaPhyPuncturing::CheckResultsSta1(uint32_t expectedRxSuccess,
                                                uint32_t expectedRxFailure,
                                                uint32_t expectedRxBytes) {
  NS_TEST_ASSERT_MSG_EQ(
      m_countRxSuccessSta1, expectedRxSuccess,
      "The number of successfully received packets by STA 1 is not correct!");
  NS_TEST_ASSERT_MSG_EQ(
      m_countRxFailureSta1, expectedRxFailure,
      "The number of unsuccessfuly received packets by STA 1 is not correct!");
  NS_TEST_ASSERT_MSG_EQ(
      m_countRxBytesSta1, expectedRxBytes,
      "The number of bytes received by STA 1 is not correct!");
}

void TestDlOfdmaPhyPuncturing::CheckResultsSta2(uint32_t expectedRxSuccess,
                                                uint32_t expectedRxFailure,
                                                uint32_t expectedRxBytes) {
  NS_TEST_ASSERT_MSG_EQ(
      m_countRxSuccessSta2, expectedRxSuccess,
      "The number of successfully received packets by STA 2 is not correct!");
  NS_TEST_ASSERT_MSG_EQ(
      m_countRxFailureSta2, expectedRxFailure,
      "The number of unsuccessfuly received packets by STA 2 is not correct!");
  NS_TEST_ASSERT_MSG_EQ(
      m_countRxBytesSta2, expectedRxBytes,
      "The number of bytes received by STA 2 is not correct!");
}

void TestDlOfdmaPhyPuncturing::CheckPhyState(Ptr<OfdmaSpectrumWifiPhy> phy,
                                             WifiPhyState expectedState) {
  Simulator::ScheduleNow(&TestDlOfdmaPhyPuncturing::DoCheckPhyState, this, phy,
                         expectedState);
}

void TestDlOfdmaPhyPuncturing::DoCheckPhyState(Ptr<OfdmaSpectrumWifiPhy> phy,
                                               WifiPhyState expectedState) {
  WifiPhyState currentState;
  PointerValue ptr;
  phy->GetAttribute("State", ptr);
  Ptr<WifiPhyStateHelper> state =
      DynamicCast<WifiPhyStateHelper>(ptr.Get<WifiPhyStateHelper>());
  currentState = state->GetState();
  NS_LOG_FUNCTION(this << currentState);
  NS_TEST_ASSERT_MSG_EQ(currentState, expectedState,
                        "PHY State "
                            << currentState << " does not match expected state "
                            << expectedState << " at " << Simulator::Now());
}

void TestDlOfdmaPhyPuncturing::DoSetup() {
  Ptr<MultiModelSpectrumChannel> spectrumChannel =
      CreateObject<MultiModelSpectrumChannel>();
  Ptr<FriisPropagationLossModel> lossModel =
      CreateObject<FriisPropagationLossModel>();
  lossModel->SetFrequency(m_frequency * 1e6);
  spectrumChannel->AddPropagationLossModel(lossModel);
  Ptr<ConstantSpeedPropagationDelayModel> delayModel =
      CreateObject<ConstantSpeedPropagationDelayModel>();
  spectrumChannel->SetPropagationDelayModel(delayModel);

  Ptr<Node> apNode = CreateObject<Node>();
  Ptr<WifiNetDevice> apDev = CreateObject<WifiNetDevice>();
  m_phyAp = CreateObject<SpectrumWifiPhy>();
  Ptr<InterferenceHelper> apInterferenceHelper =
      CreateObject<InterferenceHelper>();
  m_phyAp->SetInterferenceHelper(apInterferenceHelper);
  Ptr<ErrorRateModel> apErrorModel = CreateObject<NistErrorRateModel>();
  m_phyAp->SetErrorRateModel(apErrorModel);
  m_phyAp->SetDevice(apDev);
  m_phyAp->AddChannel(spectrumChannel);
  m_phyAp->ConfigureStandard(WIFI_STANDARD_80211ax);
  Ptr<ConstantPositionMobilityModel> apMobility =
      CreateObject<ConstantPositionMobilityModel>();
  m_phyAp->SetMobility(apMobility);
  apDev->SetPhy(m_phyAp);
  apNode->AggregateObject(apMobility);
  apNode->AddDevice(apDev);

  Ptr<Node> sta1Node = CreateObject<Node>();
  Ptr<WifiNetDevice> sta1Dev = CreateObject<WifiNetDevice>();
  m_phySta1 = CreateObject<OfdmaSpectrumWifiPhy>(1);
  Ptr<InterferenceHelper> sta1InterferenceHelper =
      CreateObject<InterferenceHelper>();
  m_phySta1->SetInterferenceHelper(sta1InterferenceHelper);
  Ptr<ErrorRateModel> sta1ErrorModel = CreateObject<NistErrorRateModel>();
  m_phySta1->SetErrorRateModel(sta1ErrorModel);
  m_phySta1->SetDevice(sta1Dev);
  m_phySta1->AddChannel(spectrumChannel);
  m_phySta1->ConfigureStandard(WIFI_STANDARD_80211ax);
  m_phySta1->SetReceiveOkCallback(
      MakeCallback(&TestDlOfdmaPhyPuncturing::RxSuccessSta1, this));
  m_phySta1->SetReceiveErrorCallback(
      MakeCallback(&TestDlOfdmaPhyPuncturing::RxFailureSta1, this));
  Ptr<ConstantPositionMobilityModel> sta1Mobility =
      CreateObject<ConstantPositionMobilityModel>();
  m_phySta1->SetMobility(sta1Mobility);
  sta1Dev->SetPhy(m_phySta1);
  sta1Node->AggregateObject(sta1Mobility);
  sta1Node->AddDevice(sta1Dev);

  Ptr<Node> sta2Node = CreateObject<Node>();
  Ptr<WifiNetDevice> sta2Dev = CreateObject<WifiNetDevice>();
  m_phySta2 = CreateObject<OfdmaSpectrumWifiPhy>(2);
  Ptr<InterferenceHelper> sta2InterferenceHelper =
      CreateObject<InterferenceHelper>();
  m_phySta2->SetInterferenceHelper(sta2InterferenceHelper);
  Ptr<ErrorRateModel> sta2ErrorModel = CreateObject<NistErrorRateModel>();
  m_phySta2->SetErrorRateModel(sta2ErrorModel);
  m_phySta2->SetDevice(sta2Dev);
  m_phySta2->AddChannel(spectrumChannel);
  m_phySta2->ConfigureStandard(WIFI_STANDARD_80211ax);
  m_phySta2->SetReceiveOkCallback(
      MakeCallback(&TestDlOfdmaPhyPuncturing::RxSuccessSta2, this));
  m_phySta2->SetReceiveErrorCallback(
      MakeCallback(&TestDlOfdmaPhyPuncturing::RxFailureSta2, this));
  Ptr<ConstantPositionMobilityModel> sta2Mobility =
      CreateObject<ConstantPositionMobilityModel>();
  m_phySta2->SetMobility(sta2Mobility);
  sta2Dev->SetPhy(m_phySta2);
  sta2Node->AggregateObject(sta2Mobility);
  sta2Node->AddDevice(sta2Dev);

  Ptr<Node> interfererNode = CreateObject<Node>();
  Ptr<NonCommunicatingNetDevice> interfererDev =
      CreateObject<NonCommunicatingNetDevice>();
  m_phyInterferer = CreateObject<WaveformGenerator>();
  m_phyInterferer->SetDevice(interfererDev);
  m_phyInterferer->SetChannel(spectrumChannel);
  m_phyInterferer->SetDutyCycle(1);
  interfererNode->AddDevice(interfererDev);
}

void TestDlOfdmaPhyPuncturing::DoTeardown() {
  m_phyAp->Dispose();
  m_phyAp = nullptr;
  m_phySta1->Dispose();
  m_phySta1 = nullptr;
  m_phySta2->Dispose();
  m_phySta2 = nullptr;
  m_phyInterferer->Dispose();
  m_phyInterferer = nullptr;
}

void TestDlOfdmaPhyPuncturing::RunOne() {
  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(1);
  int64_t streamNumber = 0;
  m_phyAp->AssignStreams(streamNumber);
  m_phySta1->AssignStreams(streamNumber);
  m_phySta2->AssignStreams(streamNumber);

  auto channelNum = std::get<0>(*WifiPhyOperatingChannel::FindFirst(
      0, m_frequency, m_channelWidth, WIFI_STANDARD_80211ax,
      WIFI_PHY_BAND_5GHZ));

  m_phyAp->SetOperatingChannel(
      WifiPhy::ChannelTuple{channelNum, m_channelWidth, WIFI_PHY_BAND_5GHZ, 0});
  m_phySta1->SetOperatingChannel(
      WifiPhy::ChannelTuple{channelNum, m_channelWidth, WIFI_PHY_BAND_5GHZ, 0});
  m_phySta2->SetOperatingChannel(
      WifiPhy::ChannelTuple{channelNum, m_channelWidth, WIFI_PHY_BAND_5GHZ, 0});

  BandInfo bandInfo;
  bandInfo.fc =
      (m_frequency - (m_channelWidth / 2) + 10 + (m_indexSubchannel * 20)) *
      1e6;
  bandInfo.fl = bandInfo.fc - (5 * 1e6);
  bandInfo.fh = bandInfo.fc + (5 * 1e6);
  Bands bands;
  bands.push_back(bandInfo);

  Ptr<SpectrumModel> spectrumInterference = Create<SpectrumModel>(bands);
  Ptr<SpectrumValue> interferencePsd =
      Create<SpectrumValue>(spectrumInterference);
  double interferencePower = 0.1;
  *interferencePsd = interferencePower / 10e6;

  Simulator::Schedule(Seconds(0.0),
                      &TestDlOfdmaPhyPuncturing::GenerateInterference, this,
                      interferencePsd, Seconds(3));

  Simulator::Schedule(Seconds(1.0), &TestDlOfdmaPhyPuncturing::SendMuPpdu, this,
                      1, 2, std::vector<bool>{});

  Simulator::Schedule(Seconds(1.0) + m_expectedPpduDuration40Mhz -
                          NanoSeconds(1),
                      &TestDlOfdmaPhyPuncturing::CheckPhyState, this, m_phySta1,
                      WifiPhyState::RX);
  Simulator::Schedule(Seconds(1.0) + m_expectedPpduDuration40Mhz -
                          NanoSeconds(1),
                      &TestDlOfdmaPhyPuncturing::CheckPhyState, this, m_phySta2,
                      WifiPhyState::RX);
  Simulator::Schedule(Seconds(1.0) + m_expectedPpduDuration40Mhz,
                      &TestDlOfdmaPhyPuncturing::CheckPhyState, this, m_phySta1,
                      WifiPhyState::IDLE);
  Simulator::Schedule(Seconds(1.0) + m_expectedPpduDuration40Mhz,
                      &TestDlOfdmaPhyPuncturing::CheckPhyState, this, m_phySta2,
                      WifiPhyState::IDLE);

  if (m_indexSubchannel < 2) {
    Simulator::Schedule(Seconds(1.1),
                        &TestDlOfdmaPhyPuncturing::CheckResultsSta1, this, 0, 1,
                        0);
    Simulator::Schedule(Seconds(1.1),
                        &TestDlOfdmaPhyPuncturing::CheckResultsSta2, this, 1, 0,
                        1500);
  } else {
    Simulator::Schedule(Seconds(1.1),
                        &TestDlOfdmaPhyPuncturing::CheckResultsSta1, this, 1, 0,
                        1000);
    Simulator::Schedule(Seconds(1.1),
                        &TestDlOfdmaPhyPuncturing::CheckResultsSta2, this, 0, 1,
                        0);
  }

  Simulator::Schedule(Seconds(1.5), &TestDlOfdmaPhyPuncturing::ResetResults,
                      this);

  std::vector<bool> puncturedSubchannels;
  for (std::size_t i = 0; i < (m_channelWidth / 20); ++i) {
    if (i == m_indexSubchannel) {
      puncturedSubchannels.push_back(true);
    } else {
      puncturedSubchannels.push_back(false);
    }
  }
  Simulator::Schedule(Seconds(2.0), &TestDlOfdmaPhyPuncturing::SendMuPpdu, this,
                      1, 2, puncturedSubchannels);

  Simulator::Schedule(Seconds(2.0) + m_expectedPpduDuration20Mhz -
                          NanoSeconds(1),
                      &TestDlOfdmaPhyPuncturing::CheckPhyState, this, m_phySta1,
                      WifiPhyState::RX);
  Simulator::Schedule(Seconds(2.0) + m_expectedPpduDuration20Mhz -
                          NanoSeconds(1),
                      &TestDlOfdmaPhyPuncturing::CheckPhyState, this, m_phySta2,
                      WifiPhyState::RX);
  Simulator::Schedule(Seconds(2.0) + m_expectedPpduDuration20Mhz,
                      &TestDlOfdmaPhyPuncturing::CheckPhyState, this, m_phySta1,
                      WifiPhyState::IDLE);
  Simulator::Schedule(Seconds(2.0) + m_expectedPpduDuration20Mhz,
                      &TestDlOfdmaPhyPuncturing::CheckPhyState, this, m_phySta2,
                      WifiPhyState::IDLE);

  Simulator::Schedule(Seconds(2.1), &TestDlOfdmaPhyPuncturing::CheckResultsSta1,
                      this, 1, 0, 1000);
  Simulator::Schedule(Seconds(2.1), &TestDlOfdmaPhyPuncturing::CheckResultsSta2,
                      this, 1, 0, 1500);

  Simulator::Schedule(Seconds(2.5), &TestDlOfdmaPhyPuncturing::ResetResults,
                      this);

  Simulator::Run();
}

void TestDlOfdmaPhyPuncturing::DoRun() {
  for (auto index : {1, 2, 3}) {
    m_indexSubchannel = index;
    RunOne();
  }
  Simulator::Destroy();
}

class TestUlOfdmaPpduUid : public TestCase {
public:
  TestUlOfdmaPpduUid();
  ~TestUlOfdmaPpduUid() override;

private:
  void DoSetup() override;
  void DoTeardown() override;
  void DoRun() override;

  void TxPpduAp(uint64_t uid);
  void TxPpduSta1(uint64_t uid);
  void TxPpduSta2(uint64_t uid);
  void ResetPpduUid();

  void SendMuPpdu();
  void SendTbPpdu();
  void SendSuPpdu(uint16_t txStaId);

  void CheckUid(uint16_t staId, uint64_t expectedUid);

  Ptr<OfdmaSpectrumWifiPhy> m_phyAp;
  Ptr<OfdmaSpectrumWifiPhy> m_phySta1;
  Ptr<OfdmaSpectrumWifiPhy> m_phySta2;

  uint64_t m_ppduUidAp;
  uint64_t m_ppduUidSta1;
  uint64_t m_ppduUidSta2;
};

TestUlOfdmaPpduUid::TestUlOfdmaPpduUid()
    : TestCase("UL-OFDMA PPDU UID attribution test"), m_ppduUidAp(UINT64_MAX),
      m_ppduUidSta1(UINT64_MAX), m_ppduUidSta2(UINT64_MAX) {}

TestUlOfdmaPpduUid::~TestUlOfdmaPpduUid() {}

void TestUlOfdmaPpduUid::DoSetup() {
  Ptr<MultiModelSpectrumChannel> spectrumChannel =
      CreateObject<MultiModelSpectrumChannel>();
  Ptr<FriisPropagationLossModel> lossModel =
      CreateObject<FriisPropagationLossModel>();
  lossModel->SetFrequency(DEFAULT_FREQUENCY);
  spectrumChannel->AddPropagationLossModel(lossModel);
  Ptr<ConstantSpeedPropagationDelayModel> delayModel =
      CreateObject<ConstantSpeedPropagationDelayModel>();
  spectrumChannel->SetPropagationDelayModel(delayModel);

  Ptr<Node> apNode = CreateObject<Node>();
  Ptr<WifiNetDevice> apDev = CreateObject<WifiNetDevice>();
  m_phyAp = CreateObject<OfdmaSpectrumWifiPhy>(0);
  Ptr<InterferenceHelper> apInterferenceHelper =
      CreateObject<InterferenceHelper>();
  m_phyAp->SetInterferenceHelper(apInterferenceHelper);
  Ptr<ErrorRateModel> apErrorModel = CreateObject<NistErrorRateModel>();
  m_phyAp->SetErrorRateModel(apErrorModel);
  m_phyAp->AddChannel(spectrumChannel);
  m_phyAp->ConfigureStandard(WIFI_STANDARD_80211ax);
  auto channelNum = std::get<0>(*WifiPhyOperatingChannel::FindFirst(
      0, DEFAULT_FREQUENCY, DEFAULT_CHANNEL_WIDTH, WIFI_STANDARD_80211ax,
      WIFI_PHY_BAND_5GHZ));
  m_phyAp->SetOperatingChannel(WifiPhy::ChannelTuple{
      channelNum, DEFAULT_CHANNEL_WIDTH, (int)(WIFI_PHY_BAND_5GHZ), 0});
  m_phyAp->SetDevice(apDev);
  m_phyAp->TraceConnectWithoutContext(
      "TxPpduUid", MakeCallback(&TestUlOfdmaPpduUid::TxPpduAp, this));
  Ptr<ConstantPositionMobilityModel> apMobility =
      CreateObject<ConstantPositionMobilityModel>();
  m_phyAp->SetMobility(apMobility);
  apDev->SetPhy(m_phyAp);
  apNode->AggregateObject(apMobility);
  apNode->AddDevice(apDev);
  apDev->SetStandard(WIFI_STANDARD_80211ax);
  apDev->SetHeConfiguration(CreateObject<HeConfiguration>());

  Ptr<Node> sta1Node = CreateObject<Node>();
  Ptr<WifiNetDevice> sta1Dev = CreateObject<WifiNetDevice>();
  m_phySta1 = CreateObject<OfdmaSpectrumWifiPhy>(1);
  Ptr<InterferenceHelper> sta1InterferenceHelper =
      CreateObject<InterferenceHelper>();
  m_phySta1->SetInterferenceHelper(sta1InterferenceHelper);
  Ptr<ErrorRateModel> sta1ErrorModel = CreateObject<NistErrorRateModel>();
  m_phySta1->SetErrorRateModel(sta1ErrorModel);
  m_phySta1->AddChannel(spectrumChannel);
  m_phySta1->ConfigureStandard(WIFI_STANDARD_80211ax);
  m_phySta1->SetOperatingChannel(WifiPhy::ChannelTuple{
      channelNum, DEFAULT_CHANNEL_WIDTH, (int)(WIFI_PHY_BAND_5GHZ), 0});
  m_phySta1->SetDevice(sta1Dev);
  m_phySta1->TraceConnectWithoutContext(
      "TxPpduUid", MakeCallback(&TestUlOfdmaPpduUid::TxPpduSta1, this));
  Ptr<ConstantPositionMobilityModel> sta1Mobility =
      CreateObject<ConstantPositionMobilityModel>();
  m_phySta1->SetMobility(sta1Mobility);
  sta1Dev->SetPhy(m_phySta1);
  sta1Node->AggregateObject(sta1Mobility);
  sta1Node->AddDevice(sta1Dev);

  Ptr<Node> sta2Node = CreateObject<Node>();
  Ptr<WifiNetDevice> sta2Dev = CreateObject<WifiNetDevice>();
  m_phySta2 = CreateObject<OfdmaSpectrumWifiPhy>(2);
  Ptr<InterferenceHelper> sta2InterferenceHelper =
      CreateObject<InterferenceHelper>();
  m_phySta2->SetInterferenceHelper(sta2InterferenceHelper);
  Ptr<ErrorRateModel> sta2ErrorModel = CreateObject<NistErrorRateModel>();
  m_phySta2->SetErrorRateModel(sta2ErrorModel);
  m_phySta2->AddChannel(spectrumChannel);
  m_phySta2->ConfigureStandard(WIFI_STANDARD_80211ax);
  m_phySta2->SetOperatingChannel(WifiPhy::ChannelTuple{
      channelNum, DEFAULT_CHANNEL_WIDTH, (int)(WIFI_PHY_BAND_5GHZ), 0});
  m_phySta2->SetDevice(sta2Dev);
  m_phySta2->TraceConnectWithoutContext(
      "TxPpduUid", MakeCallback(&TestUlOfdmaPpduUid::TxPpduSta2, this));
  Ptr<ConstantPositionMobilityModel> sta2Mobility =
      CreateObject<ConstantPositionMobilityModel>();
  m_phySta2->SetMobility(sta2Mobility);
  sta2Dev->SetPhy(m_phySta2);
  sta2Node->AggregateObject(sta2Mobility);
  sta2Node->AddDevice(sta2Dev);
}

void TestUlOfdmaPpduUid::DoTeardown() {
  m_phyAp->Dispose();
  m_phyAp = nullptr;
  m_phySta1->Dispose();
  m_phySta1 = nullptr;
  m_phySta2->Dispose();
  m_phySta2 = nullptr;
}

void TestUlOfdmaPpduUid::CheckUid(uint16_t staId, uint64_t expectedUid) {
  uint64_t uid;
  std::string device;
  switch (staId) {
  case 0:
    uid = m_ppduUidAp;
    device = "AP";
    break;
  case 1:
    uid = m_ppduUidSta1;
    device = "STA1";
    break;
  case 2:
    uid = m_ppduUidSta2;
    device = "STA2";
    break;
  default:
    NS_ABORT_MSG("Unexpected STA-ID");
  }
  NS_TEST_ASSERT_MSG_EQ(uid, expectedUid,
                        "UID " << uid << " does not match expected one "
                               << expectedUid << " for " << device << " at "
                               << Simulator::Now());
}

void TestUlOfdmaPpduUid::TxPpduAp(uint64_t uid) {
  NS_LOG_FUNCTION(this << uid);
  m_ppduUidAp = uid;
}

void TestUlOfdmaPpduUid::TxPpduSta1(uint64_t uid) {
  NS_LOG_FUNCTION(this << uid);
  m_ppduUidSta1 = uid;
}

void TestUlOfdmaPpduUid::TxPpduSta2(uint64_t uid) {
  NS_LOG_FUNCTION(this << uid);
  m_ppduUidSta2 = uid;
}

void TestUlOfdmaPpduUid::ResetPpduUid() {
  NS_LOG_FUNCTION(this);
  m_phyAp->SetPpduUid(0);
}

void TestUlOfdmaPpduUid::SendMuPpdu() {
  WifiConstPsduMap psdus;
  WifiTxVector txVector =
      WifiTxVector(HePhy::GetHeMcs7(), 0, WIFI_PREAMBLE_HE_MU, 800, 1, 1, 0,
                   DEFAULT_CHANNEL_WIDTH, false, false);

  uint16_t rxStaId1 = 1;
  HeRu::RuSpec ru1(HeRu::RU_106_TONE, 1, true);
  txVector.SetRu(ru1, rxStaId1);
  txVector.SetMode(HePhy::GetHeMcs7(), rxStaId1);
  txVector.SetNss(1, rxStaId1);

  uint16_t rxStaId2 = 2;
  HeRu::RuSpec ru2(HeRu::RU_106_TONE, 2, true);
  txVector.SetRu(ru2, rxStaId2);
  txVector.SetMode(HePhy::GetHeMcs9(), rxStaId2);
  txVector.SetNss(1, rxStaId2);
  txVector.SetSigBMode(VhtPhy::GetVhtMcs5());
  txVector.SetRuAllocation({96}, 0);

  Ptr<Packet> pkt1 = Create<Packet>(1000);
  WifiMacHeader hdr1;
  hdr1.SetType(WIFI_MAC_QOSDATA);
  hdr1.SetQosTid(0);
  hdr1.SetAddr1(Mac48Address("00:00:00:00:00:01"));
  hdr1.SetSequenceNumber(1);
  Ptr<WifiPsdu> psdu1 = Create<WifiPsdu>(pkt1, hdr1);
  psdus.insert(std::make_pair(rxStaId1, psdu1));

  Ptr<Packet> pkt2 = Create<Packet>(1500);
  WifiMacHeader hdr2;
  hdr2.SetType(WIFI_MAC_QOSDATA);
  hdr2.SetQosTid(0);
  hdr2.SetAddr1(Mac48Address("00:00:00:00:00:02"));
  hdr2.SetSequenceNumber(2);
  Ptr<WifiPsdu> psdu2 = Create<WifiPsdu>(pkt2, hdr2);
  psdus.insert(std::make_pair(rxStaId2, psdu2));

  m_phyAp->Send(psdus, txVector);
}

void TestUlOfdmaPpduUid::SendTbPpdu() {
  WifiConstPsduMap psdus1;
  WifiConstPsduMap psdus2;

  WifiTxVector txVector1 =
      WifiTxVector(HePhy::GetHeMcs7(), 0, WIFI_PREAMBLE_HE_TB, 1600, 1, 1, 0,
                   DEFAULT_CHANNEL_WIDTH, false, false);
  WifiTxVector txVector2 = txVector1;
  WifiTxVector trigVector = txVector2;

  uint16_t rxStaId1 = 1;
  HeRu::RuSpec ru1(HeRu::RU_106_TONE, 1, false);
  txVector1.SetRu(ru1, rxStaId1);
  txVector1.SetMode(HePhy::GetHeMcs7(), rxStaId1);
  txVector1.SetNss(1, rxStaId1);
  trigVector.SetRu(ru1, rxStaId1);
  trigVector.SetMode(HePhy::GetHeMcs7(), rxStaId1);
  trigVector.SetNss(1, rxStaId1);

  Ptr<Packet> pkt1 = Create<Packet>(1000);
  WifiMacHeader hdr1;
  hdr1.SetType(WIFI_MAC_QOSDATA);
  hdr1.SetQosTid(0);
  hdr1.SetAddr1(Mac48Address("00:00:00:00:00:00"));
  hdr1.SetSequenceNumber(1);
  Ptr<WifiPsdu> psdu1 = Create<WifiPsdu>(pkt1, hdr1);
  psdus1.insert(std::make_pair(rxStaId1, psdu1));

  uint16_t rxStaId2 = 2;
  HeRu::RuSpec ru2(HeRu::RU_106_TONE, 2, false);
  txVector2.SetRu(ru2, rxStaId2);
  txVector2.SetMode(HePhy::GetHeMcs9(), rxStaId2);
  txVector2.SetNss(1, rxStaId2);
  trigVector.SetRu(ru2, rxStaId2);
  trigVector.SetMode(HePhy::GetHeMcs9(), rxStaId2);
  trigVector.SetNss(1, rxStaId2);

  Ptr<Packet> pkt2 = Create<Packet>(1500);
  WifiMacHeader hdr2;
  hdr2.SetType(WIFI_MAC_QOSDATA);
  hdr2.SetQosTid(0);
  hdr2.SetAddr1(Mac48Address("00:00:00:00:00:00"));
  hdr2.SetSequenceNumber(2);
  Ptr<WifiPsdu> psdu2 = Create<WifiPsdu>(pkt2, hdr2);
  psdus2.insert(std::make_pair(rxStaId2, psdu2));

  Time txDuration1 = m_phySta1->CalculateTxDuration(
      psdu1->GetSize(), txVector1, m_phySta1->GetPhyBand(), rxStaId1);
  Time txDuration2 = m_phySta2->CalculateTxDuration(
      psdu2->GetSize(), txVector2, m_phySta1->GetPhyBand(), rxStaId2);
  Time txDuration = std::max(txDuration1, txDuration2);

  txVector1.SetLength(HePhy::ConvertHeTbPpduDurationToLSigLength(
                          txDuration, txVector1, m_phySta1->GetPhyBand())
                          .first);
  txVector2.SetLength(HePhy::ConvertHeTbPpduDurationToLSigLength(
                          txDuration, txVector2, m_phySta2->GetPhyBand())
                          .first);

  auto hePhyAp = DynamicCast<HePhy>(m_phyAp->GetPhyEntity(WIFI_MOD_CLASS_HE));
  hePhyAp->SetTrigVector(trigVector, txDuration);

  m_phySta1->Send(psdus1, txVector1);
  m_phySta2->Send(psdus2, txVector2);
}

void TestUlOfdmaPpduUid::SendSuPpdu(uint16_t txStaId) {
  WifiConstPsduMap psdus;
  WifiTxVector txVector =
      WifiTxVector(HePhy::GetHeMcs7(), 0, WIFI_PREAMBLE_HE_SU, 800, 1, 1, 0,
                   DEFAULT_CHANNEL_WIDTH, false, false);

  Ptr<Packet> pkt = Create<Packet>(1000);
  WifiMacHeader hdr;
  hdr.SetType(WIFI_MAC_QOSDATA);
  hdr.SetQosTid(0);
  hdr.SetAddr1(Mac48Address::GetBroadcast());
  hdr.SetSequenceNumber(1);
  Ptr<WifiPsdu> psdu = Create<WifiPsdu>(pkt, hdr);
  psdus.insert(std::make_pair(SU_STA_ID, psdu));

  switch (txStaId) {
  case 0:
    m_phyAp->Send(psdus, txVector);
    break;
  case 1:
    m_phySta1->Send(psdus, txVector);
    break;
  case 2:
    m_phySta2->Send(psdus, txVector);
    break;
  default:
    NS_ABORT_MSG("Unexpected STA-ID");
  }
}

void TestUlOfdmaPpduUid::DoRun() {
  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(1);
  int64_t streamNumber = 0;
  m_phyAp->AssignStreams(streamNumber);
  m_phySta1->AssignStreams(streamNumber);
  m_phySta2->AssignStreams(streamNumber);

  ResetPpduUid();

  Simulator::Schedule(Seconds(1.0), &TestUlOfdmaPpduUid::SendMuPpdu, this);
  Simulator::Schedule(Seconds(1.0), &TestUlOfdmaPpduUid::CheckUid, this, 0, 0);

  Simulator::Schedule(Seconds(1.1), &TestUlOfdmaPpduUid::SendSuPpdu, this, 0);
  Simulator::Schedule(Seconds(1.1), &TestUlOfdmaPpduUid::CheckUid, this, 0, 1);

  Simulator::Schedule(Seconds(1.15), &TestUlOfdmaPpduUid::SendTbPpdu, this);
  Simulator::Schedule(Seconds(1.15), &TestUlOfdmaPpduUid::CheckUid, this, 1, 1);
  Simulator::Schedule(Seconds(1.15), &TestUlOfdmaPpduUid::CheckUid, this, 2, 1);

  Simulator::Schedule(Seconds(1.2), &TestUlOfdmaPpduUid::SendSuPpdu, this, 1);
  Simulator::Schedule(Seconds(1.2), &TestUlOfdmaPpduUid::CheckUid, this, 1, 2);

  Simulator::Run();
  Simulator::Destroy();
}

class TestMultipleHeTbPreambles : public TestCase {
public:
  TestMultipleHeTbPreambles();
  ~TestMultipleHeTbPreambles() override;

private:
  void DoSetup() override;
  void DoTeardown() override;
  void DoRun() override;

  void RxHeTbPpdu(uint64_t uid, uint16_t staId, double txPowerWatts,
                  size_t payloadSize);

  void RxHeTbPpduOfdmaPart(Ptr<WifiSpectrumSignalParameters> rxParamsOfdma);
  void DoRxHeTbPpduOfdmaPart(Ptr<WifiSpectrumSignalParameters> rxParamsOfdma);

  void RxDropped(Ptr<const Packet> p, WifiPhyRxfailureReason reason);

  void Reset();

  void CheckHeTbPreambles(size_t nEvents, std::vector<uint64_t> uids);

  void CheckBytesDropped(size_t expectedBytesDropped);

  Ptr<OfdmaSpectrumWifiPhy> m_phy;

  uint64_t m_totalBytesDropped;
  WifiTxVector m_trigVector;
};

TestMultipleHeTbPreambles::TestMultipleHeTbPreambles()
    : TestCase("UL-OFDMA multiple RX events test"), m_totalBytesDropped(0),
      m_trigVector(HePhy::GetHeMcs7(), 0, WIFI_PREAMBLE_HE_TB, 1600, 1, 1, 0,
                   DEFAULT_CHANNEL_WIDTH, false, false) {}

TestMultipleHeTbPreambles::~TestMultipleHeTbPreambles() {}

void TestMultipleHeTbPreambles::Reset() {
  NS_LOG_FUNCTION(this);
  m_totalBytesDropped = 0;
  m_phy->Reset();
  m_trigVector.GetHeMuUserInfoMap().clear();
}

void TestMultipleHeTbPreambles::RxDropped(Ptr<const Packet> p,
                                          WifiPhyRxfailureReason reason) {
  NS_LOG_FUNCTION(this << p << reason);
  m_totalBytesDropped += (p->GetSize() - 30);
}

void TestMultipleHeTbPreambles::CheckHeTbPreambles(size_t nEvents,
                                                   std::vector<uint64_t> uids) {
  auto events = m_phy->GetCurrentPreambleEvents();
  NS_TEST_ASSERT_MSG_EQ(events.size(), nEvents,
                        "The number of UL MU events is not correct!");
  for (const auto &uid : uids) {
    auto pair = std::make_pair(uid, WIFI_PREAMBLE_HE_TB);
    auto it = events.find(pair);
    bool found = (it != events.end());
    NS_TEST_ASSERT_MSG_EQ(found, true,
                          "HE TB PPDU with UID " << uid
                                                 << " has not been received!");
  }
}

void TestMultipleHeTbPreambles::CheckBytesDropped(size_t expectedBytesDropped) {
  NS_TEST_ASSERT_MSG_EQ(m_totalBytesDropped, expectedBytesDropped,
                        "The number of dropped bytes is not correct!");
}

void TestMultipleHeTbPreambles::RxHeTbPpdu(uint64_t uid, uint16_t staId,
                                           double txPowerWatts,
                                           size_t payloadSize) {
  WifiConstPsduMap psdus;
  WifiTxVector txVector =
      WifiTxVector(HePhy::GetHeMcs7(), 0, WIFI_PREAMBLE_HE_TB, 1600, 1, 1, 0,
                   DEFAULT_CHANNEL_WIDTH, false, false);

  HeRu::RuSpec ru(HeRu::RU_106_TONE, staId, false);
  txVector.SetRu(ru, staId);
  txVector.SetMode(HePhy::GetHeMcs7(), staId);
  txVector.SetNss(1, staId);

  m_trigVector.SetHeMuUserInfo(staId, {ru, 7, 1});

  Ptr<Packet> pkt = Create<Packet>(payloadSize);
  WifiMacHeader hdr;
  hdr.SetType(WIFI_MAC_QOSDATA);
  hdr.SetQosTid(0);
  hdr.SetAddr1(Mac48Address("00:00:00:00:00:00"));
  hdr.SetSequenceNumber(1);
  Ptr<WifiPsdu> psdu = Create<WifiPsdu>(pkt, hdr);
  psdus.insert(std::make_pair(staId, psdu));

  Time ppduDuration = m_phy->CalculateTxDuration(psdu->GetSize(), txVector,
                                                 m_phy->GetPhyBand(), staId);
  Ptr<HePpdu> ppdu =
      Create<HePpdu>(psdus, txVector, m_phy->GetOperatingChannel(),
                     ppduDuration, uid, HePpdu::PSD_NON_HE_PORTION);

  Time nonOfdmaDuration =
      m_phy->GetHePhy()->CalculateNonHeDurationForHeTb(txVector);
  uint32_t centerFrequency =
      m_phy->GetHePhy()->GetCenterFrequencyForNonHePart(txVector, staId);
  uint16_t ruWidth = HeRu::GetBandwidth(txVector.GetRu(staId).GetRuType());
  uint16_t channelWidth = ruWidth < 20 ? 20 : ruWidth;
  Ptr<SpectrumValue> rxPsd =
      WifiSpectrumValueHelper::CreateHeOfdmTxPowerSpectralDensity(
          centerFrequency, channelWidth, txPowerWatts,
          m_phy->GetGuardBandwidth(channelWidth));
  Ptr<WifiSpectrumSignalParameters> rxParams =
      Create<WifiSpectrumSignalParameters>();
  rxParams->psd = rxPsd;
  rxParams->txPhy = nullptr;
  rxParams->duration = nonOfdmaDuration;
  rxParams->ppdu = ppdu;

  uint16_t length;
  std::tie(length, ppduDuration) = HePhy::ConvertHeTbPpduDurationToLSigLength(
      ppduDuration, txVector, m_phy->GetPhyBand());
  txVector.SetLength(length);
  m_trigVector.SetLength(length);
  auto hePhy = DynamicCast<HePhy>(m_phy->GetLatestPhyEntity());
  hePhy->SetTrigVector(m_trigVector, ppduDuration);
  ppdu->ResetTxVector();
  m_phy->StartRx(rxParams, nullptr);

  Ptr<HePpdu> ppduOfdma = DynamicCast<HePpdu>(ppdu->Copy());
  ppduOfdma->SetTxPsdFlag(HePpdu::PSD_HE_PORTION);
  const auto band = m_phy->GetHePhy()->GetRuBandForRx(txVector, staId);
  Ptr<SpectrumValue> rxPsdOfdma =
      WifiSpectrumValueHelper::CreateHeMuOfdmTxPowerSpectralDensity(
          DEFAULT_FREQUENCY, DEFAULT_CHANNEL_WIDTH, txPowerWatts,
          DEFAULT_GUARD_WIDTH, band.indices);
  Ptr<WifiSpectrumSignalParameters> rxParamsOfdma =
      Create<WifiSpectrumSignalParameters>();
  rxParamsOfdma->psd = rxPsd;
  rxParamsOfdma->txPhy = nullptr;
  rxParamsOfdma->duration = ppduDuration - nonOfdmaDuration;
  rxParamsOfdma->ppdu = ppduOfdma;
  Simulator::Schedule(nonOfdmaDuration,
                      &TestMultipleHeTbPreambles::RxHeTbPpduOfdmaPart, this,
                      rxParamsOfdma);
}

void TestMultipleHeTbPreambles::RxHeTbPpduOfdmaPart(
    Ptr<WifiSpectrumSignalParameters> rxParamsOfdma) {
  Simulator::ScheduleNow(&TestMultipleHeTbPreambles::DoRxHeTbPpduOfdmaPart,
                         this, rxParamsOfdma);
}

void TestMultipleHeTbPreambles::DoRxHeTbPpduOfdmaPart(
    Ptr<WifiSpectrumSignalParameters> rxParamsOfdma) {
  m_phy->StartRx(rxParamsOfdma, nullptr);
}

void TestMultipleHeTbPreambles::DoSetup() {
  Ptr<MultiModelSpectrumChannel> spectrumChannel =
      CreateObject<MultiModelSpectrumChannel>();
  Ptr<Node> node = CreateObject<Node>();
  Ptr<WifiNetDevice> dev = CreateObject<WifiNetDevice>();
  dev->SetStandard(WIFI_STANDARD_80211ax);
  m_phy = CreateObject<OfdmaSpectrumWifiPhy>(0);
  Ptr<InterferenceHelper> interferenceHelper =
      CreateObject<InterferenceHelper>();
  Ptr<ErrorRateModel> error = CreateObject<NistErrorRateModel>();
  Ptr<ApWifiMac> mac = CreateObject<ApWifiMac>();
  mac->SetAttribute("BeaconGeneration", BooleanValue(false));
  dev->SetMac(mac);
  m_phy->SetInterferenceHelper(interferenceHelper);
  m_phy->SetErrorRateModel(error);
  m_phy->AddChannel(spectrumChannel);
  m_phy->ConfigureStandard(WIFI_STANDARD_80211ax);
  m_phy->SetOperatingChannel(WifiPhy::ChannelTuple{
      DEFAULT_CHANNEL_NUMBER, DEFAULT_CHANNEL_WIDTH, WIFI_PHY_BAND_5GHZ, 0});
  m_phy->TraceConnectWithoutContext(
      "PhyRxDrop", MakeCallback(&TestMultipleHeTbPreambles::RxDropped, this));
  m_phy->SetDevice(dev);
  Ptr<ThresholdPreambleDetectionModel> preambleDetectionModel =
      CreateObject<ThresholdPreambleDetectionModel>();
  preambleDetectionModel->SetAttribute("Threshold", DoubleValue(4));
  preambleDetectionModel->SetAttribute("MinimumRssi", DoubleValue(-82));
  m_phy->SetPreambleDetectionModel(preambleDetectionModel);
  Ptr<HeConfiguration> heConfiguration = CreateObject<HeConfiguration>();
  heConfiguration->SetMaxTbPpduDelay(NanoSeconds(400));
  dev->SetHeConfiguration(heConfiguration);
  dev->SetPhy(m_phy);
  node->AddDevice(dev);
}

void TestMultipleHeTbPreambles::DoTeardown() {
  m_phy->Dispose();
  m_phy = nullptr;
}

void TestMultipleHeTbPreambles::DoRun() {
  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(1);
  int64_t streamNumber = 0;
  m_phy->AssignStreams(streamNumber);

  double txPowerWatts = 0.01;

  {
    std::vector<uint64_t> uids{0};
    Simulator::Schedule(Seconds(1), &TestMultipleHeTbPreambles::RxHeTbPpdu,
                        this, uids[0], 1, txPowerWatts, 1001);
    Simulator::Schedule(Seconds(1) + NanoSeconds(100),
                        &TestMultipleHeTbPreambles::RxHeTbPpdu, this, uids[0],
                        2, txPowerWatts, 1002);
    Simulator::Schedule(Seconds(1.0) + MicroSeconds(1),
                        &TestMultipleHeTbPreambles::CheckHeTbPreambles, this, 1,
                        uids);
    Simulator::Schedule(Seconds(1.5), &TestMultipleHeTbPreambles::Reset, this);
  }

  {
    std::vector<uint64_t> uids{1, 2};
    Simulator::Schedule(Seconds(2), &TestMultipleHeTbPreambles::RxHeTbPpdu,
                        this, uids[0], 1, txPowerWatts, 1001);
    Simulator::Schedule(Seconds(2) + NanoSeconds(100),
                        &TestMultipleHeTbPreambles::RxHeTbPpdu, this, uids[0],
                        2, txPowerWatts, 1002);
    Simulator::Schedule(Seconds(2) + NanoSeconds(200),
                        &TestMultipleHeTbPreambles::RxHeTbPpdu, this, uids[1],
                        1, txPowerWatts / 2, 1003);
    Simulator::Schedule(Seconds(2) + NanoSeconds(300),
                        &TestMultipleHeTbPreambles::RxHeTbPpdu, this, uids[1],
                        2, txPowerWatts / 2, 1004);
    Simulator::Schedule(Seconds(2.0) + MicroSeconds(1),
                        &TestMultipleHeTbPreambles::CheckHeTbPreambles, this, 2,
                        uids);
    Simulator::Schedule(Seconds(2.5), &TestMultipleHeTbPreambles::Reset, this);
  }

  {
    std::vector<uint64_t> uids{3, 4};
    Simulator::Schedule(Seconds(3), &TestMultipleHeTbPreambles::RxHeTbPpdu,
                        this, uids[0], 1, txPowerWatts / 2, 1001);
    Simulator::Schedule(Seconds(3) + NanoSeconds(100),
                        &TestMultipleHeTbPreambles::RxHeTbPpdu, this, uids[0],
                        2, txPowerWatts / 2, 1002);
    Simulator::Schedule(Seconds(3) + NanoSeconds(200),
                        &TestMultipleHeTbPreambles::RxHeTbPpdu, this, uids[1],
                        1, txPowerWatts, 1003);
    Simulator::Schedule(Seconds(3) + NanoSeconds(300),
                        &TestMultipleHeTbPreambles::RxHeTbPpdu, this, uids[1],
                        2, txPowerWatts, 1004);
    Simulator::Schedule(Seconds(3.0) + MicroSeconds(1),
                        &TestMultipleHeTbPreambles::CheckHeTbPreambles, this, 2,
                        uids);
    Simulator::Schedule(Seconds(3.5), &TestMultipleHeTbPreambles::Reset, this);
  }

  {
    std::vector<uint64_t> uids{5, 6};
    Simulator::Schedule(Seconds(4), &TestMultipleHeTbPreambles::RxHeTbPpdu,
                        this, uids[0], 1, txPowerWatts, 1001);
    Simulator::Schedule(Seconds(4) + NanoSeconds(100),
                        &TestMultipleHeTbPreambles::RxHeTbPpdu, this, uids[0],
                        2, txPowerWatts, 1002);
    Simulator::Schedule(Seconds(4) + MicroSeconds(5),
                        &TestMultipleHeTbPreambles::RxHeTbPpdu, this, uids[1],
                        1, txPowerWatts, 1003);
    Simulator::Schedule(Seconds(4) + MicroSeconds(5) + NanoSeconds(100),
                        &TestMultipleHeTbPreambles::RxHeTbPpdu, this, uids[1],
                        2, txPowerWatts, 1004);
    Simulator::Schedule(Seconds(4.0) + MicroSeconds(10),
                        &TestMultipleHeTbPreambles::CheckHeTbPreambles, this, 1,
                        std::vector<uint64_t>{uids[0]});
    Simulator::Schedule(Seconds(4.0) + MicroSeconds(10),
                        &TestMultipleHeTbPreambles::CheckBytesDropped, this,
                        1003 + 1004);
    Simulator::Schedule(Seconds(4.5), &TestMultipleHeTbPreambles::Reset, this);
  }

  {
    std::vector<uint64_t> uids{7, 8};
    Simulator::Schedule(Seconds(5), &TestMultipleHeTbPreambles::RxHeTbPpdu,
                        this, uids[0], 1, txPowerWatts, 1001);
    Simulator::Schedule(Seconds(5) + NanoSeconds(100),
                        &TestMultipleHeTbPreambles::RxHeTbPpdu, this, uids[0],
                        2, txPowerWatts, 1002);
    Simulator::Schedule(Seconds(5) + MicroSeconds(50),
                        &TestMultipleHeTbPreambles::RxHeTbPpdu, this, uids[1],
                        1, txPowerWatts, 1003);
    Simulator::Schedule(Seconds(5) + MicroSeconds(50) + NanoSeconds(100),
                        &TestMultipleHeTbPreambles::RxHeTbPpdu, this, uids[1],
                        2, txPowerWatts, 1004);
    Simulator::Schedule(Seconds(5.0) + MicroSeconds(100),
                        &TestMultipleHeTbPreambles::CheckHeTbPreambles, this, 1,
                        std::vector<uint64_t>{uids[0]});
    Simulator::Schedule(Seconds(5.0) + MicroSeconds(100),
                        &TestMultipleHeTbPreambles::CheckBytesDropped, this,
                        1003 + 1004);
    Simulator::Schedule(Seconds(5.5), &TestMultipleHeTbPreambles::Reset, this);
  }

  {
    std::vector<uint64_t> uids{9};
    Simulator::Schedule(Seconds(6), &TestMultipleHeTbPreambles::RxHeTbPpdu,
                        this, uids[0], 1, txPowerWatts, 1001);
    Simulator::Schedule(Seconds(6) + NanoSeconds(500),
                        &TestMultipleHeTbPreambles::RxHeTbPpdu, this, uids[0],
                        2, txPowerWatts, 1002);
    Simulator::Schedule(Seconds(6.0) + MicroSeconds(1),
                        &TestMultipleHeTbPreambles::CheckHeTbPreambles, this, 1,
                        uids);
    Simulator::Schedule(Seconds(6.0) + MicroSeconds(5),
                        &TestMultipleHeTbPreambles::CheckBytesDropped, this,
                        1001 + 1002);
    Simulator::Schedule(Seconds(6.5), &TestMultipleHeTbPreambles::Reset, this);
  }

  Simulator::Run();
  Simulator::Destroy();
}

class OfdmaTestPhyListener : public ns3::WifiPhyListener {
public:
  OfdmaTestPhyListener() = default;

  void NotifyRxStart(Time duration) override {
    NS_LOG_FUNCTION(this << duration);
    m_lastRxStart = Simulator::Now();
    ++m_notifyRxStart;
    m_lastRxSuccess = false;
  }

  void NotifyRxEndOk() override {
    NS_LOG_FUNCTION(this);
    m_lastRxEnd = Simulator::Now();
    ++m_notifyRxEnd;
    m_lastRxSuccess = true;
  }

  void NotifyRxEndError() override {
    NS_LOG_FUNCTION(this);
    m_lastRxEnd = Simulator::Now();
    ++m_notifyRxEnd;
    m_lastRxSuccess = false;
  }

  void NotifyTxStart(Time duration, double txPowerDbm) override {
    NS_LOG_FUNCTION(this << duration << txPowerDbm);
  }

  void NotifyCcaBusyStart(Time duration, WifiChannelListType channelType,
                          const std::vector<Time> &) override {
    NS_LOG_FUNCTION(this << duration << channelType);
  }

  void NotifySwitchingStart(Time duration) override {}

  void NotifySleep() override {}

  void NotifyOff() override {}

  void NotifyWakeup() override {}

  void NotifyOn() override {}

  void Reset() {
    m_notifyRxStart = 0;
    m_notifyRxEnd = 0;
    m_lastRxStart = Seconds(0);
    m_lastRxEnd = Seconds(0);
    m_lastRxSuccess = false;
  }

  uint32_t GetNumRxStartNotifications() const { return m_notifyRxStart; }

  uint32_t GetNumRxEndNotifications() const { return m_notifyRxEnd; }

  Time GetLastRxStartNotification() const { return m_lastRxStart; }

  Time GetLastRxEndNotification() const { return m_lastRxEnd; }

  bool IsLastRxSuccess() const { return m_lastRxSuccess; }

private:
  uint32_t m_notifyRxStart{0};
  uint32_t m_notifyRxEnd{0};
  Time m_lastRxStart{Seconds(0)};
  Time m_lastRxEnd{Seconds(0)};
  bool m_lastRxSuccess{false};
};

class TestUlOfdmaPhyTransmission : public TestCase {
public:
  enum TrigVectorInfo {
    NONE = 0,
    CHANNEL_WIDTH,
    UL_LENGTH,
    AID,
  };

  TestUlOfdmaPhyTransmission();
  ~TestUlOfdmaPhyTransmission() override;

private:
  void DoSetup() override;
  void DoTeardown() override;
  void DoRun() override;

  WifiTxVector GetTxVectorForHeTbPpdu(uint16_t txStaId, std::size_t index,
                                      uint8_t bssColor) const;
  void SetTrigVector(uint8_t bssColor, TrigVectorInfo error);
  void SendHeTbPpdu(uint16_t txStaId, std::size_t index,
                    std::size_t payloadSize, uint64_t uid, uint8_t bssColor,
                    bool incrementUid);

  void SendHeSuPpdu(uint16_t txStaId, std::size_t payloadSize, uint64_t uid,
                    uint8_t bssColor);

  void SetBssColor(Ptr<WifiPhy> phy, uint8_t bssColor);

  void SetPsdLimit(Ptr<WifiPhy> phy, double psdLimit);

  void GenerateInterference(Ptr<SpectrumValue> interferencePsd, Time duration);
  void StopInterference();

  void RunOne();

  void CheckRxFromSta1(uint32_t expectedSuccess, uint32_t expectedFailures,
                       uint32_t expectedBytes);

  void CheckRxFromSta2(uint32_t expectedSuccess, uint32_t expectedFailures,
                       uint32_t expectedBytes);

  void CheckNonOfdmaRxPower(Ptr<OfdmaSpectrumWifiPhy> phy,
                            WifiSpectrumBandInfo band, double expectedRxPower);
  void CheckOfdmaRxPower(Ptr<OfdmaSpectrumWifiPhy> phy,
                         WifiSpectrumBandInfo band, double expectedRxPower);

  void VerifyEventsCleared();

  void CheckPhyState(Ptr<OfdmaSpectrumWifiPhy> phy, WifiPhyState expectedState);
  void DoCheckPhyState(Ptr<OfdmaSpectrumWifiPhy> phy,
                       WifiPhyState expectedState);

  void CheckApRxStart(uint32_t expectedNotifications,
                      Time expectedLastNotification);
  void CheckApRxEnd(uint32_t expectedNotifications,
                    Time expectedLastNotification, bool expectedSuccess);

  void Reset();

  void RxSuccess(Ptr<const WifiPsdu> psdu, RxSignalInfo rxSignalInfo,
                 WifiTxVector txVector, std::vector<bool> statusPerMpdu);

  void RxFailure(Ptr<const WifiPsdu> psdu);

  void ScheduleTest(Time delay, bool solicited, WifiPhyState expectedStateAtEnd,
                    uint32_t expectedSuccessFromSta1,
                    uint32_t expectedFailuresFromSta1,
                    uint32_t expectedBytesFromSta1,
                    uint32_t expectedSuccessFromSta2,
                    uint32_t expectedFailuresFromSta2,
                    uint32_t expectedBytesFromSta2, bool scheduleTxSta1 = true,
                    Time ulTimeDifference = Seconds(0),
                    WifiPhyState expectedStateBeforeEnd = WifiPhyState::RX,
                    TrigVectorInfo error = NONE);

  void SchedulePowerMeasurementChecks(Time delay, double rxPowerNonOfdmaRu1,
                                      double rxPowerNonOfdmaRu2,
                                      double rxPowerOfdmaRu1,
                                      double rxPowerOfdmaRu2);
  void LogScenario(std::string log) const;

  Ptr<OfdmaSpectrumWifiPhy> m_phyAp;
  Ptr<OfdmaSpectrumWifiPhy> m_phySta1;
  Ptr<OfdmaSpectrumWifiPhy> m_phySta2;
  Ptr<OfdmaSpectrumWifiPhy> m_phySta3;

  std::unique_ptr<OfdmaTestPhyListener> m_apPhyStateListener;

  Ptr<WaveformGenerator> m_phyInterferer;

  uint32_t m_countRxSuccessFromSta1;
  uint32_t m_countRxSuccessFromSta2;
  uint32_t m_countRxFailureFromSta1;
  uint32_t m_countRxFailureFromSta2;
  uint32_t m_countRxBytesFromSta1;
  uint32_t m_countRxBytesFromSta2;

  uint16_t m_frequency;
  uint16_t m_channelWidth;
  Time m_expectedPpduDuration;
};

TestUlOfdmaPhyTransmission::TestUlOfdmaPhyTransmission()
    : TestCase("UL-OFDMA PHY test"), m_countRxSuccessFromSta1(0),
      m_countRxSuccessFromSta2(0), m_countRxFailureFromSta1(0),
      m_countRxFailureFromSta2(0), m_countRxBytesFromSta1(0),
      m_countRxBytesFromSta2(0), m_frequency(DEFAULT_FREQUENCY),
      m_channelWidth(DEFAULT_CHANNEL_WIDTH),
      m_expectedPpduDuration(NanoSeconds(271200)) {}

void TestUlOfdmaPhyTransmission::SendHeSuPpdu(uint16_t txStaId,
                                              std::size_t payloadSize,
                                              uint64_t uid, uint8_t bssColor) {
  NS_LOG_FUNCTION(this << txStaId << payloadSize << uid << +bssColor);
  WifiConstPsduMap psdus;

  WifiTxVector txVector =
      WifiTxVector(HePhy::GetHeMcs7(), 0, WIFI_PREAMBLE_HE_SU, 800, 1, 1, 0,
                   m_channelWidth, false, false, false, bssColor);

  Ptr<Packet> pkt = Create<Packet>(payloadSize);
  WifiMacHeader hdr;
  hdr.SetType(WIFI_MAC_QOSDATA);
  hdr.SetQosTid(0);
  hdr.SetAddr1(Mac48Address("00:00:00:00:00:00"));
  std::ostringstream addr;
  addr << "00:00:00:00:00:0" << txStaId;
  hdr.SetAddr2(Mac48Address(addr.str().c_str()));
  hdr.SetSequenceNumber(1);
  Ptr<WifiPsdu> psdu = Create<WifiPsdu>(pkt, hdr);
  psdus.insert(std::make_pair(SU_STA_ID, psdu));

  Ptr<OfdmaSpectrumWifiPhy> phy;
  if (txStaId == 1) {
    phy = m_phySta1;
  } else if (txStaId == 2) {
    phy = m_phySta2;
  } else if (txStaId == 3) {
    phy = m_phySta3;
  } else if (txStaId == 0) {
    phy = m_phyAp;
  }
  phy->SetPpduUid(uid);
  phy->Send(psdus, txVector);
}

WifiTxVector TestUlOfdmaPhyTransmission::GetTxVectorForHeTbPpdu(
    uint16_t txStaId, std::size_t index, uint8_t bssColor) const {
  WifiTxVector txVector =
      WifiTxVector(HePhy::GetHeMcs7(), 0, WIFI_PREAMBLE_HE_TB, 1600, 1, 1, 0,
                   m_channelWidth, false, false, false, bssColor);

  HeRu::RuType ruType = HeRu::RU_106_TONE;
  if (m_channelWidth == 20) {
    ruType = HeRu::RU_106_TONE;
  } else if (m_channelWidth == 40) {
    ruType = HeRu::RU_242_TONE;
  } else if (m_channelWidth == 80) {
    ruType = HeRu::RU_484_TONE;
  } else if (m_channelWidth == 160) {
    ruType = HeRu::RU_996_TONE;
  } else {
    NS_ASSERT_MSG(false, "Unsupported channel width");
  }

  bool primary80MHz = true;
  if (m_channelWidth == 160 && index == 2) {
    primary80MHz = false;
    index = 1;
  }
  HeRu::RuSpec ru(ruType, index, primary80MHz);
  txVector.SetRu(ru, txStaId);
  txVector.SetMode(HePhy::GetHeMcs7(), txStaId);
  txVector.SetNss(1, txStaId);
  return txVector;
}

void TestUlOfdmaPhyTransmission::SetTrigVector(uint8_t bssColor,
                                               TrigVectorInfo error) {
  uint16_t channelWidth = m_channelWidth;
  if (error == CHANNEL_WIDTH) {
    channelWidth = (channelWidth == 160 ? 20 : channelWidth * 2);
  }

  WifiTxVector txVector(HePhy::GetHeMcs7(), 0, WIFI_PREAMBLE_HE_TB, 1600, 1, 1,
                        0, channelWidth, false, false, false, bssColor);

  HeRu::RuType ruType = HeRu::RU_106_TONE;
  if (channelWidth == 20) {
    ruType = HeRu::RU_106_TONE;
  } else if (channelWidth == 40) {
    ruType = HeRu::RU_242_TONE;
  } else if (channelWidth == 80) {
    ruType = HeRu::RU_484_TONE;
  } else if (channelWidth == 160) {
    ruType = HeRu::RU_996_TONE;
  } else {
    NS_ASSERT_MSG(false, "Unsupported channel width");
  }

  uint16_t aid1 = (error == AID ? 3 : 1);
  uint16_t aid2 = (error == AID ? 4 : 2);

  HeRu::RuSpec ru1(ruType, 1, true);
  txVector.SetRu(ru1, aid1);
  txVector.SetMode(HePhy::GetHeMcs7(), aid1);
  txVector.SetNss(1, aid1);

  HeRu::RuSpec ru2(ruType, (channelWidth == 160 ? 1 : 2),
                   (channelWidth != 160));
  txVector.SetRu(ru2, aid2);
  txVector.SetMode(HePhy::GetHeMcs7(), aid2);
  txVector.SetNss(1, aid2);

  uint16_t length;
  std::tie(length, m_expectedPpduDuration) =
      HePhy::ConvertHeTbPpduDurationToLSigLength(
          m_expectedPpduDuration, txVector, m_phyAp->GetPhyBand());
  if (error == UL_LENGTH) {
    ++length;
  }
  txVector.SetLength(length);
  auto hePhyAp = DynamicCast<HePhy>(m_phyAp->GetLatestPhyEntity());
  hePhyAp->SetTrigVector(txVector, m_expectedPpduDuration);
}

void TestUlOfdmaPhyTransmission::SendHeTbPpdu(uint16_t txStaId,
                                              std::size_t index,
                                              std::size_t payloadSize,
                                              uint64_t uid, uint8_t bssColor,
                                              bool incrementUid) {
  NS_LOG_FUNCTION(this << txStaId << index << payloadSize << uid << +bssColor
                       << (incrementUid));
  WifiConstPsduMap psdus;

  if (incrementUid) {
    ++uid;
  }

  WifiTxVector txVector = GetTxVectorForHeTbPpdu(txStaId, index, bssColor);
  Ptr<Packet> pkt = Create<Packet>(payloadSize);
  WifiMacHeader hdr;
  hdr.SetType(WIFI_MAC_QOSDATA);
  hdr.SetQosTid(0);
  hdr.SetAddr1(Mac48Address("00:00:00:00:00:00"));
  std::ostringstream addr;
  addr << "00:00:00:00:00:0" << txStaId;
  hdr.SetAddr2(Mac48Address(addr.str().c_str()));
  hdr.SetSequenceNumber(1);
  Ptr<WifiPsdu> psdu = Create<WifiPsdu>(pkt, hdr);
  psdus.insert(std::make_pair(txStaId, psdu));

  Ptr<OfdmaSpectrumWifiPhy> phy;
  if (txStaId == 1) {
    phy = m_phySta1;
  } else if (txStaId == 2) {
    phy = m_phySta2;
  } else if (txStaId == 3) {
    phy = m_phySta3;
  }

  Time txDuration = phy->CalculateTxDuration(psdu->GetSize(), txVector,
                                             phy->GetPhyBand(), txStaId);
  txVector.SetLength(HePhy::ConvertHeTbPpduDurationToLSigLength(
                         txDuration, txVector, phy->GetPhyBand())
                         .first);

  phy->SetPpduUid(uid);
  phy->Send(psdus, txVector);
}

void TestUlOfdmaPhyTransmission::GenerateInterference(
    Ptr<SpectrumValue> interferencePsd, Time duration) {
  NS_LOG_FUNCTION(this << duration);
  m_phyInterferer->SetTxPowerSpectralDensity(interferencePsd);
  m_phyInterferer->SetPeriod(duration);
  m_phyInterferer->Start();
  Simulator::Schedule(duration, &TestUlOfdmaPhyTransmission::StopInterference,
                      this);
}

void TestUlOfdmaPhyTransmission::StopInterference() { m_phyInterferer->Stop(); }

TestUlOfdmaPhyTransmission::~TestUlOfdmaPhyTransmission() {}

void TestUlOfdmaPhyTransmission::RxSuccess(Ptr<const WifiPsdu> psdu,
                                           RxSignalInfo rxSignalInfo,
                                           WifiTxVector txVector,
                                           std::vector<bool>) {
  NS_LOG_FUNCTION(this << *psdu << psdu->GetAddr2() << rxSignalInfo
                       << txVector);
  if (psdu->GetAddr2() == Mac48Address("00:00:00:00:00:01")) {
    m_countRxSuccessFromSta1++;
    m_countRxBytesFromSta1 += (psdu->GetSize() - 30);
  } else if (psdu->GetAddr2() == Mac48Address("00:00:00:00:00:02")) {
    m_countRxSuccessFromSta2++;
    m_countRxBytesFromSta2 += (psdu->GetSize() - 30);
  }
}

void TestUlOfdmaPhyTransmission::RxFailure(Ptr<const WifiPsdu> psdu) {
  NS_LOG_FUNCTION(this << *psdu << psdu->GetAddr2());
  if (psdu->GetAddr2() == Mac48Address("00:00:00:00:00:01")) {
    m_countRxFailureFromSta1++;
  } else if (psdu->GetAddr2() == Mac48Address("00:00:00:00:00:02")) {
    m_countRxFailureFromSta2++;
  }
}

void TestUlOfdmaPhyTransmission::CheckRxFromSta1(uint32_t expectedSuccess,
                                                 uint32_t expectedFailures,
                                                 uint32_t expectedBytes) {
  NS_TEST_ASSERT_MSG_EQ(
      m_countRxSuccessFromSta1, expectedSuccess,
      "The number of successfully received packets from STA 1 is not correct!");
  NS_TEST_ASSERT_MSG_EQ(m_countRxFailureFromSta1, expectedFailures,
                        "The number of unsuccessfuly received packets from STA "
                        "1 is not correct!");
  NS_TEST_ASSERT_MSG_EQ(
      m_countRxBytesFromSta1, expectedBytes,
      "The number of bytes received from STA 1 is not correct!");
}

void TestUlOfdmaPhyTransmission::CheckRxFromSta2(uint32_t expectedSuccess,
                                                 uint32_t expectedFailures,
                                                 uint32_t expectedBytes) {
  NS_TEST_ASSERT_MSG_EQ(
      m_countRxSuccessFromSta2, expectedSuccess,
      "The number of successfully received packets from STA 2 is not correct!");
  NS_TEST_ASSERT_MSG_EQ(m_countRxFailureFromSta2, expectedFailures,
                        "The number of unsuccessfuly received packets from STA "
                        "2 is not correct!");
  NS_TEST_ASSERT_MSG_EQ(
      m_countRxBytesFromSta2, expectedBytes,
      "The number of bytes received from STA 2 is not correct!");
}

void TestUlOfdmaPhyTransmission::CheckNonOfdmaRxPower(
    Ptr<OfdmaSpectrumWifiPhy> phy, WifiSpectrumBandInfo band,
    double expectedRxPower) {
  Ptr<Event> event = phy->GetCurrentEvent();
  NS_ASSERT(event);
  auto rxPower = event->GetRxPowerW(band);
  NS_LOG_FUNCTION(this << band << expectedRxPower << rxPower);
  NS_TEST_ASSERT_MSG_EQ_TOL(rxPower, expectedRxPower, 5e-3,
                            "RX power " << rxPower << " over (" << band
                                        << ") does not match expected power "
                                        << expectedRxPower << " at "
                                        << Simulator::Now());
}

void TestUlOfdmaPhyTransmission::CheckOfdmaRxPower(
    Ptr<OfdmaSpectrumWifiPhy> phy, WifiSpectrumBandInfo band,
    double expectedRxPower) {
  NS_LOG_FUNCTION(this << band << expectedRxPower);
  double step = 5e-3;
  if (expectedRxPower > 0.0) {
    NS_TEST_ASSERT_MSG_EQ(phy->GetEnergyDuration(expectedRxPower - step, band)
                              .IsStrictlyPositive(),
                          true,
                          "At least " << expectedRxPower
                                      << " W expected for OFDMA part over ("
                                      << band << ") at " << Simulator::Now());
    NS_TEST_ASSERT_MSG_EQ(phy->GetEnergyDuration(expectedRxPower + step, band)
                              .IsStrictlyPositive(),
                          false,
                          "At most " << expectedRxPower
                                     << " W expected for OFDMA part over ("
                                     << band << ") at " << Simulator::Now());
  } else {
    NS_TEST_ASSERT_MSG_EQ(phy->GetEnergyDuration(expectedRxPower + step, band)
                              .IsStrictlyPositive(),
                          false,
                          "At most " << expectedRxPower
                                     << " W expected for OFDMA part over ("
                                     << band << ") at " << Simulator::Now());
  }
}

void TestUlOfdmaPhyTransmission::VerifyEventsCleared() {
  NS_TEST_ASSERT_MSG_EQ(m_phyAp->GetCurrentEvent(), nullptr,
                        "m_currentEvent for AP was not cleared");
  NS_TEST_ASSERT_MSG_EQ(m_phySta1->GetCurrentEvent(), nullptr,
                        "m_currentEvent for STA 1 was not cleared");
  NS_TEST_ASSERT_MSG_EQ(m_phySta2->GetCurrentEvent(), nullptr,
                        "m_currentEvent for STA 2 was not cleared");
}

void TestUlOfdmaPhyTransmission::CheckPhyState(Ptr<OfdmaSpectrumWifiPhy> phy,
                                               WifiPhyState expectedState) {
  Simulator::ScheduleNow(&TestUlOfdmaPhyTransmission::DoCheckPhyState, this,
                         phy, expectedState);
}

void TestUlOfdmaPhyTransmission::DoCheckPhyState(Ptr<OfdmaSpectrumWifiPhy> phy,
                                                 WifiPhyState expectedState) {
  WifiPhyState currentState;
  PointerValue ptr;
  phy->GetAttribute("State", ptr);
  Ptr<WifiPhyStateHelper> state =
      DynamicCast<WifiPhyStateHelper>(ptr.Get<WifiPhyStateHelper>());
  currentState = state->GetState();
  NS_LOG_FUNCTION(this << currentState);
  NS_TEST_ASSERT_MSG_EQ(currentState, expectedState,
                        "PHY State "
                            << currentState << " does not match expected state "
                            << expectedState << " at " << Simulator::Now());
}

void TestUlOfdmaPhyTransmission::CheckApRxStart(uint32_t expectedNotifications,
                                                Time expectedLastNotification) {
  NS_TEST_ASSERT_MSG_EQ(
      m_apPhyStateListener->GetNumRxStartNotifications(), expectedNotifications,
      "Number of RX start notifications "
          << m_apPhyStateListener->GetNumRxStartNotifications()
          << " does not match expected count " << expectedNotifications
          << " for AP at " << Simulator::Now());
  NS_TEST_ASSERT_MSG_EQ(
      m_apPhyStateListener->GetLastRxStartNotification(),
      expectedLastNotification,
      "Last time RX start notification has been received "
          << m_apPhyStateListener->GetLastRxStartNotification()
          << " does not match expected time " << expectedLastNotification
          << " for AP at " << Simulator::Now());
}

void TestUlOfdmaPhyTransmission::CheckApRxEnd(uint32_t expectedNotifications,
                                              Time expectedLastNotification,
                                              bool expectedSuccess) {
  NS_TEST_ASSERT_MSG_EQ(
      m_apPhyStateListener->GetNumRxEndNotifications(), expectedNotifications,
      "Number of RX end notifications "
          << m_apPhyStateListener->GetNumRxEndNotifications()
          << " does not match expected count " << expectedNotifications
          << " for AP at " << Simulator::Now());
  NS_TEST_ASSERT_MSG_EQ(m_apPhyStateListener->GetLastRxEndNotification(),
                        expectedLastNotification,
                        "Last time RX end notification has been received "
                            << m_apPhyStateListener->GetLastRxEndNotification()
                            << " does not match expected time "
                            << expectedLastNotification << " for AP at "
                            << Simulator::Now());
  NS_TEST_ASSERT_MSG_EQ(
      m_apPhyStateListener->IsLastRxSuccess(), expectedSuccess,
      "Last time RX end notification indicated a "
          << (m_apPhyStateListener->IsLastRxSuccess() ? "success" : "failure")
          << " but expected a " << (expectedSuccess ? "success" : "failure")
          << " for AP at " << Simulator::Now());
}

void TestUlOfdmaPhyTransmission::Reset() {
  m_countRxSuccessFromSta1 = 0;
  m_countRxSuccessFromSta2 = 0;
  m_countRxFailureFromSta1 = 0;
  m_countRxFailureFromSta2 = 0;
  m_countRxBytesFromSta1 = 0;
  m_countRxBytesFromSta2 = 0;
  m_phySta1->SetPpduUid(0);
  m_phySta1->SetTriggerFrameUid(0);
  m_phySta2->SetTriggerFrameUid(0);
  SetBssColor(m_phyAp, 0);
  m_apPhyStateListener->Reset();
}

void TestUlOfdmaPhyTransmission::SetBssColor(Ptr<WifiPhy> phy,
                                             uint8_t bssColor) {
  Ptr<WifiNetDevice> device = DynamicCast<WifiNetDevice>(phy->GetDevice());
  Ptr<HeConfiguration> heConfiguration = device->GetHeConfiguration();
  heConfiguration->SetAttribute("BssColor", UintegerValue(bssColor));
}

void TestUlOfdmaPhyTransmission::SetPsdLimit(Ptr<WifiPhy> phy,
                                             double psdLimit) {
  NS_LOG_FUNCTION(this << phy << psdLimit);
  phy->SetAttribute("PowerDensityLimit", DoubleValue(psdLimit));
}

void TestUlOfdmaPhyTransmission::DoSetup() {
  Ptr<MultiModelSpectrumChannel> spectrumChannel =
      CreateObject<MultiModelSpectrumChannel>();
  Ptr<FriisPropagationLossModel> lossModel =
      CreateObject<FriisPropagationLossModel>();
  lossModel->SetFrequency(m_frequency);
  spectrumChannel->AddPropagationLossModel(lossModel);
  Ptr<ConstantSpeedPropagationDelayModel> delayModel =
      CreateObject<ConstantSpeedPropagationDelayModel>();
  spectrumChannel->SetPropagationDelayModel(delayModel);

  Ptr<ThresholdPreambleDetectionModel> preambleDetectionModel =
      CreateObject<ThresholdPreambleDetectionModel>();
  preambleDetectionModel->SetAttribute("MinimumRssi", DoubleValue(-8));
  preambleDetectionModel->SetAttribute("Threshold", DoubleValue(-100));

  Ptr<Node> apNode = CreateObject<Node>();
  Ptr<WifiNetDevice> apDev = CreateObject<WifiNetDevice>();
  apDev->SetStandard(WIFI_STANDARD_80211ax);
  Ptr<ApWifiMac> apMac = CreateObject<ApWifiMac>();
  apMac->SetAttribute("BeaconGeneration", BooleanValue(false));
  apDev->SetMac(apMac);
  m_phyAp = CreateObject<OfdmaSpectrumWifiPhy>(0);
  Ptr<HeConfiguration> heConfiguration = CreateObject<HeConfiguration>();
  apDev->SetHeConfiguration(heConfiguration);
  Ptr<InterferenceHelper> apInterferenceHelper =
      CreateObject<InterferenceHelper>();
  m_phyAp->SetInterferenceHelper(apInterferenceHelper);
  Ptr<ErrorRateModel> apErrorModel = CreateObject<NistErrorRateModel>();
  m_phyAp->SetErrorRateModel(apErrorModel);
  m_phyAp->SetDevice(apDev);
  m_phyAp->AddChannel(spectrumChannel);
  m_phyAp->ConfigureStandard(WIFI_STANDARD_80211ax);
  m_phyAp->SetReceiveOkCallback(
      MakeCallback(&TestUlOfdmaPhyTransmission::RxSuccess, this));
  m_phyAp->SetReceiveErrorCallback(
      MakeCallback(&TestUlOfdmaPhyTransmission::RxFailure, this));
  m_phyAp->SetPreambleDetectionModel(preambleDetectionModel);
  Ptr<ConstantPositionMobilityModel> apMobility =
      CreateObject<ConstantPositionMobilityModel>();
  m_phyAp->SetMobility(apMobility);
  m_apPhyStateListener = std::make_unique<OfdmaTestPhyListener>();
  m_phyAp->RegisterListener(m_apPhyStateListener.get());
  apDev->SetPhy(m_phyAp);
  apMac->SetWifiPhys({m_phyAp});
  apNode->AggregateObject(apMobility);
  apNode->AddDevice(apDev);

  Ptr<Node> sta1Node = CreateObject<Node>();
  Ptr<WifiNetDevice> sta1Dev = CreateObject<WifiNetDevice>();
  sta1Dev->SetStandard(WIFI_STANDARD_80211ax);
  sta1Dev->SetHeConfiguration(CreateObject<HeConfiguration>());
  m_phySta1 = CreateObject<OfdmaSpectrumWifiPhy>(1);
  Ptr<InterferenceHelper> sta1InterferenceHelper =
      CreateObject<InterferenceHelper>();
  m_phySta1->SetInterferenceHelper(sta1InterferenceHelper);
  Ptr<ErrorRateModel> sta1ErrorModel = CreateObject<NistErrorRateModel>();
  m_phySta1->SetErrorRateModel(sta1ErrorModel);
  m_phySta1->SetDevice(sta1Dev);
  m_phySta1->AddChannel(spectrumChannel);
  m_phySta1->ConfigureStandard(WIFI_STANDARD_80211ax);
  m_phySta1->SetPreambleDetectionModel(preambleDetectionModel);
  Ptr<ConstantPositionMobilityModel> sta1Mobility =
      CreateObject<ConstantPositionMobilityModel>();
  m_phySta1->SetMobility(sta1Mobility);
  sta1Dev->SetPhy(m_phySta1);
  sta1Node->AggregateObject(sta1Mobility);
  sta1Node->AddDevice(sta1Dev);

  Ptr<Node> sta2Node = CreateObject<Node>();
  Ptr<WifiNetDevice> sta2Dev = CreateObject<WifiNetDevice>();
  sta2Dev->SetStandard(WIFI_STANDARD_80211ax);
  sta2Dev->SetHeConfiguration(CreateObject<HeConfiguration>());
  m_phySta2 = CreateObject<OfdmaSpectrumWifiPhy>(2);
  Ptr<InterferenceHelper> sta2InterferenceHelper =
      CreateObject<InterferenceHelper>();
  m_phySta2->SetInterferenceHelper(sta2InterferenceHelper);
  Ptr<ErrorRateModel> sta2ErrorModel = CreateObject<NistErrorRateModel>();
  m_phySta2->SetErrorRateModel(sta2ErrorModel);
  m_phySta2->SetDevice(sta2Dev);
  m_phySta2->AddChannel(spectrumChannel);
  m_phySta2->ConfigureStandard(WIFI_STANDARD_80211ax);
  m_phySta2->SetPreambleDetectionModel(preambleDetectionModel);
  Ptr<ConstantPositionMobilityModel> sta2Mobility =
      CreateObject<ConstantPositionMobilityModel>();
  m_phySta2->SetMobility(sta2Mobility);
  sta2Dev->SetPhy(m_phySta2);
  sta2Node->AggregateObject(sta2Mobility);
  sta2Node->AddDevice(sta2Dev);

  Ptr<Node> sta3Node = CreateObject<Node>();
  Ptr<WifiNetDevice> sta3Dev = CreateObject<WifiNetDevice>();
  sta3Dev->SetStandard(WIFI_STANDARD_80211ax);
  sta3Dev->SetHeConfiguration(CreateObject<HeConfiguration>());
  m_phySta3 = CreateObject<OfdmaSpectrumWifiPhy>(3);
  Ptr<InterferenceHelper> sta3InterferenceHelper =
      CreateObject<InterferenceHelper>();
  m_phySta3->SetInterferenceHelper(sta3InterferenceHelper);
  Ptr<ErrorRateModel> sta3ErrorModel = CreateObject<NistErrorRateModel>();
  m_phySta3->SetErrorRateModel(sta3ErrorModel);
  m_phySta3->SetDevice(sta3Dev);
  m_phySta3->AddChannel(spectrumChannel);
  m_phySta3->ConfigureStandard(WIFI_STANDARD_80211ax);
  m_phySta3->SetPreambleDetectionModel(preambleDetectionModel);
  Ptr<ConstantPositionMobilityModel> sta3Mobility =
      CreateObject<ConstantPositionMobilityModel>();
  m_phySta3->SetMobility(sta3Mobility);
  sta3Dev->SetPhy(m_phySta3);
  sta3Node->AggregateObject(sta3Mobility);
  sta3Node->AddDevice(sta3Dev);

  Ptr<Node> interfererNode = CreateObject<Node>();
  Ptr<NonCommunicatingNetDevice> interfererDev =
      CreateObject<NonCommunicatingNetDevice>();
  m_phyInterferer = CreateObject<WaveformGenerator>();
  m_phyInterferer->SetDevice(interfererDev);
  m_phyInterferer->SetChannel(spectrumChannel);
  m_phyInterferer->SetDutyCycle(1);
  interfererNode->AddDevice(interfererDev);

  std::list<Ptr<WifiPhy>> phys{m_phyAp, m_phySta1, m_phySta2, m_phySta3};
  for (auto &phy : phys) {
    phy->SetAttribute("TxGain", DoubleValue(1.0));
    phy->SetAttribute("TxPowerStart", DoubleValue(16.0));
    phy->SetAttribute("TxPowerEnd", DoubleValue(16.0));
    phy->SetAttribute("PowerDensityLimit", DoubleValue(100.0));
    phy->SetAttribute("RxGain", DoubleValue(2.0));
    phy->SetAttribute("TxMaskInnerBandMinimumRejection", DoubleValue(-100.0));
    phy->SetAttribute("TxMaskOuterBandMinimumRejection", DoubleValue(-100.0));
    phy->SetAttribute("TxMaskOuterBandMaximumRejection", DoubleValue(-100.0));
  }
}

void TestUlOfdmaPhyTransmission::DoTeardown() {
  m_phyAp->Dispose();
  m_phyAp = nullptr;
  m_phySta1->Dispose();
  m_phySta1 = nullptr;
  m_phySta2->Dispose();
  m_phySta2 = nullptr;
  m_phySta3->Dispose();
  m_phySta3 = nullptr;
  m_phyInterferer->Dispose();
  m_phyInterferer = nullptr;
}

void TestUlOfdmaPhyTransmission::LogScenario(std::string log) const {
  NS_LOG_INFO(log);
}

void TestUlOfdmaPhyTransmission::ScheduleTest(
    Time delay, bool solicited, WifiPhyState expectedStateAtEnd,
    uint32_t expectedSuccessFromSta1, uint32_t expectedFailuresFromSta1,
    uint32_t expectedBytesFromSta1, uint32_t expectedSuccessFromSta2,
    uint32_t expectedFailuresFromSta2, uint32_t expectedBytesFromSta2,
    bool scheduleTxSta1, Time ulTimeDifference,
    WifiPhyState expectedStateBeforeEnd, TrigVectorInfo error) {
  static uint64_t uid = 0;

  Simulator::Schedule(delay - MilliSeconds(10),
                      &TestUlOfdmaPhyTransmission::SendHeSuPpdu, this, 0, 50,
                      ++uid, 0);
  if (!solicited) {
    ++uid;
  } else {
    Simulator::Schedule(delay, &TestUlOfdmaPhyTransmission::SetTrigVector, this,
                        0, error);
  }
  Simulator::Schedule(delay - MilliSeconds(1), &OfdmaTestPhyListener::Reset,
                      m_apPhyStateListener.get());
  if (scheduleTxSta1) {
    Simulator::Schedule(delay, &TestUlOfdmaPhyTransmission::SendHeTbPpdu, this,
                        1, 1, 1000, uid, 0, false);
  }
  Simulator::Schedule(delay + ulTimeDifference,
                      &TestUlOfdmaPhyTransmission::SendHeTbPpdu, this, 2, 2,
                      1001, uid, 0, false);

  Simulator::Schedule(delay + m_expectedPpduDuration - NanoSeconds(1),
                      &TestUlOfdmaPhyTransmission::CheckPhyState, this, m_phyAp,
                      expectedStateBeforeEnd);
  Simulator::Schedule(delay + m_expectedPpduDuration + ulTimeDifference,
                      &TestUlOfdmaPhyTransmission::CheckPhyState, this, m_phyAp,
                      expectedStateAtEnd);

  if (expectedSuccessFromSta1 + expectedFailuresFromSta1 +
          expectedSuccessFromSta2 + expectedFailuresFromSta2 >
      0) {
    const bool isSuccess =
        (expectedSuccessFromSta1 > 0) || (expectedSuccessFromSta2 > 0);
    const Time expectedPayloadStart = delay + MicroSeconds(48);
    const Time expectedPayloadEnd =
        delay + m_expectedPpduDuration + ulTimeDifference;
    Simulator::Schedule(expectedPayloadEnd,
                        &TestUlOfdmaPhyTransmission::CheckApRxStart, this, 1,
                        Simulator::Now() + expectedPayloadStart);
    Simulator::Schedule(expectedPayloadEnd + NanoSeconds(1),
                        &TestUlOfdmaPhyTransmission::CheckApRxEnd, this, 1,
                        Simulator::Now() + expectedPayloadEnd, isSuccess);
  }

  delay += MilliSeconds(100);
  Simulator::Schedule(delay, &TestUlOfdmaPhyTransmission::CheckRxFromSta1, this,
                      expectedSuccessFromSta1, expectedFailuresFromSta1,
                      expectedBytesFromSta1);
  Simulator::Schedule(delay, &TestUlOfdmaPhyTransmission::CheckRxFromSta2, this,
                      expectedSuccessFromSta2, expectedFailuresFromSta2,
                      expectedBytesFromSta2);
  Simulator::Schedule(delay, &TestUlOfdmaPhyTransmission::VerifyEventsCleared,
                      this);

  delay += MilliSeconds(100);
  Simulator::Schedule(delay, &TestUlOfdmaPhyTransmission::Reset, this);
}

void TestUlOfdmaPhyTransmission::SchedulePowerMeasurementChecks(
    Time delay, double rxPowerNonOfdmaRu1, double rxPowerNonOfdmaRu2,
    double rxPowerOfdmaRu1, double rxPowerOfdmaRu2) {
  Time detectionDuration = WifiPhy::GetPreambleDetectionDuration();
  WifiTxVector txVectorSta1 = GetTxVectorForHeTbPpdu(1, 1, 0);
  WifiTxVector txVectorSta2 = GetTxVectorForHeTbPpdu(2, 2, 0);
  Ptr<const HePhy> hePhy = m_phyAp->GetHePhy();
  Time nonOfdmaDuration = hePhy->CalculateNonHeDurationForHeTb(txVectorSta2);
  NS_ASSERT(nonOfdmaDuration ==
            hePhy->CalculateNonHeDurationForHeTb(txVectorSta1));

  std::vector<double> rxPowerNonOfdma{rxPowerNonOfdmaRu1, rxPowerNonOfdmaRu2};
  std::vector<WifiSpectrumBandInfo> nonOfdmaBand{
      hePhy->GetNonOfdmaBand(txVectorSta1, 1),
      hePhy->GetNonOfdmaBand(txVectorSta2, 2)};
  std::vector<double> rxPowerOfdma{rxPowerOfdmaRu1, rxPowerOfdmaRu2};
  std::vector<WifiSpectrumBandInfo> ofdmaBand{
      hePhy->GetRuBandForRx(txVectorSta1, 1),
      hePhy->GetRuBandForRx(txVectorSta2, 2)};

  for (uint8_t i = 0; i < 2; ++i) {
    Simulator::Schedule(delay + detectionDuration + NanoSeconds(1),
                        &TestUlOfdmaPhyTransmission::CheckNonOfdmaRxPower, this,
                        m_phyAp, nonOfdmaBand[i], rxPowerNonOfdma[i]);
    Simulator::Schedule(delay + nonOfdmaDuration - NanoSeconds(1),
                        &TestUlOfdmaPhyTransmission::CheckNonOfdmaRxPower, this,
                        m_phyAp, nonOfdmaBand[i], rxPowerNonOfdma[i]);
    Simulator::Schedule(delay + nonOfdmaDuration + NanoSeconds(1),
                        &TestUlOfdmaPhyTransmission::CheckOfdmaRxPower, this,
                        m_phyAp, ofdmaBand[i], rxPowerOfdma[i]);
    Simulator::Schedule(delay + m_expectedPpduDuration - NanoSeconds(1),
                        &TestUlOfdmaPhyTransmission::CheckOfdmaRxPower, this,
                        m_phyAp, ofdmaBand[i], rxPowerOfdma[i]);

    Simulator::Schedule(delay + detectionDuration + NanoSeconds(1),
                        &TestUlOfdmaPhyTransmission::CheckOfdmaRxPower, this,
                        m_phySta3, nonOfdmaBand[i], rxPowerNonOfdma[i]);
    Simulator::Schedule(delay + nonOfdmaDuration - NanoSeconds(1),
                        &TestUlOfdmaPhyTransmission::CheckOfdmaRxPower, this,
                        m_phySta3, nonOfdmaBand[i], rxPowerNonOfdma[i]);
    Simulator::Schedule(delay + nonOfdmaDuration + NanoSeconds(1),
                        &TestUlOfdmaPhyTransmission::CheckOfdmaRxPower, this,
                        m_phySta3, ofdmaBand[i], rxPowerOfdma[i]);
    Simulator::Schedule(delay + m_expectedPpduDuration - NanoSeconds(1),
                        &TestUlOfdmaPhyTransmission::CheckOfdmaRxPower, this,
                        m_phySta3, ofdmaBand[i], rxPowerOfdma[i]);
  }

  if (rxPowerOfdmaRu1 != 0.0) {
    double rxPowerNonOfdmaSta1Only =
        (m_channelWidth >= 40) ? rxPowerNonOfdma[0] : rxPowerNonOfdma[0] / 2;
    Simulator::Schedule(delay + detectionDuration + NanoSeconds(1),
                        &TestUlOfdmaPhyTransmission::CheckOfdmaRxPower, this,
                        m_phySta2, nonOfdmaBand[0], rxPowerNonOfdmaSta1Only);
    Simulator::Schedule(delay + nonOfdmaDuration - NanoSeconds(1),
                        &TestUlOfdmaPhyTransmission::CheckOfdmaRxPower, this,
                        m_phySta2, nonOfdmaBand[0], rxPowerNonOfdmaSta1Only);
    Simulator::Schedule(delay + nonOfdmaDuration + NanoSeconds(1),
                        &TestUlOfdmaPhyTransmission::CheckOfdmaRxPower, this,
                        m_phySta2, ofdmaBand[0], rxPowerOfdma[0]);
    Simulator::Schedule(delay + m_expectedPpduDuration - NanoSeconds(1),
                        &TestUlOfdmaPhyTransmission::CheckOfdmaRxPower, this,
                        m_phySta2, ofdmaBand[0], rxPowerOfdma[0]);
  }
}

void TestUlOfdmaPhyTransmission::RunOne() {
  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(1);
  int64_t streamNumber = 0;
  m_phyAp->AssignStreams(streamNumber);
  m_phySta1->AssignStreams(streamNumber);
  m_phySta2->AssignStreams(streamNumber);
  m_phySta3->AssignStreams(streamNumber);

  auto channelNum = std::get<0>(*WifiPhyOperatingChannel::FindFirst(
      0, m_frequency, m_channelWidth, WIFI_STANDARD_80211ax,
      WIFI_PHY_BAND_5GHZ));

  m_phyAp->SetOperatingChannel(WifiPhy::ChannelTuple{
      channelNum, m_channelWidth, (int)(WIFI_PHY_BAND_5GHZ), 0});
  m_phySta1->SetOperatingChannel(WifiPhy::ChannelTuple{
      channelNum, m_channelWidth, (int)(WIFI_PHY_BAND_5GHZ), 0});
  m_phySta2->SetOperatingChannel(WifiPhy::ChannelTuple{
      channelNum, m_channelWidth, (int)(WIFI_PHY_BAND_5GHZ), 0});
  m_phySta3->SetOperatingChannel(WifiPhy::ChannelTuple{
      channelNum, m_channelWidth, (int)(WIFI_PHY_BAND_5GHZ), 0});

  Time delay = Seconds(0.0);
  Simulator::Schedule(delay, &TestUlOfdmaPhyTransmission::Reset, this);
  delay += Seconds(1.0);

  Simulator::Schedule(delay, &TestUlOfdmaPhyTransmission::LogScenario, this,
                      "Reception of solicited HE TB PPDUs");
  ScheduleTest(delay, true, WifiPhyState::IDLE, 1, 0, 1000, 1, 0, 1001);
  delay += Seconds(1.0);

  Simulator::Schedule(delay, &TestUlOfdmaPhyTransmission::LogScenario, this,
                      "Reception of solicited HE TB PPDUs with delay (< 400ns) "
                      "between the two signals");
  ScheduleTest(delay, true, WifiPhyState::IDLE, 1, 0, 1000, 1, 0, 1001, true,
               NanoSeconds(100));
  delay += Seconds(1.0);

  Simulator::Schedule(delay, &TestUlOfdmaPhyTransmission::LogScenario, this,
                      "Dropping of unsolicited HE TB PPDUs");
  ScheduleTest(delay, false, WifiPhyState::IDLE, 0, 0, 0, 0, 0, 0, true,
               Seconds(0), WifiPhyState::CCA_BUSY);
  delay += Seconds(1.0);

  Simulator::Schedule(
      delay, &TestUlOfdmaPhyTransmission::LogScenario, this,
      "Dropping of HE TB PPDUs with channel width differing from TRIGVECTOR");
  ScheduleTest(delay, true, WifiPhyState::IDLE, 0, 0, 0, 0, 0, 0, true,
               Seconds(0), WifiPhyState::CCA_BUSY, CHANNEL_WIDTH);
  delay += Seconds(1.0);

  Simulator::Schedule(
      delay, &TestUlOfdmaPhyTransmission::LogScenario, this,
      "Dropping of HE TB PPDUs with UL Length differing from TRIGVECTOR");
  ScheduleTest(delay, true, WifiPhyState::IDLE, 0, 0, 0, 0, 0, 0, true,
               Seconds(0), WifiPhyState::CCA_BUSY, UL_LENGTH);
  delay += Seconds(1.0);

  Simulator::Schedule(
      delay, &TestUlOfdmaPhyTransmission::LogScenario, this,
      "Dropping of HE TB PPDUs with AIDs differing from TRIGVECTOR");
  ScheduleTest(delay, true, WifiPhyState::IDLE, 0, 0, 0, 0, 0, 0, true,
               Seconds(0), WifiPhyState::CCA_BUSY, AID);
  delay += Seconds(1.0);

  Simulator::Schedule(delay, &TestUlOfdmaPhyTransmission::LogScenario, this,
                      "Reception of solicited HE TB PPDUs with interference on "
                      "RU 1 during PSDU reception");
  BandInfo bandInfo;
  bandInfo.fc = (m_frequency - (m_channelWidth / 4)) * 1e6;
  bandInfo.fl = bandInfo.fc - ((m_channelWidth / 4) * 1e6);
  bandInfo.fh = bandInfo.fc + ((m_channelWidth / 4) * 1e6);
  Bands bands;
  bands.push_back(bandInfo);

  Ptr<SpectrumModel> SpectrumInterferenceRu1 = Create<SpectrumModel>(bands);
  Ptr<SpectrumValue> interferencePsdRu1 =
      Create<SpectrumValue>(SpectrumInterferenceRu1);
  double interferencePower = 0.1;
  *interferencePsdRu1 = interferencePower / ((m_channelWidth / 2) * 20e6);

  Simulator::Schedule(delay + MicroSeconds(50),
                      &TestUlOfdmaPhyTransmission::GenerateInterference, this,
                      interferencePsdRu1, MilliSeconds(100));
  ScheduleTest(delay, true, WifiPhyState::CCA_BUSY, 0, 1, 0, 1, 0, 1001);
  delay += Seconds(1.0);

  Simulator::Schedule(delay, &TestUlOfdmaPhyTransmission::LogScenario, this,
                      "Reception of solicited HE TB PPDUs with interference on "
                      "RU 2 during PSDU reception");
  bandInfo.fc = (m_frequency + (m_channelWidth / 4)) * 1e6;
  bandInfo.fl = bandInfo.fc - ((m_channelWidth / 4) * 1e6);
  bandInfo.fh = bandInfo.fc + ((m_channelWidth / 4) * 1e6);
  bands.clear();
  bands.push_back(bandInfo);

  Ptr<SpectrumModel> SpectrumInterferenceRu2 = Create<SpectrumModel>(bands);
  Ptr<SpectrumValue> interferencePsdRu2 =
      Create<SpectrumValue>(SpectrumInterferenceRu2);
  *interferencePsdRu2 = interferencePower / ((m_channelWidth / 2) * 20e6);

  Simulator::Schedule(delay + MicroSeconds(50),
                      &TestUlOfdmaPhyTransmission::GenerateInterference, this,
                      interferencePsdRu2, MilliSeconds(100));
  ScheduleTest(delay, true,
               (m_channelWidth >= 40) ? WifiPhyState::IDLE
                                      : WifiPhyState::CCA_BUSY,
               1, 0, 1000, 0, 1, 0);
  delay += Seconds(1.0);

  Simulator::Schedule(
      delay, &TestUlOfdmaPhyTransmission::LogScenario, this,
      "Reception of solicited HE TB PPDUs with interference on the full band "
      "during PSDU reception");
  bandInfo.fc = m_frequency * 1e6;
  bandInfo.fl = bandInfo.fc - ((m_channelWidth / 2) * 1e6);
  bandInfo.fh = bandInfo.fc + ((m_channelWidth / 2) * 1e6);
  bands.clear();
  bands.push_back(bandInfo);

  Ptr<SpectrumModel> SpectrumInterferenceAll = Create<SpectrumModel>(bands);
  Ptr<SpectrumValue> interferencePsdAll =
      Create<SpectrumValue>(SpectrumInterferenceAll);
  *interferencePsdAll = interferencePower / (m_channelWidth * 20e6);

  Simulator::Schedule(delay + MicroSeconds(50),
                      &TestUlOfdmaPhyTransmission::GenerateInterference, this,
                      interferencePsdAll, MilliSeconds(100));
  ScheduleTest(delay, true, WifiPhyState::CCA_BUSY, 0, 1, 0, 0, 1, 0);
  delay += Seconds(1.0);

  Simulator::Schedule(delay, &TestUlOfdmaPhyTransmission::LogScenario, this,
                      "Reception of solicited HE TB PPDUs with another HE TB "
                      "PPDU arriving on RU "
                      "1 during PSDU reception");
  Simulator::Schedule(delay + MicroSeconds(50),
                      &TestUlOfdmaPhyTransmission::SendHeTbPpdu, this, 3, 1,
                      1002, 1, 0, false);
  uint32_t succ;
  uint32_t fail;
  uint32_t bytes;
  if (m_channelWidth > 20) {
    succ = 1;
    fail = 0;
    bytes = 1001;
  } else {
    succ = 0;
    fail = 1;
    bytes = 0;
  }
  ScheduleTest(delay, true, WifiPhyState::CCA_BUSY, 0, 1, 0, succ, fail, bytes);
  delay += Seconds(1.0);

  Simulator::Schedule(delay, &TestUlOfdmaPhyTransmission::LogScenario, this,
                      "Reception of solicited HE TB PPDUs with another HE TB "
                      "PPDU arriving on RU "
                      "2 during PSDU reception");
  Simulator::Schedule(delay + MicroSeconds(50),
                      &TestUlOfdmaPhyTransmission::SendHeTbPpdu, this, 3, 2,
                      1002, 1, 0, false);
  if (m_channelWidth > 20) {
    succ = 1;
    fail = 0;
    bytes = 1000;
  } else {
    succ = 0;
    fail = 1;
    bytes = 0;
  }
  ScheduleTest(delay, true,
               (m_channelWidth >= 40) ? WifiPhyState::IDLE
                                      : WifiPhyState::CCA_BUSY,
               succ, fail, bytes, 0, 1, 0);
  delay += Seconds(1.0);

  Simulator::Schedule(delay, &TestUlOfdmaPhyTransmission::LogScenario, this,
                      "Reception of solicited HE TB PPDUs with an HE SU PPDU "
                      "arriving during the 400 ns window");
  Simulator::Schedule(delay + NanoSeconds(300),
                      &TestUlOfdmaPhyTransmission::SendHeSuPpdu, this, 3, 1002,
                      1, 0);
  ScheduleTest(delay, true, WifiPhyState::IDLE, 0, 1, 0, 0, 1, 0);
  delay += Seconds(1.0);

  Simulator::Schedule(delay, &TestUlOfdmaPhyTransmission::LogScenario, this,
                      "Reception of solicited HE TB PPDU only on RU 2");
  Simulator::Schedule(
      delay + m_expectedPpduDuration - NanoSeconds(1),
      &TestUlOfdmaPhyTransmission::CheckPhyState, this, m_phySta3,
      (m_channelWidth >= 40) ? WifiPhyState::IDLE : WifiPhyState::CCA_BUSY);
  ScheduleTest(delay, true, WifiPhyState::IDLE, 0, 0, 0, 1, 0, 1001, false,
               Seconds(0), WifiPhyState::RX);
  delay += Seconds(1.0);

  Simulator::Schedule(delay, &TestUlOfdmaPhyTransmission::LogScenario, this,
                      "Measure power for reception of HE TB PPDU only on RU 2");
  double rxPower = DbmToW(19);
  SchedulePowerMeasurementChecks(delay, (m_channelWidth >= 40) ? 0.0 : rxPower,
                                 rxPower, 0.0, rxPower);
  ScheduleTest(delay, true, WifiPhyState::IDLE, 0, 0, 0, 1, 0, 1001, false,
               Seconds(0), WifiPhyState::RX);
  delay += Seconds(1.0);

  Simulator::Schedule(delay, &TestUlOfdmaPhyTransmission::LogScenario, this,
                      "Measure power for reception of HE TB PPDU only on RU 2 "
                      "with PSD limitation");
  Simulator::Schedule(delay - NanoSeconds(1),
                      &TestUlOfdmaPhyTransmission::SetPsdLimit, this, m_phySta2,
                      3.0);

  rxPower = (m_channelWidth > 40) ? DbmToW(19) : DbmToW(18.0103);
  double rxPowerOfdma = rxPower;
  if (m_channelWidth <= 40) {
    rxPowerOfdma = (m_channelWidth == 20) ? DbmToW(14.0309) : DbmToW(18.0103);
  }
  SchedulePowerMeasurementChecks(delay, (m_channelWidth >= 40) ? 0.0 : rxPower,
                                 rxPower, 0.0, rxPowerOfdma);

  Simulator::Schedule(delay + m_expectedPpduDuration,
                      &TestUlOfdmaPhyTransmission::SetPsdLimit, this, m_phySta2,
                      100.0);
  ScheduleTest(delay, true, WifiPhyState::IDLE, 0, 0, 0, 1, 0, 1001, false,
               Seconds(0), WifiPhyState::RX);
  delay += Seconds(1.0);

  Simulator::Schedule(delay, &TestUlOfdmaPhyTransmission::LogScenario, this,
                      "Measure power for reception of HE TB PPDU on both RUs");
  rxPower = DbmToW(19);
  double rxPowerNonOfdma = (m_channelWidth >= 40) ? rxPower : rxPower * 2;
  SchedulePowerMeasurementChecks(delay, rxPowerNonOfdma, rxPowerNonOfdma,
                                 rxPower, rxPower);
  ScheduleTest(delay, true, WifiPhyState::IDLE, 1, 0, 1000, 1, 0, 1001);
  delay += Seconds(1.0);

  Simulator::Schedule(delay, &TestUlOfdmaPhyTransmission::LogScenario, this,
                      "Reception of an HE TB PPDU from another BSS");
  Simulator::Schedule(delay, &TestUlOfdmaPhyTransmission::SetBssColor, this,
                      m_phyAp, 1);
  Simulator::Schedule(delay + MilliSeconds(100),
                      &TestUlOfdmaPhyTransmission::SendHeTbPpdu, this, 3, 1,
                      1002, 1, 2, false);

  Simulator::Schedule(delay + MilliSeconds(200),
                      &TestUlOfdmaPhyTransmission::VerifyEventsCleared, this);

  Simulator::Schedule(delay + MilliSeconds(500),
                      &TestUlOfdmaPhyTransmission::Reset, this);
  delay += Seconds(1.0);

  Simulator::Schedule(delay, &TestUlOfdmaPhyTransmission::LogScenario, this,
                      "Reception of solicited HE TB PPDUs with delay (< 400ns) "
                      "between the two signals and "
                      "reception of an HE TB PPDU from another BSS between the "
                      "ends of the two HE TB PPDUs");
  Simulator::Schedule(delay, &TestUlOfdmaPhyTransmission::SetBssColor, this,
                      m_phyAp, 1);
  Simulator::Schedule(delay + m_expectedPpduDuration + NanoSeconds(100),
                      &TestUlOfdmaPhyTransmission::SendHeTbPpdu, this, 3, 1,
                      1002, 1, 2, true);
  ScheduleTest(delay, true, WifiPhyState::CCA_BUSY, 1, 0, 1000, 1, 0, 1001,
               true, NanoSeconds(200));
  delay += Seconds(1.0);

  Simulator::Run();
}

void TestUlOfdmaPhyTransmission::DoRun() {
  m_frequency = 5180;
  m_channelWidth = 20;
  m_expectedPpduDuration = NanoSeconds(292800);
  NS_LOG_DEBUG("Run UL OFDMA PHY transmission test for " << m_channelWidth
                                                         << " MHz");
  RunOne();

  m_frequency = 5190;
  m_channelWidth = 40;
  m_expectedPpduDuration = NanoSeconds(163200);
  NS_LOG_DEBUG("Run UL OFDMA PHY transmission test for " << m_channelWidth
                                                         << " MHz");
  RunOne();

  m_frequency = 5210;
  m_channelWidth = 80;
  m_expectedPpduDuration = NanoSeconds(105600);
  NS_LOG_DEBUG("Run UL OFDMA PHY transmission test for " << m_channelWidth
                                                         << " MHz");
  RunOne();

  m_frequency = 5250;
  m_channelWidth = 160;
  m_expectedPpduDuration = NanoSeconds(76800);
  NS_LOG_DEBUG("Run UL OFDMA PHY transmission test for " << m_channelWidth
                                                         << " MHz");
  RunOne();

  Simulator::Destroy();
}

class TestPhyPaddingExclusion : public TestCase {
public:
  TestPhyPaddingExclusion();
  ~TestPhyPaddingExclusion() override;

private:
  void DoSetup() override;
  void DoTeardown() override;
  void DoRun() override;

  void SendHeTbPpdu(uint16_t txStaId, std::size_t index,
                    std::size_t payloadSize, Time txDuration);
  void SetTrigVector(Time ppduDuration);

  void GenerateInterference(Ptr<SpectrumValue> interferencePsd, Time duration);
  void StopInterference();

  void RunOne();

  void CheckRxFromSta1(uint32_t expectedSuccess, uint32_t expectedFailures,
                       uint32_t expectedBytes);

  void CheckRxFromSta2(uint32_t expectedSuccess, uint32_t expectedFailures,
                       uint32_t expectedBytes);

  void VerifyEventsCleared();

  void CheckPhyState(Ptr<OfdmaSpectrumWifiPhy> phy, WifiPhyState expectedState);
  void DoCheckPhyState(Ptr<OfdmaSpectrumWifiPhy> phy,
                       WifiPhyState expectedState);

  void Reset();

  void RxSuccess(Ptr<const WifiPsdu> psdu, RxSignalInfo rxSignalInfo,
                 WifiTxVector txVector, std::vector<bool> statusPerMpdu);

  void RxFailure(Ptr<const WifiPsdu> psdu);

  Ptr<OfdmaSpectrumWifiPhy> m_phyAp;
  Ptr<OfdmaSpectrumWifiPhy> m_phySta1;
  Ptr<OfdmaSpectrumWifiPhy> m_phySta2;

  Ptr<WaveformGenerator> m_phyInterferer;

  uint32_t m_countRxSuccessFromSta1;
  uint32_t m_countRxSuccessFromSta2;
  uint32_t m_countRxFailureFromSta1;
  uint32_t m_countRxFailureFromSta2;
  uint32_t m_countRxBytesFromSta1;
  uint32_t m_countRxBytesFromSta2;
};

TestPhyPaddingExclusion::TestPhyPaddingExclusion()
    : TestCase("PHY padding exclusion test"), m_countRxSuccessFromSta1(0),
      m_countRxSuccessFromSta2(0), m_countRxFailureFromSta1(0),
      m_countRxFailureFromSta2(0), m_countRxBytesFromSta1(0),
      m_countRxBytesFromSta2(0) {}

void TestPhyPaddingExclusion::SendHeTbPpdu(uint16_t txStaId, std::size_t index,
                                           std::size_t payloadSize,
                                           Time txDuration) {
  WifiConstPsduMap psdus;

  WifiTxVector txVector =
      WifiTxVector(HePhy::GetHeMcs7(), 0, WIFI_PREAMBLE_HE_TB, 1600, 1, 1, 0,
                   DEFAULT_CHANNEL_WIDTH, false, false, true);

  HeRu::RuSpec ru(HeRu::RU_106_TONE, index, false);
  txVector.SetRu(ru, txStaId);
  txVector.SetMode(HePhy::GetHeMcs7(), txStaId);
  txVector.SetNss(1, txStaId);

  Ptr<Packet> pkt = Create<Packet>(payloadSize);
  WifiMacHeader hdr;
  hdr.SetType(WIFI_MAC_QOSDATA);
  hdr.SetQosTid(0);
  hdr.SetAddr1(Mac48Address("00:00:00:00:00:00"));
  std::ostringstream addr;
  addr << "00:00:00:00:00:0" << txStaId;
  hdr.SetAddr2(Mac48Address(addr.str().c_str()));
  hdr.SetSequenceNumber(1);
  Ptr<WifiPsdu> psdu = Create<WifiPsdu>(pkt, hdr);
  psdus.insert(std::make_pair(txStaId, psdu));

  Ptr<OfdmaSpectrumWifiPhy> phy;
  if (txStaId == 1) {
    phy = m_phySta1;
  } else if (txStaId == 2) {
    phy = m_phySta2;
  }

  txVector.SetLength(HePhy::ConvertHeTbPpduDurationToLSigLength(
                         txDuration, txVector, phy->GetPhyBand())
                         .first);

  phy->SetPpduUid(0);
  phy->Send(psdus, txVector);
}

void TestPhyPaddingExclusion::GenerateInterference(
    Ptr<SpectrumValue> interferencePsd, Time duration) {
  m_phyInterferer->SetTxPowerSpectralDensity(interferencePsd);
  m_phyInterferer->SetPeriod(duration);
  m_phyInterferer->Start();
  Simulator::Schedule(duration, &TestPhyPaddingExclusion::StopInterference,
                      this);
}

void TestPhyPaddingExclusion::StopInterference() { m_phyInterferer->Stop(); }

TestPhyPaddingExclusion::~TestPhyPaddingExclusion() {}

void TestPhyPaddingExclusion::RxSuccess(Ptr<const WifiPsdu> psdu,
                                        RxSignalInfo rxSignalInfo,
                                        WifiTxVector txVector,
                                        std::vector<bool>) {
  NS_LOG_FUNCTION(this << *psdu << psdu->GetAddr2() << rxSignalInfo
                       << txVector);
  if (psdu->GetAddr2() == Mac48Address("00:00:00:00:00:01")) {
    m_countRxSuccessFromSta1++;
    m_countRxBytesFromSta1 += (psdu->GetSize() - 30);
  } else if (psdu->GetAddr2() == Mac48Address("00:00:00:00:00:02")) {
    m_countRxSuccessFromSta2++;
    m_countRxBytesFromSta2 += (psdu->GetSize() - 30);
  }
}

void TestPhyPaddingExclusion::RxFailure(Ptr<const WifiPsdu> psdu) {
  NS_LOG_FUNCTION(this << *psdu << psdu->GetAddr2());
  if (psdu->GetAddr2() == Mac48Address("00:00:00:00:00:01")) {
    m_countRxFailureFromSta1++;
  } else if (psdu->GetAddr2() == Mac48Address("00:00:00:00:00:02")) {
    m_countRxFailureFromSta2++;
  }
}

void TestPhyPaddingExclusion::CheckRxFromSta1(uint32_t expectedSuccess,
                                              uint32_t expectedFailures,
                                              uint32_t expectedBytes) {
  NS_TEST_ASSERT_MSG_EQ(
      m_countRxSuccessFromSta1, expectedSuccess,
      "The number of successfully received packets from STA 1 is not correct!");
  NS_TEST_ASSERT_MSG_EQ(m_countRxFailureFromSta1, expectedFailures,
                        "The number of unsuccessfuly received packets from STA "
                        "1 is not correct!");
  NS_TEST_ASSERT_MSG_EQ(
      m_countRxBytesFromSta1, expectedBytes,
      "The number of bytes received from STA 1 is not correct!");
}

void TestPhyPaddingExclusion::CheckRxFromSta2(uint32_t expectedSuccess,
                                              uint32_t expectedFailures,
                                              uint32_t expectedBytes) {
  NS_TEST_ASSERT_MSG_EQ(
      m_countRxSuccessFromSta2, expectedSuccess,
      "The number of successfully received packets from STA 2 is not correct!");
  NS_TEST_ASSERT_MSG_EQ(m_countRxFailureFromSta2, expectedFailures,
                        "The number of unsuccessfuly received packets from STA "
                        "2 is not correct!");
  NS_TEST_ASSERT_MSG_EQ(
      m_countRxBytesFromSta2, expectedBytes,
      "The number of bytes received from STA 2 is not correct!");
}

void TestPhyPaddingExclusion::VerifyEventsCleared() {
  NS_TEST_ASSERT_MSG_EQ(m_phyAp->GetCurrentEvent(), nullptr,
                        "m_currentEvent for AP was not cleared");
  NS_TEST_ASSERT_MSG_EQ(m_phySta1->GetCurrentEvent(), nullptr,
                        "m_currentEvent for STA 1 was not cleared");
  NS_TEST_ASSERT_MSG_EQ(m_phySta2->GetCurrentEvent(), nullptr,
                        "m_currentEvent for STA 2 was not cleared");
}

void TestPhyPaddingExclusion::CheckPhyState(Ptr<OfdmaSpectrumWifiPhy> phy,
                                            WifiPhyState expectedState) {
  Simulator::ScheduleNow(&TestPhyPaddingExclusion::DoCheckPhyState, this, phy,
                         expectedState);
}

void TestPhyPaddingExclusion::DoCheckPhyState(Ptr<OfdmaSpectrumWifiPhy> phy,
                                              WifiPhyState expectedState) {
  WifiPhyState currentState = phy->GetState()->GetState();
  NS_LOG_FUNCTION(this << currentState);
  NS_TEST_ASSERT_MSG_EQ(currentState, expectedState,
                        "PHY State "
                            << currentState << " does not match expected state "
                            << expectedState << " at " << Simulator::Now());
}

void TestPhyPaddingExclusion::Reset() {
  m_countRxSuccessFromSta1 = 0;
  m_countRxSuccessFromSta2 = 0;
  m_countRxFailureFromSta1 = 0;
  m_countRxFailureFromSta2 = 0;
  m_countRxBytesFromSta1 = 0;
  m_countRxBytesFromSta2 = 0;
  m_phySta1->SetPpduUid(0);
  m_phySta1->SetTriggerFrameUid(0);
  m_phySta2->SetTriggerFrameUid(0);
}

void TestPhyPaddingExclusion::DoSetup() {
  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(1);
  int64_t streamNumber = 0;

  Ptr<MultiModelSpectrumChannel> spectrumChannel =
      CreateObject<MultiModelSpectrumChannel>();
  Ptr<FriisPropagationLossModel> lossModel =
      CreateObject<FriisPropagationLossModel>();
  lossModel->SetFrequency(DEFAULT_FREQUENCY * 1e6);
  spectrumChannel->AddPropagationLossModel(lossModel);
  Ptr<ConstantSpeedPropagationDelayModel> delayModel =
      CreateObject<ConstantSpeedPropagationDelayModel>();
  spectrumChannel->SetPropagationDelayModel(delayModel);

  Ptr<Node> apNode = CreateObject<Node>();
  Ptr<WifiNetDevice> apDev = CreateObject<WifiNetDevice>();
  Ptr<ApWifiMac> apMac = CreateObject<ApWifiMac>();
  apMac->SetAttribute("BeaconGeneration", BooleanValue(false));
  apDev->SetMac(apMac);
  m_phyAp = CreateObject<OfdmaSpectrumWifiPhy>(0);
  Ptr<HeConfiguration> heConfiguration = CreateObject<HeConfiguration>();
  apDev->SetHeConfiguration(heConfiguration);
  Ptr<InterferenceHelper> apInterferenceHelper =
      CreateObject<InterferenceHelper>();
  m_phyAp->SetInterferenceHelper(apInterferenceHelper);
  Ptr<ErrorRateModel> apErrorModel = CreateObject<NistErrorRateModel>();
  m_phyAp->SetErrorRateModel(apErrorModel);
  m_phyAp->SetDevice(apDev);
  m_phyAp->AddChannel(spectrumChannel);
  m_phyAp->ConfigureStandard(WIFI_STANDARD_80211ax);
  m_phyAp->AssignStreams(streamNumber);
  auto channelNum = std::get<0>(*WifiPhyOperatingChannel::FindFirst(
      0, DEFAULT_FREQUENCY, DEFAULT_CHANNEL_WIDTH, WIFI_STANDARD_80211ax,
      WIFI_PHY_BAND_5GHZ));

  m_phyAp->SetOperatingChannel(WifiPhy::ChannelTuple{
      channelNum, DEFAULT_CHANNEL_WIDTH, (int)(WIFI_PHY_BAND_5GHZ), 0});
  m_phyAp->SetReceiveOkCallback(
      MakeCallback(&TestPhyPaddingExclusion::RxSuccess, this));
  m_phyAp->SetReceiveErrorCallback(
      MakeCallback(&TestPhyPaddingExclusion::RxFailure, this));
  Ptr<ConstantPositionMobilityModel> apMobility =
      CreateObject<ConstantPositionMobilityModel>();
  m_phyAp->SetMobility(apMobility);
  apDev->SetPhy(m_phyAp);
  apDev->SetStandard(WIFI_STANDARD_80211ax);
  apDev->SetHeConfiguration(CreateObject<HeConfiguration>());
  apMac->SetWifiPhys({m_phyAp});
  apNode->AggregateObject(apMobility);
  apNode->AddDevice(apDev);

  Ptr<Node> sta1Node = CreateObject<Node>();
  Ptr<WifiNetDevice> sta1Dev = CreateObject<WifiNetDevice>();
  m_phySta1 = CreateObject<OfdmaSpectrumWifiPhy>(1);
  Ptr<InterferenceHelper> sta1InterferenceHelper =
      CreateObject<InterferenceHelper>();
  m_phySta1->SetInterferenceHelper(sta1InterferenceHelper);
  Ptr<ErrorRateModel> sta1ErrorModel = CreateObject<NistErrorRateModel>();
  m_phySta1->SetErrorRateModel(sta1ErrorModel);
  m_phySta1->SetDevice(sta1Dev);
  m_phySta1->AddChannel(spectrumChannel);
  m_phySta1->ConfigureStandard(WIFI_STANDARD_80211ax);
  m_phySta1->AssignStreams(streamNumber);
  m_phySta1->SetOperatingChannel(WifiPhy::ChannelTuple{
      channelNum, DEFAULT_CHANNEL_WIDTH, (int)(WIFI_PHY_BAND_5GHZ), 0});
  Ptr<ConstantPositionMobilityModel> sta1Mobility =
      CreateObject<ConstantPositionMobilityModel>();
  m_phySta1->SetMobility(sta1Mobility);
  sta1Dev->SetPhy(m_phySta1);
  sta1Dev->SetStandard(WIFI_STANDARD_80211ax);
  sta1Dev->SetHeConfiguration(CreateObject<HeConfiguration>());
  sta1Node->AggregateObject(sta1Mobility);
  sta1Node->AddDevice(sta1Dev);

  Ptr<Node> sta2Node = CreateObject<Node>();
  Ptr<WifiNetDevice> sta2Dev = CreateObject<WifiNetDevice>();
  m_phySta2 = CreateObject<OfdmaSpectrumWifiPhy>(2);
  Ptr<InterferenceHelper> sta2InterferenceHelper =
      CreateObject<InterferenceHelper>();
  m_phySta2->SetInterferenceHelper(sta2InterferenceHelper);
  Ptr<ErrorRateModel> sta2ErrorModel = CreateObject<NistErrorRateModel>();
  m_phySta2->SetErrorRateModel(sta2ErrorModel);
  m_phySta2->SetDevice(sta2Dev);
  m_phySta2->AddChannel(spectrumChannel);
  m_phySta2->ConfigureStandard(WIFI_STANDARD_80211ax);
  m_phySta2->AssignStreams(streamNumber);
  m_phySta2->SetOperatingChannel(WifiPhy::ChannelTuple{
      channelNum, DEFAULT_CHANNEL_WIDTH, (int)(WIFI_PHY_BAND_5GHZ), 0});
  Ptr<ConstantPositionMobilityModel> sta2Mobility =
      CreateObject<ConstantPositionMobilityModel>();
  m_phySta2->SetMobility(sta2Mobility);
  sta2Dev->SetPhy(m_phySta2);
  sta2Dev->SetStandard(WIFI_STANDARD_80211ax);
  sta2Dev->SetHeConfiguration(CreateObject<HeConfiguration>());
  sta2Node->AggregateObject(sta2Mobility);
  sta2Node->AddDevice(sta2Dev);

  Ptr<Node> interfererNode = CreateObject<Node>();
  Ptr<NonCommunicatingNetDevice> interfererDev =
      CreateObject<NonCommunicatingNetDevice>();
  m_phyInterferer = CreateObject<WaveformGenerator>();
  m_phyInterferer->SetDevice(interfererDev);
  m_phyInterferer->SetChannel(spectrumChannel);
  m_phyInterferer->SetDutyCycle(1);
  interfererNode->AddDevice(interfererDev);
}

void TestPhyPaddingExclusion::DoTeardown() {
  m_phyAp->Dispose();
  m_phyAp = nullptr;
  m_phySta1->Dispose();
  m_phySta1 = nullptr;
  m_phySta2->Dispose();
  m_phySta2 = nullptr;
  m_phyInterferer->Dispose();
  m_phyInterferer = nullptr;
}

void TestPhyPaddingExclusion::SetTrigVector(Time ppduDuration) {
  WifiTxVector trigVector(HePhy::GetHeMcs7(), 0, WIFI_PREAMBLE_HE_TB, 1600, 1,
                          1, 0, DEFAULT_CHANNEL_WIDTH, false, false, true);
  trigVector.SetRu(HeRu::RuSpec(HeRu::RU_106_TONE, 1, false), 1);
  trigVector.SetMode(HePhy::GetHeMcs7(), 1);
  trigVector.SetNss(1, 1);
  trigVector.SetRu(HeRu::RuSpec(HeRu::RU_106_TONE, 2, false), 2);
  trigVector.SetMode(HePhy::GetHeMcs7(), 2);
  trigVector.SetNss(1, 2);
  uint16_t length;
  std::tie(length, ppduDuration) = HePhy::ConvertHeTbPpduDurationToLSigLength(
      ppduDuration, trigVector, m_phyAp->GetPhyBand());
  trigVector.SetLength(length);
  auto hePhyAp = DynamicCast<HePhy>(m_phyAp->GetLatestPhyEntity());
  hePhyAp->SetTrigVector(trigVector, ppduDuration);
}

void TestPhyPaddingExclusion::DoRun() {
  Time expectedPpduDuration = NanoSeconds(292800);
  Time ppduWithPaddingDuration =
      expectedPpduDuration + 10 * NanoSeconds(12800 + 1600);

  Simulator::Schedule(Seconds(0.0), &TestPhyPaddingExclusion::Reset, this);

  Simulator::Schedule(Seconds(1.0), &TestPhyPaddingExclusion::SendHeTbPpdu,
                      this, 1, 1, 1000, ppduWithPaddingDuration);
  Simulator::Schedule(Seconds(1.0), &TestPhyPaddingExclusion::SendHeTbPpdu,
                      this, 2, 2, 1001, ppduWithPaddingDuration);

  Simulator::Schedule(Seconds(1.0), &TestPhyPaddingExclusion::SetTrigVector,
                      this, ppduWithPaddingDuration);

  Simulator::Schedule(Seconds(1.0) + ppduWithPaddingDuration - NanoSeconds(1),
                      &TestPhyPaddingExclusion::CheckPhyState, this, m_phyAp,
                      WifiPhyState::RX);
  Simulator::Schedule(Seconds(1.0) + ppduWithPaddingDuration,
                      &TestPhyPaddingExclusion::CheckPhyState, this, m_phyAp,
                      WifiPhyState::IDLE);

  Simulator::Schedule(Seconds(1.1), &TestPhyPaddingExclusion::CheckRxFromSta1,
                      this, 1, 0, 1000);
  Simulator::Schedule(Seconds(1.1), &TestPhyPaddingExclusion::CheckRxFromSta2,
                      this, 1, 0, 1001);
  Simulator::Schedule(Seconds(1.1),
                      &TestPhyPaddingExclusion::VerifyEventsCleared, this);

  Simulator::Schedule(Seconds(1.5), &TestPhyPaddingExclusion::Reset, this);

  Simulator::Schedule(Seconds(2.0), &TestPhyPaddingExclusion::SendHeTbPpdu,
                      this, 1, 1, 1000, ppduWithPaddingDuration);
  Simulator::Schedule(Seconds(2.0), &TestPhyPaddingExclusion::SendHeTbPpdu,
                      this, 2, 2, 1001, ppduWithPaddingDuration);

  Simulator::Schedule(Seconds(2.0), &TestPhyPaddingExclusion::SetTrigVector,
                      this, ppduWithPaddingDuration);

  BandInfo bandInfo;
  bandInfo.fc = (DEFAULT_FREQUENCY - (DEFAULT_CHANNEL_WIDTH / 4)) * 1e6;
  bandInfo.fl = bandInfo.fc - ((DEFAULT_CHANNEL_WIDTH / 4) * 1e6);
  bandInfo.fh = bandInfo.fc + ((DEFAULT_CHANNEL_WIDTH / 4) * 1e6);
  Bands bands;
  bands.push_back(bandInfo);

  Ptr<SpectrumModel> SpectrumInterferenceRu1 = Create<SpectrumModel>(bands);
  Ptr<SpectrumValue> interferencePsdRu1 =
      Create<SpectrumValue>(SpectrumInterferenceRu1);
  double interferencePower = 0.1;
  *interferencePsdRu1 =
      interferencePower / ((DEFAULT_CHANNEL_WIDTH / 2) * 20e6);

  Simulator::Schedule(Seconds(2.0) + MicroSeconds(50) + expectedPpduDuration,
                      &TestPhyPaddingExclusion::GenerateInterference, this,
                      interferencePsdRu1, MilliSeconds(100));

  Simulator::Schedule(Seconds(2.0) + ppduWithPaddingDuration - NanoSeconds(1),
                      &TestPhyPaddingExclusion::CheckPhyState, this, m_phyAp,
                      WifiPhyState::RX);
  Simulator::Schedule(Seconds(2.0) + ppduWithPaddingDuration,
                      &TestPhyPaddingExclusion::CheckPhyState, this, m_phyAp,
                      WifiPhyState::CCA_BUSY);

  Simulator::Schedule(Seconds(2.1), &TestPhyPaddingExclusion::CheckRxFromSta1,
                      this, 1, 0, 1000);
  Simulator::Schedule(Seconds(2.1), &TestPhyPaddingExclusion::CheckRxFromSta2,
                      this, 1, 0, 1001);
  Simulator::Schedule(Seconds(2.1),
                      &TestPhyPaddingExclusion::VerifyEventsCleared, this);

  Simulator::Schedule(Seconds(2.5), &TestPhyPaddingExclusion::Reset, this);

  Simulator::Run();

  Simulator::Destroy();
}

class TestUlOfdmaPowerControl : public TestCase {
public:
  TestUlOfdmaPowerControl();
  ~TestUlOfdmaPowerControl() override;

private:
  void DoSetup() override;
  void DoTeardown() override;
  void DoRun() override;

  void SendMuBar(std::vector<uint16_t> staIds);

  void SetupBa(Address destination);

  void RunOne(bool setupBa);

  void ReplaceReceiveOkCallbackOfAp();

  void ReceiveOkCallbackAtAp(Ptr<const WifiPsdu> psdu,
                             RxSignalInfo rxSignalInfo, WifiTxVector txVector,
                             std::vector<bool> statusPerMpdu);

  uint8_t m_bssColor;

  Ptr<WifiNetDevice> m_apDev;
  Ptr<WifiNetDevice> m_sta1Dev;
  Ptr<WifiNetDevice> m_sta2Dev;

  Ptr<SpectrumWifiPhy> m_phyAp;

  double m_txPowerAp;
  double m_txPowerStart;
  double m_txPowerEnd;
  uint8_t m_txPowerLevels;

  double m_requestedRssiSta1;
  double m_requestedRssiSta2;

  double m_rssiSta1;
  double m_rssiSta2;

  double m_tol;
};

TestUlOfdmaPowerControl::TestUlOfdmaPowerControl()
    : TestCase("UL-OFDMA power control test"), m_bssColor(1), m_txPowerAp(0),
      m_txPowerStart(0), m_txPowerEnd(0), m_txPowerLevels(0),
      m_requestedRssiSta1(0), m_requestedRssiSta2(0), m_rssiSta1(0),
      m_rssiSta2(0), m_tol(0.1) {}

TestUlOfdmaPowerControl::~TestUlOfdmaPowerControl() {
  m_phyAp = nullptr;
  m_apDev = nullptr;
  m_sta1Dev = nullptr;
  m_sta2Dev = nullptr;
}

void TestUlOfdmaPowerControl::SetupBa(Address destination) {
  Ptr<Packet> pkt = Create<Packet>(100);
  m_apDev->Send(pkt, destination, 0);
}

void TestUlOfdmaPowerControl::SendMuBar(std::vector<uint16_t> staIds) {
  NS_ASSERT(!staIds.empty() && staIds.size() <= 2);

  CtrlTriggerHeader muBar;
  muBar.SetType(TriggerFrameType::MU_BAR_TRIGGER);
  muBar.SetMoreTF(true);
  muBar.SetCsRequired(true);
  muBar.SetUlBandwidth(DEFAULT_CHANNEL_WIDTH);
  muBar.SetGiAndLtfType(1600, 2);
  muBar.SetApTxPower(static_cast<int8_t>(m_txPowerAp));
  muBar.SetUlSpatialReuse(60500);

  HeRu::RuType ru =
      (staIds.size() == 1) ? HeRu::RU_242_TONE : HeRu::RU_106_TONE;
  std::size_t index = 1;
  int8_t ulTargetRssi = -40;
  for (const auto &staId : staIds) {
    CtrlTriggerUserInfoField &ui = muBar.AddUserInfoField();
    ui.SetAid12(staId);
    ui.SetRuAllocation({ru, index, true});
    ui.SetUlFecCodingType(true);
    ui.SetUlMcs(7);
    ui.SetUlDcm(false);
    ui.SetSsAllocation(1, 1);
    if (staId == 1) {
      ulTargetRssi = m_requestedRssiSta1;
    } else if (staId == 2) {
      ulTargetRssi = m_requestedRssiSta2;
    } else {
      NS_ABORT_MSG("Unknown STA-ID (" << staId << ")");
    }
    ui.SetUlTargetRssi(ulTargetRssi);

    CtrlBAckRequestHeader bar;
    bar.SetType(BlockAckReqType::COMPRESSED);
    bar.SetTidInfo(0);
    bar.SetStartingSequence(4095);
    ui.SetMuBarTriggerDepUserInfo(bar);

    ++index;
  }

  WifiTxVector tbTxVector = muBar.GetHeTbTxVector(staIds.front());
  muBar.SetUlLength(HePhy::ConvertHeTbPpduDurationToLSigLength(
                        MicroSeconds(128), tbTxVector, WIFI_PHY_BAND_5GHZ)
                        .first);

  WifiConstPsduMap psdus;
  WifiTxVector txVector =
      WifiTxVector(HePhy::GetHeMcs7(), 0, WIFI_PREAMBLE_HE_SU, 800, 1, 1, 0,
                   DEFAULT_CHANNEL_WIDTH, false, false, false, m_bssColor);

  Ptr<Packet> bar = Create<Packet>();
  bar->AddHeader(muBar);

  Mac48Address receiver = Mac48Address::GetBroadcast();
  if (staIds.size() == 1) {
    uint16_t aidSta1 =
        DynamicCast<StaWifiMac>(m_sta1Dev->GetMac())->GetAssociationId();
    if (staIds.front() == aidSta1) {
      receiver = Mac48Address::ConvertFrom(m_sta1Dev->GetAddress());
    } else {
      NS_ASSERT(
          staIds.front() ==
          DynamicCast<StaWifiMac>(m_sta2Dev->GetMac())->GetAssociationId());
      receiver = Mac48Address::ConvertFrom(m_sta2Dev->GetAddress());
    }
  }

  WifiMacHeader hdr;
  hdr.SetType(WIFI_MAC_CTL_TRIGGER);
  hdr.SetAddr1(receiver);
  hdr.SetAddr2(Mac48Address::ConvertFrom(m_apDev->GetAddress()));
  hdr.SetAddr3(Mac48Address::ConvertFrom(m_apDev->GetAddress()));
  hdr.SetDsNotTo();
  hdr.SetDsFrom();
  hdr.SetNoRetry();
  hdr.SetNoMoreFragments();
  Ptr<WifiPsdu> psdu = Create<WifiPsdu>(bar, hdr);

  Time nav = m_apDev->GetPhy()->GetSifs();
  uint16_t staId = staIds.front();
  nav += m_phyAp->CalculateTxDuration(GetBlockAckSize(BlockAckType::COMPRESSED),
                                      tbTxVector, DEFAULT_WIFI_BAND, staId);
  psdu->SetDuration(nav);
  psdus.insert(std::make_pair(SU_STA_ID, psdu));

  m_phyAp->Send(psdus, txVector);
}

void TestUlOfdmaPowerControl::ReceiveOkCallbackAtAp(Ptr<const WifiPsdu> psdu,
                                                    RxSignalInfo rxSignalInfo,
                                                    WifiTxVector txVector,
                                                    std::vector<bool>) {
  NS_TEST_ASSERT_MSG_EQ(txVector.GetPreambleType(), WIFI_PREAMBLE_HE_TB,
                        "HE TB PPDU expected");
  double rssi = rxSignalInfo.rssi;
  NS_ASSERT(psdu->GetNMpdus() == 1);
  WifiMacHeader hdr = psdu->GetHeader(0);
  NS_TEST_ASSERT_MSG_EQ(hdr.GetType(), WIFI_MAC_CTL_BACKRESP,
                        "Block ACK expected");
  if (hdr.GetAddr2() == m_sta1Dev->GetAddress()) {
    NS_TEST_ASSERT_MSG_EQ_TOL(
        rssi, m_rssiSta1, m_tol,
        "The obtained RSSI from STA 1 at AP is different from the expected one "
        "(" << rssi
            << " vs " << m_rssiSta1 << ", with tolerance of " << m_tol << ")");
  } else if (psdu->GetAddr2() == m_sta2Dev->GetAddress()) {
    NS_TEST_ASSERT_MSG_EQ_TOL(
        rssi, m_rssiSta2, m_tol,
        "The obtained RSSI from STA 2 at AP is different from the expected one "
        "(" << rssi
            << " vs " << m_rssiSta2 << ", with tolerance of " << m_tol << ")");
  } else {
    NS_ABORT_MSG("The receiver address is unknown");
  }
}

void TestUlOfdmaPowerControl::ReplaceReceiveOkCallbackOfAp() {
  m_phyAp->SetReceiveOkCallback(
      MakeCallback(&TestUlOfdmaPowerControl::ReceiveOkCallbackAtAp, this));
}

void TestUlOfdmaPowerControl::DoSetup() {
  Ptr<Node> apNode = CreateObject<Node>();
  NodeContainer staNodes;
  staNodes.Create(2);

  Ptr<MultiModelSpectrumChannel> spectrumChannel =
      CreateObject<MultiModelSpectrumChannel>();
  Ptr<MatrixPropagationLossModel> lossModel =
      CreateObject<MatrixPropagationLossModel>();
  spectrumChannel->AddPropagationLossModel(lossModel);
  Ptr<ConstantSpeedPropagationDelayModel> delayModel =
      CreateObject<ConstantSpeedPropagationDelayModel>();
  spectrumChannel->SetPropagationDelayModel(delayModel);

  SpectrumWifiPhyHelper spectrumPhy;
  spectrumPhy.SetChannel(spectrumChannel);
  spectrumPhy.SetErrorRateModel("ns3::NistErrorRateModel");
  spectrumPhy.Set("ChannelSettings", StringValue("{0, 0, BAND_5GHZ, 0}"));

  WifiHelper wifi;
  wifi.SetStandard(WIFI_STANDARD_80211ax);
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode",
                               StringValue("HeMcs7"), "ControlMode",
                               StringValue("HeMcs7"));

  WifiMacHelper mac;
  mac.SetType("ns3::StaWifiMac");
  NetDeviceContainer staDevs = wifi.Install(spectrumPhy, mac, staNodes);
  wifi.AssignStreams(staDevs, 0);
  m_sta1Dev = DynamicCast<WifiNetDevice>(staDevs.Get(0));
  NS_ASSERT(m_sta1Dev);
  m_sta2Dev = DynamicCast<WifiNetDevice>(staDevs.Get(1));
  NS_ASSERT(m_sta2Dev);

  mac.SetType("ns3::ApWifiMac", "BeaconGeneration", BooleanValue(true),
              "BeaconInterval", TimeValue(MicroSeconds(1024 * 600)));
  m_apDev =
      DynamicCast<WifiNetDevice>(wifi.Install(spectrumPhy, mac, apNode).Get(0));
  NS_ASSERT(m_apDev);
  m_apDev->GetHeConfiguration()->SetAttribute("BssColor",
                                              UintegerValue(m_bssColor));
  m_phyAp = DynamicCast<SpectrumWifiPhy>(m_apDev->GetPhy());
  NS_ASSERT(m_phyAp);

  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  Ptr<ListPositionAllocator> positionAlloc =
      CreateObject<ListPositionAllocator>();
  positionAlloc->Add(Vector(0.0, 0.0, 0.0));
  positionAlloc->Add(Vector(1.0, 0.0, 0.0));
  positionAlloc->Add(Vector(2.0, 0.0, 0.0));
  mobility.SetPositionAllocator(positionAlloc);

  mobility.Install(apNode);
  mobility.Install(staNodes);

  lossModel->SetDefaultLoss(50.0);
  lossModel->SetLoss(apNode->GetObject<MobilityModel>(),
                     staNodes.Get(1)->GetObject<MobilityModel>(), 56.0, true);
}

void TestUlOfdmaPowerControl::DoTeardown() {
  m_phyAp->Dispose();
  m_phyAp = nullptr;
  m_apDev->Dispose();
  m_apDev = nullptr;
  m_sta1Dev->Dispose();
  m_sta1Dev = nullptr;
  m_sta2Dev->Dispose();
  m_sta2Dev = nullptr;
}

void TestUlOfdmaPowerControl::RunOne(bool setupBa) {
  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(1);
  int64_t streamNumber = 0;

  Ptr<WifiPhy> phySta1 = m_sta1Dev->GetPhy();
  Ptr<WifiPhy> phySta2 = m_sta2Dev->GetPhy();

  m_phyAp->AssignStreams(streamNumber);
  phySta1->AssignStreams(streamNumber);
  phySta2->AssignStreams(streamNumber);

  m_phyAp->SetAttribute("TxPowerStart", DoubleValue(m_txPowerAp));
  m_phyAp->SetAttribute("TxPowerEnd", DoubleValue(m_txPowerAp));
  m_phyAp->SetAttribute("TxPowerLevels", UintegerValue(1));

  phySta1->SetAttribute("TxPowerStart", DoubleValue(m_txPowerStart));
  phySta1->SetAttribute("TxPowerEnd", DoubleValue(m_txPowerEnd));
  phySta1->SetAttribute("TxPowerLevels", UintegerValue(m_txPowerLevels));

  phySta2->SetAttribute("TxPowerStart", DoubleValue(m_txPowerStart));
  phySta2->SetAttribute("TxPowerEnd", DoubleValue(m_txPowerEnd));
  phySta2->SetAttribute("TxPowerLevels", UintegerValue(m_txPowerLevels));

  Time relativeStart = MilliSeconds(0);
  if (setupBa) {
    Simulator::Schedule(MilliSeconds(800), &TestUlOfdmaPowerControl::SetupBa,
                        this, m_sta1Dev->GetAddress());
    Simulator::Schedule(MilliSeconds(850), &TestUlOfdmaPowerControl::SetupBa,
                        this, m_sta2Dev->GetAddress());
    relativeStart = MilliSeconds(1000);
  } else {
    Ptr<ApWifiMac> apMac = DynamicCast<ApWifiMac>(m_apDev->GetMac());
    NS_ASSERT(apMac);
    apMac->SetAttribute("BeaconGeneration", BooleanValue(false));
  }

  Simulator::Schedule(relativeStart,
                      &TestUlOfdmaPowerControl::ReplaceReceiveOkCallbackOfAp,
                      this);

  {
    std::vector<uint16_t> staIds{1};
    Simulator::Schedule(relativeStart, &TestUlOfdmaPowerControl::SendMuBar,
                        this, staIds);
  }

  {
    std::vector<uint16_t> staIds{2};
    Simulator::Schedule(relativeStart + MilliSeconds(20),
                        &TestUlOfdmaPowerControl::SendMuBar, this, staIds);
  }

  {
    std::vector<uint16_t> staIds{1, 2};
    Simulator::Schedule(relativeStart + MilliSeconds(40),
                        &TestUlOfdmaPowerControl::SendMuBar, this, staIds);
  }

  Simulator::Stop(relativeStart + MilliSeconds(100));
  Simulator::Run();
}

void TestUlOfdmaPowerControl::DoRun() {
  m_txPowerAp = 20;
  m_txPowerStart = 15;

  m_requestedRssiSta1 = -30.0;
  m_requestedRssiSta2 = -36.0;

  {
    m_txPowerEnd = 15;
    m_txPowerLevels = 1;

    m_rssiSta1 = -35.0;
    m_rssiSta2 = -41.0;

    RunOne(true);
  }

  {
    m_txPowerEnd = 25;
    m_txPowerLevels = 6;

    m_rssiSta1 = -29.0;
    m_rssiSta2 = -35.0;

    RunOne(false);
  }

  {
    m_txPowerEnd = 25;
    m_txPowerLevels = 11;

    m_rssiSta1 = -30.0;
    m_rssiSta2 = -36.0;

    RunOne(false);
  }

  {
    m_txPowerEnd = 25;
    m_txPowerLevels = 11;

    m_requestedRssiSta1 = -28.0;
    m_requestedRssiSta2 = -37.0;

    m_rssiSta1 = -28.0;
    m_rssiSta2 = -37.0;

    RunOne(false);
  }

  Simulator::Destroy();
}

class WifiPhyOfdmaTestSuite : public TestSuite {
public:
  WifiPhyOfdmaTestSuite();
};

WifiPhyOfdmaTestSuite::WifiPhyOfdmaTestSuite()
    : TestSuite("wifi-phy-ofdma", UNIT) {
  AddTestCase(new TestDlOfdmaPhyTransmission, TestCase::QUICK);
  AddTestCase(new TestDlOfdmaPhyPuncturing, TestCase::QUICK);
  AddTestCase(new TestUlOfdmaPpduUid, TestCase::QUICK);
  AddTestCase(new TestMultipleHeTbPreambles, TestCase::QUICK);
  AddTestCase(new TestUlOfdmaPhyTransmission, TestCase::QUICK);
  AddTestCase(new TestPhyPaddingExclusion, TestCase::QUICK);
  AddTestCase(new TestUlOfdmaPowerControl, TestCase::QUICK);
}

static WifiPhyOfdmaTestSuite wifiPhyOfdmaTestSuite;
