
#ifndef EHT_CAPABILITIES_H
#define EHT_CAPABILITIES_H

#include "ns3/he-capabilities.h"
#include "ns3/wifi-information-element.h"

#include <map>
#include <optional>
#include <vector>

namespace ns3 {

class HeCapabilities;

struct EhtMacCapabilities {
  uint8_t epcsPriorityAccessSupported : 1;
  uint8_t ehtOmControlSupport : 1;
  uint8_t triggeredTxopSharingMode1Support : 1;
  uint8_t triggeredTxopSharingMode2Support : 1;
  uint8_t restrictedTwtSupport : 1;
  uint8_t scsTrafficDescriptionSupport : 1;
  uint8_t maxMpduLength : 2;
  uint8_t maxAmpduLengthExponentExtension : 1;

  uint16_t GetSize() const;
  void Serialize(Buffer::Iterator &start) const;
  uint16_t Deserialize(Buffer::Iterator start);
};

struct EhtPhyCapabilities {
  uint8_t support320MhzIn6Ghz : 1;
  uint8_t support242ToneRuInBwLargerThan20Mhz : 1;
  uint8_t ndpWith4TimesEhtLtfAnd32usGi : 1;
  uint8_t partialBandwidthUlMuMimo : 1;
  uint8_t suBeamformer : 1;
  uint8_t suBeamformee : 1;
  uint8_t beamformeeSsBwNotLargerThan80Mhz : 3;
  uint8_t beamformeeSs160Mhz : 3;
  uint8_t beamformeeSs320Mhz : 3;
  uint8_t nSoundingDimensionsBwNotLargerThan80Mhz : 3;
  uint8_t nSoundingDimensions160Mhz : 3;
  uint8_t nSoundingDimensions320Mhz : 3;
  uint8_t ng16SuFeedback : 1;
  uint8_t ng16MuFeedback : 1;
  uint8_t codebooksizeSuFeedback : 1;
  uint8_t codebooksizeMuFeedback : 1;
  uint8_t triggeredSuBeamformingFeedback : 1;
  uint8_t triggeredMuBeamformingPartialBwFeedback : 1;
  uint8_t triggeredCqiFeedback : 1;
  uint8_t partialBandwidthDlMuMimo : 1;
  uint8_t psrBasedSpatialReuseSupport : 1;
  uint8_t powerBoostFactorSupport : 1;
  uint8_t muPpdu4xEhtLtfAnd800nsGi : 1;
  uint8_t maxNc : 4;
  uint8_t nonTriggeredCqiFeedback : 1;
  uint8_t supportTx1024And4096QamForRuSmallerThan242Tones : 1;
  uint8_t supportRx1024And4096QamForRuSmallerThan242Tones : 1;
  uint8_t ppeThresholdsPresent : 1;
  uint8_t commonNominalPacketPadding : 2;
  uint8_t maxNumSupportedEhtLtfs : 5;
  uint8_t supportMcs15 : 4;
  uint8_t supportEhtDupIn6GHz : 1;
  uint8_t support20MhzOperatingStaReceivingNdpWithWiderBw : 1;
  uint8_t nonOfdmaUlMuMimoBwNotLargerThan80Mhz : 1;
  uint8_t nonOfdmaUlMuMimo160Mhz : 1;
  uint8_t nonOfdmaUlMuMimo320Mhz : 1;
  uint8_t muBeamformerBwNotLargerThan80Mhz : 1;
  uint8_t muBeamformer160Mhz : 1;
  uint8_t muBeamformer320Mhz : 1;
  uint8_t tbSoundingFeedbackRateLimit : 1;
  uint8_t rx1024QamInWiderBwDlOfdmaSupport : 1;
  uint8_t rx4096QamInWiderBwDlOfdmaSupport : 1;

  uint16_t GetSize() const;
  void Serialize(Buffer::Iterator &start) const;
  uint16_t Deserialize(Buffer::Iterator start);
};

struct EhtMcsAndNssSet {
  enum EhtMcsMapType : uint8_t {
    EHT_MCS_MAP_TYPE_20_MHZ_ONLY = 0,
    EHT_MCS_MAP_TYPE_NOT_LARGER_THAN_80_MHZ,
    EHT_MCS_MAP_TYPE_160_MHZ,
    EHT_MCS_MAP_TYPE_320_MHZ,
    EHT_MCS_MAP_TYPE_MAX
  };

  std::map<EhtMcsMapType, std::vector<uint8_t>> supportedEhtMcsAndNssSet;

  uint16_t GetSize() const;
  void Serialize(Buffer::Iterator &start) const;
  uint16_t Deserialize(Buffer::Iterator start, bool is2_4Ghz,
                       uint8_t heSupportedChannelWidthSet,
                       bool support320MhzIn6Ghz);
};

struct EhtPpeThresholds {
  struct EhtPpeThresholdsInfo {
    uint8_t ppetMax : 3;
    uint8_t ppet8 : 3;
  };

  uint8_t nssPe : 4;
  uint8_t ruIndexBitmask : 5;
  std::vector<EhtPpeThresholdsInfo> ppeThresholdsInfo;

  uint16_t GetSize() const;
  void Serialize(Buffer::Iterator &start) const;
  uint16_t Deserialize(Buffer::Iterator start);
};

class EhtCapabilities : public WifiInformationElement {
public:
  EhtCapabilities();
  EhtCapabilities(bool is2_4Ghz,
                  const std::optional<HeCapabilities> &heCapabilities);
  WifiInformationElementId ElementId() const override;
  WifiInformationElementId ElementIdExt() const override;
  void Print(std::ostream &os) const override;

  void SetMaxMpduLength(uint16_t length);

  void SetMaxAmpduLength(uint32_t maxAmpduLength);

  uint32_t GetMaxAmpduLength() const;

  uint16_t GetMaxMpduLength() const;

  void SetSupportedTxEhtMcsAndNss(EhtMcsAndNssSet::EhtMcsMapType mapType,
                                  uint8_t upperMcs, uint8_t maxNss);
  void SetSupportedRxEhtMcsAndNss(EhtMcsAndNssSet::EhtMcsMapType mapType,
                                  uint8_t upperMcs, uint8_t maxNss);

  uint8_t GetHighestSupportedRxMcs(EhtMcsAndNssSet::EhtMcsMapType mapType);
  uint8_t GetHighestSupportedTxMcs(EhtMcsAndNssSet::EhtMcsMapType mapType);

  void SetPpeThresholds(
      uint8_t nssPe, uint8_t ruIndexBitmask,
      const std::vector<std::pair<uint8_t, uint8_t>> &ppeThresholds);

  EhtMacCapabilities m_macCapabilities;
  EhtPhyCapabilities m_phyCapabilities;
  EhtMcsAndNssSet m_supportedEhtMcsAndNssSet;
  EhtPpeThresholds m_ppeThresholds;

private:
  uint16_t GetInformationFieldSize() const override;
  void SerializeInformationField(Buffer::Iterator start) const override;
  uint16_t DeserializeInformationField(Buffer::Iterator start,
                                       uint16_t length) override;

  bool m_is2_4Ghz;
  std::optional<HeCapabilities> m_heCapabilities;
};

} // namespace ns3

#endif
