
#ifndef EHT_CONFIGURATION_H
#define EHT_CONFIGURATION_H

#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/wifi-utils.h"

#include <list>
#include <map>

namespace ns3 {

enum WifiTidToLinkMappingNegSupport : uint8_t {
  WIFI_TID_TO_LINK_MAPPING_NOT_SUPPORTED = 0,
  WIFI_TID_TO_LINK_MAPPING_SAME_LINK_SET = 1,
  WIFI_TID_TO_LINK_MAPPING_ANY_LINK_SET = 3
};

class EhtConfiguration : public Object {
public:
  EhtConfiguration();
  ~EhtConfiguration() override;

  static TypeId GetTypeId();

  WifiTidLinkMapping GetTidLinkMapping(WifiDirection dir) const;

  void SetTidLinkMapping(
      WifiDirection dir,
      const std::map<std::list<uint8_t>, std::list<uint8_t>> &mapping);

private:
  bool m_emlsrActivated;
  Time m_transitionTimeout;
  WifiTidToLinkMappingNegSupport m_tidLinkMappingSupport;
  std::map<std::list<uint64_t>, std::list<uint64_t>> m_linkMappingDl;
  std::map<std::list<uint64_t>, std::list<uint64_t>> m_linkMappingUl;
};

} // namespace ns3

#endif
