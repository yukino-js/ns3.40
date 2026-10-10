
#include "ap-wifi-mac.h"

#include "amsdu-subframe-header.h"
#include "channel-access-manager.h"
#include "mac-rx-middle.h"
#include "mac-tx-middle.h"
#include "mgt-headers.h"
#include "msdu-aggregator.h"
#include "qos-txop.h"
#include "reduced-neighbor-report.h"
#include "wifi-mac-queue-scheduler.h"
#include "wifi-mac-queue.h"
#include "wifi-net-device.h"
#include "wifi-phy.h"

#include "ns3/eht-configuration.h"
#include "ns3/eht-frame-exchange-manager.h"
#include "ns3/he-configuration.h"
#include "ns3/ht-configuration.h"
#include "ns3/log.h"
#include "ns3/multi-link-element.h"
#include "ns3/packet.h"
#include "ns3/pointer.h"
#include "ns3/random-variable-stream.h"
#include "ns3/simulator.h"
#include "ns3/string.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("ApWifiMac");

NS_OBJECT_ENSURE_REGISTERED(ApWifiMac);

TypeId ApWifiMac::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::ApWifiMac")
          .SetParent<WifiMac>()
          .SetGroupName("Wifi")
          .AddConstructor<ApWifiMac>()
          .AddAttribute("BeaconInterval", "Delay between two beacons",
                        TimeValue(MicroSeconds(102400)),
                        MakeTimeAccessor(&ApWifiMac::GetBeaconInterval,
                                         &ApWifiMac::SetBeaconInterval),
                        MakeTimeChecker())
          .AddAttribute("BeaconJitter",
                        "A uniform random variable to cause the initial beacon "
                        "starting time "
                        "(after simulation time 0) "
                        "to be distributed between 0 and the BeaconInterval.",
                        StringValue("ns3::UniformRandomVariable"),
                        MakePointerAccessor(&ApWifiMac::m_beaconJitter),
                        MakePointerChecker<UniformRandomVariable>())
          .AddAttribute("EnableBeaconJitter",
                        "If beacons are enabled, whether to jitter the initial "
                        "send event.",
                        BooleanValue(true),
                        MakeBooleanAccessor(&ApWifiMac::m_enableBeaconJitter),
                        MakeBooleanChecker())
          .AddAttribute("BeaconGeneration",
                        "Whether or not beacons are generated.",
                        BooleanValue(true),
                        MakeBooleanAccessor(&ApWifiMac::SetBeaconGeneration),
                        MakeBooleanChecker())
          .AddAttribute(
              "EnableNonErpProtection",
              "Whether or not protection mechanism should be used when non-ERP "
              "STAs "
              "are present within the BSS."
              "This parameter is only used when ERP is supported by the AP.",
              BooleanValue(true),
              MakeBooleanAccessor(&ApWifiMac::m_enableNonErpProtection),
              MakeBooleanChecker())
          .AddAttribute(
              "BsrLifetime",
              "Lifetime of Buffer Status Reports received from stations.",
              TimeValue(MilliSeconds(20)),
              MakeTimeAccessor(&ApWifiMac::m_bsrLifetime), MakeTimeChecker())
          .AddTraceSource("AssociatedSta",
                          "A station associated with this access point.",
                          MakeTraceSourceAccessor(&ApWifiMac::m_assocLogger),
                          "ns3::ApWifiMac::AssociationCallback")
          .AddTraceSource("DeAssociatedSta",
                          "A station lost association with this access point.",
                          MakeTraceSourceAccessor(&ApWifiMac::m_deAssocLogger),
                          "ns3::ApWifiMac::AssociationCallback");
  return tid;
}

ApWifiMac::ApWifiMac() : m_enableBeaconGeneration(false) {
  NS_LOG_FUNCTION(this);
  m_beaconTxop = CreateObject<Txop>(CreateObject<WifiMacQueue>(AC_BEACON));
  m_beaconTxop->SetTxMiddle(m_txMiddle);

  SetTypeOfStation(AP);
}

ApWifiMac::~ApWifiMac() { NS_LOG_FUNCTION(this); }

void ApWifiMac::DoDispose() {
  NS_LOG_FUNCTION(this);
  m_beaconTxop->Dispose();
  m_beaconTxop = nullptr;
  m_enableBeaconGeneration = false;
  WifiMac::DoDispose();
}

ApWifiMac::ApLinkEntity::~ApLinkEntity() {
  NS_LOG_FUNCTION_NOARGS();
  beaconEvent.Cancel();
}

std::unique_ptr<WifiMac::LinkEntity> ApWifiMac::CreateLinkEntity() const {
  return std::make_unique<ApLinkEntity>();
}

ApWifiMac::ApLinkEntity &ApWifiMac::GetLink(uint8_t linkId) const {
  return static_cast<ApLinkEntity &>(WifiMac::GetLink(linkId));
}

void ApWifiMac::ConfigureStandard(WifiStandard standard) {
  NS_LOG_FUNCTION(this << standard);
  WifiMac::ConfigureStandard(standard);
  m_beaconTxop->SetWifiMac(this);
  m_beaconTxop->SetAifsns(std::vector<uint8_t>(GetNLinks(), 1));
  m_beaconTxop->SetMinCws(std::vector<uint32_t>(GetNLinks(), 0));
  m_beaconTxop->SetMaxCws(std::vector<uint32_t>(GetNLinks(), 0));
  for (uint8_t linkId = 0; linkId < GetNLinks(); linkId++) {
    GetLink(linkId).channelAccessManager->Add(m_beaconTxop);
  }
}

Ptr<WifiMacQueue> ApWifiMac::GetTxopQueue(AcIndex ac) const {
  if (ac == AC_BEACON) {
    return m_beaconTxop->GetWifiMacQueue();
  }
  return WifiMac::GetTxopQueue(ac);
}

void ApWifiMac::SetBeaconGeneration(bool enable) {
  NS_LOG_FUNCTION(this << enable);
  for (uint8_t linkId = 0; linkId < GetNLinks(); ++linkId) {
    if (!enable) {
      GetLink(linkId).beaconEvent.Cancel();
    } else if (!m_enableBeaconGeneration) {
      GetLink(linkId).beaconEvent =
          Simulator::ScheduleNow(&ApWifiMac::SendOneBeacon, this, linkId);
    }
  }
  m_enableBeaconGeneration = enable;
}

Time ApWifiMac::GetBeaconInterval() const {
  NS_LOG_FUNCTION(this);
  return m_beaconInterval;
}

void ApWifiMac::SetLinkUpCallback(Callback<void> linkUp) {
  NS_LOG_FUNCTION(this << &linkUp);
  WifiMac::SetLinkUpCallback(linkUp);

  linkUp();
}

void ApWifiMac::SetBeaconInterval(Time interval) {
  NS_LOG_FUNCTION(this << interval);
  if ((interval.GetMicroSeconds() % 1024) != 0) {
    NS_FATAL_ERROR("beacon interval should be multiple of 1024us (802.11 time "
                   "unit), see IEEE "
                   "Std. 802.11-2012");
  }
  if (interval.GetMicroSeconds() > (1024 * 65535)) {
    NS_FATAL_ERROR("beacon interval should be smaller then or equal to 65535 * "
                   "1024us (802.11 time unit)");
  }
  m_beaconInterval = interval;
}

int64_t ApWifiMac::AssignStreams(int64_t stream) {
  NS_LOG_FUNCTION(this << stream);
  m_beaconJitter->SetStream(stream);
  return 1;
}

void ApWifiMac::UpdateShortSlotTimeEnabled(uint8_t linkId) {
  NS_LOG_FUNCTION(this << +linkId);
  auto &link = GetLink(linkId);
  if (GetErpSupported(linkId) && GetShortSlotTimeSupported() &&
      (link.numNonErpStations == 0)) {
    for (const auto &sta : link.staList) {
      if (!GetWifiRemoteStationManager(linkId)->GetShortSlotTimeSupported(
              sta.second)) {
        link.shortSlotTimeEnabled = false;
        return;
      }
    }
    link.shortSlotTimeEnabled = true;
  } else {
    link.shortSlotTimeEnabled = false;
  }
}

void ApWifiMac::UpdateShortPreambleEnabled(uint8_t linkId) {
  NS_LOG_FUNCTION(this << +linkId);
  auto &link = GetLink(linkId);
  if (GetErpSupported(linkId) &&
      GetWifiPhy(linkId)->GetShortPhyPreambleSupported()) {
    for (const auto &sta : link.staList) {
      if (!GetWifiRemoteStationManager(linkId)->GetErpOfdmSupported(
              sta.second) ||
          !GetWifiRemoteStationManager(linkId)->GetShortPreambleSupported(
              sta.second)) {
        link.shortPreambleEnabled = false;
        return;
      }
    }
    link.shortPreambleEnabled = true;
  } else {
    link.shortPreambleEnabled = false;
  }
}

void ApWifiMac::ForwardDown(Ptr<Packet> packet, Mac48Address from,
                            Mac48Address to) {
  NS_LOG_FUNCTION(this << packet << from << to);
  uint8_t tid = 0;

  if (GetQosSupported()) {
    tid = QosUtilsGetTidForPacket(packet);
    if (tid > 7) {
      tid = 0;
    }
  }

  ForwardDown(packet, from, to, tid);
}

void ApWifiMac::ForwardDown(Ptr<Packet> packet, Mac48Address from,
                            Mac48Address to, uint8_t tid) {
  NS_LOG_FUNCTION(this << packet << from << to << +tid);
  WifiMacHeader hdr;

  if (GetQosSupported()) {
    hdr.SetType(WIFI_MAC_QOSDATA);
    hdr.SetQosAckPolicy(WifiMacHeader::NORMAL_ACK);
    hdr.SetQosNoEosp();
    hdr.SetQosNoAmsdu();
    hdr.SetQosTxopLimit(0);
    hdr.SetQosTid(tid);
  } else {
    hdr.SetType(WIFI_MAC_DATA);
  }

  if (GetQosSupported()) {
    hdr.SetNoOrder();
  }

  std::list<Mac48Address> addr2Set;
  if (to.IsGroup()) {
    for (uint8_t linkId = 0; linkId < GetNLinks(); linkId++) {
      addr2Set.push_back(GetFrameExchangeManager(linkId)->GetAddress());
    }
  } else {
    addr2Set = {GetAddress()};
    auto linkId = IsAssociated(to);
    NS_ASSERT_MSG(linkId, "Station "
                              << to
                              << "is not associated, cannot send it a frame");
    if (GetNLinks() == 1 ||
        !GetWifiRemoteStationManager(*linkId)->GetMldAddress(to)) {
      addr2Set = {GetFrameExchangeManager(*linkId)->GetAddress()};
    }
  }

  for (const auto &addr2 : addr2Set) {
    hdr.SetAddr1(to);
    hdr.SetAddr2(addr2);
    hdr.SetAddr3(from);
    hdr.SetDsFrom();
    hdr.SetDsNotTo();

    if (GetQosSupported()) {
      NS_ASSERT(tid < 8);
      GetQosTxop(tid)->Queue(packet, hdr);
    } else {
      GetTxop()->Queue(packet, hdr);
    }
  }
}

