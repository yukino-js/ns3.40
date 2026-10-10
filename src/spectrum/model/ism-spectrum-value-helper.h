
#ifndef ISM_SPECTRUM_VALUE_HELPER_H
#define ISM_SPECTRUM_VALUE_HELPER_H

#include "spectrum-value.h"

namespace ns3 {

class SpectrumValue5MhzFactory {
public:
  virtual ~SpectrumValue5MhzFactory() = default;
  virtual Ptr<SpectrumValue> CreateConstant(double psd);
  virtual Ptr<SpectrumValue> CreateTxPowerSpectralDensity(double txPower,
                                                          uint8_t channel);
};

} // namespace ns3

#endif
