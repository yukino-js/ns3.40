
#include "sta-wifi-mac.h"

#include "channel-access-manager.h"
#include "frame-exchange-manager.h"
#include "mgt-headers.h"
#include "qos-txop.h"
#include "snr-tag.h"
#include "wifi-assoc-manager.h"
#include "wifi-mac-queue.h"
#include "wifi-net-device.h"
#include "wifi-phy.h"

#include "ns3/attribute-container.h"
#include "ns3/eht-configuration.h"
#include "ns3/emlsr-manager.h"
#include "ns3/he-configuration.h"
#include "ns3/ht-configuration.h"
#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/pair.h"
#include "ns3/pointer.h"
#include "ns3/random-variable-stream.h"
#include "ns3/simulator.h"
#include "ns3/string.h"

#include <numeric>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("StaWifiMac");

NS_OBJECT_ENSURE_REGISTERED(StaWifiMac);

TypeId StaWifiMac::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::StaWifiMac")
          .SetParent<WifiMac>()
          .SetGroupName("Wifi")
          .AddConstructor<StaWifiMac>()
          .AddAttribute("ProbeRequestTimeout",
                        "The duration to actively probe the channel.",
                        TimeValue(Seconds(0.05)),
                        MakeTimeAccessor(&StaWifiMac::m_probeRequestTimeout),
                        MakeTimeChecker())
          .AddAttribute("WaitBeaconTimeout",
                        "The duration to dwell on a channel while passively "
                        "scanning for beacon",
                        TimeValue(MilliSeconds(120)),
                        MakeTimeAccessor(&StaWifiMac::m_waitBeaconTimeout),
                        MakeTimeChecker())
          .AddAttribute("AssocRequestTimeout",
                        "The interval between two consecutive association "
                        "request attempts.",
                        TimeValue(Seconds(0.5)),
                        MakeTimeAccessor(&StaWifiMac::m_assocRequestTimeout),
                        MakeTimeChecker())
          .AddAttribute(
              "MaxMissedBeacons",
              "Number of beacons which much be consecutively missed before "
              "we attempt to restart association.",
              UintegerValue(10),
              MakeUintegerAccessor(&StaWifiMac::m_maxMissedBeacons),
              MakeUintegerChecker<uint32_t>())
          .AddAttribute("ActiveProbing",
                        "If true, we send probe requests. If false, we don't."
                        "NOTE: if more than one STA in your simulation is "
                        "using active probing, "
                        "you should enable it at a different simulation time "
                        "for each STA, "
                        "otherwise all the STAs will start sending probes at "
                        "the same time resulting in "
                        "collisions. "
                        "See bug 1060 for more info.",
                        BooleanValue(false),
                        MakeBooleanAccessor(&StaWifiMac::SetActiveProbing,
                                            &StaWifiMac::GetActiveProbing),
                        MakeBooleanChecker())
          .AddAttribute(
              "ProbeDelay",
              "Delay (in microseconds) to be used prior to transmitting a "
              "Probe frame during active scanning.",
              StringValue("ns3::UniformRandomVariable[Min=50.0|Max=250.0]"),
              MakePointerAccessor(&StaWifiMac::m_probeDelay),
              MakePointerChecker<RandomVariableStream>())
          .AddAttribute(
              "PowerSaveMode",
              "Enable/disable power save mode on the given link. The power "
              "management mode is "
              "actually changed when the AP acknowledges a frame sent with the "
              "Power Management "
              "field set to the value corresponding to the requested mode",
              TypeId::ATTR_GET | TypeId::ATTR_SET,
              PairValue<BooleanValue, UintegerValue>(),
              MakePairAccessor<BooleanValue, UintegerValue>(
                  &StaWifiMac::SetPowerSaveMode),
              MakePairChecker<BooleanValue, UintegerValue>(
                  MakeBooleanChecker(), MakeUintegerChecker<uint8_t>()))
          .AddAttribute(
              "PmModeSwitchTimeout",
              "If switching to a new Power Management mode is not completed "
              "within "
              "this amount of time, make another attempt at switching Power "
              "Management mode.",
              TimeValue(Seconds(0.1)),
              MakeTimeAccessor(&StaWifiMac::m_pmModeSwitchTimeout),
              MakeTimeChecker())
          .AddTraceSource("Assoc",
                          "Associated with an access point. If this is an MLD "
                          "that associated "
                          "with an AP MLD, the AP MLD address is provided.",
                          MakeTraceSourceAccessor(&StaWifiMac::m_assocLogger),
                          "ns3::Mac48Address::TracedCallback")
          .AddTraceSource(
              "LinkSetupCompleted",
              "A link was setup in the context of ML setup with an AP MLD. "
              "Provides ID of the setup link and AP MAC address",
              MakeTraceSourceAccessor(&StaWifiMac::m_setupCompleted),
              "ns3::StaWifiMac::LinkSetupCallback")
          .AddTraceSource(
              "DeAssoc",
              "Association with an access point lost. If this is an MLD "
              "that disassociated with an AP MLD, the AP MLD address is "
              "provided.",
              MakeTraceSourceAccessor(&StaWifiMac::m_deAssocLogger),
              "ns3::Mac48Address::TracedCallback")
          .AddTraceSource("LinkSetupCanceled",
                          "A link setup in the context of ML setup with an AP "
                          "MLD was torn down. "
                          "Provides ID of the setup link and AP MAC address",
                          MakeTraceSourceAccessor(&StaWifiMac::m_setupCanceled),
                          "ns3::StaWifiMac::LinkSetupCallback")
          .AddTraceSource("BeaconArrival",
                          "Time of beacons arrival from associated AP",
                          MakeTraceSourceAccessor(&StaWifiMac::m_beaconArrival),
                          "ns3::Time::TracedCallback")
          .AddTraceSource("ReceivedBeaconInfo",
                          "Information about every received Beacon frame",
                          MakeTraceSourceAccessor(&StaWifiMac::m_beaconInfo),
                          "ns3::ApInfo::TracedCallback");
  return tid;
}

StaWifiMac::StaWifiMac()
    : m_state(UNASSOCIATED), m_aid(0), m_assocRequestEvent() {
  NS_LOG_FUNCTION(this);

  SetTypeOfStation(STA);
}

void StaWifiMac::DoInitialize() {
  NS_LOG_FUNCTION(this);
  if (m_assocManager && m_emlsrManager) {
    auto mainPhyId = m_emlsrManager->GetMainPhyId();
    auto linkId = GetLinkForPhy(mainPhyId);
    NS_ASSERT(linkId);
    m_assocManager->SetAttribute(
        "AllowedLinks",
        AttributeContainerValue<UintegerValue>(std::list<uint8_t>{*linkId}));
  }
  if (m_emlsrManager) {
    m_emlsrManager->Initialize();
  }
  StartScanning();
  NS_ABORT_IF(!TraceConnectWithoutContext(
      "AckedMpdu", MakeCallback(&StaWifiMac::TxOk, this)));
  WifiMac::DoInitialize();
}

void StaWifiMac::DoDispose() {
  NS_LOG_FUNCTION(this);
  if (m_assocManager) {
    m_assocManager->Dispose();
  }
  m_assocManager = nullptr;
  if (m_emlsrManager) {
    m_emlsrManager->Dispose();
  }
  m_emlsrManager = nullptr;
  WifiMac::DoDispose();
}

StaWifiMac::~StaWifiMac() { NS_LOG_FUNCTION(this); }

StaWifiMac::StaLinkEntity::~StaLinkEntity() { NS_LOG_FUNCTION_NOARGS(); }

std::unique_ptr<WifiMac::LinkEntity> StaWifiMac::CreateLinkEntity() const {
  return std::make_unique<StaLinkEntity>();
}

StaWifiMac::StaLinkEntity &StaWifiMac::GetLink(uint8_t linkId) const {
  return static_cast<StaLinkEntity &>(WifiMac::GetLink(linkId));
}

StaWifiMac::StaLinkEntity &
StaWifiMac::GetStaLink(const std::unique_ptr<WifiMac::LinkEntity> &link) const {
  return static_cast<StaLinkEntity &>(*link);
}

