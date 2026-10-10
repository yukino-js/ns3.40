
#ifndef OKUMURA_HATA_PROPAGATION_LOSS_MODEL_H
#define OKUMURA_HATA_PROPAGATION_LOSS_MODEL_H

#include "propagation-environment.h"
#include "propagation-loss-model.h"

namespace ns3 {

class OkumuraHataPropagationLossModel : public PropagationLossModel {
public:
  static TypeId GetTypeId();

  OkumuraHataPropagationLossModel();
  ~OkumuraHataPropagationLossModel() override;

  OkumuraHataPropagationLossModel(const OkumuraHataPropagationLossModel &) =
      delete;
  OkumuraHataPropagationLossModel &
  operator=(const OkumuraHataPropagationLossModel &) = delete;

  double GetLoss(Ptr<MobilityModel> a, Ptr<MobilityModel> b) const;

private:
  double DoCalcRxPower(double txPowerDbm, Ptr<MobilityModel> a,
                       Ptr<MobilityModel> b) const override;
  int64_t DoAssignStreams(int64_t stream) override;

  EnvironmentType m_environment;
  CitySize m_citySize;
  double m_frequency;
};

} // namespace ns3

#endif