bool ApWifiMac::CanForwardPacketsTo(Mac48Address to) const {
  return (to.IsGroup() || IsAssociated(to));
}

void ApWifiMac::Enqueue(Ptr<Packet> packet, Mac48Address to,
                        Mac48Address from) {
  NS_LOG_FUNCTION(this << packet << to << from);
  if (CanForwardPacketsTo(to)) {
    ForwardDown(packet, from, to);
  } else {
    NotifyTxDrop(packet);
  }
}

void ApWifiMac::Enqueue(Ptr<Packet> packet, Mac48Address to) {
  NS_LOG_FUNCTION(this << packet << to);
  Enqueue(packet, to, GetAddress());
}

bool ApWifiMac::SupportsSendFrom() const {
  NS_LOG_FUNCTION(this);
  return true;
}

AllSupportedRates ApWifiMac::GetSupportedRates(uint8_t linkId) const {
  NS_LOG_FUNCTION(this << +linkId);
  AllSupportedRates rates;
  for (const auto &mode : GetWifiPhy(linkId)->GetModeList()) {
    uint64_t modeDataRate =
        mode.GetDataRate(GetWifiPhy(linkId)->GetChannelWidth());
    NS_LOG_DEBUG("Adding supported rate of " << modeDataRate);
    rates.AddSupportedRate(modeDataRate);
    if (mode.IsMandatory() &&
        (mode.GetModulationClass() != WIFI_MOD_CLASS_HR_DSSS)) {
      NS_LOG_DEBUG("Adding basic mode " << mode.GetUniqueName());
      GetWifiRemoteStationManager(linkId)->AddBasicMode(mode);
    }
  }
  for (uint8_t j = 0; j < GetWifiRemoteStationManager(linkId)->GetNBasicModes();
       j++) {
    WifiMode mode = GetWifiRemoteStationManager(linkId)->GetBasicMode(j);
    uint64_t modeDataRate =
        mode.GetDataRate(GetWifiPhy(linkId)->GetChannelWidth());
    NS_LOG_DEBUG("Setting basic rate " << mode.GetUniqueName());
    rates.SetBasicRate(modeDataRate);
  }
  if (GetHtSupported()) {
    for (const auto &selector :
         GetWifiPhy(linkId)->GetBssMembershipSelectorList()) {
      rates.AddBssMembershipSelectorRate(selector);
    }
  }
  return rates;
}

DsssParameterSet ApWifiMac::GetDsssParameterSet(uint8_t linkId) const {
  NS_LOG_FUNCTION(this << +linkId);
  NS_ASSERT(GetDsssSupported(linkId));
  DsssParameterSet dsssParameters;
  dsssParameters.SetCurrentChannel(GetWifiPhy(linkId)->GetChannelNumber());
  return dsssParameters;
}

CapabilityInformation ApWifiMac::GetCapabilities(uint8_t linkId) const {
  NS_LOG_FUNCTION(this << +linkId);
  CapabilityInformation capabilities;
  capabilities.SetShortPreamble(GetLink(linkId).shortPreambleEnabled);
  capabilities.SetShortSlotTime(GetLink(linkId).shortSlotTimeEnabled);
  capabilities.SetEss();
  return capabilities;
}

ErpInformation ApWifiMac::GetErpInformation(uint8_t linkId) const {
  NS_LOG_FUNCTION(this << +linkId);
  NS_ASSERT(GetErpSupported(linkId));
  ErpInformation information;

  information.SetNonErpPresent(GetLink(linkId).numNonErpStations > 0);
  information.SetUseProtection(GetUseNonErpProtection(linkId));
  if (GetLink(linkId).shortPreambleEnabled) {
    information.SetBarkerPreambleMode(0);
  } else {
    information.SetBarkerPreambleMode(1);
  }

  return information;
}

EdcaParameterSet ApWifiMac::GetEdcaParameterSet(uint8_t linkId) const {
  NS_LOG_FUNCTION(this << +linkId);
  NS_ASSERT(GetQosSupported());
  EdcaParameterSet edcaParameters;

  Ptr<QosTxop> edca;
  Time txopLimit;

  edca = GetQosTxop(AC_BE);
  txopLimit = edca->GetTxopLimit(linkId);
  edcaParameters.SetBeAci(0);
  edcaParameters.SetBeCWmin(edca->GetMinCw(linkId));
  edcaParameters.SetBeCWmax(edca->GetMaxCw(linkId));
  edcaParameters.SetBeAifsn(edca->GetAifsn(linkId));
  edcaParameters.SetBeTxopLimit(
      static_cast<uint16_t>(txopLimit.GetMicroSeconds() / 32));

  edca = GetQosTxop(AC_BK);
  txopLimit = edca->GetTxopLimit(linkId);
  edcaParameters.SetBkAci(1);
  edcaParameters.SetBkCWmin(edca->GetMinCw(linkId));
  edcaParameters.SetBkCWmax(edca->GetMaxCw(linkId));
  edcaParameters.SetBkAifsn(edca->GetAifsn(linkId));
  edcaParameters.SetBkTxopLimit(
      static_cast<uint16_t>(txopLimit.GetMicroSeconds() / 32));

  edca = GetQosTxop(AC_VI);
  txopLimit = edca->GetTxopLimit(linkId);
  edcaParameters.SetViAci(2);
  edcaParameters.SetViCWmin(edca->GetMinCw(linkId));
  edcaParameters.SetViCWmax(edca->GetMaxCw(linkId));
  edcaParameters.SetViAifsn(edca->GetAifsn(linkId));
  edcaParameters.SetViTxopLimit(
      static_cast<uint16_t>(txopLimit.GetMicroSeconds() / 32));

  edca = GetQosTxop(AC_VO);
  txopLimit = edca->GetTxopLimit(linkId);
  edcaParameters.SetVoAci(3);
  edcaParameters.SetVoCWmin(edca->GetMinCw(linkId));
  edcaParameters.SetVoCWmax(edca->GetMaxCw(linkId));
  edcaParameters.SetVoAifsn(edca->GetAifsn(linkId));
  edcaParameters.SetVoTxopLimit(
      static_cast<uint16_t>(txopLimit.GetMicroSeconds() / 32));

  edcaParameters.SetQosInfo(0);

  return edcaParameters;
}

std::optional<MuEdcaParameterSet> ApWifiMac::GetMuEdcaParameterSet() const {
  NS_LOG_FUNCTION(this);
  NS_ASSERT(GetHeSupported());

  Ptr<HeConfiguration> heConfiguration = GetHeConfiguration();
  NS_ASSERT(heConfiguration);

  MuEdcaParameterSet muEdcaParameters;
  muEdcaParameters.SetQosInfo(0);

  UintegerValue uintegerValue;
  TimeValue timeValue;

  heConfiguration->GetAttribute("MuBeAifsn", uintegerValue);
  muEdcaParameters.SetMuAifsn(AC_BE, uintegerValue.Get());
  heConfiguration->GetAttribute("MuBeCwMin", uintegerValue);
  muEdcaParameters.SetMuCwMin(AC_BE, uintegerValue.Get());
  heConfiguration->GetAttribute("MuBeCwMax", uintegerValue);
  muEdcaParameters.SetMuCwMax(AC_BE, uintegerValue.Get());
  heConfiguration->GetAttribute("BeMuEdcaTimer", timeValue);
  muEdcaParameters.SetMuEdcaTimer(AC_BE, timeValue.Get());

  heConfiguration->GetAttribute("MuBkAifsn", uintegerValue);
  muEdcaParameters.SetMuAifsn(AC_BK, uintegerValue.Get());
  heConfiguration->GetAttribute("MuBkCwMin", uintegerValue);
  muEdcaParameters.SetMuCwMin(AC_BK, uintegerValue.Get());
  heConfiguration->GetAttribute("MuBkCwMax", uintegerValue);
  muEdcaParameters.SetMuCwMax(AC_BK, uintegerValue.Get());
  heConfiguration->GetAttribute("BkMuEdcaTimer", timeValue);
  muEdcaParameters.SetMuEdcaTimer(AC_BK, timeValue.Get());

  heConfiguration->GetAttribute("MuViAifsn", uintegerValue);
  muEdcaParameters.SetMuAifsn(AC_VI, uintegerValue.Get());
  heConfiguration->GetAttribute("MuViCwMin", uintegerValue);
  muEdcaParameters.SetMuCwMin(AC_VI, uintegerValue.Get());
  heConfiguration->GetAttribute("MuViCwMax", uintegerValue);
  muEdcaParameters.SetMuCwMax(AC_VI, uintegerValue.Get());
  heConfiguration->GetAttribute("ViMuEdcaTimer", timeValue);
  muEdcaParameters.SetMuEdcaTimer(AC_VI, timeValue.Get());

  heConfiguration->GetAttribute("MuVoAifsn", uintegerValue);
  muEdcaParameters.SetMuAifsn(AC_VO, uintegerValue.Get());
  heConfiguration->GetAttribute("MuVoCwMin", uintegerValue);
  muEdcaParameters.SetMuCwMin(AC_VO, uintegerValue.Get());
  heConfiguration->GetAttribute("MuVoCwMax", uintegerValue);
  muEdcaParameters.SetMuCwMax(AC_VO, uintegerValue.Get());
  heConfiguration->GetAttribute("VoMuEdcaTimer", timeValue);
  muEdcaParameters.SetMuEdcaTimer(AC_VO, timeValue.Get());

  auto timerNotNull = [&muEdcaParameters](uint8_t aci) {
    return !muEdcaParameters.GetMuEdcaTimer(aci).IsZero();
  };
  auto aci = {0, 1, 2, 3};
  if (std::all_of(aci.begin(), aci.end(), timerNotNull)) {
    return muEdcaParameters;
  }

  NS_ABORT_MSG_UNLESS(
      std::none_of(aci.begin(), aci.end(), timerNotNull),
      "MU EDCA Timers must be all zero if the IE is not advertised.");

  return std::nullopt;
}

