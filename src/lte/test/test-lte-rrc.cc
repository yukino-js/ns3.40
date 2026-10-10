
#include <ns3/core-module.h>
#include <ns3/lte-module.h>
#include <ns3/mobility-module.h>
#include <ns3/network-module.h>

#include <cmath>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("LteRrcTest");

class LteRrcConnectionEstablishmentTestCase : public TestCase {
public:
  LteRrcConnectionEstablishmentTestCase(uint32_t nUes, uint32_t nBearers,
                                        uint32_t tConnBase,
                                        uint32_t tConnIncrPerUe,
                                        uint32_t delayDiscStart,
                                        bool errorExpected, bool useIdealRrc,
                                        bool admitRrcConnectionRequest,
                                        std::string description = "");

protected:
  void DoRun() override;
  uint32_t m_nUes;

  static std::string BuildNameString(uint32_t nUes, uint32_t nBearers,
                                     uint32_t tConnBase,
                                     uint32_t tConnIncrPerUe,
                                     uint32_t delayDiscStart, bool useIdealRrc,
                                     bool admitRrcConnectionRequest,
                                     std::string description = "");
  void Connect(Ptr<NetDevice> ueDevice, Ptr<NetDevice> enbDevice);
  void CheckConnected(Ptr<NetDevice> ueDevice, Ptr<NetDevice> enbDevice);
  void CheckNotConnected(Ptr<NetDevice> ueDevice, Ptr<NetDevice> enbDevice);
  void ConnectionEstablishedCallback(std::string context, uint64_t imsi,
                                     uint16_t cellId, uint16_t rnti);
  void ConnectionTimeoutCallback(std::string context, uint64_t imsi,
                                 uint16_t cellId, uint16_t rnti,
                                 uint8_t connEstFailCount);

  uint32_t m_nBearers;
  uint32_t m_tConnBase;
  uint32_t m_tConnIncrPerUe;
  uint32_t m_delayConnEnd;
  uint32_t m_delayDiscStart;
  uint32_t m_delayDiscEnd;
  bool m_useIdealRrc;
  bool m_admitRrcConnectionRequest;
  Ptr<LteHelper> m_lteHelper;

  std::map<uint64_t, bool> m_isConnectionEstablished;
};

std::string LteRrcConnectionEstablishmentTestCase::BuildNameString(
    uint32_t nUes, uint32_t nBearers, uint32_t tConnBase,
    uint32_t tConnIncrPerUe, uint32_t delayDiscStart, bool useIdealRrc,
    bool admitRrcConnectionRequest, std::string description) {
  std::ostringstream oss;
  oss << "nUes=" << nUes << ", nBearers=" << nBearers
      << ", tConnBase=" << tConnBase << ", tConnIncrPerUe=" << tConnIncrPerUe
      << ", delayDiscStart=" << delayDiscStart;

  if (useIdealRrc) {
    oss << ", ideal RRC";
  } else {
    oss << ", real RRC";
  }

  if (admitRrcConnectionRequest) {
    oss << ", admitRrcConnectionRequest = true";
  } else {
    oss << ", admitRrcConnectionRequest = false";
  }

  if (!description.empty()) {
    oss << ", " << description;
  }

  return oss.str();
}

