
#include "he-ru.h"

#include "ns3/abort.h"
#include "ns3/assert.h"

#include <optional>
#include <tuple>

namespace ns3 {

const HeRu::SubcarrierGroups HeRu::m_heRuSubcarrierGroups = {
    {{20, HeRu::RU_26_TONE},
     {{{-121, -96}},
      {{-95, -70}},
      {{-68, -43}},
      {{-42, -17}},
      {{-16, -4}, {4, 16}},
      {{17, 42}},
      {{43, 68}},
      {{70, 95}},
      {{96, 121}}}},
    {{20, HeRu::RU_52_TONE},
     {{{-121, -70}}, {{-68, -17}}, {{17, 68}}, {{70, 121}}}},
    {{20, HeRu::RU_106_TONE}, {{{-122, -17}}, {{17, 122}}}},
    {{20, HeRu::RU_242_TONE}, {{{-122, -2}, {2, 122}}}},
    {{40, HeRu::RU_26_TONE},
     {{{-243, -218}},
      {{-217, -192}},
      {{-189, -164}},
      {{-163, -138}},
      {{-136, -111}},
      {{-109, -84}},
      {{-83, -58}},
      {{-55, -30}},
      {{-29, -4}},
      {{4, 29}},
      {{30, 55}},
      {{58, 83}},
      {{84, 109}},
      {{111, 136}},
      {{138, 163}},
      {{164, 189}},
      {{192, 217}},
      {{218, 243}}}},
    {{40, HeRu::RU_52_TONE},
     {{{-243, -192}},
      {{-189, -138}},
      {{-109, -58}},
      {{-55, -4}},
      {{4, 55}},
      {{58, 109}},
      {{138, 189}},
      {{192, 243}}}},
    {{40, HeRu::RU_106_TONE},
     {{{-243, -138}}, {{-109, -4}}, {{4, 109}}, {{138, 243}}}},
    {{40, HeRu::RU_242_TONE}, {{{-244, -3}}, {{3, 244}}}},
    {{40, HeRu::RU_484_TONE}, {{{-244, -3}, {3, 244}}}},
    {{80, HeRu::RU_26_TONE},
     {{{-499, -474}}, {{-473, -448}}, {{-445, -420}},       {{-419, -394}},
      {{-392, -367}}, {{-365, -340}}, {{-339, -314}},       {{-311, -286}},
      {{-285, -260}}, {{-257, -232}}, {{-231, -206}},       {{-203, -178}},
      {{-177, -152}}, {{-150, -125}}, {{-123, -98}},        {{-97, -72}},
      {{-69, -44}},   {{-43, -18}},   {{-16, -4}, {4, 16}}, {{18, 43}},
      {{44, 69}},     {{72, 97}},     {{98, 123}},          {{125, 150}},
      {{152, 177}},   {{178, 203}},   {{206, 231}},         {{232, 257}},
      {{260, 285}},   {{286, 311}},   {{314, 339}},         {{340, 365}},
      {{367, 392}},   {{394, 419}},   {{420, 445}},         {{448, 473}},
      {{474, 499}}}},
    {{80, HeRu::RU_52_TONE},
     {{{-499, -448}},
      {{-445, -394}},
      {{-365, -314}},
      {{-311, -260}},
      {{-257, -206}},
      {{-203, -152}},
      {{-123, -72}},
      {{-69, -18}},
      {{18, 69}},
      {{72, 123}},
      {{152, 203}},
      {{206, 257}},
      {{260, 311}},
      {{314, 365}},
      {{394, 445}},
      {{448, 499}}}},
    {{80, HeRu::RU_106_TONE},
     {{{-499, -394}},
      {{-365, -260}},
      {{-257, -152}},
      {{-123, -18}},
      {{18, 123}},
      {{152, 257}},
      {{260, 365}},
      {{394, 499}}}},
    {{80, HeRu::RU_242_TONE},
     {{{-500, -259}}, {{-258, -17}}, {{17, 258}}, {{259, 500}}}},
    {{80, HeRu::RU_484_TONE}, {{{-500, -17}}, {{17, 500}}}},
    {{80, HeRu::RU_996_TONE}, {{{-500, -3}, {3, 500}}}},
};

const HeRu::RuAllocationMap HeRu::m_heRuAllocations = {
    // clang-format off
    {0,
     {HeRu::RuSpec{HeRu::RU_26_TONE, 1, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 2, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 3, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 4, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 5, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 6, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 7, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 8, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 9, true}}},
    {1,
     {HeRu::RuSpec{HeRu::RU_26_TONE, 1, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 2, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 3, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 4, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 5, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 6, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 7, true},
      HeRu::RuSpec{HeRu::RU_52_TONE, 4, true}}},
    {2,
     {HeRu::RuSpec{HeRu::RU_26_TONE, 1, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 2, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 3, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 4, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 5, true},
      HeRu::RuSpec{HeRu::RU_52_TONE, 3, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 8, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 9, true}}},
    {3,
     {HeRu::RuSpec{HeRu::RU_26_TONE, 1, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 2, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 3, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 4, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 5, true},
      HeRu::RuSpec{HeRu::RU_52_TONE, 3, true},
      HeRu::RuSpec{HeRu::RU_52_TONE, 4, true}}},
    {4,
     {HeRu::RuSpec{HeRu::RU_26_TONE, 1, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 2, true},
      HeRu::RuSpec{HeRu::RU_52_TONE, 2, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 5, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 6, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 7, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 8, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 9, true}}},
    {5,
     {HeRu::RuSpec{HeRu::RU_26_TONE, 1, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 2, true},
      HeRu::RuSpec{HeRu::RU_52_TONE, 2, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 5, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 6, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 7, true},
      HeRu::RuSpec{HeRu::RU_52_TONE, 4, true}}},
    {6,
     {HeRu::RuSpec{HeRu::RU_26_TONE, 1, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 2, true},
      HeRu::RuSpec{HeRu::RU_52_TONE, 2, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 5, true},
      HeRu::RuSpec{HeRu::RU_52_TONE, 3, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 8, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 9, true}}},
    {7,
     {HeRu::RuSpec{HeRu::RU_26_TONE, 1, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 2, true},
      HeRu::RuSpec{HeRu::RU_52_TONE, 2, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 5, true},
      HeRu::RuSpec{HeRu::RU_52_TONE, 3, true},
      HeRu::RuSpec{HeRu::RU_52_TONE, 4, true}}},
    {8,
     {HeRu::RuSpec{HeRu::RU_52_TONE, 1, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 3, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 4, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 5, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 6, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 7, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 8, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 9, true}}},
    {9,
     {HeRu::RuSpec{HeRu::RU_52_TONE, 1, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 3, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 4, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 5, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 6, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 7, true},
      HeRu::RuSpec{HeRu::RU_52_TONE, 4, true}}},
    {10,
     {HeRu::RuSpec{HeRu::RU_52_TONE, 1, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 3, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 4, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 5, true},
      HeRu::RuSpec{HeRu::RU_52_TONE, 3, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 8, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 9, true}}},
    {11,
     {HeRu::RuSpec{HeRu::RU_52_TONE, 1, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 3, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 4, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 5, true},
      HeRu::RuSpec{HeRu::RU_52_TONE, 3, true},
      HeRu::RuSpec{HeRu::RU_52_TONE, 4, true}}},
    {12,
     {HeRu::RuSpec{HeRu::RU_52_TONE, 1, true},
      HeRu::RuSpec{HeRu::RU_52_TONE, 2, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 5, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 6, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 7, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 8, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 9, true}}},
    {13,
     {HeRu::RuSpec{HeRu::RU_52_TONE, 1, true},
      HeRu::RuSpec{HeRu::RU_52_TONE, 2, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 5, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 6, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 7, true},
      HeRu::RuSpec{HeRu::RU_52_TONE, 4, true}}},
    {14,
     {HeRu::RuSpec{HeRu::RU_52_TONE, 1, true},
      HeRu::RuSpec{HeRu::RU_52_TONE, 2, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 5, true},
      HeRu::RuSpec{HeRu::RU_52_TONE, 3, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 8, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 9, true}}},
    {15,
     {HeRu::RuSpec{HeRu::RU_52_TONE, 1, true},
      HeRu::RuSpec{HeRu::RU_52_TONE, 2, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 5, true},
      HeRu::RuSpec{HeRu::RU_52_TONE, 3, true},
      HeRu::RuSpec{HeRu::RU_52_TONE, 4, true}}},
    {16,
     {HeRu::RuSpec{HeRu::RU_52_TONE, 1, true},
      HeRu::RuSpec{HeRu::RU_52_TONE, 2, true},
      HeRu::RuSpec{HeRu::RU_106_TONE, 2, true}}},
    {24,
     {HeRu::RuSpec{HeRu::RU_106_TONE, 1, true},
      HeRu::RuSpec{HeRu::RU_52_TONE, 3, true},
      HeRu::RuSpec{HeRu::RU_52_TONE, 4, true}}},
    {32,
     {HeRu::RuSpec{HeRu::RU_26_TONE, 1, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 2, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 3, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 4, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 5, true},
      HeRu::RuSpec{HeRu::RU_106_TONE, 2, true}}},
    {40,
     {HeRu::RuSpec{HeRu::RU_26_TONE, 1, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 2, true},
      HeRu::RuSpec{HeRu::RU_52_TONE, 2, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 5, true},
      HeRu::RuSpec{HeRu::RU_106_TONE, 2, true}}},
    {48,
     {HeRu::RuSpec{HeRu::RU_52_TONE, 1, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 3, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 4, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 5, true},
      HeRu::RuSpec{HeRu::RU_106_TONE, 2, true}}},
    {56,
     {HeRu::RuSpec{HeRu::RU_52_TONE, 1, true},
      HeRu::RuSpec{HeRu::RU_52_TONE, 2, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 5, true},
      HeRu::RuSpec{HeRu::RU_106_TONE, 2, true}}},
    {64,
     {HeRu::RuSpec{HeRu::RU_106_TONE, 1, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 5, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 6, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 7, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 8, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 9, true}}},
    {72,
     {HeRu::RuSpec{HeRu::RU_106_TONE, 1, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 5, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 6, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 7, true},
      HeRu::RuSpec{HeRu::RU_52_TONE, 4, true}}},
    {80,
     {HeRu::RuSpec{HeRu::RU_106_TONE, 1, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 5, true},
      HeRu::RuSpec{HeRu::RU_52_TONE, 3, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 8, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 9, true}}},
    {88,
     {HeRu::RuSpec{HeRu::RU_106_TONE, 1, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 5, true},
      HeRu::RuSpec{HeRu::RU_52_TONE, 3, true},
      HeRu::RuSpec{HeRu::RU_52_TONE, 4, true}}},
    {96,
     {HeRu::RuSpec{HeRu::RU_106_TONE, 1, true},
      HeRu::RuSpec{HeRu::RU_106_TONE, 2, true}}},
    {112,
     {HeRu::RuSpec{HeRu::RU_52_TONE, 1, true},
      HeRu::RuSpec{HeRu::RU_52_TONE, 2, true},
      HeRu::RuSpec{HeRu::RU_52_TONE, 3, true},
      HeRu::RuSpec{HeRu::RU_52_TONE, 4, true}}},
    {128,
     {HeRu::RuSpec{HeRu::RU_106_TONE, 1, true},
      HeRu::RuSpec{HeRu::RU_26_TONE, 5, true},
      HeRu::RuSpec{HeRu::RU_106_TONE, 2, true}}},
    {192,
      {HeRu::RuSpec{HeRu::RU_242_TONE, 1, true}}},
    {200,
      {HeRu::RuSpec{HeRu::RU_484_TONE, 1, true}}},
    {208,
      {HeRu::RuSpec{HeRu::RU_996_TONE, 1, true}}},
    // clang-format on
};

HeRu::RuSpecCompare::RuSpecCompare(uint16_t channelWidth, uint8_t p20Index)
    : m_channelWidth(channelWidth), m_p20Index(p20Index) {}

bool HeRu::RuSpecCompare::operator()(const HeRu::RuSpec &lhs,
                                     const HeRu::RuSpec &rhs) const {
  const auto lhsIndex = lhs.GetPhyIndex(m_channelWidth, m_p20Index);
  const auto rhsIndex = rhs.GetPhyIndex(m_channelWidth, m_p20Index);
  const auto lhsStartTone =
      HeRu::GetSubcarrierGroup(m_channelWidth, lhs.GetRuType(), lhsIndex)
          .front()
          .first;
  const auto rhsStartTone =
      HeRu::GetSubcarrierGroup(m_channelWidth, rhs.GetRuType(), rhsIndex)
          .front()
          .first;
  return lhsStartTone < rhsStartTone;
}

std::vector<HeRu::RuSpec> HeRu::GetRuSpecs(uint8_t ruAllocation) {
  std::optional<std::size_t> idx;
  switch (ruAllocation) {
  case 0 ... 15:
  case 112:
    idx = ruAllocation;
    break;
  case 16 ... 95:
  case 192 ... 215:
    idx = ruAllocation & 0xF8;
    break;
  case 96 ... 111:
    idx = ruAllocation & 0xF0;
    break;
  case 113 ... 115:
    break;
  case 128 ... 191:
    idx = ruAllocation & 0xC0;
    break;
  default:
    NS_FATAL_ERROR("Reserved RU allocation " << +ruAllocation);
  }
  return idx.has_value() ? m_heRuAllocations.at(idx.value())
                         : std::vector<HeRu::RuSpec>{};
}

uint8_t HeRu::GetEqualizedRuAllocation(RuType ruType, bool isOdd) {
  switch (ruType) {
  case HeRu::RU_26_TONE:
    return 0;
  case HeRu::RU_52_TONE:
    return isOdd ? 15 : 112;
  case HeRu::RU_106_TONE:
    return isOdd ? 128 : 96;
  case HeRu::RU_242_TONE:
    return 192;
  case HeRu::RU_484_TONE:
    return 200;
  default:
    return 208;
  }
}

HeRu::RuSpec::RuSpec() : m_index(0) {}

HeRu::RuSpec::RuSpec(RuType ruType, std::size_t index, bool primary80MHz)
    : m_ruType(ruType), m_index(index), m_primary80MHz(primary80MHz) {
  NS_ABORT_MSG_IF(index == 0, "Index cannot be zero");
}

HeRu::RuType HeRu::RuSpec::GetRuType() const {
  NS_ABORT_MSG_IF(m_index == 0, "Undefined RU");
  return m_ruType;
}

std::size_t HeRu::RuSpec::GetIndex() const {
  NS_ABORT_MSG_IF(m_index == 0, "Undefined RU");
  return m_index;
}

bool HeRu::RuSpec::GetPrimary80MHz() const {
  NS_ABORT_MSG_IF(m_index == 0, "Undefined RU");
  return m_primary80MHz;
}

std::size_t HeRu::RuSpec::GetPhyIndex(uint16_t bw, uint8_t p20Index) const {
  bool primary80IsLower80 = (p20Index < bw / 40);

  if (bw < 160 || m_ruType == HeRu::RU_2x996_TONE ||
      (primary80IsLower80 && m_primary80MHz) ||
      (!primary80IsLower80 && !m_primary80MHz)) {
    return m_index;
  } else {
    return m_index + GetNRus(bw, m_ruType) / 2;
  }
}

std::size_t HeRu::GetNRus(uint16_t bw, RuType ruType) {
  if (bw == 160 && ruType == RU_2x996_TONE) {
    return 1;
  }

  auto it = m_heRuSubcarrierGroups.find({(bw == 160 ? 80 : bw), ruType});

  if (it == m_heRuSubcarrierGroups.end()) {
    return 0;
  }

  return (bw == 160 ? 2 : 1) * it->second.size();
}

std::vector<HeRu::RuSpec> HeRu::GetRusOfType(uint16_t bw, HeRu::RuType ruType) {
  if (ruType == HeRu::RU_2x996_TONE) {
    NS_ASSERT(bw >= 160);
    return {{ruType, 1, true}};
  }

  std::vector<HeRu::RuSpec> ret;
  std::vector<bool> primary80MHzSet{true};

  if (bw == 160) {
    primary80MHzSet.push_back(false);
    bw = 80;
  }

  for (auto primary80MHz : primary80MHzSet) {
    for (std::size_t ruIndex = 1;
         ruIndex <= HeRu::m_heRuSubcarrierGroups.at({bw, ruType}).size();
         ruIndex++) {
      ret.emplace_back(ruType, ruIndex, primary80MHz);
    }
  }
  return ret;
}

std::vector<HeRu::RuSpec> HeRu::GetCentral26TonesRus(uint16_t bw,
                                                     HeRu::RuType ruType) {
  std::vector<std::size_t> indices;

  if (ruType == HeRu::RU_52_TONE || ruType == HeRu::RU_106_TONE) {
    if (bw == 20) {
      indices.push_back(5);
    } else if (bw == 40) {
      indices.insert(indices.end(), {5, 14});
    } else if (bw >= 80) {
      indices.insert(indices.end(), {5, 14, 19, 24, 33});
    }
  } else if (ruType == HeRu::RU_242_TONE || ruType == HeRu::RU_484_TONE) {
    if (bw >= 80) {
      indices.push_back(19);
    }
  }

  std::vector<HeRu::RuSpec> ret;
  std::vector<bool> primary80MHzSet{true};

  if (bw == 160) {
    primary80MHzSet.push_back(false);
  }

  for (auto primary80MHz : primary80MHzSet) {
    for (const auto &index : indices) {
      ret.emplace_back(HeRu::RU_26_TONE, index, primary80MHz);
    }
  }
  return ret;
}

HeRu::SubcarrierGroup HeRu::GetSubcarrierGroup(uint16_t bw, RuType ruType,
                                               std::size_t phyIndex) {
  if (ruType == HeRu::RU_2x996_TONE) {
    NS_ABORT_MSG_IF(bw != 160,
                    "2x996 tone RU can only be used on 160 MHz band");
    return {{-1012, -3}, {3, 1012}};
  }

  std::size_t indexInLower80MHz = phyIndex;
  std::size_t numRus = GetNRus(bw, ruType);
  int16_t shift = (bw == 160) ? -512 : 0;
  if (bw == 160 && phyIndex > (numRus / 2)) {
    indexInLower80MHz = phyIndex - (numRus / 2);
    shift = 512;
  }

  auto it = m_heRuSubcarrierGroups.find({(bw == 160 ? 80 : bw), ruType});

  NS_ABORT_MSG_IF(it == m_heRuSubcarrierGroups.end(), "RU not found");
  NS_ABORT_MSG_IF(indexInLower80MHz > it->second.size(),
                  "RU index not available");

  SubcarrierGroup group = it->second.at(indexInLower80MHz - 1);
  if (bw == 160) {
    for (auto &range : group) {
      range.first += shift;
      range.second += shift;
    }
  }
  return group;
}

bool HeRu::DoesOverlap(uint16_t bw, RuSpec ru, const std::vector<RuSpec> &v) {
  if (bw == 160 && ru.GetRuType() == RU_2x996_TONE && !v.empty()) {
    return true;
  }

  SubcarrierGroup rangesRu =
      GetSubcarrierGroup(bw, ru.GetRuType(), ru.GetIndex());
  for (auto &p : v) {
    if (ru.GetPrimary80MHz() != p.GetPrimary80MHz()) {
      continue;
    }
    for (const auto &rangeRu : rangesRu) {
      SubcarrierGroup rangesP =
          GetSubcarrierGroup(bw, p.GetRuType(), p.GetIndex());
      for (auto &rangeP : rangesP) {
        if (rangeP.second >= rangeRu.first && rangeRu.second >= rangeP.first) {
          return true;
        }
      }
    }
  }
  return false;
}

bool HeRu::DoesOverlap(uint16_t bw, RuSpec ru,
                       const SubcarrierGroup &toneRanges, uint8_t p20Index) {
  for (const auto &range : toneRanges) {
    if (bw == 160 && ru.GetRuType() == RU_2x996_TONE) {
      return true;
    }

    SubcarrierGroup rangesRu =
        GetSubcarrierGroup(bw, ru.GetRuType(), ru.GetPhyIndex(bw, p20Index));
    for (auto &r : rangesRu) {
      if (range.second >= r.first && r.second >= range.first) {
        return true;
      }
    }
  }
  return false;
}

HeRu::RuSpec HeRu::FindOverlappingRu(uint16_t bw, RuSpec referenceRu,
                                     RuType searchedRuType) {
  std::size_t numRus = HeRu::GetNRus(bw, searchedRuType);

  std::size_t numRusPer80Mhz;
  std::vector<bool> primary80MhzFlags;
  if (bw == 160) {
    primary80MhzFlags.push_back(true);
    primary80MhzFlags.push_back(false);
    numRusPer80Mhz = (searchedRuType == HeRu::RU_2x996_TONE ? 1 : numRus / 2);
  } else {
    primary80MhzFlags.push_back(referenceRu.GetPrimary80MHz());
    numRusPer80Mhz = numRus;
  }

  for (const auto primary80MHz : primary80MhzFlags) {
    std::size_t index = 1;
    for (std::size_t indexPer80Mhz = 1; indexPer80Mhz <= numRusPer80Mhz;
         ++indexPer80Mhz, ++index) {
      RuSpec searchedRu(searchedRuType, index, primary80MHz);
      if (DoesOverlap(bw, referenceRu, {searchedRu})) {
        return searchedRu;
      }
    }
  }
  NS_ABORT_MSG("The searched RU type " << searchedRuType
                                       << " was not found for bw=" << bw
                                       << " and referenceRu=" << referenceRu);
  return HeRu::RuSpec();
}

std::ostream &operator<<(std::ostream &os, const HeRu::RuType &ruType) {
  switch (ruType) {
  case HeRu::RU_26_TONE:
    os << "26-tones";
    break;
  case HeRu::RU_52_TONE:
    os << "52-tones";
    break;
  case HeRu::RU_106_TONE:
    os << "106-tones";
    break;
  case HeRu::RU_242_TONE:
    os << "242-tones";
    break;
  case HeRu::RU_484_TONE:
    os << "484-tones";
    break;
  case HeRu::RU_996_TONE:
    os << "996-tones";
    break;
  case HeRu::RU_2x996_TONE:
    os << "2x996-tones";
    break;
  default:
    NS_FATAL_ERROR("Unknown RU type");
  }
  return os;
}

std::ostream &operator<<(std::ostream &os, const HeRu::RuSpec &ru) {
  os << "RU{" << ru.GetRuType() << "/" << ru.GetIndex() << "/"
     << (ru.GetPrimary80MHz() ? "primary80MHz" : "secondary80MHz");
  os << "}";
  return os;
}

uint16_t HeRu::GetBandwidth(RuType ruType) {
  switch (ruType) {
  case RU_26_TONE:
    return 2;
  case RU_52_TONE:
    return 4;
  case RU_106_TONE:
    return 8;
  case RU_242_TONE:
    return 20;
  case RU_484_TONE:
    return 40;
  case RU_996_TONE:
    return 80;
  case RU_2x996_TONE:
    return 160;
  default:
    NS_ABORT_MSG("RU type " << ruType << " not found");
    return 0;
  }
}

HeRu::RuType HeRu::GetRuType(uint16_t bandwidth) {
  switch (bandwidth) {
  case 2:
    return RU_26_TONE;
  case 4:
    return RU_52_TONE;
  case 8:
    return RU_106_TONE;
  case 20:
    return RU_242_TONE;
  case 40:
    return RU_484_TONE;
  case 80:
    return RU_996_TONE;
  case 160:
    return RU_2x996_TONE;
  default:
    NS_ABORT_MSG(bandwidth << " MHz bandwidth not found");
    return RU_242_TONE;
  }
}

HeRu::RuType
HeRu::GetEqualSizedRusForStations(uint16_t bandwidth, std::size_t &nStations,
                                  std::size_t &nCentral26TonesRus) {
  RuType ruType;
  uint8_t nRusAssigned = 0;

  for (auto &ru : m_heRuSubcarrierGroups) {
    if (ru.first.first == bandwidth && ru.second.size() <= nStations) {
      ruType = ru.first.second;
      nRusAssigned = ru.second.size();
      break;
    } else if (bandwidth == 160 && ru.first.first == 80 &&
               (2 * ru.second.size() <= nStations)) {
      ruType = ru.first.second;
      nRusAssigned = 2 * ru.second.size();
      break;
    }
  }
  if (nRusAssigned == 0) {
    NS_ABORT_IF(bandwidth != 160 || nStations != 1);
    nRusAssigned = 1;
    ruType = RU_2x996_TONE;
  }

  nStations = nRusAssigned;

  switch (ruType) {
  case RU_52_TONE:
  case RU_106_TONE:
    if (bandwidth == 20) {
      nCentral26TonesRus = 1;
    } else if (bandwidth == 40) {
      nCentral26TonesRus = 2;
    } else {
      nCentral26TonesRus = 5;
    }
    break;
  case RU_242_TONE:
  case RU_484_TONE:
    nCentral26TonesRus = (bandwidth >= 80 ? 1 : 0);
    break;
  default:
    nCentral26TonesRus = 0;
  }

  if (bandwidth == 160) {
    nCentral26TonesRus *= 2;
  }

  return ruType;
}

bool HeRu::RuSpec::operator==(const RuSpec &other) const {
  return m_ruType == other.m_ruType && m_index == other.m_index &&
         m_primary80MHz == other.m_primary80MHz;
}

bool HeRu::RuSpec::operator!=(const RuSpec &other) const {
  return !(*this == other);
}

bool HeRu::RuSpec::operator<(const RuSpec &other) const {
  return std::tie(m_ruType, m_index, m_primary80MHz) <
         std::tie(other.m_ruType, other.m_index, other.m_primary80MHz);
}

} // namespace ns3