std::optional<ReducedNeighborReport>
ApWifiMac::GetReducedNeighborReport(uint8_t linkId) const {
  NS_LOG_FUNCTION(this << +linkId);

  if (GetNLinks() <= 1) {
    return std::nullopt;
  }

  NS_ABORT_IF(!GetEhtSupported());
  ReducedNeighborReport rnr;

  for (uint8_t index = 0; index < GetNLinks(); ++index) {
    if (index != linkId) {
      rnr.AddNbrApInfoField();
      std::size_t nbrId = rnr.GetNNbrApInfoFields() - 1;
      rnr.SetOperatingChannel(nbrId, GetLink(index).phy->GetOperatingChannel());
      rnr.AddTbttInformationField(nbrId);
      rnr.SetBssid(nbrId, 0, GetLink(index).feManager->GetAddress());
      rnr.SetShortSsid(nbrId, 0, 0);
      rnr.SetBssParameters(nbrId, 0, 0);
      rnr.SetPsd20MHz(nbrId, 0, 0);
      rnr.SetMldParameters(nbrId, 0, 0, index, 0);
    }
  }
  return rnr;
}

MultiLinkElement ApWifiMac::GetMultiLinkElement(uint8_t linkId,
                                                WifiMacType frameType,
                                                const Mac48Address &to) {
  NS_LOG_FUNCTION(this << +linkId << frameType << to);
  NS_ABORT_IF(GetNLinks() == 1);

  MultiLinkElement mle(MultiLinkElement::BASIC_VARIANT);
  mle.SetMldMacAddress(GetAddress());
  mle.SetLinkIdInfo(linkId);
  mle.SetBssParamsChangeCount(0);

  auto ehtConfiguration = GetEhtConfiguration();
  NS_ASSERT(ehtConfiguration);

  if (BooleanValue emlsrActivated; ehtConfiguration->GetAttributeFailSafe(
                                       "EmlsrActivated", emlsrActivated) &&
                                   emlsrActivated.Get()) {
    mle.SetEmlsrSupported(true);
    TimeValue time;
    ehtConfiguration->GetAttribute("TransitionTimeout", time);
    mle.SetTransitionTimeout(time.Get());
  }

  if (frameType == WIFI_MAC_MGT_BEACON ||
      frameType == WIFI_MAC_MGT_PROBE_RESPONSE ||
      frameType == WIFI_MAC_MGT_ASSOCIATION_REQUEST ||
      frameType == WIFI_MAC_MGT_REASSOCIATION_REQUEST ||
      frameType == WIFI_MAC_MGT_ASSOCIATION_RESPONSE) {
    auto &mldCapabilities = mle.GetCommonInfoBasic().m_mldCapabilities;
    mldCapabilities.emplace();
    mldCapabilities->maxNSimultaneousLinks = GetNLinks() - 1;
    mldCapabilities->srsSupport = 0;
    EnumValue negSupport;
    ehtConfiguration->GetAttributeFailSafe("TidToLinkMappingNegSupport",
                                           negSupport);
    mldCapabilities->tidToLinkMappingSupport = negSupport.Get();
    mldCapabilities->freqSepForStrApMld = 0;
    mldCapabilities->aarSupport = 0;
  }

  if (auto staMldAddress =
          GetWifiRemoteStationManager(linkId)->GetMldAddress(to);
      (frameType == WIFI_MAC_MGT_ASSOCIATION_RESPONSE ||
       frameType == WIFI_MAC_MGT_REASSOCIATION_RESPONSE) &&
      staMldAddress.has_value()) {
    for (uint8_t i = 0; i < GetNLinks(); i++) {
      auto remoteStationManager = GetWifiRemoteStationManager(i);
      if (auto staAddress =
              remoteStationManager->GetAffiliatedStaAddress(*staMldAddress);
          i != linkId && staAddress.has_value() &&
          (remoteStationManager->IsWaitAssocTxOk(*staAddress) ||
           remoteStationManager->IsAssocRefused(*staAddress))) {
        mle.AddPerStaProfileSubelement();
        auto &perStaProfile =
            mle.GetPerStaProfile(mle.GetNPerStaProfileSubelements() - 1);
        perStaProfile.SetLinkId(i);
        perStaProfile.SetCompleteProfile();
        perStaProfile.SetStaMacAddress(
            GetFrameExchangeManager(i)->GetAddress());
        perStaProfile.SetAssocResponse(GetAssocResp(*staAddress, i));
      }
    }
  }

  return mle;
}

HtOperation ApWifiMac::GetHtOperation(uint8_t linkId) const {
  NS_LOG_FUNCTION(this << +linkId);
  NS_ASSERT(GetHtSupported());
  HtOperation operation;
  auto phy = GetWifiPhy(linkId);
  auto remoteStationManager = GetWifiRemoteStationManager(linkId);

  operation.SetPrimaryChannel(phy->GetPrimaryChannelNumber(20));
  operation.SetRifsMode(false);
  operation.SetNonGfHtStasPresent(true);
  if (phy->GetChannelWidth() > 20) {
    operation.SetSecondaryChannelOffset(1);
    operation.SetStaChannelWidth(1);
  }
  if (GetLink(linkId).numNonHtStations == 0) {
    operation.SetHtProtection(NO_PROTECTION);
  } else {
    operation.SetHtProtection(MIXED_MODE_PROTECTION);
  }
  uint64_t maxSupportedRate = 0;
  for (const auto &mcs : phy->GetMcsList(WIFI_MOD_CLASS_HT)) {
    uint8_t nss = (mcs.GetMcsValue() / 8) + 1;
    NS_ASSERT(nss > 0 && nss < 5);
    uint64_t dataRate = mcs.GetDataRate(
        phy->GetChannelWidth(),
        GetHtConfiguration()->GetShortGuardIntervalSupported() ? 400 : 800,
        nss);
    if (dataRate > maxSupportedRate) {
      maxSupportedRate = dataRate;
      NS_LOG_DEBUG("Updating maxSupportedRate to " << maxSupportedRate);
    }
  }
  uint8_t maxSpatialStream = phy->GetMaxSupportedTxSpatialStreams();
  auto mcsList = phy->GetMcsList(WIFI_MOD_CLASS_HT);
  uint8_t nMcs = mcsList.size();
  for (const auto &sta : GetLink(linkId).staList) {
    if (remoteStationManager->GetHtSupported(sta.second)) {
      uint64_t maxSupportedRateByHtSta = 0;
      auto itMcs = mcsList.begin();
      for (uint8_t j = 0;
           j <
           (std::min(nMcs, remoteStationManager->GetNMcsSupported(sta.second)));
           j++) {
        WifiMode mcs = *itMcs++;
        uint8_t nss = (mcs.GetMcsValue() / 8) + 1;
        NS_ASSERT(nss > 0 && nss < 5);
        uint64_t dataRate = mcs.GetDataRate(
            remoteStationManager->GetChannelWidthSupported(sta.second),
            remoteStationManager->GetShortGuardIntervalSupported(sta.second)
                ? 400
                : 800,
            nss);
        if (dataRate > maxSupportedRateByHtSta) {
          maxSupportedRateByHtSta = dataRate;
        }
      }
      if (maxSupportedRateByHtSta < maxSupportedRate) {
        maxSupportedRate = maxSupportedRateByHtSta;
      }
      if (remoteStationManager->GetNMcsSupported(sta.second) < nMcs) {
        nMcs = remoteStationManager->GetNMcsSupported(sta.second);
      }
      if (remoteStationManager->GetNumberOfSupportedStreams(sta.second) <
          maxSpatialStream) {
        maxSpatialStream =
            remoteStationManager->GetNumberOfSupportedStreams(sta.second);
      }
    }
  }
  operation.SetRxHighestSupportedDataRate(
      static_cast<uint16_t>(maxSupportedRate / 1e6));
  operation.SetTxMcsSetDefined(nMcs > 0);
  operation.SetTxMaxNSpatialStreams(maxSpatialStream);
  operation.SetObssNonHtStasPresent(0);
  operation.SetDualBeacon(0);
  operation.SetDualCtsProtection(0);
  operation.SetStbcBeacon(0);
  operation.SetLSigTxopProtectionFullSupport(0);
  operation.SetPcoActive(0);
  operation.SetPhase(0);
  operation.SetRxMcsBitmask(0);
  operation.SetTxRxMcsSetUnequal(0);
  operation.SetTxUnequalModulation(0);

  return operation;
}

VhtOperation ApWifiMac::GetVhtOperation(uint8_t linkId) const {
  NS_LOG_FUNCTION(this << +linkId);
  NS_ASSERT(GetVhtSupported(linkId));
  VhtOperation operation;
  auto phy = GetWifiPhy(linkId);
  auto remoteStationManager = GetWifiRemoteStationManager(linkId);

  const uint16_t bssBandwidth = phy->GetChannelWidth();
  operation.SetChannelWidth((bssBandwidth > 40) ? 1 : 0);
  operation.SetChannelCenterFrequencySegment0(
      (bssBandwidth == 160)
          ? phy->GetOperatingChannel().GetPrimaryChannelNumber(
                80, phy->GetStandard())
          : phy->GetChannelNumber());
  operation.SetChannelCenterFrequencySegment1(
      (bssBandwidth == 160) ? phy->GetChannelNumber() : 0);
  uint8_t maxSpatialStream = phy->GetMaxSupportedRxSpatialStreams();
  for (const auto &sta : GetLink(linkId).staList) {
    if (remoteStationManager->GetVhtSupported(sta.second)) {
      if (remoteStationManager->GetNumberOfSupportedStreams(sta.second) <
          maxSpatialStream) {
        maxSpatialStream =
            remoteStationManager->GetNumberOfSupportedStreams(sta.second);
      }
    }
  }
  for (uint8_t nss = 1; nss <= maxSpatialStream; nss++) {
    uint8_t maxMcs = 9;
    operation.SetMaxVhtMcsPerNss(nss, maxMcs);
  }

  return operation;
}

