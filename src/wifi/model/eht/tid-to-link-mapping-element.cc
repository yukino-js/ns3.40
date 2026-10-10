
#include "tid-to-link-mapping-element.h"

#include "ns3/assert.h"
#include "ns3/simulator.h"

namespace ns3 {

static constexpr uint64_t BIT_63_TO_26_MASK = 0xfffffffffc000000;

uint16_t TidToLinkMapping::Control::GetSubfieldSize() const {
  NS_ASSERT_MSG(!defaultMapping || !presenceBitmap.has_value(),
                "Presence bitmap not expected if default mapping is set");
  auto size = WIFI_TID_TO_LINK_MAPPING_CONTROL_BASIC_SIZE_B;
  if (!presenceBitmap.has_value()) {
    return size;
  }
  return size + WIFI_LINK_MAPPING_PRESENCE_IND_SIZE_B;
}

void TidToLinkMapping::Control::Serialize(Buffer::Iterator &start) const {
  auto val = static_cast<uint8_t>(direction) | ((defaultMapping ? 1 : 0) << 2) |
             ((mappingSwitchTimePresent ? 1 : 0) << 3) |
             ((expectedDurationPresent ? 1 : 0) << 4) |
             ((linkMappingSize == 1 ? 1 : 0) << 5);

  start.WriteU8(val);
  NS_ASSERT_MSG(!defaultMapping || !presenceBitmap.has_value(),
                "Presence bitmap not expected if default mapping is set");
  if (presenceBitmap.has_value()) {
    start.WriteU8(presenceBitmap.value());
  }
}

uint16_t TidToLinkMapping::Control::Deserialize(Buffer::Iterator start) {
  auto i = start;
  uint8_t count = 0;
  auto val = i.ReadU8();
  count++;

  direction = static_cast<WifiDirection>(val & 0x03);
  defaultMapping = (((val >> 2) & 0x01) == 1);
  mappingSwitchTimePresent = (((val >> 3) & 0x01) == 1);
  expectedDurationPresent = (((val >> 4) & 0x01) == 1);
  linkMappingSize = (((val >> 5) & 0x01) == 1 ? 1 : 2);
  if (defaultMapping) {
    presenceBitmap.reset();
    return count;
  }
  presenceBitmap = i.ReadU8();
  return ++count;
}

WifiInformationElementId TidToLinkMapping::ElementId() const {
  return IE_EXTENSION;
}

WifiInformationElementId TidToLinkMapping::ElementIdExt() const {
  return IE_EXT_TID_TO_LINK_MAPPING_ELEMENT;
}

void TidToLinkMapping::SetMappingSwitchTime(Time mappingSwitchTime) {
  NS_ABORT_IF(mappingSwitchTime < Simulator::Now());
  auto switchTimeUsec =
      static_cast<uint64_t>(mappingSwitchTime.GetMicroSeconds());
  m_mappingSwitchTime = (switchTimeUsec & ~BIT_63_TO_26_MASK) >> 10;
  m_control.mappingSwitchTimePresent = true;
}

std::optional<Time> TidToLinkMapping::GetMappingSwitchTime() const {
  if (!m_control.mappingSwitchTimePresent) {
    return std::nullopt;
  }

  auto nowUsec = static_cast<uint64_t>(Simulator::Now().GetMicroSeconds());
  uint64_t switchTimeUsec =
      (*m_mappingSwitchTime << 10) + (nowUsec & BIT_63_TO_26_MASK);
  if (switchTimeUsec < nowUsec) {
    switchTimeUsec += (1 << 26);
  }
  return MicroSeconds(switchTimeUsec);
}

void TidToLinkMapping::SetExpectedDuration(Time expectedDuration) {
  auto durationTu =
      static_cast<uint64_t>(expectedDuration.GetMicroSeconds()) >> 10;
  m_expectedDuration = (durationTu & 0x0000000000ffffff);
  m_control.expectedDurationPresent = true;
}

std::optional<Time> TidToLinkMapping::GetExpectedDuration() const {
  if (!m_control.expectedDurationPresent) {
    return std::nullopt;
  }

  return MicroSeconds(*m_expectedDuration << 10);
}

void TidToLinkMapping::SetLinkMappingOfTid(uint8_t tid,
                                           std::set<uint8_t> linkIds) {
  NS_ABORT_MSG_IF(tid > 7, "Invalid tid: " << +tid);
  NS_ABORT_MSG_IF(
      m_control.defaultMapping,
      "Per-TID link mapping not expected if default mapping is set");

  uint16_t linkMapping = 0;

  for (const auto &linkId : linkIds) {
    linkMapping |= (1 << linkId);
    if (linkId > 7) {
      m_control.linkMappingSize = 2;
    }
  }

  m_linkMapping[tid] = linkMapping;
  m_control.presenceBitmap = m_control.presenceBitmap.value_or(0) | (1 << tid);
}

std::set<uint8_t> TidToLinkMapping::GetLinkMappingOfTid(uint8_t tid) const {
  auto it = m_linkMapping.find(tid);

  if (it == m_linkMapping.cend()) {
    return {};
  }

  std::set<uint8_t> linkIds;
  for (uint8_t linkId = 0; linkId < 15; linkId++) {
    if (((it->second >> linkId) & 0x0001) == 1) {
      linkIds.insert(linkId);
    }
  }
  NS_ABORT_MSG_IF(linkIds.empty(),
                  "TID " << +tid << " cannot be mapped to an empty link set");

  return linkIds;
}

uint16_t TidToLinkMapping::GetInformationFieldSize() const {
  uint16_t ret = WIFI_IE_ELEMENT_ID_EXT_SIZE;
  ret += m_control.GetSubfieldSize();
  if (m_control.mappingSwitchTimePresent) {
    ret += 2;
  }
  if (m_control.expectedDurationPresent) {
    ret += 3;
  }

  NS_ASSERT_MSG(!m_control.defaultMapping || m_linkMapping.empty(),
                "Per-TID link mapping not expected if default mapping is set");
  ret += m_control.linkMappingSize * (m_linkMapping.size());
  return ret;
}

void TidToLinkMapping::SerializeInformationField(Buffer::Iterator start) const {
  m_control.Serialize(start);
  if (m_control.mappingSwitchTimePresent) {
    start.WriteHtolsbU16(*m_mappingSwitchTime);
  }
  if (m_control.expectedDurationPresent) {
    start.WriteU8((*m_expectedDuration >> 0) & 0xff);
    start.WriteU8((*m_expectedDuration >> 8) & 0xff);
    start.WriteU8((*m_expectedDuration >> 16) & 0xff);
  }

  NS_ASSERT_MSG(!m_control.defaultMapping || m_linkMapping.empty(),
                "Per-TID link mapping not expected if default mapping is set");

  for (const auto &[tid, linkMapping] : m_linkMapping) {
    if (m_control.linkMappingSize == 1) {
      start.WriteU8(linkMapping);
    } else {
      start.WriteHtolsbU16(linkMapping);
    }
  }
}

uint16_t TidToLinkMapping::DeserializeInformationField(Buffer::Iterator start,
                                                       uint16_t length) {
  auto i = start;
  uint16_t count = 0;
  auto nCtrlOctets = m_control.Deserialize(i);
  NS_ASSERT_MSG(nCtrlOctets <= length, "Tid-to-Link Mapping deserialize error");
  i.Next(nCtrlOctets);
  count += nCtrlOctets;
  if (m_control.mappingSwitchTimePresent) {
    m_mappingSwitchTime = i.ReadLsbtohU16();
    count += 2;
  }
  if (m_control.expectedDurationPresent) {
    uint8_t byte0 = i.ReadU8();
    uint8_t byte1 = i.ReadU8();
    uint8_t byte2 = i.ReadU8();
    m_expectedDuration = byte2;
    m_expectedDuration.value() <<= 8;
    m_expectedDuration.value() |= byte1;
    m_expectedDuration.value() <<= 8;
    m_expectedDuration.value() |= byte0;
    count += 3;
  }
  m_linkMapping.clear();
  if (m_control.presenceBitmap.has_value()) {
    NS_ABORT_MSG_IF(
        m_control.defaultMapping,
        "Default mapping should not be set when presence bitmap is present");
    const auto presenceBitmap = m_control.presenceBitmap.value();
    for (uint8_t tid = 0; tid < 8; tid++) {
      if (((presenceBitmap >> tid) & 0x01) == 1) {
        if (m_control.linkMappingSize == 1) {
          m_linkMapping[tid] = i.ReadU8();
          count++;
        } else {
          m_linkMapping[tid] = i.ReadLsbtohU16();
          count += 2;
        }
      }
    }
  }

  NS_ABORT_MSG_IF(count != length, "TID-to-Link Mapping Length ("
                                       << +length
                                       << ") differs "
                                          "from actual number of bytes read ("
                                       << +count << ")");
  return count;
}

} // namespace ns3
