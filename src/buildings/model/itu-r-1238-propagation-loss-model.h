
#ifndef ITU_R_1238_PROPAGATION_LOSS_MODEL_H
#define ITU_R_1238_PROPAGATION_LOSS_MODEL_H

#include <ns3/propagation-environment.h>
#include <ns3/propagation-loss-model.h>

namespace ns3 {

class ItuR1238PropagationLossModel : public PropagationLossModel {
public:
  static TypeId GetTypeId();

  double GetLoss(Ptr<MobilityModel> a, Ptr<MobilityModel> b) const;

private:
  double DoCalcRxPower(double txPowerDbm, Ptr<MobilityModel> a,
                       Ptr<MobilityModel> b) const override;
  int64_t DoAssignStreams(int64_t stream) override;

  double m_frequency;
};

} // namespace ns3

#endif