LteRrcConnectionEstablishmentTestCase::LteRrcConnectionEstablishmentTestCase(
    uint32_t nUes, uint32_t nBearers, uint32_t tConnBase,
    uint32_t tConnIncrPerUe, uint32_t delayDiscStart, bool errorExpected,
    bool useIdealRrc, bool admitRrcConnectionRequest, std::string description)
    : TestCase(BuildNameString(nUes, nBearers, tConnBase, tConnIncrPerUe,
                               delayDiscStart, useIdealRrc,
                               admitRrcConnectionRequest, description)),
      m_nUes(nUes), m_nBearers(nBearers), m_tConnBase(tConnBase),
      m_tConnIncrPerUe(tConnIncrPerUe),

      m_delayDiscStart(delayDiscStart), m_delayDiscEnd(10),
      m_useIdealRrc(useIdealRrc),
      m_admitRrcConnectionRequest(admitRrcConnectionRequest) {
  NS_LOG_FUNCTION(this << GetName());

  double dsi = 90;
  double nRaAttempts = 0;
  if (nUes <= 20) {
    nRaAttempts += 5;
  } else {
    NS_ASSERT(nUes <= 50);
    nRaAttempts += 10;
  }

  nRaAttempts += std::ceil(nUes / 4.0);
  double dra = nRaAttempts * 7;
  double dce = 10.0 + (2.0 * nUes) / 4.0;
  if (errorExpected) {
    dce += dsi + dce;
  }
  double nCrs;
  if (nUes <= 2) {
    nCrs = 0;
  } else if (nUes <= 5) {
    nCrs = 1;
  } else if (nUes <= 10) {
    nCrs = 2;
  } else if (nUes <= 20) {
    nCrs = 3;
  } else {
    nCrs = 4;
  }
  double dcr = (10.0 + (2.0 * nUes) / 4.0) * (m_nBearers + nCrs);

  m_delayConnEnd = round(dsi + dra + dce + dcr);
  NS_LOG_LOGIC(this << " " << GetName() << " dsi=" << dsi << " dra=" << dra
                    << " dce=" << dce << " dcr=" << dcr
                    << " m_delayConnEnd=" << m_delayConnEnd);
}

void LteRrcConnectionEstablishmentTestCase::DoRun() {
  NS_LOG_FUNCTION(this << GetName());
  Config::Reset();

  if (m_nUes < 25) {
    Config::SetDefault("ns3::LteEnbRrc::SrsPeriodicity", UintegerValue(40));
  } else if (m_nUes < 60) {
    Config::SetDefault("ns3::LteEnbRrc::SrsPeriodicity", UintegerValue(80));
  } else if (m_nUes < 120) {
    Config::SetDefault("ns3::LteEnbRrc::SrsPeriodicity", UintegerValue(160));
  } else {
    Config::SetDefault("ns3::LteEnbRrc::SrsPeriodicity", UintegerValue(320));
  }

  m_lteHelper = CreateObject<LteHelper>();
  m_lteHelper->SetAttribute("UseIdealRrc", BooleanValue(m_useIdealRrc));

  NodeContainer enbNodes;
  NodeContainer ueNodes;

  enbNodes.Create(1);
  ueNodes.Create(m_nUes);

  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(enbNodes);
  mobility.Install(ueNodes);

  int64_t stream = 1;
  NetDeviceContainer enbDevs;
  enbDevs = m_lteHelper->InstallEnbDevice(enbNodes);
  stream += m_lteHelper->AssignStreams(enbDevs, stream);

  NetDeviceContainer ueDevs;
  ueDevs = m_lteHelper->InstallUeDevice(ueNodes);
  stream += m_lteHelper->AssignStreams(ueDevs, stream);

  for (auto it = enbDevs.Begin(); it != enbDevs.End(); ++it) {
    Ptr<LteEnbRrc> enbRrc = (*it)->GetObject<LteEnbNetDevice>()->GetRrc();
    enbRrc->SetAttribute("AdmitRrcConnectionRequest",
                         BooleanValue(m_admitRrcConnectionRequest));
  }

  uint32_t i = 0;
  uint32_t tmax = 0;
  for (auto it = ueDevs.Begin(); it != ueDevs.End(); ++it) {
    Ptr<NetDevice> ueDevice = *it;
    Ptr<NetDevice> enbDevice = enbDevs.Get(0);
    Ptr<LteUeNetDevice> ueLteDevice = ueDevice->GetObject<LteUeNetDevice>();

    uint32_t tc = m_tConnBase + m_tConnIncrPerUe * i;
    uint32_t tcc = tc + m_delayConnEnd;
    uint32_t td = tcc + m_delayDiscStart;
    uint32_t tcd = td + m_delayDiscEnd;
    tmax = std::max(tmax, tcd);

    Simulator::Schedule(MilliSeconds(tc),
                        &LteRrcConnectionEstablishmentTestCase::Connect, this,
                        ueDevice, enbDevice);

    Simulator::Schedule(MilliSeconds(tcc),
                        &LteRrcConnectionEstablishmentTestCase::CheckConnected,
                        this, *it, enbDevice);

    uint64_t imsi = ueLteDevice->GetImsi();
    m_isConnectionEstablished[imsi] = false;

    ++i;
  }

  Config::Connect(
      "/NodeList/*/DeviceList/*/LteUeRrc/ConnectionEstablished",
      MakeCallback(
          &LteRrcConnectionEstablishmentTestCase::ConnectionEstablishedCallback,
          this));
  Config::Connect(
      "/NodeList/*/DeviceList/*/LteUeRrc/ConnectionTimeout",
      MakeCallback(
          &LteRrcConnectionEstablishmentTestCase::ConnectionTimeoutCallback,
          this));

  Simulator::Stop(MilliSeconds(tmax + 1));

  Simulator::Run();

  Simulator::Destroy();
}

