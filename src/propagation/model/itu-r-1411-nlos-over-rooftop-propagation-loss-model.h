
#ifndef ITU_R_1411_NLOS_OVER_ROOFTOP_PROPAGATION_LOSS_MODEL_H
#define ITU_R_1411_NLOS_OVER_ROOFTOP_PROPAGATION_LOSS_MODEL_H

#include "propagation-environment.h"
#include "propagation-loss-model.h"

namespace ns3 {

class ItuR1411NlosOverRooftopPropagationLossModel
    : public PropagationLossModel {
public:
  static TypeId GetTypeId();

  ItuR1411NlosOverRooftopPropagationLossModel();
  ~ItuR1411NlosOverRooftopPropagationLossModel() override;

  ItuR1411NlosOverRooftopPropagationLossModel(
      const ItuR1411NlosOverRooftopPropagationLossModel &) = delete;
  ItuR1411NlosOverRooftopPropagationLossModel &
  operator=(const ItuR1411NlosOverRooftopPropagationLossModel &) = delete;

  void SetFrequency(double freq);

  double GetLoss(Ptr<MobilityModel> a, Ptr<MobilityModel> b) const;

private:
  double DoCalcRxPower(double txPowerDbm, Ptr<MobilityModel> a,
                       Ptr<MobilityModel> b) const override;
  int64_t DoAssignStreams(int64_t stream) override;

  double m_frequency;
  double m_lambda;
  EnvironmentType m_environment;
  CitySize m_citySize;
  double m_rooftopHeight;
  double m_streetsOrientation;
  double m_streetsWidth;
  double m_buildingsExtend;
  double m_buildingSeparation;
};

} // namespace ns3

#endif
