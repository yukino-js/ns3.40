#include "lr-wpan-spectrum-value-helper.h"

#include <ns3/log.h>
#include <ns3/spectrum-value.h>

#include <cmath>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("LrWpanSpectrumValueHelper");

Ptr<SpectrumModel> g_LrWpanSpectrumModel;

class LrWpanSpectrumModelInitializer {
public:
  LrWpanSpectrumModelInitializer() {
    NS_LOG_FUNCTION(this);

    Bands bands;
    for (int i = -1; i < 83; i++) {
      BandInfo bi;
      bi.fl = 2400.5e6 + i * 1.0e6;
      bi.fh = 2400.5e6 + (i + 1) * 1.0e6;
      bi.fc = (bi.fl + bi.fh) / 2;
      bands.push_back(bi);
    }
    g_LrWpanSpectrumModel = Create<SpectrumModel>(bands);
  }

} g_LrWpanSpectrumModelInitializerInstance;

LrWpanSpectrumValueHelper::LrWpanSpectrumValueHelper() {
  NS_LOG_FUNCTION(this);
  m_noiseFactor = 1.0;
}

LrWpanSpectrumValueHelper::~LrWpanSpectrumValueHelper() {
  NS_LOG_FUNCTION(this);
}

Ptr<SpectrumValue>
LrWpanSpectrumValueHelper::CreateTxPowerSpectralDensity(double txPower,
                                                        uint32_t channel) {
  NS_LOG_FUNCTION(this);
  Ptr<SpectrumValue> txPsd = Create<SpectrumValue>(g_LrWpanSpectrumModel);

  txPower = pow(10., (txPower - 30) / 10);

  double txPowerDensity = txPower / 2.0e6;

  NS_ASSERT_MSG((channel >= 11 && channel <= 26), "Invalid channel numbers");

  (*txPsd)[2405 + 5 * (channel - 11) - 2400 - 2] = txPowerDensity * 0.005;
  (*txPsd)[2405 + 5 * (channel - 11) - 2400 - 1] = txPowerDensity * 0.495;
  (*txPsd)[2405 + 5 * (channel - 11) - 2400] = txPowerDensity;
  (*txPsd)[2405 + 5 * (channel - 11) - 2400 + 1] = txPowerDensity * 0.495;
  (*txPsd)[2405 + 5 * (channel - 11) - 2400 + 2] = txPowerDensity * 0.005;

  return txPsd;
}

Ptr<SpectrumValue>
LrWpanSpectrumValueHelper::CreateNoisePowerSpectralDensity(uint32_t channel) {
  NS_LOG_FUNCTION(this);
  Ptr<SpectrumValue> noisePsd = Create<SpectrumValue>(g_LrWpanSpectrumModel);

  static const double BOLTZMANN = 1.3803e-23;
  double Nt = BOLTZMANN * 290.0;
  double noisePowerDensity = m_noiseFactor * Nt;

  NS_ASSERT_MSG((channel >= 11 && channel <= 26), "Invalid channel numbers");

  (*noisePsd)[2405 + 5 * (channel - 11) - 2400 - 2] = noisePowerDensity;
  (*noisePsd)[2405 + 5 * (channel - 11) - 2400 - 1] = noisePowerDensity;
  (*noisePsd)[2405 + 5 * (channel - 11) - 2400] = noisePowerDensity;
  (*noisePsd)[2405 + 5 * (channel - 11) - 2400 + 1] = noisePowerDensity;
  (*noisePsd)[2405 + 5 * (channel - 11) - 2400 + 2] = noisePowerDensity;

  return noisePsd;
}

void LrWpanSpectrumValueHelper::SetNoiseFactor(double f) { m_noiseFactor = f; }

double LrWpanSpectrumValueHelper::TotalAvgPower(Ptr<const SpectrumValue> psd,
                                                uint32_t channel) {
  NS_LOG_FUNCTION(psd);
  double totalAvgPower = 0.0;

  NS_ASSERT(psd->GetSpectrumModel() == g_LrWpanSpectrumModel);

  totalAvgPower += (*psd)[2405 + 5 * (channel - 11) - 2400 - 2];
  totalAvgPower += (*psd)[2405 + 5 * (channel - 11) - 2400 - 1];
  totalAvgPower += (*psd)[2405 + 5 * (channel - 11) - 2400];
  totalAvgPower += (*psd)[2405 + 5 * (channel - 11) - 2400 + 1];
  totalAvgPower += (*psd)[2405 + 5 * (channel - 11) - 2400 + 2];
  totalAvgPower *= 1.0e6;

  return totalAvgPower;
}

} // namespace ns3
