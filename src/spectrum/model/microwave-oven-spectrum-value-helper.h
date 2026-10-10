
#ifndef MICROWAVE_OVEN_SPECTRUM_VALUE_HELPER_H
#define MICROWAVE_OVEN_SPECTRUM_VALUE_HELPER_H

#include "spectrum-value.h"

namespace ns3 {

class MicrowaveOvenSpectrumValueHelper {
public:
  static Ptr<SpectrumValue> CreatePowerSpectralDensityMwo1();

  static Ptr<SpectrumValue> CreatePowerSpectralDensityMwo2();
};

} // namespace ns3

#endif