void LteRrcConnectionEstablishmentTestCase::Connect(Ptr<NetDevice> ueDevice,
                                                    Ptr<NetDevice> enbDevice) {
  NS_LOG_FUNCTION(this);
  m_lteHelper->Attach(ueDevice, enbDevice);

  for (uint32_t b = 0; b < m_nBearers; ++b) {
    EpsBearer::Qci q = EpsBearer::NGBR_VIDEO_TCP_DEFAULT;
    EpsBearer bearer(q);
    m_lteHelper->ActivateDataRadioBearer(ueDevice, bearer);
  }
}

void LteRrcConnectionEstablishmentTestCase::CheckConnected(
    Ptr<NetDevice> ueDevice, Ptr<NetDevice> enbDevice) {
  Ptr<LteUeNetDevice> ueLteDevice = ueDevice->GetObject<LteUeNetDevice>();
  Ptr<LteUeRrc> ueRrc = ueLteDevice->GetRrc();
  const uint64_t imsi = ueLteDevice->GetImsi();
  const uint16_t rnti = ueRrc->GetRnti();
  NS_LOG_FUNCTION(this << imsi << rnti);
  NS_ASSERT_MSG(m_isConnectionEstablished.find(imsi) !=
                    m_isConnectionEstablished.end(),
                "Invalid IMSI " << imsi);

  if (!m_admitRrcConnectionRequest) {
    NS_TEST_ASSERT_MSG_EQ(m_isConnectionEstablished[imsi], false,
                          "Connection with RNTI "
                              << rnti << " should have been rejected");
    return;
  }

  NS_TEST_ASSERT_MSG_EQ(m_isConnectionEstablished[imsi], true,
                        "RNTI " << rnti << " fails to establish connection");
  NS_TEST_ASSERT_MSG_EQ(ueRrc->GetState(), LteUeRrc::CONNECTED_NORMALLY,
                        "RNTI " << rnti
                                << " is not at CONNECTED_NORMALLY state");

  Ptr<LteEnbNetDevice> enbLteDevice = enbDevice->GetObject<LteEnbNetDevice>();
  Ptr<LteEnbRrc> enbRrc = enbLteDevice->GetRrc();
  const bool hasContext = enbRrc->HasUeManager(rnti);

  if (hasContext) {
    Ptr<UeManager> ueManager = enbRrc->GetUeManager(rnti);
    NS_ASSERT(ueManager);
    NS_TEST_ASSERT_MSG_EQ(ueManager->GetState(), UeManager::CONNECTED_NORMALLY,
                          "The context of RNTI " << rnti
                                                 << " is in invalid state");
  } else {
    NS_LOG_WARN(this << " RNTI " << rnti << " thinks that it has"
                     << " established connection but the eNodeB thinks"
                     << " that the UE has failed on connection setup.");
  }

  uint16_t ueCellId = ueRrc->GetCellId();
  uint16_t enbCellId = enbLteDevice->GetCellId();
  uint16_t ueImsi = ueLteDevice->GetImsi();

  uint8_t ueDlBandwidth = ueRrc->GetDlBandwidth();
  uint8_t enbDlBandwidth = enbLteDevice->GetDlBandwidth();
  uint8_t ueUlBandwidth = ueRrc->GetUlBandwidth();
  uint8_t enbUlBandwidth = enbLteDevice->GetUlBandwidth();
  uint8_t ueDlEarfcn = ueRrc->GetDlEarfcn();
  uint8_t enbDlEarfcn = enbLteDevice->GetDlEarfcn();
  uint8_t ueUlEarfcn = ueRrc->GetUlEarfcn();
  uint8_t enbUlEarfcn = enbLteDevice->GetUlEarfcn();

  NS_TEST_ASSERT_MSG_EQ(ueCellId, enbCellId, "inconsistent CellId");
  NS_TEST_ASSERT_MSG_EQ(ueDlBandwidth, enbDlBandwidth,
                        "inconsistent DlBandwidth");
  NS_TEST_ASSERT_MSG_EQ(ueUlBandwidth, enbUlBandwidth,
                        "inconsistent UlBandwidth");
  NS_TEST_ASSERT_MSG_EQ(ueDlEarfcn, enbDlEarfcn, "inconsistent DlEarfcn");
  NS_TEST_ASSERT_MSG_EQ(ueUlEarfcn, enbUlEarfcn, "inconsistent UlEarfcn");

  if (hasContext) {
    Ptr<UeManager> ueManager = enbRrc->GetUeManager(rnti);
    NS_ASSERT(ueManager);
    UeManager::State state = ueManager->GetState();
    uint16_t enbImsi = ueManager->GetImsi();
    NS_TEST_ASSERT_MSG_EQ(ueImsi, enbImsi, "inconsistent Imsi");

    if (state == UeManager::CONNECTED_NORMALLY) {
      ObjectMapValue enbDataRadioBearerMapValue;
      ueManager->GetAttribute("DataRadioBearerMap", enbDataRadioBearerMapValue);
      NS_TEST_ASSERT_MSG_EQ(enbDataRadioBearerMapValue.GetN(), m_nBearers,
                            "wrong num bearers at eNB");
      ObjectMapValue ueDataRadioBearerMapValue;
      ueRrc->GetAttribute("DataRadioBearerMap", ueDataRadioBearerMapValue);
      NS_TEST_ASSERT_MSG_EQ(ueDataRadioBearerMapValue.GetN(), m_nBearers,
                            "wrong num bearers at UE");

      auto enbBearerIt = enbDataRadioBearerMapValue.Begin();
      auto ueBearerIt = ueDataRadioBearerMapValue.Begin();
      while (enbBearerIt != enbDataRadioBearerMapValue.End() &&
             ueBearerIt != ueDataRadioBearerMapValue.End()) {
        Ptr<LteDataRadioBearerInfo> enbDrbInfo =
            enbBearerIt->second->GetObject<LteDataRadioBearerInfo>();
        Ptr<LteDataRadioBearerInfo> ueDrbInfo =
            ueBearerIt->second->GetObject<LteDataRadioBearerInfo>();
        NS_TEST_ASSERT_MSG_EQ((uint32_t)enbDrbInfo->m_epsBearerIdentity,
                              (uint32_t)ueDrbInfo->m_epsBearerIdentity,
                              "epsBearerIdentity differs");
        NS_TEST_ASSERT_MSG_EQ((uint32_t)enbDrbInfo->m_drbIdentity,
                              (uint32_t)ueDrbInfo->m_drbIdentity,
                              "drbIdentity differs");
        NS_TEST_ASSERT_MSG_EQ((uint32_t)enbDrbInfo->m_logicalChannelIdentity,
                              (uint32_t)ueDrbInfo->m_logicalChannelIdentity,
                              "logicalChannelIdentity differs");

        ++enbBearerIt;
        ++ueBearerIt;
      }

      NS_ASSERT_MSG(enbBearerIt == enbDataRadioBearerMapValue.End(),
                    "too many bearers at eNB");
      NS_ASSERT_MSG(ueBearerIt == ueDataRadioBearerMapValue.End(),
                    "too many bearers at UE");
    }
  }
}

