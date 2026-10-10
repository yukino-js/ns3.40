
#include "wifi-assoc-manager.h"

#include "sta-wifi-mac.h"

#include "ns3/attribute-container.h"
#include "ns3/eht-configuration.h"
#include "ns3/enum.h"
#include "ns3/log.h"

#include <algorithm>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("WifiAssocManager");

NS_OBJECT_ENSURE_REGISTERED(WifiAssocManager);

WifiAssocManager::ApInfoCompare::ApInfoCompare(const WifiAssocManager &manager)
    : m_manager(manager) {}

bool WifiAssocManager::ApInfoCompare::operator()(
    const StaWifiMac::ApInfo &lhs, const StaWifiMac::ApInfo &rhs) const {
  NS_ASSERT_MSG(
      lhs.m_bssid != rhs.m_bssid,
      "Comparing two ApInfo objects with the same BSSID: " << lhs.m_bssid);

  bool lhsBefore = m_manager.Compare(lhs, rhs);
  if (lhsBefore) {
    return true;
  }

  bool rhsBefore = m_manager.Compare(rhs, lhs);
  if (rhsBefore) {
    return false;
  }

  return lhs.m_bssid < rhs.m_bssid;
}

TypeId WifiAssocManager::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::WifiAssocManager")
          .SetParent<Object>()
          .SetGroupName("Wifi")
          .AddAttribute("AllowedLinks",
                        "Only Beacon and Probe Response frames received on a "
                        "link belonging to the given "
                        "set are processed. An empty set is equivalent to the "
                        "set of all links.",
                        AttributeContainerValue<UintegerValue>(),
                        MakeAttributeContainerAccessor<UintegerValue>(
                            &WifiAssocManager::m_allowedLinks),
                        MakeAttributeContainerChecker<UintegerValue>(
                            MakeUintegerChecker<uint8_t>()));
  return tid;
}

WifiAssocManager::WifiAssocManager()
    : m_scanParams(), m_apList(ApInfoCompare(*this)) {}

WifiAssocManager::~WifiAssocManager() { NS_LOG_FUNCTION(this); }

void WifiAssocManager::DoDispose() {
  NS_LOG_FUNCTION(this);
  m_mac = nullptr;
}

void WifiAssocManager::SetStaWifiMac(Ptr<StaWifiMac> mac) {
  NS_LOG_FUNCTION(this << mac);
  m_mac = mac;
}

const WifiAssocManager::SortedList &WifiAssocManager::GetSortedList() const {
  return m_apList;
}

const WifiScanParams &WifiAssocManager::GetScanParams() const {
  return m_scanParams;
}

bool WifiAssocManager::MatchScanParams(const StaWifiMac::ApInfo &apInfo) const {
  NS_LOG_FUNCTION(this << apInfo);

  if (!m_scanParams.ssid.IsBroadcast()) {
    Ssid apSsid;
    if (auto beacon = std::get_if<MgtBeaconHeader>(&apInfo.m_frame); beacon) {
      apSsid = beacon->Get<Ssid>().value();
    } else {
      auto probeResp = std::get_if<MgtProbeResponseHeader>(&apInfo.m_frame);
      NS_ASSERT(probeResp);
      apSsid = probeResp->Get<Ssid>().value();
    }
    if (!apSsid.IsEqual(m_scanParams.ssid)) {
      NS_LOG_DEBUG("AP " << apInfo.m_bssid << " does not advertise our SSID "
                         << apSsid << "  " << m_scanParams.ssid);
      return false;
    }
  }

  auto channelMatch = [&apInfo](auto &&channel) {
    if (channel.number != 0 && channel.number != apInfo.m_channel.number) {
      return false;
    }
    if (channel.band != WIFI_PHY_BAND_UNSPECIFIED &&
        channel.band != apInfo.m_channel.band) {
      return false;
    }
    return true;
  };

  NS_ASSERT(apInfo.m_linkId < m_scanParams.channelList.size());
  if (std::find_if(m_scanParams.channelList[apInfo.m_linkId].cbegin(),
                   m_scanParams.channelList[apInfo.m_linkId].cend(),
                   channelMatch) ==
      m_scanParams.channelList[apInfo.m_linkId].cend()) {
    NS_LOG_DEBUG("AP " << apInfo.m_bssid
                       << " is not operating on a requested channel");
    return false;
  }

  return true;
}

void WifiAssocManager::StartScanning(WifiScanParams &&scanParams) {
  NS_LOG_FUNCTION(this);
  m_scanParams = std::move(scanParams);

  for (auto ap = m_apList.begin(); ap != m_apList.end();) {
    if (!MatchScanParams(*ap) ||
        (!m_allowedLinks.empty() && m_allowedLinks.count(ap->m_linkId) == 0)) {
      m_apListIt.erase(ap->m_bssid);
      ap = m_apList.erase(ap);
    } else {
      ++ap;
    }
  }

  DoStartScanning();
}