HeOperation ApWifiMac::GetHeOperation(uint8_t linkId) const {
  NS_LOG_FUNCTION(this << +linkId);
  NS_ASSERT(GetHeSupported());
  HeOperation operation;
  auto remoteStationManager = GetWifiRemoteStationManager(linkId);

  uint8_t maxSpatialStream =
      GetWifiPhy(linkId)->GetMaxSupportedRxSpatialStreams();
  for (const auto &sta : GetLink(linkId).staList) {
    if (remoteStationManager->GetHeSupported(sta.second)) {
      if (remoteStationManager->GetNumberOfSupportedStreams(sta.second) <
          maxSpatialStream) {
        maxSpatialStream =
            remoteStationManager->GetNumberOfSupportedStreams(sta.second);
      }
    }
  }
  for (uint8_t nss = 1; nss <= maxSpatialStream; nss++) {
    operation.SetMaxHeMcsPerNss(nss, 11);
  }
  operation.SetBssColor(GetHeConfiguration()->GetBssColor());

  return operation;
}

EhtOperation ApWifiMac::GetEhtOperation(uint8_t linkId) const {
  NS_LOG_FUNCTION(this << +linkId);
  NS_ASSERT(GetEhtSupported());
  EhtOperation operation;
  auto remoteStationManager = GetWifiRemoteStationManager(linkId);

  auto maxSpatialStream = GetWifiPhy(linkId)->GetMaxSupportedRxSpatialStreams();
  for (const auto &sta : GetLink(linkId).staList) {
    if (remoteStationManager->GetEhtSupported(sta.second)) {
      if (remoteStationManager->GetNumberOfSupportedStreams(sta.second) <
          maxSpatialStream) {
        maxSpatialStream =
            remoteStationManager->GetNumberOfSupportedStreams(sta.second);
      }
    }
  }
  operation.SetMaxRxNss(maxSpatialStream, 0, WIFI_EHT_MAX_MCS_INDEX);
  operation.SetMaxTxNss(maxSpatialStream, 0, WIFI_EHT_MAX_MCS_INDEX);
  return operation;
}

void ApWifiMac::SendProbeResp(Mac48Address to, uint8_t linkId) {
  NS_LOG_FUNCTION(this << to << +linkId);
  WifiMacHeader hdr;
  hdr.SetType(WIFI_MAC_MGT_PROBE_RESPONSE);
  hdr.SetAddr1(to);
  hdr.SetAddr2(GetLink(linkId).feManager->GetAddress());
  hdr.SetAddr3(GetLink(linkId).feManager->GetAddress());
  hdr.SetDsNotFrom();
  hdr.SetDsNotTo();
  Ptr<Packet> packet = Create<Packet>();
  MgtProbeResponseHeader probe;
  probe.Get<Ssid>() = GetSsid();
  auto supportedRates = GetSupportedRates(linkId);
  probe.Get<SupportedRates>() = supportedRates.rates;
  probe.Get<ExtendedSupportedRatesIE>() = supportedRates.extendedRates;
  probe.SetBeaconIntervalUs(GetBeaconInterval().GetMicroSeconds());
  probe.Capabilities() = GetCapabilities(linkId);
  GetWifiRemoteStationManager(linkId)->SetShortPreambleEnabled(
      GetLink(linkId).shortPreambleEnabled);
  GetWifiRemoteStationManager(linkId)->SetShortSlotTimeEnabled(
      GetLink(linkId).shortSlotTimeEnabled);
  if (GetDsssSupported(linkId)) {
    probe.Get<DsssParameterSet>() = GetDsssParameterSet(linkId);
  }
  if (GetErpSupported(linkId)) {
    probe.Get<ErpInformation>() = GetErpInformation(linkId);
  }
  if (GetQosSupported()) {
    probe.Get<EdcaParameterSet>() = GetEdcaParameterSet(linkId);
  }
  if (GetHtSupported()) {
    probe.Get<ExtendedCapabilities>() = GetExtendedCapabilities();
    probe.Get<HtCapabilities>() = GetHtCapabilities(linkId);
    probe.Get<HtOperation>() = GetHtOperation(linkId);
  }
  if (GetVhtSupported(linkId)) {
    probe.Get<VhtCapabilities>() = GetVhtCapabilities(linkId);
    probe.Get<VhtOperation>() = GetVhtOperation(linkId);
  }
  if (GetHeSupported()) {
    probe.Get<HeCapabilities>() = GetHeCapabilities(linkId);
    probe.Get<HeOperation>() = GetHeOperation(linkId);
    if (auto muEdcaParameterSet = GetMuEdcaParameterSet();
        muEdcaParameterSet.has_value()) {
      probe.Get<MuEdcaParameterSet>() = std::move(*muEdcaParameterSet);
    }
  }
  if (GetEhtSupported()) {
    probe.Get<EhtCapabilities>() = GetEhtCapabilities(linkId);
    probe.Get<EhtOperation>() = GetEhtOperation(linkId);

    if (GetNLinks() > 1) {
      if (auto rnr = GetReducedNeighborReport(linkId); rnr.has_value()) {
        probe.Get<ReducedNeighborReport>() = std::move(*rnr);
      }
      probe.Get<MultiLinkElement>() =
          GetMultiLinkElement(linkId, WIFI_MAC_MGT_PROBE_RESPONSE);
    }
  }
  packet->AddHeader(probe);

  if (!GetQosSupported()) {
    GetTxop()->Queue(packet, hdr);
  } else if (!GetWifiRemoteStationManager(linkId)->GetQosSupported(to)) {
    GetBEQueue()->Queue(packet, hdr);
  } else {
    GetVOQueue()->Queue(packet, hdr);
  }
}

MgtAssocResponseHeader ApWifiMac::GetAssocResp(Mac48Address to,
                                               uint8_t linkId) {
  MgtAssocResponseHeader assoc;
  StatusCode code;
  auto remoteStationManager = GetWifiRemoteStationManager(linkId);
  if (remoteStationManager->IsWaitAssocTxOk(to)) {
    code.SetSuccess();
  } else {
    NS_ABORT_IF(!remoteStationManager->IsAssocRefused(to));
    remoteStationManager->RecordDisassociated(to);
    code.SetFailure();
  }
  auto supportedRates = GetSupportedRates(linkId);
  assoc.Get<SupportedRates>() = supportedRates.rates;
  assoc.Get<ExtendedSupportedRatesIE>() = supportedRates.extendedRates;
  assoc.SetStatusCode(code);
  assoc.Capabilities() = GetCapabilities(linkId);
  if (GetQosSupported()) {
    assoc.Get<EdcaParameterSet>() = GetEdcaParameterSet(linkId);
  }
  if (GetHtSupported()) {
    assoc.Get<ExtendedCapabilities>() = GetExtendedCapabilities();
    assoc.Get<HtCapabilities>() = GetHtCapabilities(linkId);
    assoc.Get<HtOperation>() = GetHtOperation(linkId);
  }
  if (GetVhtSupported(linkId)) {
    assoc.Get<VhtCapabilities>() = GetVhtCapabilities(linkId);
    assoc.Get<VhtOperation>() = GetVhtOperation(linkId);
  }
  if (GetHeSupported()) {
    assoc.Get<HeCapabilities>() = GetHeCapabilities(linkId);
    assoc.Get<HeOperation>() = GetHeOperation(linkId);
    if (auto muEdcaParameterSet = GetMuEdcaParameterSet();
        muEdcaParameterSet.has_value()) {
      assoc.Get<MuEdcaParameterSet>() = std::move(*muEdcaParameterSet);
    }
  }
  if (GetEhtSupported()) {
    assoc.Get<EhtCapabilities>() = GetEhtCapabilities(linkId);
    assoc.Get<EhtOperation>() = GetEhtOperation(linkId);
  }
  return assoc;
}

ApWifiMac::LinkIdStaAddrMap
ApWifiMac::GetLinkIdStaAddrMap(MgtAssocResponseHeader &assoc,
                               const Mac48Address &to, uint8_t linkId) {
  std::map<uint8_t, Mac48Address> linkIdStaAddrMap;

  if (assoc.GetStatusCode().IsSuccess()) {
    linkIdStaAddrMap[linkId] = to;
  }

  if (const auto &mle = assoc.Get<MultiLinkElement>()) {
    const auto staMldAddress =
        GetWifiRemoteStationManager(linkId)->GetMldAddress(to);
    NS_ABORT_MSG_IF(!staMldAddress.has_value(),
                    "Sending a Multi-Link Element to a single link device");
    for (std::size_t idx = 0; idx < mle->GetNPerStaProfileSubelements();
         idx++) {
      auto &perStaProfile = mle->GetPerStaProfile(idx);
      if (perStaProfile.HasAssocResponse() &&
          perStaProfile.GetAssocResponse().GetStatusCode().IsSuccess()) {
        uint8_t otherLinkId = perStaProfile.GetLinkId();
        auto staAddress = GetWifiRemoteStationManager(otherLinkId)
                              ->GetAffiliatedStaAddress(*staMldAddress);
        NS_ABORT_MSG_IF(!staAddress.has_value(),
                        "No STA to associate with on link " << +otherLinkId);
        const auto [it, inserted] =
            linkIdStaAddrMap.insert({otherLinkId, *staAddress});
        NS_ABORT_MSG_IF(!inserted, "More than one Association Response to MLD "
                                       << *staMldAddress << " on link ID "
                                       << +otherLinkId);
      }
    }
  }

  return linkIdStaAddrMap;
}