int64_t StaWifiMac::AssignStreams(int64_t stream) {
  NS_LOG_FUNCTION(this << stream);
  m_probeDelay->SetStream(stream);
  return 1;
}

void StaWifiMac::SetAssocManager(Ptr<WifiAssocManager> assocManager) {
  NS_LOG_FUNCTION(this << assocManager);
  m_assocManager = assocManager;
  m_assocManager->SetStaWifiMac(this);
}

void StaWifiMac::SetEmlsrManager(Ptr<EmlsrManager> emlsrManager) {
  NS_LOG_FUNCTION(this << emlsrManager);
  m_emlsrManager = emlsrManager;
  m_emlsrManager->SetWifiMac(this);
}

Ptr<EmlsrManager> StaWifiMac::GetEmlsrManager() const { return m_emlsrManager; }

uint16_t StaWifiMac::GetAssociationId() const {
  NS_ASSERT_MSG(IsAssociated(), "This station is not associated to any AP");
  return m_aid;
}

void StaWifiMac::SetActiveProbing(bool enable) {
  NS_LOG_FUNCTION(this << enable);
  m_activeProbing = enable;
  if (m_state == SCANNING) {
    NS_LOG_DEBUG("STA is still scanning, reset scanning process");
    StartScanning();
  }
}

bool StaWifiMac::GetActiveProbing() const { return m_activeProbing; }

void StaWifiMac::SetWifiPhys(const std::vector<Ptr<WifiPhy>> &phys) {
  NS_LOG_FUNCTION(this);
  WifiMac::SetWifiPhys(phys);
  for (auto &phy : phys) {
    phy->SetCapabilitiesChangedCallback(
        MakeCallback(&StaWifiMac::PhyCapabilitiesChanged, this));
  }
}

WifiScanParams::Channel StaWifiMac::GetCurrentChannel(uint8_t linkId) const {
  auto phy = GetWifiPhy(linkId);
  uint16_t width =
      phy->GetOperatingChannel().IsOfdm() ? 20 : phy->GetChannelWidth();
  uint8_t ch = phy->GetOperatingChannel().GetPrimaryChannelNumber(
      width, phy->GetStandard());
  return {ch, phy->GetPhyBand()};
}

void StaWifiMac::NotifyEmlsrModeChanged(const std::set<uint8_t> &linkIds) {
  NS_LOG_FUNCTION(this << linkIds.size());

  for (const auto &[linkId, lnk] : GetLinks()) {
    auto &link = GetStaLink(lnk);

    if (linkIds.count(linkId) > 0) {
      link.emlsrEnabled = true;
      link.pmMode = WIFI_PM_ACTIVE;
    } else {
      if (link.emlsrEnabled) {
        link.pmMode = WIFI_PM_POWERSAVE;
      }
      link.emlsrEnabled = false;
    }
  }
}

bool StaWifiMac::IsEmlsrLink(uint8_t linkId) const {
  return GetLink(linkId).emlsrEnabled;
}

void StaWifiMac::SendProbeRequest(uint8_t linkId) {
  NS_LOG_FUNCTION(this << linkId);
  WifiMacHeader hdr;
  hdr.SetType(WIFI_MAC_MGT_PROBE_REQUEST);
  hdr.SetAddr1(Mac48Address::GetBroadcast());
  hdr.SetAddr2(GetFrameExchangeManager(linkId)->GetAddress());
  hdr.SetAddr3(Mac48Address::GetBroadcast());
  hdr.SetDsNotFrom();
  hdr.SetDsNotTo();
  Ptr<Packet> packet = Create<Packet>();
  MgtProbeRequestHeader probe;
  probe.Get<Ssid>() = GetSsid();
  auto supportedRates = GetSupportedRates(linkId);
  probe.Get<SupportedRates>() = supportedRates.rates;
  probe.Get<ExtendedSupportedRatesIE>() = supportedRates.extendedRates;
  if (GetHtSupported()) {
    probe.Get<ExtendedCapabilities>() = GetExtendedCapabilities();
    probe.Get<HtCapabilities>() = GetHtCapabilities(linkId);
  }
  if (GetVhtSupported(linkId)) {
    probe.Get<VhtCapabilities>() = GetVhtCapabilities(linkId);
  }
  if (GetHeSupported()) {
    probe.Get<HeCapabilities>() = GetHeCapabilities(linkId);
  }
  if (GetEhtSupported()) {
    probe.Get<EhtCapabilities>() = GetEhtCapabilities(linkId);
  }
  packet->AddHeader(probe);

  if (!GetQosSupported()) {
    GetTxop()->Queue(packet, hdr);
  } else {
    GetVOQueue()->Queue(packet, hdr);
  }
}

std::variant<MgtAssocRequestHeader, MgtReassocRequestHeader>
StaWifiMac::GetAssociationRequest(bool isReassoc, uint8_t linkId) const {
  NS_LOG_FUNCTION(this << isReassoc << +linkId);

  std::variant<MgtAssocRequestHeader, MgtReassocRequestHeader> mgtFrame;

  if (isReassoc) {
    MgtReassocRequestHeader reassoc;
    reassoc.SetCurrentApAddress(GetBssid(linkId));
    mgtFrame = std::move(reassoc);
  } else {
    mgtFrame = MgtAssocRequestHeader();
  }

  auto fill = [&](auto &&frame) {
    frame.template Get<Ssid>() = GetSsid();
    auto supportedRates = GetSupportedRates(linkId);
    frame.template Get<SupportedRates>() = supportedRates.rates;
    frame.template Get<ExtendedSupportedRatesIE>() =
        supportedRates.extendedRates;
    frame.Capabilities() = GetCapabilities(linkId);
    frame.SetListenInterval(0);
    if (GetHtSupported()) {
      frame.template Get<ExtendedCapabilities>() = GetExtendedCapabilities();
      frame.template Get<HtCapabilities>() = GetHtCapabilities(linkId);
    }
    if (GetVhtSupported(linkId)) {
      frame.template Get<VhtCapabilities>() = GetVhtCapabilities(linkId);
    }
    if (GetHeSupported()) {
      frame.template Get<HeCapabilities>() = GetHeCapabilities(linkId);
    }
    if (GetEhtSupported()) {
      frame.template Get<EhtCapabilities>() = GetEhtCapabilities(linkId);
    }
  };

  std::visit(fill, mgtFrame);
  return mgtFrame;
}

MultiLinkElement StaWifiMac::GetMultiLinkElement(bool isReassoc,
                                                 uint8_t linkId) const {
  NS_LOG_FUNCTION(this << isReassoc << +linkId);

  MultiLinkElement multiLinkElement(MultiLinkElement::BASIC_VARIANT);
  multiLinkElement.SetMldMacAddress(GetAddress());

  if (m_emlsrManager) {
    multiLinkElement.SetEmlsrSupported(true);
    TimeValue time;
    m_emlsrManager->GetAttribute("EmlsrPaddingDelay", time);
    multiLinkElement.SetEmlsrPaddingDelay(time.Get());
    m_emlsrManager->GetAttribute("EmlsrTransitionDelay", time);
    multiLinkElement.SetEmlsrTransitionDelay(time.Get());
  }

  auto &mldCapabilities =
      multiLinkElement.GetCommonInfoBasic().m_mldCapabilities;
  mldCapabilities.emplace();
  mldCapabilities->maxNSimultaneousLinks = GetNLinks() - 1;
  mldCapabilities->srsSupport = 0;

  auto ehtConfiguration = GetEhtConfiguration();
  NS_ASSERT(ehtConfiguration);
  EnumValue negSupport;
  ehtConfiguration->GetAttributeFailSafe("TidToLinkMappingNegSupport",
                                         negSupport);

  mldCapabilities->tidToLinkMappingSupport = negSupport.Get();
  mldCapabilities->freqSepForStrApMld = 0;
  mldCapabilities->aarSupport = 0;

  for (const auto &[index, link] : GetLinks()) {
    const auto &staLink = GetStaLink(link);

    if (index != linkId && staLink.bssid.has_value()) {
      multiLinkElement.AddPerStaProfileSubelement();
      auto &perStaProfile = multiLinkElement.GetPerStaProfile(
          multiLinkElement.GetNPerStaProfileSubelements() - 1);
      perStaProfile.SetLinkId(index);
      perStaProfile.SetCompleteProfile();
      perStaProfile.SetStaMacAddress(staLink.feManager->GetAddress());
      perStaProfile.SetAssocRequest(GetAssociationRequest(isReassoc, index));
    }
  }

  return multiLinkElement;
}