void LteRrcConnectionEstablishmentTestCase::CheckNotConnected(
    Ptr<NetDevice> ueDevice, Ptr<NetDevice> enbDevice) {
  Ptr<LteUeNetDevice> ueLteDevice = ueDevice->GetObject<LteUeNetDevice>();
  Ptr<LteUeRrc> ueRrc = ueLteDevice->GetRrc();
  const uint64_t imsi = ueLteDevice->GetImsi();
  const uint16_t rnti = ueRrc->GetRnti();
  NS_LOG_FUNCTION(this << imsi << rnti);
  NS_ASSERT_MSG(m_isConnectionEstablished.find(imsi) !=
                    m_isConnectionEstablished.end(),
                "Invalid IMSI " << imsi);

  bool ueStateIsConnectedNormally =
      (LteUeRrc::CONNECTED_NORMALLY == ueRrc->GetState());

  Ptr<LteEnbNetDevice> enbLteDevice = enbDevice->GetObject<LteEnbNetDevice>();
  Ptr<LteEnbRrc> enbRrc = enbLteDevice->GetRrc();
  const bool hasContext = enbRrc->HasUeManager(rnti);
  bool contextStateIsConnectedNormally = false;
  if (hasContext) {
    Ptr<UeManager> ueManager = enbRrc->GetUeManager(rnti);
    NS_ASSERT(ueManager);
    contextStateIsConnectedNormally =
        (UeManager::CONNECTED_NORMALLY == ueManager->GetState());
  }
  NS_TEST_ASSERT_MSG_EQ((!m_isConnectionEstablished[imsi] ||
                         !ueStateIsConnectedNormally || !hasContext ||
                         !contextStateIsConnectedNormally),
                        true,
                        "it should not happen that connection is completed "
                        "both at the UE and at the eNB side");
}