void ApWifiMac::SetAid(MgtAssocResponseHeader &assoc,
                       const LinkIdStaAddrMap &linkIdStaAddrMap) {
  if (linkIdStaAddrMap.empty()) {
    return;
  }

  std::set<uint16_t> aids;
  std::map<uint8_t, uint16_t> linkIdAidMap;

  for (const auto &[id, staAddr] : linkIdStaAddrMap) {
    for (const auto &[aid, addr] : GetLink(id).staList) {
      if (addr == staAddr) {
        aids.insert(aid);
        linkIdAidMap[id] = aid;
        break;
      }
    }
  }

  while (!aids.empty()) {
    const uint16_t aid = *aids.begin();
    bool good = true;

    for (const auto &[id, staAddr] : linkIdStaAddrMap) {
      if (auto it = GetLink(id).staList.find(aid);
          it != GetLink(id).staList.end() && it->second != staAddr) {
        aids.erase(aids.begin());
        good = false;
        break;
      }
    }

    if (good) {
      break;
    }
  }

  uint16_t aid = 0;

  if (!aids.empty()) {
    aid = *aids.begin();
  } else {
    std::list<uint8_t> linkIds;
    std::transform(linkIdStaAddrMap.cbegin(), linkIdStaAddrMap.cend(),
                   std::back_inserter(linkIds), [](auto &&linkIdStaAddrPair) {
                     return linkIdStaAddrPair.first;
                   });
    aid = GetNextAssociationId(linkIds);
  }

  const auto &[linkId, staAddr] = *linkIdStaAddrMap.cbegin();
  m_aidToMldOrLinkAddress[aid] =
      GetWifiRemoteStationManager(linkId)->GetMldAddress(staAddr).value_or(
          staAddr);

  for (const auto &[id, staAddr] : linkIdStaAddrMap) {
    auto remoteStationManager = GetWifiRemoteStationManager(id);
    auto &link = GetLink(id);

    if (auto it = linkIdAidMap.find(id);
        it == linkIdAidMap.end() || it->second != aid) {
      link.staList.insert(std::make_pair(aid, staAddr));
      m_assocLogger(aid, staAddr);
      remoteStationManager->SetAssociationId(staAddr, aid);

      if (it == linkIdAidMap.end()) {
        if (remoteStationManager->GetDsssSupported(staAddr) &&
            !remoteStationManager->GetErpOfdmSupported(staAddr)) {
          link.numNonErpStations++;
        }
        if (!remoteStationManager->GetHtSupported(staAddr)) {
          link.numNonHtStations++;
        }
        UpdateShortSlotTimeEnabled(id);
        UpdateShortPreambleEnabled(id);
      } else {
        link.staList.erase(it->second);
      }
    }
  }

  if (assoc.GetStatusCode().IsSuccess()) {
    assoc.SetAssociationId(aid);
  }
  if (const auto &mle = assoc.Get<MultiLinkElement>()) {
    for (std::size_t idx = 0; idx < mle->GetNPerStaProfileSubelements();
         idx++) {
      if (const auto &perStaProfile = mle->GetPerStaProfile(idx);
          perStaProfile.HasAssocResponse() &&
          perStaProfile.GetAssocResponse().GetStatusCode().IsSuccess()) {
        perStaProfile.GetAssocResponse().SetAssociationId(aid);
      }
    }
  }
}

void ApWifiMac::SendAssocResp(Mac48Address to, bool isReassoc, uint8_t linkId) {
  NS_LOG_FUNCTION(this << to << isReassoc << +linkId);
  WifiMacHeader hdr;
  hdr.SetType(isReassoc ? WIFI_MAC_MGT_REASSOCIATION_RESPONSE
                        : WIFI_MAC_MGT_ASSOCIATION_RESPONSE);
  hdr.SetAddr1(to);
  hdr.SetAddr2(GetFrameExchangeManager(linkId)->GetAddress());
  hdr.SetAddr3(GetFrameExchangeManager(linkId)->GetAddress());
  hdr.SetDsNotFrom();
  hdr.SetDsNotTo();

  MgtAssocResponseHeader assoc = GetAssocResp(to, linkId);

  if (GetNLinks() > 1 &&
      GetWifiRemoteStationManager(linkId)->GetMldAddress(to).has_value()) {
    assoc.Get<MultiLinkElement>() =
        GetMultiLinkElement(linkId, hdr.GetType(), to);
  }

  auto linkIdStaAddrMap = GetLinkIdStaAddrMap(assoc, to, linkId);
  SetAid(assoc, linkIdStaAddrMap);

  Ptr<Packet> packet = Create<Packet>();
  packet->AddHeader(assoc);

  if (!GetQosSupported()) {
    GetTxop()->Queue(packet, hdr);
  } else if (!GetWifiRemoteStationManager(linkId)->GetQosSupported(to)) {
    GetBEQueue()->Queue(packet, hdr);
  } else {
    GetVOQueue()->Queue(packet, hdr);
  }
}

void ApWifiMac::SendOneBeacon(uint8_t linkId) {
  NS_LOG_FUNCTION(this << +linkId);
  auto &link = GetLink(linkId);
  WifiMacHeader hdr;
  hdr.SetType(WIFI_MAC_MGT_BEACON);
  hdr.SetAddr1(Mac48Address::GetBroadcast());
  hdr.SetAddr2(link.feManager->GetAddress());
  hdr.SetAddr3(link.feManager->GetAddress());
  hdr.SetDsNotFrom();
  hdr.SetDsNotTo();
  Ptr<Packet> packet = Create<Packet>();
  MgtBeaconHeader beacon;
  beacon.Get<Ssid>() = GetSsid();
  auto supportedRates = GetSupportedRates(linkId);
  beacon.Get<SupportedRates>() = supportedRates.rates;
  beacon.Get<ExtendedSupportedRatesIE>() = supportedRates.extendedRates;
  beacon.SetBeaconIntervalUs(GetBeaconInterval().GetMicroSeconds());
  beacon.Capabilities() = GetCapabilities(linkId);
  GetWifiRemoteStationManager(linkId)->SetShortPreambleEnabled(
      link.shortPreambleEnabled);
  GetWifiRemoteStationManager(linkId)->SetShortSlotTimeEnabled(
      link.shortSlotTimeEnabled);
  if (GetDsssSupported(linkId)) {
    beacon.Get<DsssParameterSet>() = GetDsssParameterSet(linkId);
  }
  if (GetErpSupported(linkId)) {
    beacon.Get<ErpInformation>() = GetErpInformation(linkId);
  }
  if (GetQosSupported()) {
    beacon.Get<EdcaParameterSet>() = GetEdcaParameterSet(linkId);
  }
  if (GetHtSupported()) {
    beacon.Get<ExtendedCapabilities>() = GetExtendedCapabilities();
    beacon.Get<HtCapabilities>() = GetHtCapabilities(linkId);
    beacon.Get<HtOperation>() = GetHtOperation(linkId);
  }
  if (GetVhtSupported(linkId)) {
    beacon.Get<VhtCapabilities>() = GetVhtCapabilities(linkId);
    beacon.Get<VhtOperation>() = GetVhtOperation(linkId);
  }
  if (GetHeSupported()) {
    beacon.Get<HeCapabilities>() = GetHeCapabilities(linkId);
    beacon.Get<HeOperation>() = GetHeOperation(linkId);
    if (auto muEdcaParameterSet = GetMuEdcaParameterSet();
        muEdcaParameterSet.has_value()) {
      beacon.Get<MuEdcaParameterSet>() = std::move(*muEdcaParameterSet);
    }
  }
  if (GetEhtSupported()) {
    beacon.Get<EhtCapabilities>() = GetEhtCapabilities(linkId);
    beacon.Get<EhtOperation>() = GetEhtOperation(linkId);

    if (GetNLinks() > 1) {
      if (auto rnr = GetReducedNeighborReport(linkId); rnr.has_value()) {
        beacon.Get<ReducedNeighborReport>() = std::move(*rnr);
      }
      beacon.Get<MultiLinkElement>() =
          GetMultiLinkElement(linkId, WIFI_MAC_MGT_BEACON);
    }
  }
  packet->AddHeader(beacon);

  m_beaconTxop->Queue(packet, hdr);
  link.beaconEvent = Simulator::Schedule(
      GetBeaconInterval(), &ApWifiMac::SendOneBeacon, this, linkId);

  if (GetErpSupported(linkId)) {
    if (link.shortSlotTimeEnabled) {
      GetWifiPhy(linkId)->SetSlot(MicroSeconds(9));
    } else {
      GetWifiPhy(linkId)->SetSlot(MicroSeconds(20));
    }
  }
}

void ApWifiMac::TxOk(Ptr<const WifiMpdu> mpdu) {
  NS_LOG_FUNCTION(this << *mpdu);
  const WifiMacHeader &hdr = mpdu->GetHeader();

  if (hdr.IsAssocResp() || hdr.IsReassocResp()) {
    auto linkId = GetLinkIdByAddress(hdr.GetAddr2());
    NS_ABORT_MSG_IF(!linkId.has_value(), "No link ID matching the TA");

    if (GetWifiRemoteStationManager(*linkId)->IsWaitAssocTxOk(hdr.GetAddr1())) {
      NS_LOG_DEBUG("AP=" << hdr.GetAddr2()
                         << " associated with STA=" << hdr.GetAddr1());
      GetWifiRemoteStationManager(*linkId)->RecordGotAssocTxOk(hdr.GetAddr1());
    }

    if (auto staMldAddress =
            GetWifiRemoteStationManager(*linkId)->GetMldAddress(hdr.GetAddr1());
        staMldAddress.has_value()) {
      for (uint8_t i = 0; i < GetNLinks(); i++) {
        auto stationManager = GetWifiRemoteStationManager(i);
        if (auto staAddress =
                stationManager->GetAffiliatedStaAddress(*staMldAddress);
            staAddress.has_value() && i != *linkId &&
            stationManager->IsWaitAssocTxOk(*staAddress)) {
          NS_LOG_DEBUG("AP=" << GetFrameExchangeManager(i)->GetAddress()
                             << " associated with STA=" << *staAddress);
          stationManager->RecordGotAssocTxOk(*staAddress);
          StaSwitchingToPsMode(*staAddress, i);
        }
      }

      ApplyTidLinkMapping(*staMldAddress, WifiDirection::DOWNLINK);
    }
  } else if (hdr.IsAction()) {
    if (auto [category, action] = WifiActionHeader::Peek(mpdu->GetPacket());
        category == WifiActionHeader::PROTECTED_EHT &&
        action.protectedEhtAction ==
            WifiActionHeader::PROTECTED_EHT_EML_OPERATING_MODE_NOTIFICATION) {
      if (auto eventIt = m_transitionTimeoutEvents.find(hdr.GetAddr1());
          eventIt != m_transitionTimeoutEvents.cend() &&
          eventIt->second.IsRunning()) {
        eventIt->second.PeekEventImpl()->Invoke();
        eventIt->second.Cancel();
      }
    }
  }
}