std::vector<TidToLinkMapping>
StaWifiMac::GetTidToLinkMappingElements(uint8_t apNegSupport) {
  NS_LOG_FUNCTION(this << apNegSupport);

  auto ehtConfig = GetEhtConfiguration();
  NS_ASSERT(ehtConfig);

  EnumValue negSupport;
  ehtConfig->GetAttributeFailSafe("TidToLinkMappingNegSupport", negSupport);

  NS_ABORT_MSG_IF(
      negSupport.Get() == 0,
      "Cannot request TID-to-Link Mapping if negotiation is not supported");

  m_dlTidLinkMappingInAssocReq =
      ehtConfig->GetTidLinkMapping(WifiDirection::DOWNLINK);
  m_ulTidLinkMappingInAssocReq =
      ehtConfig->GetTidLinkMapping(WifiDirection::UPLINK);

  bool mappingValidForNegType1 = TidToLinkMappingValidForNegType1(
      m_dlTidLinkMappingInAssocReq, m_ulTidLinkMappingInAssocReq);
  NS_ABORT_MSG_IF(negSupport.Get() == 1 && !mappingValidForNegType1,
                  "Mapping TIDs to distinct link sets is incompatible with "
                  "negotiation support of 1");

  if (apNegSupport == 1 && !mappingValidForNegType1) {
    NS_LOG_DEBUG("Using default mapping because AP MLD advertised negotiation "
                 "support of 1");
    m_dlTidLinkMappingInAssocReq.clear();
    m_ulTidLinkMappingInAssocReq.clear();
  }

  std::vector<TidToLinkMapping> ret(1);

  ret.back().m_control.direction = WifiDirection::DOWNLINK;

  auto fillIe = [&ret](const auto &mapping) {
    ret.back().m_control.defaultMapping = mapping.empty();

    for (const auto &[tid, linkSet] : mapping) {
      NS_ABORT_MSG_IF(linkSet.empty(), "Cannot map a TID to an empty link set");
      ret.back().SetLinkMappingOfTid(tid, linkSet);
    }
  };

  fillIe(m_dlTidLinkMappingInAssocReq);

  if (m_ulTidLinkMappingInAssocReq == m_dlTidLinkMappingInAssocReq) {
    ret.back().m_control.direction = WifiDirection::BOTH_DIRECTIONS;
    return ret;
  }

  ret.emplace_back();
  ret.back().m_control.direction = WifiDirection::UPLINK;
  fillIe(m_ulTidLinkMappingInAssocReq);

  return ret;
}

void StaWifiMac::SendAssociationRequest(bool isReassoc) {
  auto it = GetLinks().cbegin();
  while (it != GetLinks().cend()) {
    if (GetStaLink(it->second).sendAssocReq) {
      break;
    }
    it++;
  }
  NS_ABORT_MSG_IF(it == GetLinks().cend(),
                  "No link selected to send the (Re)Association Request");
  uint8_t linkId = it->first;
  auto &link = GetLink(linkId);
  NS_ABORT_MSG_IF(!link.bssid.has_value(),
                  "No BSSID set for the link on which the (Re)Association "
                  "Request is to be sent");

  NS_LOG_FUNCTION(this << *link.bssid << isReassoc);
  WifiMacHeader hdr;
  hdr.SetType(isReassoc ? WIFI_MAC_MGT_REASSOCIATION_REQUEST
                        : WIFI_MAC_MGT_ASSOCIATION_REQUEST);
  hdr.SetAddr1(*link.bssid);
  hdr.SetAddr2(link.feManager->GetAddress());
  hdr.SetAddr3(*link.bssid);
  hdr.SetDsNotFrom();
  hdr.SetDsNotTo();
  Ptr<Packet> packet = Create<Packet>();

  auto frame = GetAssociationRequest(isReassoc, linkId);

  if (GetNLinks() > 1 && GetWifiRemoteStationManager(linkId)
                             ->GetMldAddress(*link.bssid)
                             .has_value()) {
    auto addMle = [&](auto &&frame) {
      frame.template Get<MultiLinkElement>() =
          GetMultiLinkElement(isReassoc, linkId);
    };
    std::visit(addMle, frame);

    uint8_t negSupport;
    if (const auto &mldCapabilities =
            GetWifiRemoteStationManager(linkId)->GetStationMldCapabilities(
                *link.bssid);
        mldCapabilities &&
        (negSupport = mldCapabilities->get().tidToLinkMappingSupport) > 0) {
      auto addTlm = [&](auto &&frame) {
        frame.template Get<TidToLinkMapping>() =
            GetTidToLinkMappingElements(negSupport);
      };
      std::visit(addTlm, frame);
    }
  }

  if (!isReassoc) {
    packet->AddHeader(std::get<MgtAssocRequestHeader>(frame));
  } else {
    packet->AddHeader(std::get<MgtReassocRequestHeader>(frame));
  }

  if (!GetQosSupported()) {
    GetTxop()->Queue(packet, hdr);
  } else if (!GetWifiRemoteStationManager(linkId)->GetQosSupported(
                 *link.bssid)) {
    GetBEQueue()->Queue(packet, hdr);
  } else {
    GetVOQueue()->Queue(packet, hdr);
  }

  if (m_assocRequestEvent.IsRunning()) {
    m_assocRequestEvent.Cancel();
  }
  m_assocRequestEvent = Simulator::Schedule(
      m_assocRequestTimeout, &StaWifiMac::AssocRequestTimeout, this);
}

void StaWifiMac::TryToEnsureAssociated() {
  NS_LOG_FUNCTION(this);
  switch (m_state) {
  case ASSOCIATED:
    return;
  case SCANNING:
    break;
  case UNASSOCIATED:
    m_linkDown();
    StartScanning();
    break;
  case WAIT_ASSOC_RESP:
    break;
  case REFUSED:
    break;
  }
}

void StaWifiMac::StartScanning() {
  NS_LOG_FUNCTION(this);
  SetState(SCANNING);
  NS_ASSERT(m_assocManager);

  WifiScanParams scanParams;
  scanParams.ssid = GetSsid();
  for (const auto &[id, link] : GetLinks()) {
    WifiScanParams::ChannelList channel{
        (link->phy->HasFixedPhyBand())
            ? WifiScanParams::Channel{0, link->phy->GetPhyBand()}
            : WifiScanParams::Channel{0, WIFI_PHY_BAND_UNSPECIFIED}};

    scanParams.channelList.push_back(channel);
  }
  if (m_activeProbing) {
    scanParams.type = WifiScanType::ACTIVE;
    scanParams.probeDelay = MicroSeconds(m_probeDelay->GetValue());
    scanParams.minChannelTime = scanParams.maxChannelTime =
        m_probeRequestTimeout;
  } else {
    scanParams.type = WifiScanType::PASSIVE;
    scanParams.maxChannelTime = m_waitBeaconTimeout;
  }

  m_assocManager->StartScanning(std::move(scanParams));
}

