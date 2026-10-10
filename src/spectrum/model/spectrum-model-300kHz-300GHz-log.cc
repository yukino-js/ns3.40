
#include "spectrum-model-300kHz-300GHz-log.h"

namespace ns3 {

Ptr<SpectrumModel> SpectrumModel300Khz300GhzLog;

class static_SpectrumModel300Khz300GhzLog_initializer {
public:
  static_SpectrumModel300Khz300GhzLog_initializer() {
    std::vector<double> freqs;
    for (double f = 3e5; f < 3e11; f = f * 2) {
      freqs.push_back(f);
    }
    SpectrumModel300Khz300GhzLog = Create<SpectrumModel>(freqs);
  }
};

static_SpectrumModel300Khz300GhzLog_initializer
    g_static_SpectrumModel300Khz300GhzLog_initializer_instance;

} // namespace ns3
