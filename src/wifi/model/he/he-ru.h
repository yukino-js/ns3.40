
#ifndef HE_RU_H
#define HE_RU_H

#include <cstdint>
#include <map>
#include <ostream>
#include <vector>

namespace ns3 {

class HeRu {
public:
  enum RuType {
    RU_26_TONE = 0,
    RU_52_TONE,
    RU_106_TONE,
    RU_242_TONE,
    RU_484_TONE,
    RU_996_TONE,
    RU_2x996_TONE
  };

  typedef std::pair<int16_t, int16_t> SubcarrierRange;

  typedef std::vector<SubcarrierRange> SubcarrierGroup;

  class RuSpec {
  public:
    RuSpec();
    RuSpec(RuType ruType, std::size_t index, bool primary80MHz);

    RuType GetRuType() const;
    std::size_t GetIndex() const;
    bool GetPrimary80MHz() const;
    std::size_t GetPhyIndex(uint16_t bw, uint8_t p20Index) const;

    bool operator==(const RuSpec &other) const;
    bool operator!=(const RuSpec &other) const;
    bool operator<(const RuSpec &other) const;

  private:
    RuType m_ruType;
    std::size_t m_index;
    bool m_primary80MHz;
  };

  struct RuSpecCompare {
    RuSpecCompare(uint16_t channelWidth, uint8_t p20Index);
    bool operator()(const RuSpec &lhs, const RuSpec &rhs) const;

  private:
    uint16_t m_channelWidth;
    uint8_t m_p20Index;
  };

  static std::size_t GetNRus(uint16_t bw, RuType ruType);

  static std::vector<HeRu::RuSpec> GetRusOfType(uint16_t bw,
                                                HeRu::RuType ruType);

  static std::vector<HeRu::RuSpec> GetCentral26TonesRus(uint16_t bw,
                                                        HeRu::RuType ruType);

  static SubcarrierGroup GetSubcarrierGroup(uint16_t bw, RuType ruType,
                                            std::size_t phyIndex);

  static bool DoesOverlap(uint16_t bw, RuSpec ru, const std::vector<RuSpec> &v);

  static bool DoesOverlap(uint16_t bw, RuSpec ru,
                          const SubcarrierGroup &toneRanges, uint8_t p20Index);

  static RuSpec FindOverlappingRu(uint16_t bw, RuSpec referenceRu,
                                  RuType searchedRuType);

  static uint16_t GetBandwidth(RuType ruType);

  static RuType GetRuType(uint16_t bandwidth);

  static RuType GetEqualSizedRusForStations(uint16_t bandwidth,
                                            std::size_t &nStations,
                                            std::size_t &nCentral26TonesRus);

  typedef std::pair<uint8_t, RuType> BwTonesPair;

  typedef std::map<BwTonesPair, std::vector<SubcarrierGroup>> SubcarrierGroups;

  static const SubcarrierGroups m_heRuSubcarrierGroups;

  using RuAllocationMap = std::map<uint8_t, std::vector<RuSpec>>;

  static const RuAllocationMap m_heRuAllocations;

  static std::vector<RuSpec> GetRuSpecs(uint8_t ruAllocation);

  static uint8_t GetEqualizedRuAllocation(RuType ruType, bool isOdd);

  static constexpr uint8_t EMPTY_242_TONE_RU = 113;
};

std::ostream &operator<<(std::ostream &os, const HeRu::RuType &ruType);

std::ostream &operator<<(std::ostream &os, const HeRu::RuSpec &ru);

} // namespace ns3

#endif