void ApWifiMac::TxFailed(WifiMacDropReason timeoutReason,
                         Ptr<const WifiMpdu> mpdu) {
  NS_LOG_FUNCTION(this << +timeoutReason << *mpdu);
  const WifiMacHeader &hdr = mpdu->GetHeader();

  if (hdr.IsAssocResp() || hdr.IsReassocResp()) {
    auto linkId = GetLinkIdByAddress(hdr.GetAddr2());
    NS_ABORT_MSG_IF(!linkId.has_value(), "No link ID matching the TA");

    if (GetWifiRemoteStationManager(*linkId)->IsWaitAssocTxOk(hdr.GetAddr1())) {
      NS_LOG_DEBUG("AP=" << hdr.GetAddr2()
                         << " association failed with STA=" << hdr.GetAddr1());
      GetWifiRemoteStationManager(*linkId)->RecordGotAssocTxFailed(
          hdr.GetAddr1());
    }

    if (auto staMldAddress =
            GetWifiRemoteStationManager(*linkId)->GetMldAddress(hdr.GetAddr1());
        staMldAddress.has_value()) {
      for (uint8_t i = 0; i < GetNLinks(); i++) {
        auto stationManager = GetWifiRemoteStationManager(i);
        if (auto staAddress =
                stationManager->GetAffiliatedStaAddress(*staMldAddress);
            staAddress.has_value() && i != *linkId &&
            stationManager->IsWaitAssocTxOk(*staAddress)) {
          NS_LOG_DEBUG("AP=" << GetFrameExchangeManager(i)->GetAddress()
                             << " association failed with STA=" << *staAddress);
          stationManager->RecordGotAssocTxFailed(*staAddress);
        }
      }
    }
  }
}

void ApWifiMac::ProcessPowerManagementFlag(Ptr<const WifiMpdu> mpdu,
                                           uint8_t linkId) {
  NS_LOG_FUNCTION(this << *mpdu << linkId);

  Mac48Address staAddr = mpdu->GetHeader().GetAddr2();
  bool staInPsMode = GetWifiRemoteStationManager(linkId)->IsInPsMode(staAddr);

  if (!staInPsMode && mpdu->GetHeader().IsPowerManagement()) {
    StaSwitchingToPsMode(staAddr, linkId);
  } else if (staInPsMode && !mpdu->GetHeader().IsPowerManagement()) {
    StaSwitchingToActiveModeOrDeassociated(staAddr, linkId);
  }
}

void ApWifiMac::StaSwitchingToPsMode(const Mac48Address &staAddr,
                                     uint8_t linkId) {
  NS_LOG_FUNCTION(this << staAddr << linkId);

  GetWifiRemoteStationManager(linkId)->SetPsMode(staAddr, true);

  NS_LOG_DEBUG("Block destination " << staAddr << " on link " << +linkId);
  auto staMldAddr =
      GetWifiRemoteStationManager(linkId)->GetMldAddress(staAddr).value_or(
          staAddr);
  BlockUnicastTxOnLinks(WifiQueueBlockedReason::POWER_SAVE_MODE, staMldAddr,
                        {linkId});
}

void ApWifiMac::StaSwitchingToActiveModeOrDeassociated(
    const Mac48Address &staAddr, uint8_t linkId) {
  NS_LOG_FUNCTION(this << staAddr << linkId);

  GetWifiRemoteStationManager(linkId)->SetPsMode(staAddr, false);

  if (GetWifiRemoteStationManager(linkId)->IsAssociated(staAddr)) {
    NS_LOG_DEBUG("Unblock destination " << staAddr << " on link " << +linkId);
    auto staMldAddr =
        GetWifiRemoteStationManager(linkId)->GetMldAddress(staAddr).value_or(
            staAddr);
    UnblockUnicastTxOnLinks(WifiQueueBlockedReason::POWER_SAVE_MODE, staMldAddr,
                            {linkId});
  }
}

std::optional<uint8_t>
ApWifiMac::IsAssociated(const Mac48Address &address) const {
  for (uint8_t linkId = 0; linkId < GetNLinks(); linkId++) {
    if (GetWifiRemoteStationManager(linkId)->IsAssociated(address)) {
      return linkId;
    }
  }
  NS_LOG_DEBUG(address << " is not associated");
  return std::nullopt;
}

Mac48Address
ApWifiMac::DoGetLocalAddress(const Mac48Address &remoteAddr) const {
  auto linkId = IsAssociated(remoteAddr);
  NS_ASSERT_MSG(linkId, remoteAddr << " is not associated");
  return GetFrameExchangeManager(*linkId)->GetAddress();
}

std::optional<Mac48Address>
ApWifiMac::GetMldOrLinkAddressByAid(uint16_t aid) const {
  if (const auto staIt = m_aidToMldOrLinkAddress.find(aid);
      staIt != m_aidToMldOrLinkAddress.cend()) {
    return staIt->second;
  }
  return std::nullopt;
}

void ApWifiMac::Receive(Ptr<const WifiMpdu> mpdu, uint8_t linkId) {
  NS_LOG_FUNCTION(this << *mpdu << +linkId);
  const WifiMacHeader *hdr = &mpdu->GetOriginal()->GetHeader();
  Ptr<const Packet> packet = mpdu->GetPacket();
  Mac48Address from = hdr->GetAddr2();
  if (hdr->IsData()) {
    std::optional<uint8_t> apLinkId;
    if (!hdr->IsFromDs() && hdr->IsToDs() &&
        (apLinkId = IsAssociated(mpdu->GetHeader().GetAddr2())) &&
        mpdu->GetHeader().GetAddr1() ==
            GetFrameExchangeManager(*apLinkId)->GetAddress()) {
      ProcessPowerManagementFlag(mpdu, *apLinkId);

      Mac48Address to = hdr->GetAddr3();
      if (to == GetAddress() || (hdr->IsQosData() && hdr->IsQosAmsdu() &&
                                 to == mpdu->GetHeader().GetAddr1())) {
        NS_LOG_DEBUG("frame for me from=" << from);
        if (hdr->IsQosData()) {
          if (hdr->IsQosAmsdu()) {
            NS_LOG_DEBUG("Received A-MSDU from=" << from << ", size="
                                                 << packet->GetSize());
            DeaggregateAmsduAndForward(mpdu);
            packet = nullptr;
          } else if (hdr->HasData()) {
            ForwardUp(packet, from, GetAddress());
          }
        } else if (hdr->HasData()) {
          ForwardUp(packet, from, GetAddress());
        }
      } else if (to.IsGroup() || IsAssociated(to)) {
        NS_LOG_DEBUG("forwarding frame from=" << from << ", to=" << to);
        Ptr<Packet> copy = packet->Copy();

        if (hdr->IsQosData()) {
          ForwardDown(copy, from, to, hdr->GetQosTid());
        } else {
          ForwardDown(copy, from, to);
        }
        ForwardUp(packet, from, to);
      } else if (hdr->HasData()) {
        ForwardUp(packet, from, to);
      }
    } else if (hdr->IsFromDs() && hdr->IsToDs()) {
      NotifyRxDrop(packet);
    } else {
      NotifyRxDrop(packet);
    }
    return;
  } else if (hdr->IsMgt()) {
    if (hdr->GetAddr1() == GetFrameExchangeManager(linkId)->GetAddress() &&
        GetWifiRemoteStationManager(linkId)->IsAssociated(from)) {
      ProcessPowerManagementFlag(mpdu, linkId);
    }
    if (hdr->IsProbeReq() &&
        (hdr->GetAddr1().IsGroup() ||
         hdr->GetAddr1() == GetFrameExchangeManager(linkId)->GetAddress())) {
      if (hdr->GetAddr1().IsGroup() && !hdr->GetAddr3().IsBroadcast() &&
          hdr->GetAddr3() != GetFrameExchangeManager(linkId)->GetAddress()) {
        return;
      }
      MgtProbeRequestHeader probeRequestHeader;
      packet->PeekHeader(probeRequestHeader);
      const auto &ssid = probeRequestHeader.Get<Ssid>();
      if (ssid == GetSsid() || ssid->IsBroadcast()) {
        NS_LOG_DEBUG("Probe request received from " << from
                                                    << ": send probe response");
        SendProbeResp(from, linkId);
      }
      return;
    } else if (hdr->GetAddr1() ==
               GetFrameExchangeManager(linkId)->GetAddress()) {
      switch (hdr->GetType()) {
      case WIFI_MAC_MGT_ASSOCIATION_REQUEST:
      case WIFI_MAC_MGT_REASSOCIATION_REQUEST: {
        NS_LOG_DEBUG(((hdr->IsAssocReq()) ? "Association" : "Reassociation")
                     << " request received from " << from
                     << ((GetNLinks() > 1)
                             ? " on link ID " + std::to_string(linkId)
                             : ""));

        MgtAssocRequestHeader assocReq;
        MgtReassocRequestHeader reassocReq;
        AssocReqRefVariant frame = assocReq;
        if (hdr->IsAssocReq()) {
          packet->PeekHeader(assocReq);
        } else {
          packet->PeekHeader(reassocReq);
          frame = reassocReq;
        }
        if (ReceiveAssocRequest(frame, from, linkId) && GetNLinks() > 1) {
          ParseReportedStaInfo(frame, from, linkId);
        }
        SendAssocResp(hdr->GetAddr2(), hdr->IsReassocReq(), linkId);
        return;
      }
      case WIFI_MAC_MGT_DISASSOCIATION: {
        NS_LOG_DEBUG("Disassociation received from " << from);
        GetWifiRemoteStationManager(linkId)->RecordDisassociated(from);
        auto &staList = GetLink(linkId).staList;
        for (auto it = staList.begin(); it != staList.end(); ++it) {
          if (it->second == from) {
            staList.erase(it);
            m_deAssocLogger(it->first, it->second);
            if (GetWifiRemoteStationManager(linkId)->GetDsssSupported(from) &&
                !GetWifiRemoteStationManager(linkId)->GetErpOfdmSupported(
                    from)) {
              GetLink(linkId).numNonErpStations--;
            }
            if (!GetWifiRemoteStationManager(linkId)->GetHtSupported(from)) {
              GetLink(linkId).numNonHtStations--;
            }
            UpdateShortSlotTimeEnabled(linkId);
            UpdateShortPreambleEnabled(linkId);
            StaSwitchingToActiveModeOrDeassociated(from, linkId);
            break;
          }
        }
        return;
      }
      case WIFI_MAC_MGT_ACTION: {
        auto pkt = mpdu->GetPacket()->Copy();
        auto [category, action] = WifiActionHeader::Remove(pkt);
        if (category == WifiActionHeader::PROTECTED_EHT &&
            action.protectedEhtAction ==
                WifiActionHeader::
                    PROTECTED_EHT_EML_OPERATING_MODE_NOTIFICATION &&
            IsAssociated(hdr->GetAddr2())) {
          MgtEmlOmn frame;
          pkt->RemoveHeader(frame);
          ReceiveEmlOmn(frame, hdr->GetAddr2(), linkId);
          return;
        }
        break;
      }
      default:;
      }
    }
  }

  WifiMac::Receive(Create<WifiMpdu>(packet, *hdr), linkId);
}

