
#include "ism-spectrum-value-helper.h"

#include "ns3/assert.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("IsmSpectrumValueHelper");

static Ptr<SpectrumModel> g_WifiSpectrumModel5Mhz;

static class WifiSpectrumModel5MhzInitializer {
public:
  WifiSpectrumModel5MhzInitializer() {
    Bands bands;
    for (int i = -4; i < 13 + 7; i++) {
      BandInfo bi;
      bi.fl = 2407.0e6 + i * 5.0e6;
      bi.fh = 2407.0e6 + (i + 1) * 5.0e6;
      bi.fc = (bi.fl + bi.fh) / 2;
      bands.push_back(bi);
    }
    g_WifiSpectrumModel5Mhz = Create<SpectrumModel>(bands);
  }
} g_WifiSpectrumModel5MhzInitializerInstance;

Ptr<SpectrumValue> SpectrumValue5MhzFactory::CreateConstant(double v) {
  Ptr<SpectrumValue> c = Create<SpectrumValue>(g_WifiSpectrumModel5Mhz);
  (*c) = v;
  return c;
}

Ptr<SpectrumValue>
SpectrumValue5MhzFactory::CreateTxPowerSpectralDensity(double txPower,
                                                       uint8_t channel) {
  Ptr<SpectrumValue> txPsd = Create<SpectrumValue>(g_WifiSpectrumModel5Mhz);

  double txPowerDensity = txPower / 20e6;

  NS_ASSERT(channel >= 1);
  NS_ASSERT(channel <= 13);

  (*txPsd)[channel - 1] = txPowerDensity * 1e-4;
  (*txPsd)[channel] = txPowerDensity * 1e-4;
  (*txPsd)[channel + 1] = txPowerDensity * 0.0015849;
  (*txPsd)[channel + 2] = txPowerDensity * 0.0015849;
  (*txPsd)[channel + 3] = txPowerDensity;
  (*txPsd)[channel + 4] = txPowerDensity;
  (*txPsd)[channel + 5] = txPowerDensity;
  (*txPsd)[channel + 6] = txPowerDensity;
  (*txPsd)[channel + 7] = txPowerDensity * 0.0015849;
  (*txPsd)[channel + 8] = txPowerDensity * 0.0015849;
  (*txPsd)[channel + 9] = txPowerDensity * 1e-4;
  (*txPsd)[channel + 10] = txPowerDensity * 1e-4;

  return txPsd;
}

} // namespace ns3
