#ifndef LR_WPAN_SPECTRUM_VALUE_HELPER_H
#define LR_WPAN_SPECTRUM_VALUE_HELPER_H

#include <ns3/ptr.h>

namespace ns3 {

class SpectrumValue;

class LrWpanSpectrumValueHelper {
public:
  LrWpanSpectrumValueHelper();
  virtual ~LrWpanSpectrumValueHelper();

  Ptr<SpectrumValue> CreateTxPowerSpectralDensity(double txPower,
                                                  uint32_t channel);

  Ptr<SpectrumValue> CreateNoisePowerSpectralDensity(uint32_t channel);

  void SetNoiseFactor(double f);

  static double TotalAvgPower(Ptr<const SpectrumValue> psd, uint32_t channel);

private:
  double m_noiseFactor;
};

} // namespace ns3

#endif