bool ApWifiMac::ReceiveAssocRequest(const AssocReqRefVariant &assoc,
                                    const Mac48Address &from, uint8_t linkId) {
  NS_LOG_FUNCTION(this << from << +linkId);

  auto remoteStationManager = GetWifiRemoteStationManager(linkId);

  auto failure = [&](const std::string &msg) -> bool {
    NS_LOG_DEBUG("Association Request from " << from << " refused: " << msg);
    remoteStationManager->RecordAssocRefused(from);
    return false;
  };

  auto recvAssocRequest = [&](auto &&frameRefWrapper) -> bool {
    const auto &frame = frameRefWrapper.get();

    const CapabilityInformation &capabilities = frame.Capabilities();
    remoteStationManager->AddSupportedPhyPreamble(
        from, capabilities.IsShortPreamble());
    NS_ASSERT(frame.template Get<SupportedRates>());
    const auto rates =
        AllSupportedRates{*frame.template Get<SupportedRates>(),
                          frame.template Get<ExtendedSupportedRatesIE>()};

    if (rates.GetNRates() == 0) {
      return failure(
          "STA's supported rate set not compatible with our Basic Rate set");
    }

    if (GetHtSupported()) {
      const auto &htCapabilities = frame.template Get<HtCapabilities>();
      if (htCapabilities.has_value() && htCapabilities->IsSupportedMcs(0)) {
        for (uint8_t i = 0; i < remoteStationManager->GetNBasicMcs(); i++) {
          WifiMode mcs = remoteStationManager->GetBasicMcs(i);
          if (!htCapabilities->IsSupportedMcs(mcs.GetMcsValue())) {
            return failure("HT STA does not support all MCSs in Basic MCS Set");
          }
        }
      }
    }
    if (GetVhtSupported(linkId)) {
      const auto &vhtCapabilities = frame.template Get<VhtCapabilities>();
      if (vhtCapabilities.has_value() &&
          vhtCapabilities->GetVhtCapabilitiesInfo() != 0) {
        for (uint8_t i = 0; i < remoteStationManager->GetNBasicMcs(); i++) {
          WifiMode mcs = remoteStationManager->GetBasicMcs(i);
          if (!vhtCapabilities->IsSupportedTxMcs(mcs.GetMcsValue())) {
            return failure(
                "VHT STA does not support all MCSs in Basic MCS Set");
          }
        }
      }
    }
    if (GetHeSupported()) {
      const auto &heCapabilities = frame.template Get<HeCapabilities>();
      if (heCapabilities.has_value() &&
          heCapabilities->GetSupportedMcsAndNss() != 0) {
        for (uint8_t i = 0; i < remoteStationManager->GetNBasicMcs(); i++) {
          WifiMode mcs = remoteStationManager->GetBasicMcs(i);
          if (!heCapabilities->IsSupportedTxMcs(mcs.GetMcsValue())) {
            return failure("HE STA does not support all MCSs in Basic MCS Set");
          }
        }
      }
    }
    if (GetEhtSupported()) {
      auto ehtConfig = GetEhtConfiguration();
      NS_ASSERT(ehtConfig);

      if (const auto &tidLinkMapping = frame.template Get<TidToLinkMapping>();
          !tidLinkMapping.empty()) {
        if (tidLinkMapping.size() > 2) {
          return failure("More than two TID-to-Link Mapping IEs");
        }

        bool bothDirIfOneTlm = tidLinkMapping.size() != 1 ||
                               tidLinkMapping[0].m_control.direction ==
                                   WifiDirection::BOTH_DIRECTIONS;
        bool distinctDirsIfTwoTlms =
            tidLinkMapping.size() != 2 ||
            (tidLinkMapping[0].m_control.direction !=
                 WifiDirection::BOTH_DIRECTIONS &&
             tidLinkMapping[1].m_control.direction !=
                 WifiDirection::BOTH_DIRECTIONS &&
             tidLinkMapping[0].m_control.direction !=
                 tidLinkMapping[1].m_control.direction);

        if (!bothDirIfOneTlm || !distinctDirsIfTwoTlms) {
          return failure("Incorrect directions in TID-to-Link Mapping IEs");
        }

        EnumValue negSupport;
        ehtConfig->GetAttributeFailSafe("TidToLinkMappingNegSupport",
                                        negSupport);

        if (negSupport.Get() == 0) {
          return failure("TID-to-Link Mapping negotiation not supported");
        }

        auto getMapping = [](const TidToLinkMapping &tlmIe,
                             WifiTidLinkMapping &mapping) {
          if (tlmIe.m_control.defaultMapping) {
            return;
          }
          for (uint8_t tid = 0; tid < 8; tid++) {
            if (auto linkSet = tlmIe.GetLinkMappingOfTid(tid);
                !linkSet.empty()) {
              mapping.emplace(tid, std::move(linkSet));
            }
          }
        };

        WifiTidLinkMapping dlMapping;
        WifiTidLinkMapping ulMapping;

        switch (tidLinkMapping[0].m_control.direction) {
        case WifiDirection::BOTH_DIRECTIONS:
          getMapping(tidLinkMapping.at(0), dlMapping);
          ulMapping = dlMapping;
          break;
        case WifiDirection::DOWNLINK:
          getMapping(tidLinkMapping.at(0), dlMapping);
          getMapping(tidLinkMapping.at(1), ulMapping);
          break;
        case WifiDirection::UPLINK:
          getMapping(tidLinkMapping.at(0), ulMapping);
          getMapping(tidLinkMapping.at(1), dlMapping);
          break;
        }

        if (negSupport.Get() == 1 &&
            !TidToLinkMappingValidForNegType1(dlMapping, ulMapping)) {
          return failure(
              "Mapping TIDs to distinct link sets is incompatible with "
              "negotiation support of 1");
        }

        const auto &mle = frame.template Get<MultiLinkElement>();
        NS_ASSERT_MSG(mle, "Multi-Link Element not present in an Association "
                           "Request including "
                           "TID-to-Link Mapping element(s)");
        auto mldAddr = mle->GetMldMacAddress();

        UpdateTidToLinkMapping(mldAddr, WifiDirection::DOWNLINK, dlMapping);
        UpdateTidToLinkMapping(mldAddr, WifiDirection::UPLINK, ulMapping);
      }
    }

    auto phy = GetWifiPhy(linkId);

    for (const auto &mode : phy->GetModeList()) {
      if (rates.IsSupportedRate(mode.GetDataRate(phy->GetChannelWidth()))) {
        remoteStationManager->AddSupportedMode(from, mode);
      }
    }
    if (GetErpSupported(linkId) &&
        remoteStationManager->GetErpOfdmSupported(from) &&
        capabilities.IsShortSlotTime()) {
      remoteStationManager->AddSupportedErpSlotTime(from, true);
    }
    if (GetHtSupported()) {
      const auto &htCapabilities = frame.template Get<HtCapabilities>();
      if (htCapabilities.has_value() && htCapabilities->IsSupportedMcs(0)) {
        remoteStationManager->AddStationHtCapabilities(from, *htCapabilities);
      }
    }
    if (GetVhtSupported(linkId)) {
      const auto &vhtCapabilities = frame.template Get<VhtCapabilities>();
      if (vhtCapabilities.has_value() &&
          vhtCapabilities->GetRxHighestSupportedLgiDataRate() > 0) {
        remoteStationManager->AddStationVhtCapabilities(from, *vhtCapabilities);
        for (const auto &mcs : phy->GetMcsList(WIFI_MOD_CLASS_VHT)) {
          if (vhtCapabilities->IsSupportedTxMcs(mcs.GetMcsValue())) {
            remoteStationManager->AddSupportedMcs(from, mcs);
          }
        }
      }
    }
    if (GetHeSupported()) {
      const auto &heCapabilities = frame.template Get<HeCapabilities>();
      if (heCapabilities.has_value() &&
          heCapabilities->GetSupportedMcsAndNss() != 0) {
        remoteStationManager->AddStationHeCapabilities(from, *heCapabilities);
        for (const auto &mcs : phy->GetMcsList(WIFI_MOD_CLASS_HE)) {
          if (heCapabilities->IsSupportedTxMcs(mcs.GetMcsValue())) {
            remoteStationManager->AddSupportedMcs(from, mcs);
          }
        }
      }
    }
    if (GetEhtSupported()) {
      if (const auto &ehtCapabilities = frame.template Get<EhtCapabilities>()) {
        remoteStationManager->AddStationEhtCapabilities(from, *ehtCapabilities);
      }
      for (const auto &mcs : phy->GetMcsList(WIFI_MOD_CLASS_EHT)) {
        remoteStationManager->AddSupportedMcs(from, mcs);
      }
    }

    NS_LOG_DEBUG("Association Request from " << from << " accepted");
    remoteStationManager->RecordWaitAssocTxOk(from);
    return true;
  };

  return std::visit(recvAssocRequest, assoc);
}

