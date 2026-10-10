
#include "ns3/ap-wifi-mac.h"
#include "ns3/boolean.h"
#include "ns3/double.h"
#include "ns3/he-configuration.h"
#include "ns3/he-phy.h"
#include "ns3/interference-helper.h"
#include "ns3/log.h"
#include "ns3/multi-model-spectrum-channel.h"
#include "ns3/nist-error-rate-model.h"
#include "ns3/node.h"
#include "ns3/pointer.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/simulator.h"
#include "ns3/spectrum-wifi-helper.h"
#include "ns3/spectrum-wifi-phy.h"
#include "ns3/test.h"
#include "ns3/uinteger.h"
#include "ns3/wifi-mac-header.h"
#include "ns3/wifi-net-device.h"
#include "ns3/wifi-psdu.h"
#include "ns3/wifi-spectrum-value-helper.h"
#include "ns3/wifi-utils.h"

#include <list>
#include <tuple>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("WifiPhyMuMimoTest");

constexpr uint32_t DEFAULT_FREQUENCY = 5180;
constexpr uint16_t DEFAULT_CHANNEL_WIDTH = 20;

class TestDlMuTxVector : public TestCase {
public:
  TestDlMuTxVector();

private:
  void DoRun() override;

  static WifiTxVector BuildTxVector(uint16_t bw,
                                    const std::list<HeMuUserInfo> &userInfos);
};

TestDlMuTxVector::TestDlMuTxVector()
    : TestCase("Check for valid combinations of MU TX-VECTOR") {}

WifiTxVector
TestDlMuTxVector::BuildTxVector(uint16_t bw,
                                const std::list<HeMuUserInfo> &userInfos) {
  WifiTxVector txVector;
  txVector.SetPreambleType(WIFI_PREAMBLE_HE_MU);
  txVector.SetChannelWidth(bw);
  std::list<uint16_t> staIds;
  uint16_t staId = 1;
  for (const auto &userInfo : userInfos) {
    txVector.SetHeMuUserInfo(staId, userInfo);
    staIds.push_back(staId++);
  }
  return txVector;
}

void TestDlMuTxVector::DoRun() {
  std::list<HeMuUserInfo> userInfos;
  userInfos.push_back({{HeRu::RU_106_TONE, 1, true}, 11, 1});
  userInfos.push_back({{HeRu::RU_106_TONE, 2, true}, 10, 2});
  WifiTxVector txVector = BuildTxVector(20, userInfos);
  NS_TEST_EXPECT_MSG_EQ(txVector.IsDlOfdma(), true,
                        "TX-VECTOR should indicate an OFDMA transmission");
  NS_TEST_EXPECT_MSG_EQ(txVector.IsDlMuMimo(), false,
                        "TX-VECTOR should not indicate a MU-MIMO transmission");
  NS_TEST_EXPECT_MSG_EQ(txVector.IsSigBCompression(), false,
                        "TX-VECTOR should not indicate a SIG-B compression");
  NS_TEST_EXPECT_MSG_EQ(txVector.IsValid(), true,
                        "TX-VECTOR should indicate all checks are passed");
  userInfos.clear();

  userInfos.push_back({{HeRu::RU_242_TONE, 1, true}, 11, 1});
  userInfos.push_back({{HeRu::RU_242_TONE, 1, true}, 10, 2});
  txVector = BuildTxVector(20, userInfos);
  NS_TEST_EXPECT_MSG_EQ(txVector.IsDlOfdma(), false,
                        "TX-VECTOR should indicate a MU-MIMO transmission");
  NS_TEST_EXPECT_MSG_EQ(txVector.IsDlMuMimo(), true,
                        "TX-VECTOR should not indicate an OFDMA transmission");
  NS_TEST_EXPECT_MSG_EQ(txVector.IsSigBCompression(), true,
                        "TX-VECTOR should indicate a SIG-B compression");
  NS_TEST_EXPECT_MSG_EQ(txVector.IsValid(), true,
                        "TX-VECTOR should indicate all checks are passed");
  userInfos.clear();

  userInfos.push_back({{HeRu::RU_242_TONE, 1, true}, 11, 1});
  userInfos.push_back({{HeRu::RU_242_TONE, 1, true}, 10, 1});
  userInfos.push_back({{HeRu::RU_242_TONE, 1, true}, 9, 1});
  userInfos.push_back({{HeRu::RU_242_TONE, 1, true}, 8, 1});
  userInfos.push_back({{HeRu::RU_242_TONE, 1, true}, 7, 1});
  userInfos.push_back({{HeRu::RU_242_TONE, 1, true}, 6, 1});
  userInfos.push_back({{HeRu::RU_242_TONE, 1, true}, 5, 1});
  userInfos.push_back({{HeRu::RU_242_TONE, 1, true}, 4, 1});
  userInfos.push_back({{HeRu::RU_242_TONE, 1, true}, 3, 1});
  txVector = BuildTxVector(20, userInfos);
  NS_TEST_EXPECT_MSG_EQ(txVector.IsDlOfdma(), false,
                        "TX-VECTOR should indicate a MU-MIMO transmission");
  NS_TEST_EXPECT_MSG_EQ(txVector.IsDlMuMimo(), true,
                        "TX-VECTOR should not indicate an OFDMA transmission");
  NS_TEST_EXPECT_MSG_EQ(txVector.IsSigBCompression(), true,
                        "TX-VECTOR should indicate a SIG-B compression");
  NS_TEST_EXPECT_MSG_EQ(txVector.IsValid(), false,
                        "TX-VECTOR should not indicate all checks are passed");

  userInfos.push_back({{HeRu::RU_242_TONE, 1, true}, 11, 2});
  userInfos.push_back({{HeRu::RU_242_TONE, 1, true}, 10, 2});
  userInfos.push_back({{HeRu::RU_242_TONE, 1, true}, 9, 3});
  userInfos.push_back({{HeRu::RU_242_TONE, 1, true}, 8, 3});
  txVector = BuildTxVector(20, userInfos);
  NS_TEST_EXPECT_MSG_EQ(txVector.IsDlOfdma(), false,
                        "TX-VECTOR should indicate a MU-MIMO transmission");
  NS_TEST_EXPECT_MSG_EQ(txVector.IsDlMuMimo(), true,
                        "TX-VECTOR should not indicate an OFDMA transmission");
  NS_TEST_EXPECT_MSG_EQ(txVector.IsSigBCompression(), true,
                        "TX-VECTOR should indicate a SIG-B compression");
  NS_TEST_EXPECT_MSG_EQ(txVector.IsValid(), false,
                        "TX-VECTOR should not indicate all checks are passed");
}

class MuMimoTestHePhy : public HePhy {
public:
  MuMimoTestHePhy(uint16_t staId);

  uint16_t GetStaId(const Ptr<const WifiPpdu> ppdu) const override;