void StaWifiMac::ScanningTimeout(const std::optional<ApInfo> &bestAp) {
  NS_LOG_FUNCTION(this);

  if (!bestAp.has_value()) {
    NS_LOG_DEBUG("Exhausted list of candidate AP; restart scanning");
    StartScanning();
    return;
  }

  NS_LOG_DEBUG("Attempting to associate with AP: " << *bestAp);
  UpdateApInfo(bestAp->m_frame, bestAp->m_apAddr, bestAp->m_bssid,
               bestAp->m_linkId);
  for (auto &[id, link] : GetLinks()) {
    auto &staLink = GetStaLink(link);
    staLink.sendAssocReq = false;
    staLink.bssid = std::nullopt;
  }
  GetLink(bestAp->m_linkId).sendAssocReq = true;
  GetLink(bestAp->m_linkId).bssid = bestAp->m_bssid;
  std::shared_ptr<CommonInfoBasicMle> mleCommonInfo;
  const auto &mle = std::visit(
      [](auto &&frame) { return frame.template Get<MultiLinkElement>(); },
      bestAp->m_frame);
  std::map<uint8_t, uint8_t> swapInfo;
  for (const auto &[localLinkId, apLinkId, bssid] : bestAp->m_setupLinks) {
    NS_ASSERT_MSG(mle, "We get here only for ML setup");
    NS_LOG_DEBUG("Setting up link (local ID=" << +localLinkId << ", AP ID="
                                              << +apLinkId << ")");
    GetLink(localLinkId).bssid = bssid;
    if (!mleCommonInfo) {
      mleCommonInfo =
          std::make_shared<CommonInfoBasicMle>(mle->GetCommonInfoBasic());
    }
    GetWifiRemoteStationManager(localLinkId)
        ->AddStationMleCommonInfo(bssid, mleCommonInfo);
    swapInfo.emplace(localLinkId, apLinkId);
  }

  SwapLinks(swapInfo);

  auto getBeaconInterval = [](auto &&frame) {
    using T = std::decay_t<decltype(frame)>;
    if constexpr (std::is_same_v<T, MgtBeaconHeader> ||
                  std::is_same_v<T, MgtProbeResponseHeader>) {
      return MicroSeconds(frame.GetBeaconIntervalUs());
    } else {
      NS_ABORT_MSG("Unexpected frame type");
      return Seconds(0);
    }
  };
  Time beaconInterval = std::visit(getBeaconInterval, bestAp->m_frame);
  Time delay = beaconInterval * m_maxMissedBeacons;
  for (const auto &[id, link] : GetLinks()) {
    if (GetStaLink(link).bssid.has_value() || GetNLinks() == 1) {
      RestartBeaconWatchdog(delay, id);
    }
  }
  SetState(WAIT_ASSOC_RESP);
  SendAssociationRequest(false);
}

void StaWifiMac::AssocRequestTimeout() {
  NS_LOG_FUNCTION(this);
  SetState(WAIT_ASSOC_RESP);
  SendAssociationRequest(false);
}

void StaWifiMac::MissedBeacons(uint8_t linkId) {
  NS_LOG_FUNCTION(this << +linkId);
  auto &link = GetLink(linkId);
  if (link.beaconWatchdogEnd > Simulator::Now()) {
    if (link.beaconWatchdog.IsRunning()) {
      link.beaconWatchdog.Cancel();
    }
    link.beaconWatchdog =
        Simulator::Schedule(link.beaconWatchdogEnd - Simulator::Now(),
                            &StaWifiMac::MissedBeacons, this, linkId);
    return;
  }
  NS_LOG_DEBUG("beacon missed");
  Time delay = Seconds(0);
  if (GetWifiPhy(linkId)->IsStateRx()) {
    delay = GetWifiPhy(linkId)->GetDelayUntilIdle();
  }
  Simulator::Schedule(delay, &StaWifiMac::Disassociated, this, linkId);
}

void StaWifiMac::Disassociated(uint8_t linkId) {
  NS_LOG_FUNCTION(this << +linkId);

  auto &link = GetLink(linkId);
  if (link.bssid.has_value()) {
    m_setupCanceled(linkId, GetBssid(linkId));
  }

  link.bssid = std::nullopt;
  link.phy->SetOffMode();

  for (const auto &[id, lnk] : GetLinks()) {
    if (GetStaLink(lnk).bssid.has_value()) {
      return;
    }
  }

  NS_LOG_DEBUG("Set state to UNASSOCIATED and start scanning");
  SetState(UNASSOCIATED);
  m_assocRequestEvent.Cancel();
  auto mldAddress =
      GetWifiRemoteStationManager(linkId)->GetMldAddress(GetBssid(linkId));
  if (GetNLinks() > 1 && mldAddress.has_value()) {
    m_deAssocLogger(*mldAddress);
  } else {
    m_deAssocLogger(GetBssid(linkId));
  }
  m_aid = 0;
  for (const auto &[id, lnk] : GetLinks()) {
    lnk->phy->ResumeFromOff();
  }
  TryToEnsureAssociated();
}

void StaWifiMac::RestartBeaconWatchdog(Time delay, uint8_t linkId) {
  NS_LOG_FUNCTION(this << delay << +linkId);
  auto &link = GetLink(linkId);
  link.beaconWatchdogEnd =
      std::max(Simulator::Now() + delay, link.beaconWatchdogEnd);
  if (Simulator::GetDelayLeft(link.beaconWatchdog) < delay &&
      link.beaconWatchdog.IsExpired()) {
    NS_LOG_DEBUG("really restart watchdog.");
    link.beaconWatchdog =
        Simulator::Schedule(delay, &StaWifiMac::MissedBeacons, this, linkId);
  }
}

bool StaWifiMac::IsAssociated() const { return m_state == ASSOCIATED; }

bool StaWifiMac::IsWaitAssocResp() const { return m_state == WAIT_ASSOC_RESP; }

std::set<uint8_t> StaWifiMac::GetSetupLinkIds() const {
  if (!IsAssociated()) {
    return {};
  }

  std::set<uint8_t> linkIds;
  for (const auto &[id, link] : GetLinks()) {
    if (GetStaLink(link).bssid) {
      linkIds.insert(id);
    }
  }
  return linkIds;
}

Mac48Address
StaWifiMac::DoGetLocalAddress(const Mac48Address &remoteAddr) const {
  auto linkIds = GetSetupLinkIds();
  NS_ASSERT_MSG(!linkIds.empty(), "Not associated");
  uint8_t linkId = *linkIds.begin();
  return GetFrameExchangeManager(linkId)->GetAddress();
}

bool StaWifiMac::CanForwardPacketsTo(Mac48Address to) const {
  return (IsAssociated());
}

void StaWifiMac::Enqueue(Ptr<Packet> packet, Mac48Address to) {
  NS_LOG_FUNCTION(this << packet << to);
  if (!CanForwardPacketsTo(to)) {
    NotifyTxDrop(packet);
    TryToEnsureAssociated();
    return;
  }
  WifiMacHeader hdr;

  uint8_t tid = 0;

  if (GetQosSupported()) {
    hdr.SetType(WIFI_MAC_QOSDATA);
    hdr.SetQosAckPolicy(WifiMacHeader::NORMAL_ACK);
    hdr.SetQosNoEosp();
    hdr.SetQosNoAmsdu();
    hdr.SetQosTxopLimit(0);

    tid = QosUtilsGetTidForPacket(packet);
    if (tid > 7) {
      tid = 0;
    }
    hdr.SetQosTid(tid);
  } else {
    hdr.SetType(WIFI_MAC_DATA);
  }
  if (GetQosSupported()) {
    hdr.SetNoOrder();
  }

  auto linkIds = GetSetupLinkIds();
  NS_ASSERT(!linkIds.empty());
  uint8_t linkId = *linkIds.begin();
  if (const auto apMldAddr = GetWifiRemoteStationManager(linkId)->GetMldAddress(
          GetBssid(linkId))) {
    hdr.SetAddr1(*apMldAddr);
    hdr.SetAddr2(GetAddress());
  } else {
    hdr.SetAddr1(GetBssid(linkId));
    hdr.SetAddr2(GetFrameExchangeManager(linkId)->GetAddress());
  }

  hdr.SetAddr3(to);
  hdr.SetDsNotFrom();
  hdr.SetDsTo();

  if (GetQosSupported()) {
    NS_ASSERT(tid < 8);
    GetQosTxop(tid)->Queue(packet, hdr);
  } else {
    GetTxop()->Queue(packet, hdr);
  }
}

void StaWifiMac::BlockTxOnLink(uint8_t linkId, WifiQueueBlockedReason reason) {
  NS_LOG_FUNCTION(this << linkId << reason);

  auto bssid = GetBssid(linkId);
  auto apAddress =
      GetWifiRemoteStationManager(linkId)->GetMldAddress(bssid).value_or(bssid);

  BlockUnicastTxOnLinks(reason, apAddress, {linkId});
  for (const auto [acIndex, ac] : wifiAcList) {
    GetMacQueueScheduler()->BlockQueues(
        reason, acIndex, {WIFI_MGT_QUEUE}, Mac48Address::GetBroadcast(),
        GetFrameExchangeManager(linkId)->GetAddress(), {}, {linkId});
  }
}

