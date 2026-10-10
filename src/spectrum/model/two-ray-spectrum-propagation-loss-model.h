
#ifndef TWO_RAY_SPECTRUM_PROPAGATION_LOSS_H
#define TWO_RAY_SPECTRUM_PROPAGATION_LOSS_H

#include "phased-array-spectrum-propagation-loss-model.h"
#include "spectrum-signal-parameters.h"

#include "ns3/channel-condition-model.h"

#include <map>

class FtrFadingModelAverageTest;
class ArrayResponseTest;
class OverallGainAverageTest;

namespace ns3 {

class NetDevice;

class TwoRaySpectrumPropagationLossModel
    : public PhasedArraySpectrumPropagationLossModel {
  friend class ::FtrFadingModelAverageTest;
  friend class ::ArrayResponseTest;
  friend class ::OverallGainAverageTest;

public:
  struct FtrParams {
    FtrParams(double m, double sigma, double k, double delta) {
      NS_ASSERT(delta >= 0.0 && delta <= 1.0);

      m_m = m;
      m_sigma = sigma;
      m_k = k;
      m_delta = delta;
    }

    FtrParams() = delete;

    double m_m = 0;

    double m_sigma = 0;

    double m_k = 0;

    double m_delta = 0;
  };

  using CarrierFrequencyFtrParamsTuple =
      std::tuple<std::vector<double>, std::vector<FtrParams>>;

  using FtrParamsLookupTable =
      std::map<std::string, std::map<ChannelCondition::LosConditionValue,
                                     CarrierFrequencyFtrParamsTuple>>;

  TwoRaySpectrumPropagationLossModel();

  ~TwoRaySpectrumPropagationLossModel() override;

  void DoDispose() override;

  static TypeId GetTypeId();

  void SetScenario(const std::string &scenario);

  void SetFrequency(double f);

  int64_t AssignStreams(int64_t stream);

  Ptr<SpectrumValue> DoCalcRxPowerSpectralDensity(
      Ptr<const SpectrumSignalParameters> txPsd, Ptr<const MobilityModel> a,
      Ptr<const MobilityModel> b, Ptr<const PhasedArrayModel> aPhasedArrayModel,
      Ptr<const PhasedArrayModel> bPhasedArrayModel) const override;

private:
  ChannelCondition::LosConditionValue
  GetLosCondition(Ptr<const MobilityModel> a, Ptr<const MobilityModel> b) const;

  FtrParams GetFtrParameters(Ptr<const MobilityModel> a,
                             Ptr<const MobilityModel> b) const;

  double GetFtrFastFading(const FtrParams &params) const;

  double
  CalcBeamformingGain(Ptr<const MobilityModel> a, Ptr<const MobilityModel> b,
                      Ptr<const PhasedArrayModel> aPhasedArrayModel,
                      Ptr<const PhasedArrayModel> bPhasedArrayModel) const;

  std::size_t SearchClosestFc(const std::vector<double> &frequencies,
                              double targetFc) const;

  double m_frequency;

  Ptr<UniformRandomVariable> m_uniformRv;

  Ptr<NormalRandomVariable> m_normalRv;

  Ptr<GammaRandomVariable> m_gammaRv;

  std::string m_scenario;

  Ptr<ChannelConditionModel> m_channelConditionModel;
};

} // namespace ns3

#endif