  void SetGlobalPpduUid(uint64_t uid);

private:
  uint16_t m_staId;
};

MuMimoTestHePhy::MuMimoTestHePhy(uint16_t staId) : HePhy(), m_staId(staId) {}

uint16_t MuMimoTestHePhy::GetStaId(const Ptr<const WifiPpdu> ppdu) const {
  if (ppdu->GetType() == WIFI_PPDU_TYPE_DL_MU) {
    return m_staId;
  }
  return HePhy::GetStaId(ppdu);
}

void MuMimoTestHePhy::SetGlobalPpduUid(uint64_t uid) { m_globalPpduUid = uid; }

class MuMimoSpectrumWifiPhy : public SpectrumWifiPhy {
public:
  static TypeId GetTypeId();
  MuMimoSpectrumWifiPhy(uint16_t staId);
  ~MuMimoSpectrumWifiPhy() override;

  void SetPpduUid(uint64_t uid);

  void SetTriggerFrameUid(uint64_t uid);

  Ptr<Event> GetCurrentEvent();

private:
  void DoInitialize() override;
  void DoDispose() override;

  Ptr<MuMimoTestHePhy> m_ofdmTestHePhy;
};

TypeId MuMimoSpectrumWifiPhy::GetTypeId() {
  static TypeId tid = TypeId("ns3::MuMimoSpectrumWifiPhy")
                          .SetParent<SpectrumWifiPhy>()
                          .SetGroupName("Wifi");
  return tid;
}

MuMimoSpectrumWifiPhy::MuMimoSpectrumWifiPhy(uint16_t staId)
    : SpectrumWifiPhy() {
  m_ofdmTestHePhy = Create<MuMimoTestHePhy>(staId);
  m_ofdmTestHePhy->SetOwner(this);
}

MuMimoSpectrumWifiPhy::~MuMimoSpectrumWifiPhy() {}

void MuMimoSpectrumWifiPhy::DoInitialize() {
  m_phyEntities[WIFI_MOD_CLASS_HE] = m_ofdmTestHePhy;
  SpectrumWifiPhy::DoInitialize();
}

void MuMimoSpectrumWifiPhy::DoDispose() {
  m_ofdmTestHePhy = nullptr;
  SpectrumWifiPhy::DoDispose();
}

void MuMimoSpectrumWifiPhy::SetPpduUid(uint64_t uid) {
  m_ofdmTestHePhy->SetGlobalPpduUid(uid);
  m_previouslyRxPpduUid = uid;
}

void MuMimoSpectrumWifiPhy::SetTriggerFrameUid(uint64_t uid) {
  m_previouslyRxPpduUid = uid;
}

Ptr<Event> MuMimoSpectrumWifiPhy::GetCurrentEvent() { return m_currentEvent; }

class TestDlMuMimoPhyTransmission : public TestCase {
public:
  TestDlMuMimoPhyTransmission();

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

  struct StaInfo {
    uint16_t staId;
    uint8_t staNss;
  };

  void SendMuPpdu(const std::vector<StaInfo> &staInfos);

  void GenerateInterference(Ptr<SpectrumValue> interferencePsd, Time duration);
  void StopInterference();

  void RunOne();