void WifiAssocManager::NotifyApInfo(const StaWifiMac::ApInfo &&apInfo) {
  NS_LOG_FUNCTION(this << apInfo);

  if (!CanBeInserted(apInfo) || !MatchScanParams(apInfo) ||
      (!m_allowedLinks.empty() && m_allowedLinks.count(apInfo.m_linkId) == 0)) {
    return;
  }

  auto [hashIt, hashInserted] = m_apListIt.insert({apInfo.m_bssid, {}});
  if (!hashInserted) {
    m_apList.erase(hashIt->second);
  }
  auto [listIt, listInserted] = m_apList.insert(std::move(apInfo));
  NS_ASSERT_MSG(listInserted,
                "An entry (" << listIt->m_apAddr << ", " << listIt->m_bssid
                             << ", " << +listIt->m_linkId
                             << ") prevented insertion of given ApInfo object");
  hashIt->second = listIt;
}

void WifiAssocManager::ScanningTimeout() {
  NS_LOG_FUNCTION(this);

  StaWifiMac::ApInfo bestAp;

  do {
    if (m_apList.empty()) {
      m_mac->ScanningTimeout(std::nullopt);
      return;
    }

    bestAp = std::move(m_apList.extract(m_apList.begin()).value());
    m_apListIt.erase(bestAp.m_bssid);
  } while (!CanBeReturned(bestAp));

  m_mac->ScanningTimeout(std::move(bestAp));
}

std::list<StaWifiMac::ApInfo::SetupLinksInfo> &
WifiAssocManager::GetSetupLinks(const StaWifiMac::ApInfo &apInfo) {
  return const_cast<std::list<StaWifiMac::ApInfo::SetupLinksInfo> &>(
      apInfo.m_setupLinks);
}

bool WifiAssocManager::CanSetupMultiLink(OptMleConstRef &mle,
                                         OptRnrConstRef &rnr) {
  NS_LOG_FUNCTION(this);

  if (m_mac->GetNLinks() == 1 || GetSortedList().empty()) {
    return false;
  }

  if (auto beacon = std::get_if<MgtBeaconHeader>(&m_apList.begin()->m_frame);
      beacon) {
    mle = beacon->Get<MultiLinkElement>();
    rnr = beacon->Get<ReducedNeighborReport>();
  } else {
    auto probeResp =
        std::get_if<MgtProbeResponseHeader>(&m_apList.begin()->m_frame);
    NS_ASSERT(probeResp);
    mle = probeResp->Get<MultiLinkElement>();
    rnr = probeResp->Get<ReducedNeighborReport>();
  }

  if (!mle.has_value()) {
    NS_LOG_DEBUG("No Multi-Link Element in Beacon/Probe Response");
    return false;
  }

  if (!rnr.has_value() || rnr->get().GetNNbrApInfoFields() == 0) {
    NS_LOG_DEBUG("No Reduced Neighbor Report Element in Beacon/Probe Response");
    return false;
  }

  if (!mle->get().HasLinkIdInfo()) {
    NS_LOG_DEBUG("No Link ID Info subfield in the Multi-Link Element");
    return false;
  }

  if (const auto &mldCapabilities =
          mle->get().GetCommonInfoBasic().m_mldCapabilities) {
    auto ehtConfig = m_mac->GetEhtConfiguration();
    NS_ASSERT(ehtConfig);
    EnumValue negSupport;
    ehtConfig->GetAttributeFailSafe("TidToLinkMappingNegSupport", negSupport);

    if (mldCapabilities->tidToLinkMappingSupport > 0 && negSupport.Get() == 0) {
      NS_LOG_DEBUG(
          "AP MLD supports TID-to-Link Mapping negotiation, while we don't");
      return false;
    }
  }

  return true;
}

std::optional<WifiAssocManager::RnrLinkInfo>
WifiAssocManager::GetNextAffiliatedAp(const ReducedNeighborReport &rnr,
                                      std::size_t nbrApInfoId) {
  NS_LOG_FUNCTION(nbrApInfoId);

  while (nbrApInfoId < rnr.GetNNbrApInfoFields()) {
    if (!rnr.HasMldParameters(nbrApInfoId)) {
      nbrApInfoId++;
      continue;
    }

    std::size_t tbttInfoFieldIndex = 0;
    while (tbttInfoFieldIndex < rnr.GetNTbttInformationFields(nbrApInfoId) &&
           rnr.GetMldId(nbrApInfoId, tbttInfoFieldIndex) != 0) {
      tbttInfoFieldIndex++;
    }

    if (tbttInfoFieldIndex < rnr.GetNTbttInformationFields(nbrApInfoId)) {
      return RnrLinkInfo{nbrApInfoId, tbttInfoFieldIndex};
    }
    nbrApInfoId++;
  }

  return std::nullopt;
}

std::list<WifiAssocManager::RnrLinkInfo>
WifiAssocManager::GetAllAffiliatedAps(const ReducedNeighborReport &rnr) {
  std::list<WifiAssocManager::RnrLinkInfo> apList;
  std::size_t nbrApInfoId = 0;
  std::optional<WifiAssocManager::RnrLinkInfo> next;

  while ((next = GetNextAffiliatedAp(rnr, nbrApInfoId)).has_value()) {
    apList.push_back({*next});
    nbrApInfoId = next->m_nbrApInfoId + 1;
  }

  return apList;
}

} // namespace ns3