void LteRrcConnectionEstablishmentTestCase::ConnectionEstablishedCallback(
    std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti) {
  NS_LOG_FUNCTION(this << imsi << cellId);
  m_isConnectionEstablished[imsi] = true;
}

void LteRrcConnectionEstablishmentTestCase::ConnectionTimeoutCallback(
    std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti,
    uint8_t connEstFailCount) {
  NS_LOG_FUNCTION(this << imsi << cellId);
}

class LteRrcConnectionEstablishmentErrorTestCase
    : public LteRrcConnectionEstablishmentTestCase {
public:
  LteRrcConnectionEstablishmentErrorTestCase(Time jumpAwayTime,
                                             std::string description = "");

protected:
  void DoRun() override;

private:
  void JumpAway();
  void JumpBack();

  Time m_jumpAwayTime;
  Ptr<MobilityModel> m_ueMobility;
};

LteRrcConnectionEstablishmentErrorTestCase::
    LteRrcConnectionEstablishmentErrorTestCase(Time jumpAwayTime,
                                               std::string description)
    : LteRrcConnectionEstablishmentTestCase(1, 1, 0, 0, 1, true, false, true,
                                            description),
      m_jumpAwayTime(jumpAwayTime) {
  NS_LOG_FUNCTION(this << GetName());
}