void ApWifiMac::ParseReportedStaInfo(const AssocReqRefVariant &assoc,
                                     Mac48Address from, uint8_t linkId) {
  NS_LOG_FUNCTION(this << from << +linkId);

  auto recvMle = [&](auto &&frame) {
    const auto &mle = frame.get().template Get<MultiLinkElement>();

    if (!mle.has_value()) {
      return;
    }

    auto mleCommonInfo =
        std::make_shared<CommonInfoBasicMle>(mle->GetCommonInfoBasic());
    GetWifiRemoteStationManager(linkId)->AddStationMleCommonInfo(from,
                                                                 mleCommonInfo);

    for (std::size_t i = 0; i < mle->GetNPerStaProfileSubelements(); i++) {
      auto &perStaProfile = mle->GetPerStaProfile(i);
      if (!perStaProfile.HasStaMacAddress()) {
        NS_LOG_DEBUG(
            "[i=" << i
                  << "] Cannot setup a link if the STA MAC address is missing");
        continue;
      }
      uint8_t newLinkId = perStaProfile.GetLinkId();
      if (newLinkId == linkId || newLinkId >= GetNLinks()) {
        NS_LOG_DEBUG("[i=" << i << "] Link ID " << newLinkId << " not valid");
        continue;
      }
      if (!perStaProfile.HasAssocRequest() &&
          !perStaProfile.HasReassocRequest()) {
        NS_LOG_DEBUG(
            "[i=" << i << "] No (Re)Association Request frame body present");
        continue;
      }

      ReceiveAssocRequest(perStaProfile.GetAssocRequest(),
                          perStaProfile.GetStaMacAddress(), newLinkId);
      GetWifiRemoteStationManager(newLinkId)->AddStationMleCommonInfo(
          perStaProfile.GetStaMacAddress(), mleCommonInfo);
    }
  };

  std::visit(recvMle, assoc);
}

void ApWifiMac::ReceiveEmlOmn(MgtEmlOmn &frame, const Mac48Address &sender,
                              uint8_t linkId) {
  NS_LOG_FUNCTION(this << frame << sender << linkId);

  auto ehtConfiguration = GetEhtConfiguration();

  if (BooleanValue emlsrActivated; !ehtConfiguration ||
                                   !ehtConfiguration->GetAttributeFailSafe(
                                       "EmlsrActivated", emlsrActivated) ||
                                   !emlsrActivated.Get()) {
    NS_LOG_DEBUG("Received an EML Operating Mode Notification frame but EMLSR "
                 "is not activated");
    return;
  }

  if (frame.m_emlControl.emlsrParamUpdateCtrl) {
    NS_ASSERT(frame.m_emlsrParamUpdate);
    auto emlCapabilities =
        GetWifiRemoteStationManager(linkId)->GetStationEmlCapabilities(sender);
    NS_ASSERT_MSG(emlCapabilities,
                  "EML Capabilities not stored for STA " << sender);

    emlCapabilities->get().emlsrPaddingDelay =
        frame.m_emlsrParamUpdate->paddingDelay;
    emlCapabilities->get().emlsrTransitionDelay =
        frame.m_emlsrParamUpdate->transitionDelay;
  }

  auto mldAddress = GetWifiRemoteStationManager(linkId)->GetMldAddress(sender);
  NS_ASSERT_MSG(mldAddress, "No MLD address stored for STA " << sender);
  auto emlsrLinks = frame.m_emlControl.emlsrMode == 1 ? frame.GetLinkBitmap()
                                                      : std::list<uint8_t>{};

  CallbackBase cb = Callback<void, WifiConstPsduMap, WifiTxVector, double>(
      [=](WifiConstPsduMap psduMap, WifiTxVector txVector, double) {
        NS_ASSERT_MSG(psduMap.size() == 1 &&
                          psduMap.begin()->second->GetNMpdus() == 1 &&
                          psduMap.begin()->second->GetHeader(0).IsAck(),
                      "Expected a Normal Ack after EML Notification frame");

        auto ackDuration = WifiPhy::CalculateTxDuration(
            psduMap, txVector, GetLink(linkId).phy->GetPhyBand());

        TimeValue transitionTimeout;
        ehtConfiguration->GetAttribute("TransitionTimeout", transitionTimeout);

        m_transitionTimeoutEvents[sender] =
            Simulator::Schedule(ackDuration + transitionTimeout.Get(), [=]() {
              for (uint8_t id = 0; id < GetNLinks(); id++) {
                auto linkAddress =
                    GetWifiRemoteStationManager(id)->GetAffiliatedStaAddress(
                        *mldAddress);
                if (!linkAddress) {
                  continue;
                }

                if (!emlsrLinks.empty()) {
                  auto enabled =
                      std::find(emlsrLinks.cbegin(), emlsrLinks.cend(), id) !=
                      emlsrLinks.cend();
                  if (enabled) {
                    StaSwitchingToActiveModeOrDeassociated(*linkAddress, id);
                  }
                  GetWifiRemoteStationManager(id)->SetEmlsrEnabled(*linkAddress,
                                                                   enabled);
                } else {
                  if (id != linkId &&
                      GetWifiRemoteStationManager(id)->GetEmlsrEnabled(
                          *linkAddress)) {
                    StaSwitchingToPsMode(*linkAddress, id);
                  }
                  GetWifiRemoteStationManager(id)->SetEmlsrEnabled(*linkAddress,
                                                                   false);
                }
              }
            });
      });

  auto phy = GetLink(linkId).phy;
  phy->TraceConnectWithoutContext("PhyTxPsduBegin", cb);
  Simulator::Schedule(phy->GetSifs() + NanoSeconds(1), [=]() {
    phy->TraceDisconnectWithoutContext("PhyTxPsduBegin", cb);
  });

  frame.m_emlControl.emlsrParamUpdateCtrl = 0;

  frame.m_emlsrParamUpdate.reset();

  auto ehtFem =
      StaticCast<EhtFrameExchangeManager>(GetFrameExchangeManager(linkId));
  ehtFem->SendEmlOmn(sender, frame);
}

void ApWifiMac::DeaggregateAmsduAndForward(Ptr<const WifiMpdu> mpdu) {
  NS_LOG_FUNCTION(this << *mpdu);
  for (auto &i : *PeekPointer(mpdu)) {
    auto from = i.second.GetSourceAddr();
    auto to = i.second.GetDestinationAddr();

    if (to.IsGroup() || IsAssociated(to)) {
      NS_LOG_DEBUG("forwarding QoS frame from=" << from << ", to=" << to);
      ForwardDown(i.first->Copy(), from, to, mpdu->GetHeader().GetQosTid());
    }

    ForwardUp(i.first, from, to);
  }
}

void ApWifiMac::DoInitialize() {
  NS_LOG_FUNCTION(this);
  m_beaconTxop->Initialize();

  for (uint8_t linkId = 0; linkId < GetNLinks(); ++linkId) {
    GetLink(linkId).beaconEvent.Cancel();
    if (m_enableBeaconGeneration) {
      uint64_t jitterUs =
          (m_enableBeaconJitter
               ? static_cast<uint64_t>(m_beaconJitter->GetValue(0, 1) *
                                       (GetBeaconInterval().GetMicroSeconds()))
               : 0);
      NS_LOG_DEBUG("Scheduling initial beacon for access point "
                   << GetAddress() << " at time " << jitterUs << "us");
      GetLink(linkId).beaconEvent = Simulator::Schedule(
          MicroSeconds(jitterUs), &ApWifiMac::SendOneBeacon, this, linkId);
    }
    UpdateShortSlotTimeEnabled(linkId);
    UpdateShortPreambleEnabled(linkId);
  }

  NS_ABORT_IF(!TraceConnectWithoutContext(
      "AckedMpdu", MakeCallback(&ApWifiMac::TxOk, this)));
  NS_ABORT_IF(!TraceConnectWithoutContext(
      "DroppedMpdu", MakeCallback(&ApWifiMac::TxFailed, this)));
  WifiMac::DoInitialize();
}

bool ApWifiMac::GetUseNonErpProtection(uint8_t linkId) const {
  bool useProtection =
      (GetLink(linkId).numNonErpStations > 0) && m_enableNonErpProtection;
  GetWifiRemoteStationManager(linkId)->SetUseNonErpProtection(useProtection);
  return useProtection;
}

uint16_t ApWifiMac::GetNextAssociationId(std::list<uint8_t> linkIds) {
  for (uint16_t nextAid = 1; nextAid <= 2007; nextAid++) {
    if (std::all_of(linkIds.begin(), linkIds.end(), [&](auto &&linkId) {
          auto &staList = GetLink(linkId).staList;
          return staList.find(nextAid) == staList.end();
        })) {
      return nextAid;
    }
  }
  NS_FATAL_ERROR("No free association ID available!");
  return 0;
}

const std::map<uint16_t, Mac48Address> &
ApWifiMac::GetStaList(uint8_t linkId) const {
  return GetLink(linkId).staList;
}

uint16_t ApWifiMac::GetAssociationId(Mac48Address addr, uint8_t linkId) const {
  return GetWifiRemoteStationManager(linkId)->GetAssociationId(addr);
}

uint8_t ApWifiMac::GetBufferStatus(uint8_t tid, Mac48Address address) const {
  auto it = m_bufferStatus.find(WifiAddressTidPair(address, tid));
  if (it == m_bufferStatus.end() ||
      it->second.timestamp + m_bsrLifetime < Simulator::Now()) {
    return 255;
  }
  return it->second.value;
}

void ApWifiMac::SetBufferStatus(uint8_t tid, Mac48Address address,
                                uint8_t size) {
  if (size == 255) {
    m_bufferStatus.erase(WifiAddressTidPair(address, tid));
  } else {
    m_bufferStatus[WifiAddressTidPair(address, tid)] = {size, Simulator::Now()};
  }
}

uint8_t ApWifiMac::GetMaxBufferStatus(Mac48Address address) const {
  uint8_t maxSize = 0;
  bool found = false;

  for (uint8_t tid = 0; tid < 8; tid++) {
    uint8_t size = GetBufferStatus(tid, address);
    if (size != 255) {
      maxSize = std::max(maxSize, size);
      found = true;
    }
  }

  if (found) {
    return maxSize;
  }
  return 255;
}

} // namespace ns3
