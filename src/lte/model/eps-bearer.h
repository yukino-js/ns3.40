
#ifndef EPS_BEARER
#define EPS_BEARER

#include <ns3/object-base.h>
#include <ns3/uinteger.h>

#include <unordered_map>

namespace ns3 {

struct GbrQosInformation {
  GbrQosInformation();

  uint64_t gbrDl;
  uint64_t gbrUl;
  uint64_t mbrDl;
  uint64_t mbrUl;
};

struct AllocationRetentionPriority {
  AllocationRetentionPriority();
  uint8_t priorityLevel;
  bool preemptionCapability;
  bool preemptionVulnerability;
};

class EpsBearer : public ObjectBase {
public:
  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  enum Qci : uint8_t {
    GBR_CONV_VOICE = 1,
    GBR_CONV_VIDEO = 2,
    GBR_GAMING = 3,
    GBR_NON_CONV_VIDEO = 4,
    GBR_MC_PUSH_TO_TALK = 65,
    GBR_NMC_PUSH_TO_TALK = 66,
    GBR_MC_VIDEO = 67,
    GBR_V2X = 75,
    GBR_LIVE_UL_71 = 71,
    GBR_LIVE_UL_72 = 72,
    GBR_LIVE_UL_73 = 73,
    GBR_LIVE_UL_74 = 74,
    GBR_LIVE_UL_76 = 76,
    NGBR_IMS = 5,
    NGBR_VIDEO_TCP_OPERATOR = 6,
    NGBR_VOICE_VIDEO_GAMING = 7,
    NGBR_VIDEO_TCP_PREMIUM = 8,
    NGBR_VIDEO_TCP_DEFAULT = 9,
    NGBR_MC_DELAY_SIGNAL = 69,
    NGBR_MC_DATA = 70,
    NGBR_V2X = 79,
    NGBR_LOW_LAT_EMBB = 80,
    DGBR_DISCRETE_AUT_SMALL = 82,
    DGBR_DISCRETE_AUT_LARGE = 83,
    DGBR_ITS = 84,
    DGBR_ELECTRICITY = 85,
    DGBR_V2X = 86,
    DGBR_INTER_SERV_87 = 87,
    DGBR_INTER_SERV_88 = 88,
    DGBR_VISUAL_CONTENT_89 = 89,
    DGBR_VISUAL_CONTENT_90 = 90,
  };

  Qci qci;

  GbrQosInformation gbrQosInfo;
  AllocationRetentionPriority arp;

  EpsBearer();

  EpsBearer(Qci x);

  EpsBearer(Qci x, GbrQosInformation y);

  EpsBearer(const EpsBearer &o);

  ~EpsBearer() override {}

  void SetRelease(uint8_t release);

  uint8_t GetRelease() const { return m_release; }

  uint8_t GetResourceType() const;

  uint8_t GetPriority() const;

  uint16_t GetPacketDelayBudgetMs() const;

  double GetPacketErrorLossRate() const;

private:
  struct QciHash {
    std::size_t operator()(const Qci &s) const noexcept {
      return std::hash<uint8_t>{}(s);
    }
  };

  typedef std::unordered_map<
      Qci, std::tuple<uint8_t, uint8_t, uint16_t, double, uint32_t, uint32_t>,
      QciHash>
      BearerRequirementsMap;

  static uint8_t GetResourceType(const BearerRequirementsMap &map, Qci qci) {
    return std::get<0>(map.at(qci));
  }

  static uint8_t GetPriority(const BearerRequirementsMap &map, Qci qci) {
    return std::get<1>(map.at(qci));
  }

  static uint16_t GetPacketDelayBudgetMs(const BearerRequirementsMap &map,
                                         Qci qci) {
    return std::get<2>(map.at(qci));
  }

  static double GetPacketErrorLossRate(const BearerRequirementsMap &map,
                                       Qci qci) {
    return std::get<3>(map.at(qci));
  }

  static uint32_t GetMaxDataBurst(const BearerRequirementsMap &map, Qci qci) {
    return std::get<4>(map.at(qci));
  }

  static uint32_t GetAvgWindow(const BearerRequirementsMap &map, Qci qci) {
    return std::get<5>(map.at(qci));
  }

  static const BearerRequirementsMap &GetRequirementsRel11();

  static const BearerRequirementsMap &GetRequirementsRel15();

  static const BearerRequirementsMap &GetRequirementsRel18();

  BearerRequirementsMap m_requirements;

  uint8_t m_release{30};
};

} // namespace ns3

#endif