void LteRrcConnectionEstablishmentErrorTestCase::DoRun() {
  NS_LOG_FUNCTION(this << GetName());
  Config::Reset();

  if (m_nUes < 25) {
    Config::SetDefault("ns3::LteEnbRrc::SrsPeriodicity", UintegerValue(40));
  } else if (m_nUes < 60) {
    Config::SetDefault("ns3::LteEnbRrc::SrsPeriodicity", UintegerValue(80));
  } else if (m_nUes < 120) {
    Config::SetDefault("ns3::LteEnbRrc::SrsPeriodicity", UintegerValue(160));
  } else {
    Config::SetDefault("ns3::LteEnbRrc::SrsPeriodicity", UintegerValue(320));
  }

  m_lteHelper = CreateObject<LteHelper>();
  m_lteHelper->SetAttribute("UseIdealRrc", BooleanValue(m_useIdealRrc));

  NodeContainer enbNodes;
  NodeContainer ueNodes;

  enbNodes.Create(4);
  ueNodes.Create(1);

  MobilityHelper mobility;
  mobility.Install(ueNodes);
  m_ueMobility = ueNodes.Get(0)->GetObject<MobilityModel>();

  Ptr<ListPositionAllocator> enbPosition =
      CreateObject<ListPositionAllocator>();
  enbPosition->Add(Vector(0, 0, 0));
  enbPosition->Add(Vector(100.0, 0, 0));
  enbPosition->Add(Vector(0, 100.0, 0));
  enbPosition->Add(Vector(100.0, 100.0, 0));
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator(enbPosition);
  mobility.Install(enbNodes);

  int64_t stream = 1;
  NetDeviceContainer enbDevs;
  enbDevs = m_lteHelper->InstallEnbDevice(enbNodes);
  stream += m_lteHelper->AssignStreams(enbDevs, stream);

  NetDeviceContainer ueDevs;
  ueDevs = m_lteHelper->InstallUeDevice(ueNodes);
  stream += m_lteHelper->AssignStreams(ueDevs, stream);

  for (auto it = enbDevs.Begin(); it != enbDevs.End(); ++it) {
    Ptr<LteEnbRrc> enbRrc = (*it)->GetObject<LteEnbNetDevice>()->GetRrc();
    enbRrc->SetAttribute("AdmitRrcConnectionRequest",
                         BooleanValue(m_admitRrcConnectionRequest));
  }

  uint32_t i = 0;
  uint32_t tmax = 0;
  for (auto it = ueDevs.Begin(); it != ueDevs.End(); ++it) {
    Ptr<NetDevice> ueDevice = *it;
    Ptr<NetDevice> enbDevice = enbDevs.Get(0);
    Ptr<LteUeNetDevice> ueLteDevice = ueDevice->GetObject<LteUeNetDevice>();

    uint32_t tc = m_tConnBase + m_tConnIncrPerUe * i;
    uint32_t tcc = tc + m_delayConnEnd;
    uint32_t td = tcc + m_delayDiscStart;
    uint32_t tcd = td + m_delayDiscEnd;
    tmax = std::max(tmax, tcd);

    Simulator::Schedule(MilliSeconds(tc),
                        &LteRrcConnectionEstablishmentErrorTestCase::Connect,
                        this, ueDevice, enbDevice);

    uint64_t imsi = ueLteDevice->GetImsi();
    m_isConnectionEstablished[imsi] = false;

    ++i;
  }

  Config::Connect("/NodeList/*/DeviceList/*/LteUeRrc/ConnectionEstablished",
                  MakeCallback(&LteRrcConnectionEstablishmentErrorTestCase::
                                   ConnectionEstablishedCallback,
                               this));
  Config::Connect("/NodeList/*/DeviceList/*/LteUeRrc/ConnectionTimeout",
                  MakeCallback(&LteRrcConnectionEstablishmentErrorTestCase::
                                   ConnectionTimeoutCallback,
                               this));

  Simulator::Schedule(m_jumpAwayTime,
                      &LteRrcConnectionEstablishmentErrorTestCase::JumpAway,
                      this);
  Simulator::Schedule(
      m_jumpAwayTime + MilliSeconds(99),
      &LteRrcConnectionEstablishmentErrorTestCase::CheckNotConnected, this,
      ueDevs.Get(0), enbDevs.Get(0));
  Simulator::Schedule(m_jumpAwayTime + MilliSeconds(100),
                      &LteRrcConnectionEstablishmentErrorTestCase::JumpBack,
                      this);

  Simulator::Stop(MilliSeconds(tmax + 1));

  Simulator::Run();

  Simulator::Destroy();
}

void LteRrcConnectionEstablishmentErrorTestCase::JumpAway() {
  NS_LOG_FUNCTION(this);
  m_ueMobility->SetPosition(Vector(100000.0, 100000.0, 0.0));
}

void LteRrcConnectionEstablishmentErrorTestCase::JumpBack() {
  NS_LOG_FUNCTION(this);
  m_ueMobility->SetPosition(Vector(0.0, 0.0, 0.0));
}

class LteRrcTestSuite : public TestSuite {
public:
  LteRrcTestSuite();
};