void StaWifiMac::UnblockTxOnLink(uint8_t linkId,
                                 WifiQueueBlockedReason reason) {
  NS_LOG_FUNCTION(this << linkId << reason);

  auto bssid = GetBssid(linkId);
  auto apAddress =
      GetWifiRemoteStationManager(linkId)->GetMldAddress(bssid).value_or(bssid);

  UnblockUnicastTxOnLinks(reason, apAddress, {linkId});
  for (const auto [acIndex, ac] : wifiAcList) {
    GetMacQueueScheduler()->UnblockQueues(
        reason, acIndex, {WIFI_MGT_QUEUE}, Mac48Address::GetBroadcast(),
        GetFrameExchangeManager(linkId)->GetAddress(), {}, {linkId});
  }
}

void StaWifiMac::Receive(Ptr<const WifiMpdu> mpdu, uint8_t linkId) {
  NS_LOG_FUNCTION(this << *mpdu << +linkId);
  const WifiMacHeader *hdr = &mpdu->GetOriginal()->GetHeader();
  Ptr<const Packet> packet = mpdu->GetPacket();
  NS_ASSERT(!hdr->IsCtl());
  Mac48Address myAddr =
      hdr->IsData() ? Mac48Address::ConvertFrom(GetDevice()->GetAddress())
                    : GetFrameExchangeManager(linkId)->GetAddress();
  if (hdr->GetAddr3() == myAddr) {
    NS_LOG_LOGIC("packet sent by us.");
    return;
  }
  if (hdr->GetAddr1() != myAddr && !hdr->GetAddr1().IsGroup()) {
    NS_LOG_LOGIC("packet is not for us");
    NotifyRxDrop(packet);
    return;
  }
  if (hdr->IsData()) {
    if (!IsAssociated()) {
      NS_LOG_LOGIC("Received data frame while not associated: ignore");
      NotifyRxDrop(packet);
      return;
    }
    if (!(hdr->IsFromDs() && !hdr->IsToDs())) {
      NS_LOG_LOGIC("Received data frame not from the DS: ignore");
      NotifyRxDrop(packet);
      return;
    }
    std::set<Mac48Address> apAddresses;
    for (auto id : GetSetupLinkIds()) {
      apAddresses.insert(GetBssid(id));
    }
    if (apAddresses.count(mpdu->GetHeader().GetAddr2()) == 0) {
      NS_LOG_LOGIC("Received data frame not from the BSS we are associated "
                   "with: ignore");
      NotifyRxDrop(packet);
      return;
    }
    if (!hdr->HasData()) {
      NS_LOG_LOGIC("Received (QoS) Null Data frame: ignore");
      NotifyRxDrop(packet);
      return;
    }
    if (hdr->IsQosData()) {
      if (hdr->IsQosAmsdu()) {
        NS_ASSERT(apAddresses.count(mpdu->GetHeader().GetAddr3()) != 0);
        DeaggregateAmsduAndForward(mpdu);
        packet = nullptr;
      } else {
        ForwardUp(packet, hdr->GetAddr3(), hdr->GetAddr1());
      }
    } else {
      ForwardUp(packet, hdr->GetAddr3(), hdr->GetAddr1());
    }
    return;
  }

  switch (hdr->GetType()) {
  case WIFI_MAC_MGT_PROBE_REQUEST:
  case WIFI_MAC_MGT_ASSOCIATION_REQUEST:
  case WIFI_MAC_MGT_REASSOCIATION_REQUEST:
    NotifyRxDrop(packet);
    break;

  case WIFI_MAC_MGT_BEACON:
    ReceiveBeacon(mpdu, linkId);
    break;

  case WIFI_MAC_MGT_PROBE_RESPONSE:
    ReceiveProbeResp(mpdu, linkId);
    break;

  case WIFI_MAC_MGT_ASSOCIATION_RESPONSE:
  case WIFI_MAC_MGT_REASSOCIATION_RESPONSE:
    ReceiveAssocResp(mpdu, linkId);
    break;

  case WIFI_MAC_MGT_ACTION:
    if (auto [category, action] = WifiActionHeader::Peek(packet);
        category == WifiActionHeader::PROTECTED_EHT &&
        action.protectedEhtAction ==
            WifiActionHeader::PROTECTED_EHT_EML_OPERATING_MODE_NOTIFICATION) {
      break;
    }

  default:
    WifiMac::Receive(mpdu, linkId);
  }

  if (m_emlsrManager) {
    m_emlsrManager->NotifyMgtFrameReceived(mpdu, linkId);
  }
}

void StaWifiMac::ReceiveBeacon(Ptr<const WifiMpdu> mpdu, uint8_t linkId) {
  NS_LOG_FUNCTION(this << *mpdu << +linkId);
  const WifiMacHeader &hdr = mpdu->GetHeader();
  NS_ASSERT(hdr.IsBeacon());

  NS_LOG_DEBUG("Beacon received");
  MgtBeaconHeader beacon;
  mpdu->GetPacket()->PeekHeader(beacon);
  const auto &capabilities = beacon.Capabilities();
  NS_ASSERT(capabilities.IsEss());
  bool goodBeacon;
  if (IsWaitAssocResp() || IsAssociated()) {
    auto bssid = GetLink(linkId).bssid;
    goodBeacon = bssid.has_value() && (hdr.GetAddr3() == *bssid);
  } else {
    goodBeacon = CheckSupportedRates(beacon, linkId);
  }

  SnrTag snrTag;
  bool found = mpdu->GetPacket()->PeekPacketTag(snrTag);
  NS_ASSERT(found);
  ApInfo apInfo = {.m_bssid = hdr.GetAddr3(),
                   .m_apAddr = hdr.GetAddr2(),
                   .m_snr = snrTag.Get(),
                   .m_frame = std::move(beacon),
                   .m_channel = {GetCurrentChannel(linkId)},
                   .m_linkId = linkId};

  if (!m_beaconInfo.IsEmpty()) {
    m_beaconInfo(apInfo);
  }

  if (!goodBeacon) {
    NS_LOG_LOGIC("Beacon is not for us");
    return;
  }
  if (m_state == ASSOCIATED) {
    m_beaconArrival(Simulator::Now());
    Time delay = MicroSeconds(
        std::get<MgtBeaconHeader>(apInfo.m_frame).GetBeaconIntervalUs() *
        m_maxMissedBeacons);
    RestartBeaconWatchdog(delay, linkId);
    UpdateApInfo(apInfo.m_frame, hdr.GetAddr2(), hdr.GetAddr3(), linkId);
  } else {
    NS_LOG_DEBUG("Beacon received from " << hdr.GetAddr2());
    m_assocManager->NotifyApInfo(std::move(apInfo));
  }
}

void StaWifiMac::ReceiveProbeResp(Ptr<const WifiMpdu> mpdu, uint8_t linkId) {
  NS_LOG_FUNCTION(this << *mpdu << +linkId);
  const WifiMacHeader &hdr = mpdu->GetHeader();
  NS_ASSERT(hdr.IsProbeResp());

  NS_LOG_DEBUG("Probe response received from " << hdr.GetAddr2());
  MgtProbeResponseHeader probeResp;
  mpdu->GetPacket()->PeekHeader(probeResp);
  if (!CheckSupportedRates(probeResp, linkId)) {
    return;
  }
  SnrTag snrTag;
  bool found = mpdu->GetPacket()->PeekPacketTag(snrTag);
  NS_ASSERT(found);
  m_assocManager->NotifyApInfo(ApInfo{.m_bssid = hdr.GetAddr3(),
                                      .m_apAddr = hdr.GetAddr2(),
                                      .m_snr = snrTag.Get(),
                                      .m_frame = std::move(probeResp),
                                      .m_channel = {GetCurrentChannel(linkId)},
                                      .m_linkId = linkId});
}