  void CheckPhyState(Ptr<MuMimoSpectrumWifiPhy> phy,
                     WifiPhyState expectedState);
  void DoCheckPhyState(Ptr<MuMimoSpectrumWifiPhy> phy,
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
  Ptr<MuMimoSpectrumWifiPhy> m_phySta1;
  Ptr<MuMimoSpectrumWifiPhy> m_phySta2;
  Ptr<MuMimoSpectrumWifiPhy> m_phySta3;

  uint8_t m_nss;
  uint16_t m_frequency;
  uint16_t m_channelWidth;
  Time m_expectedPpduDuration;
};

TestDlMuMimoPhyTransmission::TestDlMuMimoPhyTransmission()
    : TestCase("DL MU-MIMO PHY test"), m_countRxSuccessSta1{0},
      m_countRxSuccessSta2{0}, m_countRxSuccessSta3{0}, m_countRxFailureSta1{0},
      m_countRxFailureSta2{0}, m_countRxFailureSta3{0}, m_countRxBytesSta1{0},
      m_countRxBytesSta2{0}, m_countRxBytesSta3{0}, m_nss{1},
      m_frequency{DEFAULT_FREQUENCY}, m_channelWidth{DEFAULT_CHANNEL_WIDTH},
      m_expectedPpduDuration{NanoSeconds(306400)} {}

void TestDlMuMimoPhyTransmission::ResetResults() {
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

void TestDlMuMimoPhyTransmission::SendMuPpdu(
    const std::vector<StaInfo> &staInfos) {
  NS_LOG_FUNCTION(this << staInfos.size());
  NS_ASSERT(staInfos.size() > 1);

  WifiTxVector txVector =
      WifiTxVector(HePhy::GetHeMcs7(), 0, WIFI_PREAMBLE_HE_MU, 800, 1, 1, 0,
                   m_channelWidth, false, false);

  WifiConstPsduMap psdus;
  HeRu::RuSpec ru(HeRu::GetRuType(m_channelWidth), 1, true);
  for (const auto &staInfo : staInfos) {
    txVector.SetRu(ru, staInfo.staId);
    txVector.SetMode(HePhy::GetHeMcs7(), staInfo.staId);
    txVector.SetNss(staInfo.staNss, staInfo.staId);

    Ptr<Packet> pkt = Create<Packet>(1000 + (8 * staInfo.staId));
    WifiMacHeader hdr;
    hdr.SetType(WIFI_MAC_QOSDATA);
    hdr.SetQosTid(0);
    std::ostringstream addr;
    addr << "00:00:00:00:00:0" << staInfo.staId;
    hdr.SetAddr1(Mac48Address(addr.str().c_str()));
    hdr.SetSequenceNumber(1 + staInfo.staId);
    Ptr<WifiPsdu> psdu = Create<WifiPsdu>(pkt, hdr);
    psdus.insert(std::make_pair(staInfo.staId, psdu));
  }

  txVector.SetSigBMode(VhtPhy::GetVhtMcs5());

  NS_ASSERT(txVector.IsDlMuMimo());
  NS_ASSERT(!txVector.IsDlOfdma());

  m_phyAp->Send(psdus, txVector);
}

void TestDlMuMimoPhyTransmission::RxSuccessSta1(Ptr<const WifiPsdu> psdu,
                                                RxSignalInfo rxSignalInfo,
                                                WifiTxVector txVector,
                                                std::vector<bool>) {
  NS_LOG_FUNCTION(this << *psdu << rxSignalInfo << txVector);
  m_countRxSuccessSta1++;
  m_countRxBytesSta1 += (psdu->GetSize() - 30);
}

void TestDlMuMimoPhyTransmission::RxSuccessSta2(Ptr<const WifiPsdu> psdu,
                                                RxSignalInfo rxSignalInfo,
                                                WifiTxVector txVector,
                                                std::vector<bool>) {
  NS_LOG_FUNCTION(this << *psdu << rxSignalInfo << txVector);
  m_countRxSuccessSta2++;
  m_countRxBytesSta2 += (psdu->GetSize() - 30);
}

void TestDlMuMimoPhyTransmission::RxSuccessSta3(Ptr<const WifiPsdu> psdu,
                                                RxSignalInfo rxSignalInfo,
                                                WifiTxVector txVector,
                                                std::vector<bool>) {
  NS_LOG_FUNCTION(this << *psdu << rxSignalInfo << txVector);
  m_countRxSuccessSta3++;
  m_countRxBytesSta3 += (psdu->GetSize() - 30);
}

void TestDlMuMimoPhyTransmission::RxFailureSta1(Ptr<const WifiPsdu> psdu) {
  NS_LOG_FUNCTION(this << *psdu);
  m_countRxFailureSta1++;
}

void TestDlMuMimoPhyTransmission::RxFailureSta2(Ptr<const WifiPsdu> psdu) {
  NS_LOG_FUNCTION(this << *psdu);
  m_countRxFailureSta2++;
}

void TestDlMuMimoPhyTransmission::RxFailureSta3(Ptr<const WifiPsdu> psdu) {
  NS_LOG_FUNCTION(this << *psdu);
  m_countRxFailureSta3++;
}

void TestDlMuMimoPhyTransmission::CheckResultsSta1(uint32_t expectedRxSuccess,
                                                   uint32_t expectedRxFailure,
                                                   uint32_t expectedRxBytes) {
  NS_TEST_ASSERT_MSG_EQ(
      m_countRxSuccessSta1, expectedRxSuccess,
      "The number of successfully received packets by STA 1 is not correct!");
  NS_TEST_ASSERT_MSG_EQ(
      m_countRxFailureSta1, expectedRxFailure,
      "The number of unsuccessfully received packets by STA 1 is not correct!");
  NS_TEST_ASSERT_MSG_EQ(
      m_countRxBytesSta1, expectedRxBytes,
      "The number of bytes received by STA 1 is not correct!");
}

void TestDlMuMimoPhyTransmission::CheckResultsSta2(uint32_t expectedRxSuccess,
                                                   uint32_t expectedRxFailure,
                                                   uint32_t expectedRxBytes) {
  NS_TEST_ASSERT_MSG_EQ(
      m_countRxSuccessSta2, expectedRxSuccess,
      "The number of successfully received packets by STA 2 is not correct!");
  NS_TEST_ASSERT_MSG_EQ(
      m_countRxFailureSta2, expectedRxFailure,
      "The number of unsuccessfully received packets by STA 2 is not correct!");
  NS_TEST_ASSERT_MSG_EQ(
      m_countRxBytesSta2, expectedRxBytes,
      "The number of bytes received by STA 2 is not correct!");
}

void TestDlMuMimoPhyTransmission::CheckResultsSta3(uint32_t expectedRxSuccess,
                                                   uint32_t expectedRxFailure,
                                                   uint32_t expectedRxBytes) {
  NS_TEST_ASSERT_MSG_EQ(
      m_countRxSuccessSta3, expectedRxSuccess,
      "The number of successfully received packets by STA 3 is not correct!");
  NS_TEST_ASSERT_MSG_EQ(
      m_countRxFailureSta3, expectedRxFailure,
      "The number of unsuccessfully received packets by STA 3 is not correct!");
  NS_TEST_ASSERT_MSG_EQ(
      m_countRxBytesSta3, expectedRxBytes,
      "The number of bytes received by STA 3 is not correct!");
}

void TestDlMuMimoPhyTransmission::CheckPhyState(Ptr<MuMimoSpectrumWifiPhy> phy,
                                                WifiPhyState expectedState) {
  Simulator::ScheduleNow(&TestDlMuMimoPhyTransmission::DoCheckPhyState, this,
                         phy, expectedState);
}

void TestDlMuMimoPhyTransmission::DoCheckPhyState(
    Ptr<MuMimoSpectrumWifiPhy> phy, WifiPhyState expectedState) {
  WifiPhyState currentState;
  PointerValue ptr;
  phy->GetAttribute("State", ptr);
  Ptr<WifiPhyStateHelper> state =
      DynamicCast<WifiPhyStateHelper>(ptr.Get<WifiPhyStateHelper>());
  currentState = state->GetState();
  NS_LOG_FUNCTION(this << currentState << expectedState);
  NS_TEST_ASSERT_MSG_EQ(currentState, expectedState,
                        "PHY State "
                            << currentState << " does not match expected state "
                            << expectedState << " at " << Simulator::Now());
}

void TestDlMuMimoPhyTransmission::DoSetup() {
  Ptr<MultiModelSpectrumChannel> spectrumChannel =
      CreateObject<MultiModelSpectrumChannel>();
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
  apDev->SetPhy(m_phyAp);
  apNode->AddDevice(apDev);

  Ptr<Node> sta1Node = CreateObject<Node>();
  Ptr<WifiNetDevice> sta1Dev = CreateObject<WifiNetDevice>();
  m_phySta1 = CreateObject<MuMimoSpectrumWifiPhy>(1);
  Ptr<InterferenceHelper> sta1InterferenceHelper =
      CreateObject<InterferenceHelper>();
  m_phySta1->SetInterferenceHelper(sta1InterferenceHelper);
  Ptr<ErrorRateModel> sta1ErrorModel = CreateObject<NistErrorRateModel>();
  m_phySta1->SetErrorRateModel(sta1ErrorModel);
  m_phySta1->SetDevice(sta1Dev);
  m_phySta1->AddChannel(spectrumChannel);
  m_phySta1->ConfigureStandard(WIFI_STANDARD_80211ax);
  m_phySta1->SetReceiveOkCallback(
      MakeCallback(&TestDlMuMimoPhyTransmission::RxSuccessSta1, this));
  m_phySta1->SetReceiveErrorCallback(
      MakeCallback(&TestDlMuMimoPhyTransmission::RxFailureSta1, this));
  sta1Dev->SetPhy(m_phySta1);
  sta1Node->AddDevice(sta1Dev);

  Ptr<Node> sta2Node = CreateObject<Node>();
  Ptr<WifiNetDevice> sta2Dev = CreateObject<WifiNetDevice>();
  m_phySta2 = CreateObject<MuMimoSpectrumWifiPhy>(2);
  Ptr<InterferenceHelper> sta2InterferenceHelper =
      CreateObject<InterferenceHelper>();
  m_phySta2->SetInterferenceHelper(sta2InterferenceHelper);
  Ptr<ErrorRateModel> sta2ErrorModel = CreateObject<NistErrorRateModel>();
  m_phySta2->SetErrorRateModel(sta2ErrorModel);
  m_phySta2->SetDevice(sta2Dev);
  m_phySta2->AddChannel(spectrumChannel);
  m_phySta2->ConfigureStandard(WIFI_STANDARD_80211ax);
  m_phySta2->SetReceiveOkCallback(
      MakeCallback(&TestDlMuMimoPhyTransmission::RxSuccessSta2, this));
  m_phySta2->SetReceiveErrorCallback(
      MakeCallback(&TestDlMuMimoPhyTransmission::RxFailureSta2, this));
  sta2Dev->SetPhy(m_phySta2);
  sta2Node->AddDevice(sta2Dev);

  Ptr<Node> sta3Node = CreateObject<Node>();
  Ptr<WifiNetDevice> sta3Dev = CreateObject<WifiNetDevice>();
  m_phySta3 = CreateObject<MuMimoSpectrumWifiPhy>(3);
  Ptr<InterferenceHelper> sta3InterferenceHelper =
      CreateObject<InterferenceHelper>();
  m_phySta3->SetInterferenceHelper(sta3InterferenceHelper);
  Ptr<ErrorRateModel> sta3ErrorModel = CreateObject<NistErrorRateModel>();
  m_phySta3->SetErrorRateModel(sta3ErrorModel);
  m_phySta3->SetDevice(sta3Dev);
  m_phySta3->AddChannel(spectrumChannel);
  m_phySta3->ConfigureStandard(WIFI_STANDARD_80211ax);
  m_phySta3->SetReceiveOkCallback(
      MakeCallback(&TestDlMuMimoPhyTransmission::RxSuccessSta3, this));
  m_phySta3->SetReceiveErrorCallback(
      MakeCallback(&TestDlMuMimoPhyTransmission::RxFailureSta3, this));
  sta3Dev->SetPhy(m_phySta3);
  sta3Node->AddDevice(sta3Dev);
}

void TestDlMuMimoPhyTransmission::DoTeardown() {
  m_phyAp->Dispose();
  m_phyAp = nullptr;
  m_phySta1->Dispose();
  m_phySta1 = nullptr;
  m_phySta2->Dispose();
  m_phySta2 = nullptr;
  m_phySta3->Dispose();
  m_phySta3 = nullptr;
}

void TestDlMuMimoPhyTransmission::RunOne() {
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
  m_phySta3->SetOperatingChannel(
      WifiPhy::ChannelTuple{channelNum, m_channelWidth, WIFI_PHY_BAND_5GHZ, 0});

  m_phyAp->SetNumberOfAntennas(8);
  m_phyAp->SetMaxSupportedTxSpatialStreams(8);

  Simulator::Schedule(Seconds(1.0), &TestDlMuMimoPhyTransmission::SendMuPpdu,
                      this, std::vector<StaInfo>{{1, m_nss}, {2, m_nss}});

  Simulator::Schedule(Seconds(1.0) + m_expectedPpduDuration - NanoSeconds(1),
                      &TestDlMuMimoPhyTransmission::CheckPhyState, this,
                      m_phySta1, WifiPhyState::RX);
  Simulator::Schedule(Seconds(1.0) + m_expectedPpduDuration - NanoSeconds(1),
                      &TestDlMuMimoPhyTransmission::CheckPhyState, this,
                      m_phySta2, WifiPhyState::RX);
  Simulator::Schedule(Seconds(1.0) + m_expectedPpduDuration - NanoSeconds(1),
                      &TestDlMuMimoPhyTransmission::CheckPhyState, this,
                      m_phySta3, WifiPhyState::CCA_BUSY);
  Simulator::Schedule(Seconds(1.0) + m_expectedPpduDuration,
                      &TestDlMuMimoPhyTransmission::CheckPhyState, this,
                      m_phySta1, WifiPhyState::IDLE);
  Simulator::Schedule(Seconds(1.0) + m_expectedPpduDuration,
                      &TestDlMuMimoPhyTransmission::CheckPhyState, this,
                      m_phySta2, WifiPhyState::IDLE);
  Simulator::Schedule(Seconds(1.0) + m_expectedPpduDuration,
                      &TestDlMuMimoPhyTransmission::CheckPhyState, this,
                      m_phySta3, WifiPhyState::IDLE);

  Simulator::Schedule(Seconds(1.1),
                      &TestDlMuMimoPhyTransmission::CheckResultsSta1, this, 1,
                      0, 1008);
  Simulator::Schedule(Seconds(1.1),
                      &TestDlMuMimoPhyTransmission::CheckResultsSta2, this, 1,
                      0, 1016);
  Simulator::Schedule(Seconds(1.1),
                      &TestDlMuMimoPhyTransmission::CheckResultsSta3, this, 0,
                      0, 0);

  Simulator::Schedule(Seconds(1.5), &TestDlMuMimoPhyTransmission::ResetResults,
                      this);

  Simulator::Schedule(Seconds(2.0), &TestDlMuMimoPhyTransmission::SendMuPpdu,
                      this, std::vector<StaInfo>{{1, m_nss}, {3, m_nss}});

  Simulator::Schedule(Seconds(2.0) + m_expectedPpduDuration - NanoSeconds(1),
                      &TestDlMuMimoPhyTransmission::CheckPhyState, this,
                      m_phySta1, WifiPhyState::RX);
  Simulator::Schedule(Seconds(2.0) + m_expectedPpduDuration - NanoSeconds(1),
                      &TestDlMuMimoPhyTransmission::CheckPhyState, this,
                      m_phySta2, WifiPhyState::CCA_BUSY);
  Simulator::Schedule(Seconds(2.0) + m_expectedPpduDuration - NanoSeconds(1),
                      &TestDlMuMimoPhyTransmission::CheckPhyState, this,
                      m_phySta3, WifiPhyState::RX);
  Simulator::Schedule(Seconds(2.0) + m_expectedPpduDuration,
                      &TestDlMuMimoPhyTransmission::CheckPhyState, this,
                      m_phySta1, WifiPhyState::IDLE);
  Simulator::Schedule(Seconds(2.0) + m_expectedPpduDuration,
                      &TestDlMuMimoPhyTransmission::CheckPhyState, this,
                      m_phySta2, WifiPhyState::IDLE);
  Simulator::Schedule(Seconds(2.0) + m_expectedPpduDuration,
                      &TestDlMuMimoPhyTransmission::CheckPhyState, this,
                      m_phySta3, WifiPhyState::IDLE);

  Simulator::Schedule(Seconds(2.1),
                      &TestDlMuMimoPhyTransmission::CheckResultsSta1, this, 1,
                      0, 1008);
  Simulator::Schedule(Seconds(2.1),
                      &TestDlMuMimoPhyTransmission::CheckResultsSta2, this, 0,
                      0, 0);
  Simulator::Schedule(Seconds(2.1),
                      &TestDlMuMimoPhyTransmission::CheckResultsSta3, this, 1,
                      0, 1024);

  Simulator::Schedule(Seconds(2.5), &TestDlMuMimoPhyTransmission::ResetResults,
                      this);

  Simulator::Schedule(Seconds(3.0), &TestDlMuMimoPhyTransmission::SendMuPpdu,
                      this, std::vector<StaInfo>{{2, m_nss}, {3, m_nss}});

  Simulator::Schedule(Seconds(3.0) + m_expectedPpduDuration - NanoSeconds(1),
                      &TestDlMuMimoPhyTransmission::CheckPhyState, this,
                      m_phySta1, WifiPhyState::CCA_BUSY);
  Simulator::Schedule(Seconds(3.0) + m_expectedPpduDuration - NanoSeconds(1),
                      &TestDlMuMimoPhyTransmission::CheckPhyState, this,
                      m_phySta2, WifiPhyState::RX);
  Simulator::Schedule(Seconds(3.0) + m_expectedPpduDuration - NanoSeconds(1),
                      &TestDlMuMimoPhyTransmission::CheckPhyState, this,
                      m_phySta3, WifiPhyState::RX);
  Simulator::Schedule(Seconds(3.0) + m_expectedPpduDuration,
                      &TestDlMuMimoPhyTransmission::CheckPhyState, this,
                      m_phySta1, WifiPhyState::IDLE);
  Simulator::Schedule(Seconds(3.0) + m_expectedPpduDuration,
                      &TestDlMuMimoPhyTransmission::CheckPhyState, this,
                      m_phySta2, WifiPhyState::IDLE);
  Simulator::Schedule(Seconds(3.0) + m_expectedPpduDuration,
                      &TestDlMuMimoPhyTransmission::CheckPhyState, this,
                      m_phySta3, WifiPhyState::IDLE);

  Simulator::Schedule(Seconds(3.1),
                      &TestDlMuMimoPhyTransmission::CheckResultsSta1, this, 0,
                      0, 0);
  Simulator::Schedule(Seconds(3.1),
                      &TestDlMuMimoPhyTransmission::CheckResultsSta2, this, 1,
                      0, 1016);
  Simulator::Schedule(Seconds(3.1),
                      &TestDlMuMimoPhyTransmission::CheckResultsSta3, this, 1,
                      0, 1024);

  Simulator::Schedule(Seconds(3.5), &TestDlMuMimoPhyTransmission::ResetResults,
                      this);

  Simulator::Schedule(Seconds(4.0), &TestDlMuMimoPhyTransmission::SendMuPpdu,
                      this,
                      std::vector<StaInfo>{{1, m_nss}, {2, m_nss}, {3, m_nss}});

  Simulator::Schedule(Seconds(4.0) + m_expectedPpduDuration - NanoSeconds(1),
                      &TestDlMuMimoPhyTransmission::CheckPhyState, this,
                      m_phySta1, WifiPhyState::RX);
  Simulator::Schedule(Seconds(4.0) + m_expectedPpduDuration - NanoSeconds(1),
                      &TestDlMuMimoPhyTransmission::CheckPhyState, this,
                      m_phySta2, WifiPhyState::RX);
  Simulator::Schedule(Seconds(4.0) + m_expectedPpduDuration - NanoSeconds(1),
                      &TestDlMuMimoPhyTransmission::CheckPhyState, this,
                      m_phySta3, WifiPhyState::RX);
  Simulator::Schedule(Seconds(4.0) + m_expectedPpduDuration,
                      &TestDlMuMimoPhyTransmission::CheckPhyState, this,
                      m_phySta1, WifiPhyState::IDLE);
  Simulator::Schedule(Seconds(4.0) + m_expectedPpduDuration,
                      &TestDlMuMimoPhyTransmission::CheckPhyState, this,
                      m_phySta2, WifiPhyState::IDLE);
  Simulator::Schedule(Seconds(4.0) + m_expectedPpduDuration,
                      &TestDlMuMimoPhyTransmission::CheckPhyState, this,
                      m_phySta3, WifiPhyState::IDLE);

  Simulator::Schedule(Seconds(4.1),
                      &TestDlMuMimoPhyTransmission::CheckResultsSta1, this, 1,
                      0, 1008);
  Simulator::Schedule(Seconds(4.1),
                      &TestDlMuMimoPhyTransmission::CheckResultsSta2, this, 1,
                      0, 1016);
  Simulator::Schedule(Seconds(4.1),
                      &TestDlMuMimoPhyTransmission::CheckResultsSta3, this, 1,
                      0, 1024);

  Simulator::Schedule(Seconds(4.5), &TestDlMuMimoPhyTransmission::ResetResults,
                      this);

  Simulator::Run();
}

void TestDlMuMimoPhyTransmission::DoRun() {
  std::vector<uint8_t> nssToTest{1, 2};
  for (auto nss : nssToTest) {
    m_nss = nss;
    m_frequency = 5180;
    m_channelWidth = 20;
    m_expectedPpduDuration =
        (nss > 1) ? NanoSeconds(110400) : NanoSeconds(156800);
    RunOne();

    m_frequency = 5190;
    m_channelWidth = 40;
    m_expectedPpduDuration =
        (nss > 1) ? NanoSeconds(83200) : NanoSeconds(102400);
    RunOne();

    m_frequency = 5210;
    m_channelWidth = 80;
    m_expectedPpduDuration =
        (nss > 1) ? NanoSeconds(69600) : NanoSeconds(75200);
    RunOne();

    m_frequency = 5250;
    m_channelWidth = 160;
    m_expectedPpduDuration =
        (nss > 1) ? NanoSeconds(69600) : NanoSeconds(61600);
    RunOne();
  }

  Simulator::Destroy();
}

class TestUlMuMimoPhyTransmission : public TestCase {
public:
  TestUlMuMimoPhyTransmission();

private:
  void DoSetup() override;
  void DoTeardown() override;
  void DoRun() override;

  WifiTxVector GetTxVectorForHeTbPpdu(uint16_t txStaId, uint8_t nss,
                                      uint8_t bssColor) const;
  void SetTrigVector(const std::vector<uint16_t> &staIds, uint8_t bssColor);
  void SendHeTbPpdu(uint16_t txStaId, uint8_t nss, std::size_t payloadSize,
                    uint64_t uid, uint8_t bssColor);

  void SendHeSuPpdu(uint16_t txStaId, std::size_t payloadSize, uint64_t uid,
                    uint8_t bssColor);

  void SetBssColor(Ptr<WifiPhy> phy, uint8_t bssColor);

  void RunOne();

  void CheckRxFromSta(uint16_t staId, uint32_t expectedSuccess,
                      uint32_t expectedFailures, uint32_t expectedBytes);

  void VerifyEventsCleared();

  void CheckPhyState(Ptr<MuMimoSpectrumWifiPhy> phy,
                     WifiPhyState expectedState);
  void DoCheckPhyState(Ptr<MuMimoSpectrumWifiPhy> phy,
                       WifiPhyState expectedState);

  void Reset();

  void RxSuccess(Ptr<const WifiPsdu> psdu, RxSignalInfo rxSignalInfo,
                 WifiTxVector txVector, std::vector<bool> statusPerMpdu);

  void RxFailure(Ptr<const WifiPsdu> psdu);

  void ScheduleTest(Time delay, const std::vector<uint16_t> &txStaIds,
                    WifiPhyState expectedStateAtEnd,
                    const std::vector<std::tuple<uint32_t, uint32_t, uint32_t>>
                        &expectedCountersPerSta);

  void LogScenario(const std::string &log) const;

  Ptr<MuMimoSpectrumWifiPhy> m_phyAp;
  std::vector<Ptr<MuMimoSpectrumWifiPhy>> m_phyStas;

  std::vector<uint32_t> m_countRxSuccessFromStas;
  std::vector<uint32_t> m_countRxFailureFromStas;
  std::vector<uint32_t> m_countRxBytesFromStas;

  Time m_delayStart;
  uint16_t m_frequency;
  uint16_t m_channelWidth;
  Time m_expectedPpduDuration;
};

TestUlMuMimoPhyTransmission::TestUlMuMimoPhyTransmission()
    : TestCase("UL MU-MIMO PHY test"), m_countRxSuccessFromStas{},
      m_countRxFailureFromStas{}, m_countRxBytesFromStas{},
      m_delayStart{Seconds(0)}, m_frequency{DEFAULT_FREQUENCY},
      m_channelWidth{DEFAULT_CHANNEL_WIDTH},
      m_expectedPpduDuration{NanoSeconds(271200)} {}

void TestUlMuMimoPhyTransmission::SendHeSuPpdu(uint16_t txStaId,
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

  Ptr<MuMimoSpectrumWifiPhy> phy =
      (txStaId == 0) ? m_phyAp : m_phyStas.at(txStaId - 1);
  phy->SetPpduUid(uid);
  phy->Send(psdus, txVector);
}

WifiTxVector TestUlMuMimoPhyTransmission::GetTxVectorForHeTbPpdu(
    uint16_t txStaId, uint8_t nss, uint8_t bssColor) const {
  WifiTxVector txVector =
      WifiTxVector(HePhy::GetHeMcs7(), 0, WIFI_PREAMBLE_HE_TB, 1600, 1, nss, 0,
                   m_channelWidth, false, false, false, bssColor);

  HeRu::RuSpec ru(HeRu::GetRuType(m_channelWidth), 1, true);
  txVector.SetRu(ru, txStaId);
  txVector.SetMode(HePhy::GetHeMcs7(), txStaId);
  txVector.SetNss(nss, txStaId);

  return txVector;
}

void TestUlMuMimoPhyTransmission::SetTrigVector(
    const std::vector<uint16_t> &staIds, uint8_t bssColor) {
  WifiTxVector txVector(HePhy::GetHeMcs7(), 0, WIFI_PREAMBLE_HE_TB, 1600, 1, 1,
                        0, m_channelWidth, false, false, false, bssColor);

  HeRu::RuSpec ru(HeRu::GetRuType(m_channelWidth), 1, true);
  for (auto staId : staIds) {
    txVector.SetRu(ru, staId);
    txVector.SetMode(HePhy::GetHeMcs7(), staId);
    txVector.SetNss(1, staId);
  }

  uint16_t length;
  std::tie(length, m_expectedPpduDuration) =
      HePhy::ConvertHeTbPpduDurationToLSigLength(
          m_expectedPpduDuration, txVector, m_phyAp->GetPhyBand());
  txVector.SetLength(length);
  auto hePhyAp = DynamicCast<HePhy>(m_phyAp->GetPhyEntity(WIFI_MOD_CLASS_HE));
  hePhyAp->SetTrigVector(txVector, m_expectedPpduDuration);
}

void TestUlMuMimoPhyTransmission::SendHeTbPpdu(uint16_t txStaId, uint8_t nss,
                                               std::size_t payloadSize,
                                               uint64_t uid, uint8_t bssColor) {
  NS_LOG_FUNCTION(this << txStaId << +nss << payloadSize << uid << +bssColor);
  WifiConstPsduMap psdus;

  WifiTxVector txVector = GetTxVectorForHeTbPpdu(txStaId, nss, bssColor);
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

  Ptr<MuMimoSpectrumWifiPhy> phy = m_phyStas.at(txStaId - 1);
  Time txDuration = phy->CalculateTxDuration(psdu->GetSize(), txVector,
                                             phy->GetPhyBand(), txStaId);
  txVector.SetLength(HePhy::ConvertHeTbPpduDurationToLSigLength(
                         txDuration, txVector, phy->GetPhyBand())
                         .first);

  phy->SetPpduUid(uid);
  phy->Send(psdus, txVector);
}

void TestUlMuMimoPhyTransmission::RxSuccess(Ptr<const WifiPsdu> psdu,
                                            RxSignalInfo rxSignalInfo,
                                            WifiTxVector txVector,
                                            std::vector<bool>) {
  NS_LOG_FUNCTION(this << *psdu << psdu->GetAddr2()
                       << RatioToDb(rxSignalInfo.snr) << txVector);
  NS_TEST_ASSERT_MSG_EQ((RatioToDb(rxSignalInfo.snr) > 0), true,
                        "Incorrect SNR value");
  for (std::size_t index = 0; index < m_countRxSuccessFromStas.size();
       ++index) {
    std::ostringstream addr;
    addr << "00:00:00:00:00:0" << index + 1;
    if (psdu->GetAddr2() == Mac48Address(addr.str().c_str())) {
      m_countRxSuccessFromStas.at(index)++;
      m_countRxBytesFromStas.at(index) += (psdu->GetSize() - 30);
      break;
    }
  }
}

void TestUlMuMimoPhyTransmission::RxFailure(Ptr<const WifiPsdu> psdu) {
  NS_LOG_FUNCTION(this << *psdu << psdu->GetAddr2());
  for (std::size_t index = 0; index < m_countRxFailureFromStas.size();
       ++index) {
    std::ostringstream addr;
    addr << "00:00:00:00:00:0" << index + 1;
    if (psdu->GetAddr2() == Mac48Address(addr.str().c_str())) {
      m_countRxFailureFromStas.at(index)++;
      break;
    }
  }
}

void TestUlMuMimoPhyTransmission::CheckRxFromSta(uint16_t staId,
                                                 uint32_t expectedSuccess,
                                                 uint32_t expectedFailures,
                                                 uint32_t expectedBytes) {
  NS_LOG_FUNCTION(this << staId << expectedSuccess << expectedFailures
                       << expectedBytes);
  NS_TEST_ASSERT_MSG_EQ(m_countRxSuccessFromStas[staId - 1], expectedSuccess,
                        "The number of successfully received packets from STA "
                            << staId << " is not correct!");
  NS_TEST_ASSERT_MSG_EQ(
      m_countRxFailureFromStas[staId - 1], expectedFailures,
      "The number of unsuccessfully received packets from STA "
          << staId << " is not correct!");
  NS_TEST_ASSERT_MSG_EQ(m_countRxBytesFromStas[staId - 1], expectedBytes,
                        "The number of bytes received from STA "
                            << staId << " is not correct!");
}

void TestUlMuMimoPhyTransmission::VerifyEventsCleared() {
  NS_TEST_ASSERT_MSG_EQ(m_phyAp->GetCurrentEvent(), nullptr,
                        "m_currentEvent for AP was not cleared");
  std::size_t sta = 1;
  for (auto &phy : m_phyStas) {
    NS_TEST_ASSERT_MSG_EQ(phy->GetCurrentEvent(), nullptr,
                          "m_currentEvent for STA " << sta
                                                    << " was not cleared");
    sta++;
  }
}

void TestUlMuMimoPhyTransmission::CheckPhyState(Ptr<MuMimoSpectrumWifiPhy> phy,
                                                WifiPhyState expectedState) {
  Simulator::ScheduleNow(&TestUlMuMimoPhyTransmission::DoCheckPhyState, this,
                         phy, expectedState);
}

void TestUlMuMimoPhyTransmission::DoCheckPhyState(
    Ptr<MuMimoSpectrumWifiPhy> phy, WifiPhyState expectedState) {
  WifiPhyState currentState;
  PointerValue ptr;
  phy->GetAttribute("State", ptr);
  Ptr<WifiPhyStateHelper> state =
      DynamicCast<WifiPhyStateHelper>(ptr.Get<WifiPhyStateHelper>());
  currentState = state->GetState();
  NS_LOG_FUNCTION(this << currentState << expectedState);
  NS_TEST_ASSERT_MSG_EQ(currentState, expectedState,
                        "PHY State "
                            << currentState << " does not match expected state "
                            << expectedState << " at " << Simulator::Now());
}

void TestUlMuMimoPhyTransmission::Reset() {
  for (auto &counter : m_countRxSuccessFromStas) {
    counter = 0;
  }
  for (auto &counter : m_countRxFailureFromStas) {
    counter = 0;
  }
  for (auto &counter : m_countRxBytesFromStas) {
    counter = 0;
  }
  for (auto &phy : m_phyStas) {
    phy->SetPpduUid(0);
    phy->SetTriggerFrameUid(0);
  }
  SetBssColor(m_phyAp, 0);
}

void TestUlMuMimoPhyTransmission::SetBssColor(Ptr<WifiPhy> phy,
                                              uint8_t bssColor) {
  Ptr<WifiNetDevice> device = DynamicCast<WifiNetDevice>(phy->GetDevice());
  Ptr<HeConfiguration> heConfiguration = device->GetHeConfiguration();
  heConfiguration->SetAttribute("BssColor", UintegerValue(bssColor));
}

void TestUlMuMimoPhyTransmission::DoSetup() {

  Ptr<MultiModelSpectrumChannel> spectrumChannel =
      CreateObject<MultiModelSpectrumChannel>();
  Ptr<ConstantSpeedPropagationDelayModel> delayModel =
      CreateObject<ConstantSpeedPropagationDelayModel>();
  spectrumChannel->SetPropagationDelayModel(delayModel);

  Ptr<Node> apNode = CreateObject<Node>();
  Ptr<WifiNetDevice> apDev = CreateObject<WifiNetDevice>();
  apDev->SetStandard(WIFI_STANDARD_80211ax);
  Ptr<ApWifiMac> apMac = CreateObject<ApWifiMac>();
  apMac->SetAttribute("BeaconGeneration", BooleanValue(false));
  apDev->SetMac(apMac);
  m_phyAp = CreateObject<MuMimoSpectrumWifiPhy>(0);
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
      MakeCallback(&TestUlMuMimoPhyTransmission::RxSuccess, this));
  m_phyAp->SetReceiveErrorCallback(
      MakeCallback(&TestUlMuMimoPhyTransmission::RxFailure, this));
  apDev->SetPhy(m_phyAp);
  apNode->AddDevice(apDev);

  for (std::size_t i = 1; i <= 4; ++i) {
    Ptr<Node> staNode = CreateObject<Node>();
    Ptr<WifiNetDevice> staDev = CreateObject<WifiNetDevice>();
    staDev->SetStandard(WIFI_STANDARD_80211ax);
    Ptr<MuMimoSpectrumWifiPhy> phy = CreateObject<MuMimoSpectrumWifiPhy>(i);
    staDev->SetHeConfiguration(CreateObject<HeConfiguration>());
    Ptr<InterferenceHelper> staInterferenceHelper =
        CreateObject<InterferenceHelper>();
    phy->SetInterferenceHelper(staInterferenceHelper);
    Ptr<ErrorRateModel> staErrorModel = CreateObject<NistErrorRateModel>();
    phy->SetErrorRateModel(staErrorModel);
    phy->SetDevice(staDev);
    phy->AddChannel(spectrumChannel);
    phy->ConfigureStandard(WIFI_STANDARD_80211ax);
    phy->SetAttribute("TxGain", DoubleValue(1.0));
    phy->SetAttribute("TxPowerStart", DoubleValue(16.0));
    phy->SetAttribute("TxPowerEnd", DoubleValue(16.0));
    phy->SetAttribute("PowerDensityLimit", DoubleValue(100.0));
    phy->SetAttribute("RxGain", DoubleValue(2.0));
    staDev->SetPhy(phy);
    staNode->AddDevice(staDev);
    m_phyStas.push_back(phy);
    m_countRxSuccessFromStas.push_back(0);
    m_countRxFailureFromStas.push_back(0);
    m_countRxBytesFromStas.push_back(0);
  }
}

void TestUlMuMimoPhyTransmission::DoTeardown() {
  for (auto &phy : m_phyStas) {
    phy->Dispose();
    phy = nullptr;
  }
}

void TestUlMuMimoPhyTransmission::LogScenario(const std::string &log) const {
  NS_LOG_INFO(log);
}

void TestUlMuMimoPhyTransmission::ScheduleTest(
    Time delay, const std::vector<uint16_t> &txStaIds,
    WifiPhyState expectedStateAtEnd,
    const std::vector<std::tuple<uint32_t, uint32_t, uint32_t>>
        &expectedCountersPerSta) {
  static uint64_t uid = 0;

  Simulator::Schedule(delay - MilliSeconds(10),
                      &TestUlMuMimoPhyTransmission::SendHeSuPpdu, this, 0, 50,
                      ++uid, 0);

  Simulator::Schedule(delay, &TestUlMuMimoPhyTransmission::SetTrigVector, this,
                      txStaIds, 0);

  uint16_t payloadSize = 1000;
  std::size_t index = 0;
  for (auto txStaId : txStaIds) {
    Simulator::Schedule(delay + (index * m_delayStart),
                        &TestUlMuMimoPhyTransmission::SendHeTbPpdu, this,
                        txStaId, 1, payloadSize, uid, 0);
    payloadSize++;
    index++;
  }

  Simulator::Schedule(delay + m_expectedPpduDuration - NanoSeconds(1),
                      &TestUlMuMimoPhyTransmission::CheckPhyState, this,
                      m_phyAp, WifiPhyState::RX);
  Simulator::Schedule(delay + m_expectedPpduDuration +
                          (m_delayStart * expectedCountersPerSta.size()),
                      &TestUlMuMimoPhyTransmission::CheckPhyState, this,
                      m_phyAp, expectedStateAtEnd);

  delay += MilliSeconds(100);
  uint16_t staId = 1;
  for (const auto &expectedCounters : expectedCountersPerSta) {
    uint16_t expectedSuccessFromSta = std::get<0>(expectedCounters);
    uint16_t expectedFailuresFromSta = std::get<1>(expectedCounters);
    uint16_t expectedBytesFromSta = std::get<2>(expectedCounters);
    Simulator::Schedule(delay + (m_delayStart * (staId - 1)),
                        &TestUlMuMimoPhyTransmission::CheckRxFromSta, this,
                        staId, expectedSuccessFromSta, expectedFailuresFromSta,
                        expectedBytesFromSta);
    staId++;
  }

  Simulator::Schedule(delay, &TestUlMuMimoPhyTransmission::VerifyEventsCleared,
                      this);

  delay += MilliSeconds(100);
  Simulator::Schedule(delay, &TestUlMuMimoPhyTransmission::Reset, this);
}

void TestUlMuMimoPhyTransmission::RunOne() {
  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(1);
  int64_t streamNumber = 0;
  m_phyAp->AssignStreams(streamNumber);
  for (auto &phy : m_phyStas) {
    phy->AssignStreams(streamNumber);
  }

  auto channelNum = std::get<0>(*WifiPhyOperatingChannel::FindFirst(
      0, m_frequency, m_channelWidth, WIFI_STANDARD_80211ax,
      WIFI_PHY_BAND_5GHZ));

  m_phyAp->SetOperatingChannel(
      WifiPhy::ChannelTuple{channelNum, m_channelWidth, WIFI_PHY_BAND_5GHZ, 0});
  for (auto &phy : m_phyStas) {
    phy->SetOperatingChannel(WifiPhy::ChannelTuple{channelNum, m_channelWidth,
                                                   WIFI_PHY_BAND_5GHZ, 0});
  }

  Time delay = Seconds(0.0);
  Simulator::Schedule(delay, &TestUlMuMimoPhyTransmission::Reset, this);
  delay += Seconds(1.0);

  Simulator::Schedule(delay, &TestUlMuMimoPhyTransmission::LogScenario, this,
                      "Reception of HE TB PPDUs using full BW MU-MIMO");
  ScheduleTest(delay, {1, 2, 3}, WifiPhyState::IDLE,
               {std::make_tuple(1, 0, 1000), std::make_tuple(1, 0, 1001),
                std::make_tuple(1, 0, 1002)});
  delay += Seconds(1.0);

  Simulator::Schedule(delay, &TestUlMuMimoPhyTransmission::LogScenario, this,
                      "Reception of HE TB PPDUs HE TB PPDUs using full BW "
                      "MU-MIMO with an HE SU "
                      "PPDU arriving during the 400 ns window");
  Simulator::Schedule(delay + NanoSeconds(150),
                      &TestUlMuMimoPhyTransmission::SendHeSuPpdu, this, 4, 1002,
                      2, 0);
  ScheduleTest(delay, {1, 2, 3}, WifiPhyState::IDLE,
               {std::make_tuple(0, 1, 0), std::make_tuple(0, 1, 0),
                std::make_tuple(0, 1, 0)});
  delay += Seconds(1.0);

  Simulator::Schedule(
      delay, &TestUlMuMimoPhyTransmission::LogScenario, this,
      "Reception of HE TB PPDUs using full BW MU-MIMO with an HE SU PPDU "
      "arriving during the HE portion");
  Simulator::Schedule(delay + MicroSeconds(40),
                      &TestUlMuMimoPhyTransmission::SendHeSuPpdu, this, 4, 1002,
                      2, 0);
  ScheduleTest(delay, {1, 2, 3}, WifiPhyState::CCA_BUSY,
               {std::make_tuple(0, 1, 0), std::make_tuple(0, 1, 0),
                std::make_tuple(0, 1, 0)});
  delay += Seconds(1.0);

  Simulator::Run();
}

void TestUlMuMimoPhyTransmission::DoRun() {
  std::vector<Time> startDelays{NanoSeconds(0), NanoSeconds(100)};

  for (const auto &delayStart : startDelays) {
    m_delayStart = delayStart;

    m_frequency = 5180;
    m_channelWidth = 20;
    m_expectedPpduDuration = NanoSeconds(163200);
    NS_LOG_DEBUG("Run UL MU-MIMO PHY transmission test for "
                 << m_channelWidth
                 << " MHz with delay between each HE TB PPDUs of "
                 << m_delayStart);
    RunOne();

    m_frequency = 5190;
    m_channelWidth = 40;
    m_expectedPpduDuration = NanoSeconds(105600);
    NS_LOG_DEBUG("Run UL MU-MIMO PHY transmission test for "
                 << m_channelWidth
                 << " MHz with delay between each HE TB PPDUs of "
                 << m_delayStart);
    RunOne();

    m_frequency = 5210;
    m_channelWidth = 80;
    m_expectedPpduDuration = NanoSeconds(76800);
    NS_LOG_DEBUG("Run UL MU-MIMO PHY transmission test for "
                 << m_channelWidth
                 << " MHz with delay between each HE TB PPDUs of "
                 << m_delayStart);
    RunOne();

    m_frequency = 5250;
    m_channelWidth = 160;
    m_expectedPpduDuration = NanoSeconds(62400);
    NS_LOG_DEBUG("Run UL MU-MIMO PHY transmission test for "
                 << m_channelWidth
                 << " MHz with delay between each HE TB PPDUs of "
                 << m_delayStart);
    RunOne();
  }

  Simulator::Destroy();
}

class WifiPhyMuMimoTestSuite : public TestSuite {
public:
  WifiPhyMuMimoTestSuite();
};

WifiPhyMuMimoTestSuite::WifiPhyMuMimoTestSuite()
    : TestSuite("wifi-phy-mu-mimo", UNIT) {
  AddTestCase(new TestDlMuTxVector, TestCase::QUICK);
  AddTestCase(new TestDlMuMimoPhyTransmission, TestCase::QUICK);
  AddTestCase(new TestUlMuMimoPhyTransmission, TestCase::QUICK);
}

static WifiPhyMuMimoTestSuite WifiPhyMuMimoTestSuite;
