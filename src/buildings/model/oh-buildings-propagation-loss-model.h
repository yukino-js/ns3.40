
#ifndef OH_BUILDINGS_PROPAGATION_LOSS_MODEL_H_
#define OH_BUILDINGS_PROPAGATION_LOSS_MODEL_H_

#include "buildings-propagation-loss-model.h"

namespace ns3 {

class OkumuraHataPropagationLossModel;

class OhBuildingsPropagationLossModel : public BuildingsPropagationLossModel {
public:
  static TypeId GetTypeId();
  OhBuildingsPropagationLossModel();
  ~OhBuildingsPropagationLossModel() override;

  double GetLoss(Ptr<MobilityModel> a, Ptr<MobilityModel> b) const override;

private:
  Ptr<OkumuraHataPropagationLossModel> m_okumuraHata;
};

} // namespace ns3

#endif