void StaWifiMac::ReceiveAssocResp(Ptr<const WifiMpdu> mpdu, uint8_t linkId) {
  NS_LOG_FUNCTION(this << *mpdu << +linkId);
  const WifiMacHeader &hdr = mpdu->GetHeader();
  NS_ASSERT(hdr.IsAssocResp() || hdr.IsReassocResp());

  if (m_state != WAIT_ASSOC_RESP) {
    return;
  }

  std::optional<Mac48Address> apMldAddress;
  MgtAssocResponseHeader assocResp;
  mpdu->GetPacket()->PeekHeader(assocResp);
  if (m_assocRequestEvent.IsRunning()) {
    m_assocRequestEvent.Cancel();
  }
  if (assocResp.GetStatusCode().IsSuccess()) {
    m_aid = assocResp.GetAssociationId();
    NS_LOG_DEBUG(
        (hdr.IsReassocResp() ? "reassociation done" : "association completed"));
    UpdateApInfo(assocResp, hdr.GetAddr2(), hdr.GetAddr3(), linkId);
    NS_ASSERT(GetLink(linkId).bssid.has_value() &&
              *GetLink(linkId).bssid == hdr.GetAddr3());
    SetBssid(hdr.GetAddr3(), linkId);
    SetState(ASSOCIATED);
    if ((GetNLinks() > 1) && assocResp.Get<MultiLinkElement>().has_value()) {
      m_setupCompleted(linkId, hdr.GetAddr3());
      apMldAddress =
          GetWifiRemoteStationManager(linkId)->GetMldAddress(hdr.GetAddr3());
      NS_ASSERT(apMldAddress);

      if (const auto &mldCapabilities =
              GetWifiRemoteStationManager(linkId)->GetStationMldCapabilities(
                  hdr.GetAddr3());
          mldCapabilities &&
          mldCapabilities->get().tidToLinkMappingSupport > 0) {
        if (assocResp.Get<TidToLinkMapping>().empty()) {
          UpdateTidToLinkMapping(*apMldAddress, WifiDirection::DOWNLINK,
                                 m_dlTidLinkMappingInAssocReq);
          UpdateTidToLinkMapping(*apMldAddress, WifiDirection::UPLINK,
                                 m_ulTidLinkMappingInAssocReq);

          ApplyTidLinkMapping(*apMldAddress, WifiDirection::UPLINK);
        }
      }
    } else {
      m_assocLogger(hdr.GetAddr3());
    }
    if (!m_linkUp.IsNull()) {
      m_linkUp();
    }
  } else {
    NS_LOG_DEBUG("association refused");
    SetState(REFUSED);
    StartScanning();
    return;
  }

  if (GetNLinks() > 1) {
    std::list<uint8_t> setupLinks;
    for (const auto &[id, link] : GetLinks()) {
      setupLinks.push_back(id);
    }
    if (assocResp.GetStatusCode().IsSuccess()) {
      setupLinks.remove(linkId);
    }

    if (const auto &mle = assocResp.Get<MultiLinkElement>()) {
      NS_ABORT_MSG_IF(!GetLink(linkId).bssid.has_value(),
                      "The link on which the Association Response was received "
                      "is not a link we requested to setup");
      NS_ABORT_MSG_IF(linkId != mle->GetLinkIdInfo(),
                      "The link ID of the AP that transmitted the Association "
                      "Response does not match the stored link ID");
      NS_ABORT_MSG_IF(
          GetWifiRemoteStationManager(linkId)->GetMldAddress(hdr.GetAddr2()) !=
              mle->GetMldMacAddress(),
          "The AP MLD MAC address in the received Multi-Link Element does not "
          "match the address stored in the station manager for link "
              << +linkId);
      for (std::size_t elem = 0; elem < mle->GetNPerStaProfileSubelements();
           elem++) {
        auto &perStaProfile = mle->GetPerStaProfile(elem);
        uint8_t apLinkId = perStaProfile.GetLinkId();
        auto it = GetLinks().find(apLinkId);
        uint8_t staLinkid = 0;
        std::optional<Mac48Address> bssid;
        NS_ABORT_MSG_IF(
            it == GetLinks().cend() ||
                !(bssid = GetLink((staLinkid = it->first)).bssid).has_value(),
            "Setup for AP link ID " << apLinkId << " was not requested");
        NS_ABORT_MSG_IF(*bssid != perStaProfile.GetStaMacAddress(),
                        "The BSSID in the Per-STA Profile for link ID "
                            << +staLinkid
                            << " does not match the stored BSSID");
        NS_ABORT_MSG_IF(
            GetWifiRemoteStationManager(staLinkid)->GetMldAddress(
                perStaProfile.GetStaMacAddress()) != mle->GetMldMacAddress(),
            "The AP MLD MAC address in the received Multi-Link Element does "
            "not "
            "match the address stored in the station manager for link "
                << +staLinkid);
        MgtAssocResponseHeader assoc = perStaProfile.GetAssocResponse();
        if (assoc.GetStatusCode().IsSuccess()) {
          NS_ABORT_MSG_IF(m_aid != 0 && m_aid != assoc.GetAssociationId(),
                          "AID should be the same for all the links");
          m_aid = assoc.GetAssociationId();
          NS_LOG_DEBUG("Setup on link " << staLinkid << " completed");
          UpdateApInfo(assoc, *bssid, *bssid, staLinkid);
          SetBssid(*bssid, staLinkid);
          m_setupCompleted(staLinkid, *bssid);
          SetState(ASSOCIATED);
          apMldAddress =
              GetWifiRemoteStationManager(staLinkid)->GetMldAddress(*bssid);
          if (!m_linkUp.IsNull()) {
            m_linkUp();
          }
        }
        setupLinks.remove(staLinkid);
      }
    }
    for (const auto &id : setupLinks) {
      GetLink(id).bssid = std::nullopt;
      GetLink(id).phy->SetOffMode();
    }
    if (apMldAddress) {
      m_assocLogger(*apMldAddress);
    }
  }

  SetPmModeAfterAssociation(linkId);
}

void StaWifiMac::SetPmModeAfterAssociation(uint8_t linkId) {
  NS_LOG_FUNCTION(this << linkId);

  CallbackBase cb = Callback<void, WifiConstPsduMap, WifiTxVector, double>(
      [=](WifiConstPsduMap psduMap, WifiTxVector txVector, double) {
        NS_ASSERT_MSG(psduMap.size() == 1 &&
                          psduMap.begin()->second->GetNMpdus() == 1 &&
                          psduMap.begin()->second->GetHeader(0).IsAck(),
                      "Expected a Normal Ack after Association Response frame");

        auto ackDuration = WifiPhy::CalculateTxDuration(
            psduMap, txVector, GetLink(linkId).phy->GetPhyBand());

        for (const auto &[id, lnk] : GetLinks()) {
          auto &link = GetStaLink(lnk);

          if (!link.bssid) {
            continue;
          }

          if (id == linkId) {
            if (link.pmMode == WIFI_PM_POWERSAVE) {
              Simulator::Schedule(ackDuration, &StaWifiMac::SetPowerSaveMode,
                                  this, std::pair<bool, uint8_t>{true, id});
            }
            link.pmMode = WIFI_PM_ACTIVE;
          } else {
            if (link.pmMode == WIFI_PM_ACTIVE) {
              Simulator::Schedule(ackDuration, &StaWifiMac::SetPowerSaveMode,
                                  this, std::pair<bool, uint8_t>{false, id});
            }
            link.pmMode = WIFI_PM_POWERSAVE;
          }
        }
      });

  auto phy = GetLink(linkId).phy;
  phy->TraceConnectWithoutContext("PhyTxPsduBegin", cb);
  Simulator::Schedule(phy->GetSifs() + NanoSeconds(1), [=]() {
    phy->TraceDisconnectWithoutContext("PhyTxPsduBegin", cb);
  });
}

