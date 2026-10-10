
#ifndef KUN_2600MHZ_PROPAGATION_LOSS_MODEL_H
#define KUN_2600MHZ_PROPAGATION_LOSS_MODEL_H

#include "propagation-loss-model.h"

namespace ns3 {

class Kun2600MhzPropagationLossModel : public PropagationLossModel {
public:
  static TypeId GetTypeId();

  Kun2600MhzPropagationLossModel();
  ~Kun2600MhzPropagationLossModel() override;

  Kun2600MhzPropagationLossModel(const Kun2600MhzPropagationLossModel &) =
      delete;
  Kun2600MhzPropagationLossModel &
  operator=(const Kun2600MhzPropagationLossModel &) = delete;

  double GetLoss(Ptr<MobilityModel> a, Ptr<MobilityModel> b) const;

private:
  double DoCalcRxPower(double txPowerDbm, Ptr<MobilityModel> a,
                       Ptr<MobilityModel> b) const override;
  int64_t DoAssignStreams(int64_t stream) override;
};

} // namespace ns3

#endif
