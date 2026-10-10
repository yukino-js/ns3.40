
#include "spectrum-model-ism2400MHz-res1MHz.h"

namespace ns3 {

Ptr<SpectrumModel> SpectrumModelIsm2400MhzRes1Mhz;

class static_SpectrumModelIsm2400MhzRes1Mhz_initializer {
public:
  static_SpectrumModelIsm2400MhzRes1Mhz_initializer() {
    std::vector<double> freqs;
    freqs.reserve(100);
    for (int i = 0; i < 100; ++i) {
      freqs.push_back((i + 2400) * 1e6);
    }

    SpectrumModelIsm2400MhzRes1Mhz = Create<SpectrumModel>(freqs);
  }
};

static_SpectrumModelIsm2400MhzRes1Mhz_initializer
    g_static_SpectrumModelIsm2400MhzRes1Mhz_initializer_instance;

} // namespace ns3