bool StaWifiMac::CheckSupportedRates(
    std::variant<MgtBeaconHeader, MgtProbeResponseHeader> frame,
    uint8_t linkId) {
  NS_LOG_FUNCTION(this << +linkId);

  auto check = [&](auto &&mgtFrame) -> bool {
    NS_ASSERT(mgtFrame.template Get<SupportedRates>());
    const auto rates =
        AllSupportedRates{*mgtFrame.template Get<SupportedRates>(),
                          mgtFrame.template Get<ExtendedSupportedRatesIE>()};
    for (const auto &selector :
         GetWifiPhy(linkId)->GetBssMembershipSelectorList()) {
      if (!rates.IsBssMembershipSelectorRate(selector)) {
        NS_LOG_DEBUG(
            "Supported rates do not fit with the BSS membership selector");
        return false;
      }
    }

    return true;
  };

  return std::visit(check, frame);
}

void StaWifiMac::UpdateApInfo(const MgtFrameType &frame,
                              const Mac48Address &apAddr,
                              const Mac48Address &bssid, uint8_t linkId) {
  NS_LOG_FUNCTION(this << frame.index() << apAddr << bssid << +linkId);

  const std::optional<ErpInformation> *erpInformation = nullptr;

  if (const auto *beacon = std::get_if<MgtBeaconHeader>(&frame)) {
    erpInformation = &beacon->Get<ErpInformation>();
  } else if (const auto *probe = std::get_if<MgtProbeResponseHeader>(&frame)) {
    erpInformation = &probe->Get<ErpInformation>();
  }

  auto commonOps = [&](auto &&frame) {
    const auto &capabilities = frame.Capabilities();
    NS_ASSERT(frame.template Get<SupportedRates>());
    const auto rates =
        AllSupportedRates{*frame.template Get<SupportedRates>(),
                          frame.template Get<ExtendedSupportedRatesIE>()};
    for (const auto &mode : GetWifiPhy(linkId)->GetModeList()) {
      if (rates.IsSupportedRate(
              mode.GetDataRate(GetWifiPhy(linkId)->GetChannelWidth()))) {
        GetWifiRemoteStationManager(linkId)->AddSupportedMode(apAddr, mode);
        if (rates.IsBasicRate(
                mode.GetDataRate(GetWifiPhy(linkId)->GetChannelWidth()))) {
          GetWifiRemoteStationManager(linkId)->AddBasicMode(mode);
        }
      }
    }

    bool isShortPreambleEnabled = capabilities.IsShortPreamble();
    if (erpInformation && erpInformation->has_value() &&
        GetErpSupported(linkId)) {
      isShortPreambleEnabled &= !(*erpInformation)->GetBarkerPreambleMode();
      if ((*erpInformation)->GetUseProtection() != 0) {
        GetWifiRemoteStationManager(linkId)->SetUseNonErpProtection(true);
      } else {
        GetWifiRemoteStationManager(linkId)->SetUseNonErpProtection(false);
      }
      if (capabilities.IsShortSlotTime() == true) {
        GetWifiPhy(linkId)->SetSlot(MicroSeconds(9));
      } else {
        GetWifiPhy(linkId)->SetSlot(MicroSeconds(20));
      }
    }
    GetWifiRemoteStationManager(linkId)->SetShortPreambleEnabled(
        isShortPreambleEnabled);
    GetWifiRemoteStationManager(linkId)->SetShortSlotTimeEnabled(
        capabilities.IsShortSlotTime());

    if (!GetQosSupported()) {
      return;
    }
    bool qosSupported = false;
    const auto &edcaParameters = frame.template Get<EdcaParameterSet>();
    if (edcaParameters.has_value()) {
      qosSupported = true;
      SetEdcaParameters({AC_BE, edcaParameters->GetBeCWmin(),
                         edcaParameters->GetBeCWmax(),
                         edcaParameters->GetBeAifsn(),
                         32 * MicroSeconds(edcaParameters->GetBeTxopLimit())},
                        linkId);
      SetEdcaParameters({AC_BK, edcaParameters->GetBkCWmin(),
                         edcaParameters->GetBkCWmax(),
                         edcaParameters->GetBkAifsn(),
                         32 * MicroSeconds(edcaParameters->GetBkTxopLimit())},
                        linkId);
      SetEdcaParameters({AC_VI, edcaParameters->GetViCWmin(),
                         edcaParameters->GetViCWmax(),
                         edcaParameters->GetViAifsn(),
                         32 * MicroSeconds(edcaParameters->GetViTxopLimit())},
                        linkId);
      SetEdcaParameters({AC_VO, edcaParameters->GetVoCWmin(),
                         edcaParameters->GetVoCWmax(),
                         edcaParameters->GetVoAifsn(),
                         32 * MicroSeconds(edcaParameters->GetVoTxopLimit())},
                        linkId);
    }
    GetWifiRemoteStationManager(linkId)->SetQosSupport(apAddr, qosSupported);

    if (!GetHtSupported()) {
      return;
    }
    if (const auto &htCapabilities = frame.template Get<HtCapabilities>();
        htCapabilities.has_value()) {
      if (!htCapabilities->IsSupportedMcs(0)) {
        GetWifiRemoteStationManager(linkId)->RemoveAllSupportedMcs(apAddr);
      } else {
        GetWifiRemoteStationManager(linkId)->AddStationHtCapabilities(
            apAddr, *htCapabilities);
      }
    }

    if (GetVhtSupported(linkId)) {
      const auto &vhtCapabilities = frame.template Get<VhtCapabilities>();
      if (vhtCapabilities.has_value() &&
          vhtCapabilities->GetRxHighestSupportedLgiDataRate() > 0) {
        GetWifiRemoteStationManager(linkId)->AddStationVhtCapabilities(
            apAddr, *vhtCapabilities);
        for (const auto &mcs :
             GetWifiPhy(linkId)->GetMcsList(WIFI_MOD_CLASS_VHT)) {
          if (vhtCapabilities->IsSupportedRxMcs(mcs.GetMcsValue())) {
            GetWifiRemoteStationManager(linkId)->AddSupportedMcs(apAddr, mcs);
          }
        }
      }
    }

    if (!GetHeSupported()) {
      return;
    }
    const auto &heCapabilities = frame.template Get<HeCapabilities>();
    if (heCapabilities.has_value() &&
        heCapabilities->GetSupportedMcsAndNss() != 0) {
      GetWifiRemoteStationManager(linkId)->AddStationHeCapabilities(
          apAddr, *heCapabilities);
      for (const auto &mcs :
           GetWifiPhy(linkId)->GetMcsList(WIFI_MOD_CLASS_HE)) {
        if (heCapabilities->IsSupportedRxMcs(mcs.GetMcsValue())) {
          GetWifiRemoteStationManager(linkId)->AddSupportedMcs(apAddr, mcs);
        }
      }
      if (const auto &heOperation = frame.template Get<HeOperation>();
          heOperation.has_value()) {
        GetHeConfiguration()->SetAttribute(
            "BssColor", UintegerValue(heOperation->GetBssColor()));
      }
    }

    const auto &muEdcaParameters = frame.template Get<MuEdcaParameterSet>();
    if (muEdcaParameters.has_value()) {
      SetMuEdcaParameters({AC_BE, muEdcaParameters->GetMuCwMin(AC_BE),
                           muEdcaParameters->GetMuCwMax(AC_BE),
                           muEdcaParameters->GetMuAifsn(AC_BE),
                           muEdcaParameters->GetMuEdcaTimer(AC_BE)},
                          linkId);
      SetMuEdcaParameters({AC_BK, muEdcaParameters->GetMuCwMin(AC_BK),
                           muEdcaParameters->GetMuCwMax(AC_BK),
                           muEdcaParameters->GetMuAifsn(AC_BK),
                           muEdcaParameters->GetMuEdcaTimer(AC_BK)},
                          linkId);
      SetMuEdcaParameters({AC_VI, muEdcaParameters->GetMuCwMin(AC_VI),
                           muEdcaParameters->GetMuCwMax(AC_VI),
                           muEdcaParameters->GetMuAifsn(AC_VI),
                           muEdcaParameters->GetMuEdcaTimer(AC_VI)},
                          linkId);
      SetMuEdcaParameters({AC_VO, muEdcaParameters->GetMuCwMin(AC_VO),
                           muEdcaParameters->GetMuCwMax(AC_VO),
                           muEdcaParameters->GetMuAifsn(AC_VO),
                           muEdcaParameters->GetMuEdcaTimer(AC_VO)},
                          linkId);
    }

    if (!GetEhtSupported()) {
      return;
    }
    const auto &ehtCapabilities = frame.template Get<EhtCapabilities>();
    GetWifiRemoteStationManager(linkId)->AddStationEhtCapabilities(
        apAddr, *ehtCapabilities);

    if (const auto &mle = frame.template Get<MultiLinkElement>();
        mle && mle->HasEmlCapabilities() && m_emlsrManager) {
      m_emlsrManager->SetTransitionTimeout(mle->GetTransitionTimeout());
    }
  };

  std::visit(commonOps, frame);
}

