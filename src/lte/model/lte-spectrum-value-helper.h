
#ifndef LTE_SPECTRUM_VALUE_HELPER_H
#define LTE_SPECTRUM_VALUE_HELPER_H

#include <ns3/spectrum-value.h>

#include <map>
#include <vector>

namespace ns3 {

class LteSpectrumValueHelper {
public:
  static double GetCarrierFrequency(uint32_t earfcn);

  static uint16_t GetDownlinkCarrierBand(uint32_t nDl);

  static uint16_t GetUplinkCarrierBand(uint32_t nUl);

  static double GetDownlinkCarrierFrequency(uint32_t earfcn);

  static double GetUplinkCarrierFrequency(uint32_t earfcn);

  static double GetChannelBandwidth(uint16_t txBandwidthConf);

  static Ptr<SpectrumModel> GetSpectrumModel(uint32_t earfcn,
                                             uint16_t bandwidth);

  static Ptr<SpectrumValue>
  CreateTxPowerSpectralDensity(uint32_t earfcn, uint16_t bandwidth,
                               double powerTx, std::vector<int> activeRbs);

  static Ptr<SpectrumValue>
  CreateTxPowerSpectralDensity(uint32_t earfcn, uint16_t bandwidth,
                               double powerTx, std::map<int, double> powerTxMap,
                               std::vector<int> activeRbs);

  static Ptr<SpectrumValue>
  CreateUlTxPowerSpectralDensity(uint16_t earfcn, uint16_t bandwidth,
                                 double powerTx, std::vector<int> activeRbs);

  static Ptr<SpectrumValue> CreateNoisePowerSpectralDensity(uint32_t earfcn,
                                                            uint16_t bandwidth,
                                                            double noiseFigure);

  static Ptr<SpectrumValue>
  CreateNoisePowerSpectralDensity(double noiseFigure,
                                  Ptr<SpectrumModel> spectrumModel);
};

} // namespace ns3

#endif
