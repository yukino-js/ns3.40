
#ifndef WIFI_ASSOC_MANAGER_H
#define WIFI_ASSOC_MANAGER_H

#include "qos-utils.h"
#include "sta-wifi-mac.h"

#include <optional>
#include <set>
#include <unordered_map>

namespace ns3 {

class WifiAssocManager : public Object {
  struct ApInfoCompare {
    ApInfoCompare(const WifiAssocManager &manager);
    bool operator()(const StaWifiMac::ApInfo &lhs,
                    const StaWifiMac::ApInfo &rhs) const;

  private:
    const WifiAssocManager &m_manager;
  };

public:
  struct RnrLinkInfo {
    std::size_t m_nbrApInfoId;
    std::size_t m_tbttInfoFieldId;
  };

  static TypeId GetTypeId();

  ~WifiAssocManager() override;

  void SetStaWifiMac(Ptr<StaWifiMac> mac);

  void StartScanning(WifiScanParams &&scanParams);

  virtual void NotifyApInfo(const StaWifiMac::ApInfo &&apInfo);

  virtual void NotifyChannelSwitched(uint8_t linkId) = 0;

  virtual bool Compare(const StaWifiMac::ApInfo &lhs,
                       const StaWifiMac::ApInfo &rhs) const = 0;

  static std::optional<WifiAssocManager::RnrLinkInfo>
  GetNextAffiliatedAp(const ReducedNeighborReport &rnr,
                      std::size_t nbrApInfoId);

  static std::list<WifiAssocManager::RnrLinkInfo>
  GetAllAffiliatedAps(const ReducedNeighborReport &rnr);

protected:
  WifiAssocManager();
  void DoDispose() override;

  using SortedList = std::set<StaWifiMac::ApInfo, ApInfoCompare>;

  const SortedList &GetSortedList() const;

  std::list<StaWifiMac::ApInfo::SetupLinksInfo> &
  GetSetupLinks(const StaWifiMac::ApInfo &apInfo);

  const WifiScanParams &GetScanParams() const;

  bool MatchScanParams(const StaWifiMac::ApInfo &apInfo) const;

  virtual bool CanBeInserted(const StaWifiMac::ApInfo &apInfo) const = 0;
  virtual bool CanBeReturned(const StaWifiMac::ApInfo &apInfo) const = 0;

  void ScanningTimeout();

  using OptRnrConstRef =
      std::optional<std::reference_wrapper<const ReducedNeighborReport>>;
  using OptMleConstRef =
      std::optional<std::reference_wrapper<const MultiLinkElement>>;

  bool CanSetupMultiLink(OptMleConstRef &mle, OptRnrConstRef &rnr);

  Ptr<StaWifiMac> m_mac;
  std::set<uint8_t> m_allowedLinks;

private:
  virtual void DoStartScanning() = 0;

  WifiScanParams m_scanParams;
  SortedList m_apList;
  std::unordered_map<Mac48Address, SortedList::const_iterator, WifiAddressHash>
      m_apListIt;
};

} // namespace ns3

#endif