void StaWifiMac::SetPowerSaveMode(
    const std::pair<bool, uint8_t> &enableLinkIdPair) {
  const auto [enable, linkId] = enableLinkIdPair;
  NS_LOG_FUNCTION(this << enable << linkId);

  auto &link = GetLink(linkId);

  if (!IsAssociated()) {
    NS_LOG_DEBUG(
        "Not associated yet, record the PM mode to switch to upon association");
    link.pmMode = enable ? WIFI_PM_POWERSAVE : WIFI_PM_ACTIVE;
    return;
  }

  if (!link.bssid) {
    NS_LOG_DEBUG("Link " << +linkId << " has not been setup, ignore request");
    return;
  }

  if ((enable && link.pmMode == WIFI_PM_POWERSAVE) ||
      (!enable && link.pmMode == WIFI_PM_ACTIVE)) {
    NS_LOG_DEBUG("No PM mode change needed");
    return;
  }

  link.pmMode = enable ? WIFI_PM_SWITCHING_TO_PS : WIFI_PM_SWITCHING_TO_ACTIVE;

  Simulator::Schedule(m_pmModeSwitchTimeout, &StaWifiMac::SetPowerSaveMode,
                      this, enableLinkIdPair);

  if (HasFramesToTransmit(linkId)) {
    NS_LOG_DEBUG("Next transmitted frame will be sent with PM=" << enable);
    return;
  }

  WifiMacHeader hdr(WIFI_MAC_DATA_NULL);

  hdr.SetAddr1(GetBssid(linkId));
  hdr.SetAddr2(GetFrameExchangeManager(linkId)->GetAddress());
  hdr.SetAddr3(GetBssid(linkId));
  hdr.SetDsNotFrom();
  hdr.SetDsTo();
  enable ? hdr.SetPowerManagement() : hdr.SetNoPowerManagement();
  if (GetQosSupported()) {
    GetQosTxop(AC_BE)->Queue(Create<WifiMpdu>(Create<Packet>(), hdr));
  } else {
    m_txop->Queue(Create<WifiMpdu>(Create<Packet>(), hdr));
  }
}

WifiPowerManagementMode StaWifiMac::GetPmMode(uint8_t linkId) const {
  return GetLink(linkId).pmMode;
}

void StaWifiMac::TxOk(Ptr<const WifiMpdu> mpdu) {
  NS_LOG_FUNCTION(this << *mpdu);

  auto linkId = GetLinkIdByAddress(mpdu->GetHeader().GetAddr2());

  if (!linkId) {
    auto linkIds = mpdu->GetInFlightLinkIds();
    NS_ASSERT_MSG(!linkIds.empty(),
                  "The TA of the acked MPDU ("
                      << *mpdu
                      << ") is not a link "
                         "address and the MPDU is not inflight");
    linkId = *linkIds.begin();
    mpdu = GetTxopQueue(mpdu->GetQueueAc())->GetAlias(mpdu, *linkId);
  }

  auto &link = GetLink(*linkId);
  const WifiMacHeader &hdr = mpdu->GetHeader();

  if (hdr.IsPowerManagement() && link.pmMode == WIFI_PM_SWITCHING_TO_PS) {
    link.pmMode = WIFI_PM_POWERSAVE;
  } else if (!hdr.IsPowerManagement() &&
             link.pmMode == WIFI_PM_SWITCHING_TO_ACTIVE) {
    link.pmMode = WIFI_PM_ACTIVE;
  }
}

AllSupportedRates StaWifiMac::GetSupportedRates(uint8_t linkId) const {
  AllSupportedRates rates;
  for (const auto &mode : GetWifiPhy(linkId)->GetModeList()) {
    uint64_t modeDataRate =
        mode.GetDataRate(GetWifiPhy(linkId)->GetChannelWidth());
    NS_LOG_DEBUG("Adding supported rate of " << modeDataRate);
    rates.AddSupportedRate(modeDataRate);
  }
  if (GetHtSupported()) {
    for (const auto &selector :
         GetWifiPhy(linkId)->GetBssMembershipSelectorList()) {
      rates.AddBssMembershipSelectorRate(selector);
    }
  }
  return rates;
}

CapabilityInformation StaWifiMac::GetCapabilities(uint8_t linkId) const {
  CapabilityInformation capabilities;
  capabilities.SetShortPreamble(
      GetWifiPhy(linkId)->GetShortPhyPreambleSupported() ||
      GetErpSupported(linkId));
  capabilities.SetShortSlotTime(GetShortSlotTimeSupported() &&
                                GetErpSupported(linkId));
  return capabilities;
}

void StaWifiMac::SetState(MacState value) { m_state = value; }

void StaWifiMac::SetEdcaParameters(const EdcaParams &params, uint8_t linkId) {
  Ptr<QosTxop> edca = GetQosTxop(params.ac);
  edca->SetMinCw(params.cwMin, linkId);
  edca->SetMaxCw(params.cwMax, linkId);
  edca->SetAifsn(params.aifsn, linkId);
  edca->SetTxopLimit(params.txopLimit, linkId);
}

void StaWifiMac::SetMuEdcaParameters(const MuEdcaParams &params,
                                     uint8_t linkId) {
  Ptr<QosTxop> edca = GetQosTxop(params.ac);
  edca->SetMuCwMin(params.cwMin, linkId);
  edca->SetMuCwMax(params.cwMax, linkId);
  edca->SetMuAifsn(params.aifsn, linkId);
  edca->SetMuEdcaTimer(params.muEdcaTimer, linkId);
}

void StaWifiMac::PhyCapabilitiesChanged() {
  NS_LOG_FUNCTION(this);
  if (IsAssociated()) {
    NS_LOG_DEBUG("PHY capabilities changed: send reassociation request");
    SetState(WAIT_ASSOC_RESP);
    SendAssociationRequest(true);
  }
}

void StaWifiMac::NotifySwitchingEmlsrLink(Ptr<WifiPhy> phy, uint8_t linkId) {
  NS_LOG_FUNCTION(this << phy << linkId);

  for (auto &[id, link] : GetLinks()) {
    if (link->phy == phy) {
      link->phy = nullptr;
    }
  }

  auto &newLink = GetLink(linkId);
  newLink.phy = phy;
  newLink.channelAccessManager->SetupPhyListener(phy);
  NS_ASSERT(m_emlsrManager);
  if (m_emlsrManager->GetCamStateReset()) {
    newLink.channelAccessManager->ResetState();
  }
  newLink.feManager->ResetPhy();
  newLink.feManager->SetWifiPhy(phy);
}

void StaWifiMac::NotifyChannelSwitching(uint8_t linkId) {
  NS_LOG_FUNCTION(this << +linkId);

  WifiMac::NotifyChannelSwitching(linkId);

  if (IsInitialized() && IsAssociated()) {
    Disassociated(linkId);
  }

  m_assocManager->NotifyChannelSwitched(linkId);
}

std::ostream &operator<<(std::ostream &os, const StaWifiMac::ApInfo &apInfo) {
  os << "BSSID=" << apInfo.m_bssid << ", AP addr=" << apInfo.m_apAddr
     << ", SNR=" << apInfo.m_snr << ", Channel={" << apInfo.m_channel.number
     << "," << apInfo.m_channel.band << "}, Link ID=" << +apInfo.m_linkId
     << ", Frame=[";
  std::visit([&os](auto &&frame) { frame.Print(os); }, apInfo.m_frame);
  os << "]";
  return os;
}

} // namespace ns3
