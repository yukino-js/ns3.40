
#ifndef ITU_R_1411_LOS_PROPAGATION_LOSS_MODEL_H
#define ITU_R_1411_LOS_PROPAGATION_LOSS_MODEL_H

#include "propagation-loss-model.h"

namespace ns3 {

class ItuR1411LosPropagationLossModel : public PropagationLossModel {
public:
  static TypeId GetTypeId();

  ItuR1411LosPropagationLossModel();
  ~ItuR1411LosPropagationLossModel() override;

  ItuR1411LosPropagationLossModel(const ItuR1411LosPropagationLossModel &) =
      delete;
  ItuR1411LosPropagationLossModel &
  operator=(const ItuR1411LosPropagationLossModel &) = delete;

  void SetFrequency(double freq);

  double GetLoss(Ptr<MobilityModel> a, Ptr<MobilityModel> b) const;

private:
  double DoCalcRxPower(double txPowerDbm, Ptr<MobilityModel> a,
                       Ptr<MobilityModel> b) const override;

  int64_t DoAssignStreams(int64_t stream) override;

  double m_lambda;
};

} // namespace ns3

#endif