LteRrcTestSuite::LteRrcTestSuite() : TestSuite("lte-rrc", SYSTEM) {

  NS_LOG_FUNCTION(this);

  for (auto useIdealRrc : {false, true}) {

    AddTestCase(new LteRrcConnectionEstablishmentTestCase(1, 0, 0, 0, 1, false,
                                                          useIdealRrc, true),
                TestCase::EXTENSIVE);
    AddTestCase(new LteRrcConnectionEstablishmentTestCase(
                    1, 0, 100, 0, 1, false, useIdealRrc, true),
                TestCase::EXTENSIVE);
    AddTestCase(new LteRrcConnectionEstablishmentTestCase(1, 1, 0, 0, 1, false,
                                                          useIdealRrc, true),
                TestCase::EXTENSIVE);
    AddTestCase(new LteRrcConnectionEstablishmentTestCase(
                    1, 1, 100, 0, 1, false, useIdealRrc, true),
                TestCase::EXTENSIVE);
    AddTestCase(new LteRrcConnectionEstablishmentTestCase(1, 2, 0, 0, 1, false,
                                                          useIdealRrc, true),
                TestCase::EXTENSIVE);
    AddTestCase(new LteRrcConnectionEstablishmentTestCase(
                    1, 2, 100, 0, 1, false, useIdealRrc, true),
                TestCase::EXTENSIVE);
    AddTestCase(new LteRrcConnectionEstablishmentTestCase(2, 0, 20, 0, 1, false,
                                                          useIdealRrc, true),
                TestCase::EXTENSIVE);
    AddTestCase(new LteRrcConnectionEstablishmentTestCase(
                    2, 0, 20, 10, 1, false, useIdealRrc, true),
                TestCase::EXTENSIVE);
    AddTestCase(new LteRrcConnectionEstablishmentTestCase(
                    2, 0, 20, 100, 1, false, useIdealRrc, true),
                TestCase::EXTENSIVE);
    AddTestCase(new LteRrcConnectionEstablishmentTestCase(2, 1, 20, 0, 1, false,
                                                          useIdealRrc, true),
                TestCase::EXTENSIVE);
    AddTestCase(new LteRrcConnectionEstablishmentTestCase(
                    2, 1, 20, 10, 1, false, useIdealRrc, true),
                TestCase::EXTENSIVE);
    AddTestCase(new LteRrcConnectionEstablishmentTestCase(
                    2, 1, 20, 100, 1, false, useIdealRrc, true),
                TestCase::EXTENSIVE);
    AddTestCase(new LteRrcConnectionEstablishmentTestCase(2, 2, 20, 0, 1, false,
                                                          useIdealRrc, true),
                TestCase::EXTENSIVE);
    AddTestCase(new LteRrcConnectionEstablishmentTestCase(
                    2, 2, 20, 10, 1, false, useIdealRrc, true),
                TestCase::QUICK);
    AddTestCase(new LteRrcConnectionEstablishmentTestCase(
                    2, 2, 20, 100, 1, false, useIdealRrc, true),
                TestCase::EXTENSIVE);
    AddTestCase(new LteRrcConnectionEstablishmentTestCase(3, 0, 20, 0, 1, false,
                                                          useIdealRrc, true),
                TestCase::EXTENSIVE);
    AddTestCase(new LteRrcConnectionEstablishmentTestCase(4, 0, 20, 0, 1, false,
                                                          useIdealRrc, true),
                TestCase::EXTENSIVE);
    AddTestCase(new LteRrcConnectionEstablishmentTestCase(
                    4, 0, 20, 300, 1, false, useIdealRrc, true),
                TestCase::EXTENSIVE);
    AddTestCase(new LteRrcConnectionEstablishmentTestCase(
                    20, 0, 10, 1, 1, false, useIdealRrc, true),
                TestCase::EXTENSIVE);
    AddTestCase(new LteRrcConnectionEstablishmentTestCase(50, 0, 0, 0, 1, false,
                                                          useIdealRrc, true),
                TestCase::EXTENSIVE);

    AddTestCase(new LteRrcConnectionEstablishmentTestCase(1, 0, 0, 0, 1, false,
                                                          useIdealRrc, false),
                TestCase::EXTENSIVE);
    AddTestCase(new LteRrcConnectionEstablishmentTestCase(
                    1, 2, 100, 0, 1, false, useIdealRrc, false),
                TestCase::EXTENSIVE);
    AddTestCase(new LteRrcConnectionEstablishmentTestCase(2, 0, 20, 0, 1, false,
                                                          useIdealRrc, false),
                TestCase::EXTENSIVE);
    AddTestCase(new LteRrcConnectionEstablishmentTestCase(2, 1, 20, 0, 1, false,
                                                          useIdealRrc, false),
                TestCase::QUICK);
    AddTestCase(new LteRrcConnectionEstablishmentTestCase(3, 0, 20, 0, 1, false,
                                                          useIdealRrc, false),
                TestCase::EXTENSIVE);
  }

  AddTestCase(new LteRrcConnectionEstablishmentErrorTestCase(
                  Seconds(0.020214), "failure at RRC Connection Request"),
              TestCase::QUICK);
  AddTestCase(new LteRrcConnectionEstablishmentErrorTestCase(
                  Seconds(0.025), "failure at RRC Connection Setup"),
              TestCase::QUICK);
  AddTestCase(new LteRrcConnectionEstablishmentErrorTestCase(
                  Seconds(0.030), "failure at RRC Connection Setup Complete"),
              TestCase::QUICK);
}

static LteRrcTestSuite g_lteRrcTestSuiteInstance;
