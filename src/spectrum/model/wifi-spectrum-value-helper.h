
#ifndef WIFI_SPECTRUM_VALUE_HELPER_H
#define WIFI_SPECTRUM_VALUE_HELPER_H

#include "spectrum-value.h"

namespace ns3 {

using WifiSpectrumBandIndices = std::pair<uint32_t, uint32_t>;

class WifiSpectrumValueHelper {
public:
  virtual ~WifiSpectrumValueHelper() = default;

  static Ptr<SpectrumModel> GetSpectrumModel(uint32_t centerFrequency,
                                             uint16_t channelWidth,
                                             uint32_t carrierSpacing,
                                             uint16_t guardBandwidth);

  static Ptr<SpectrumValue>
  CreateDsssTxPowerSpectralDensity(uint32_t centerFrequency, double txPowerW,
                                   uint16_t guardBandwidth);

  static Ptr<SpectrumValue> CreateOfdmTxPowerSpectralDensity(
      uint32_t centerFrequency, uint16_t channelWidth, double txPowerW,
      uint16_t guardBandwidth, double minInnerBandDbr = -20,
      double minOuterbandDbr = -28, double lowestPointDbr = -40);

  static Ptr<SpectrumValue> CreateDuplicated20MhzTxPowerSpectralDensity(
      uint32_t centerFrequency, uint16_t channelWidth, double txPowerW,
      uint16_t guardBandwidth, double minInnerBandDbr = -20,
      double minOuterbandDbr = -28, double lowestPointDbr = -40,
      const std::vector<bool> &puncturedSubchannels = std::vector<bool>{});

  static Ptr<SpectrumValue> CreateHtOfdmTxPowerSpectralDensity(
      uint32_t centerFrequency, uint16_t channelWidth, double txPowerW,
      uint16_t guardBandwidth, double minInnerBandDbr = -20,
      double minOuterbandDbr = -28, double lowestPointDbr = -40);

  static Ptr<SpectrumValue> CreateHeOfdmTxPowerSpectralDensity(
      uint32_t centerFrequency, uint16_t channelWidth, double txPowerW,
      uint16_t guardBandwidth, double minInnerBandDbr = -20,
      double minOuterbandDbr = -28, double lowestPointDbr = -40,
      const std::vector<bool> &puncturedSubchannels = std::vector<bool>{});

  static Ptr<SpectrumValue> CreateHeMuOfdmTxPowerSpectralDensity(
      uint32_t centerFrequency, uint16_t channelWidth, double txPowerW,
      uint16_t guardBandwidth, const WifiSpectrumBandIndices &ru);

  static Ptr<SpectrumValue> CreateNoisePowerSpectralDensity(
      uint32_t centerFrequency, uint16_t channelWidth, uint32_t carrierSpacing,
      double noiseFigure, uint16_t guardBandwidth);

  static Ptr<SpectrumValue>
  CreateNoisePowerSpectralDensity(double noiseFigure,
                                  Ptr<SpectrumModel> spectrumModel);

  static void CreateSpectrumMaskForOfdm(
      Ptr<SpectrumValue> c,
      const std::vector<WifiSpectrumBandIndices> &allocatedSubBands,
      const WifiSpectrumBandIndices &maskBand, double txPowerPerBandW,
      uint32_t nGuardBands, uint32_t innerSlopeWidth, double minInnerBandDbr,
      double minOuterbandDbr, double lowestPointDbr,
      const std::vector<WifiSpectrumBandIndices> &puncturedSubBands =
          std::vector<WifiSpectrumBandIndices>{},
      uint32_t puncturedSlopeWidth = 0);

  static void NormalizeSpectrumMask(Ptr<SpectrumValue> c, double txPowerW);

  static double DbmToW(double dbm);

  static double GetBandPowerW(Ptr<SpectrumValue> psd,
                              const WifiSpectrumBandIndices &band);
};

struct FrequencyRange {
  uint16_t minFrequency{0};
  uint16_t maxFrequency{0};
};

bool operator<(const FrequencyRange &lhs, const FrequencyRange &rhs);

bool operator==(const FrequencyRange &lhs, const FrequencyRange &rhs);

bool operator!=(const FrequencyRange &lhs, const FrequencyRange &rhs);

std::ostream &operator<<(std::ostream &os, const FrequencyRange &freqRange);

constexpr FrequencyRange WHOLE_WIFI_SPECTRUM = {2401, 7125};

constexpr FrequencyRange WIFI_SPECTRUM_2_4_GHZ = {2401, 2483};

constexpr FrequencyRange WIFI_SPECTRUM_5_GHZ = {5170, 5915};

constexpr FrequencyRange WIFI_SPECTRUM_6_GHZ = {5945, 7125};

} // namespace ns3

#endif
