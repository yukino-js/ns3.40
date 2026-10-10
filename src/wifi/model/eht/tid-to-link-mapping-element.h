
#ifndef TID_TO_LINK_MAPPING_H
#define TID_TO_LINK_MAPPING_H

#include "ns3/nstime.h"
#include "ns3/wifi-information-element.h"
#include "ns3/wifi-utils.h"

#include <map>
#include <optional>
#include <set>

namespace ns3 {

constexpr auto DEFAULT_WIFI_TID_LINK_MAPPING{true};
constexpr auto DEFAULT_WIFI_TID_LINK_MAP_DIR{WifiDirection::BOTH_DIRECTIONS};
constexpr uint16_t WIFI_TID_TO_LINK_MAPPING_CONTROL_BASIC_SIZE_B = 1;
constexpr uint16_t WIFI_LINK_MAPPING_PRESENCE_IND_SIZE_B = 1;

class TidToLinkMapping : public WifiInformationElement {
public:
  struct Control {
    friend class TidToLinkMapping;

    WifiDirection direction{DEFAULT_WIFI_TID_LINK_MAP_DIR};
    bool defaultMapping{DEFAULT_WIFI_TID_LINK_MAPPING};

    uint16_t GetSubfieldSize() const;

    void Serialize(Buffer::Iterator &start) const;
    uint16_t Deserialize(Buffer::Iterator start);

  private:
    bool mappingSwitchTimePresent{false};
    bool expectedDurationPresent{false};
    uint8_t linkMappingSize{1};
    std::optional<uint8_t> presenceBitmap;
  };

  WifiInformationElementId ElementId() const override;
  WifiInformationElementId ElementIdExt() const override;

  void SetMappingSwitchTime(Time mappingSwitchTime);

  std::optional<Time> GetMappingSwitchTime() const;

  void SetExpectedDuration(Time expectedDuration);

  std::optional<Time> GetExpectedDuration() const;

  void SetLinkMappingOfTid(uint8_t tid, std::set<uint8_t> linkIds);
  std::set<uint8_t> GetLinkMappingOfTid(uint8_t tid) const;

  TidToLinkMapping::Control m_control;
  std::map<uint8_t, uint16_t> m_linkMapping;

private:
  std::optional<uint16_t> m_mappingSwitchTime;
  std::optional<uint32_t> m_expectedDuration;

  uint16_t GetInformationFieldSize() const override;
  void SerializeInformationField(Buffer::Iterator start) const override;
  uint16_t DeserializeInformationField(Buffer::Iterator start,
                                       uint16_t length) override;
};

} // namespace ns3

#endif
