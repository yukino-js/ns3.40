
#ifndef PROBABILISTIC_V2V_CHANNEL_CONDITION_MODEL_H
#define PROBABILISTIC_V2V_CHANNEL_CONDITION_MODEL_H

#include "channel-condition-model.h"

namespace ns3 {

class MobilityModel;

class ProbabilisticV2vUrbanChannelConditionModel
    : public ThreeGppChannelConditionModel {
public:
  static TypeId GetTypeId();

  ProbabilisticV2vUrbanChannelConditionModel();

  ~ProbabilisticV2vUrbanChannelConditionModel() override;

private:
  double ComputePlos(Ptr<const MobilityModel> a,
                     Ptr<const MobilityModel> b) const override;

  double ComputePnlos(Ptr<const MobilityModel> a,
                      Ptr<const MobilityModel> b) const override;

  VehicleDensity m_densityUrban{VehicleDensity::INVALID};
};

class ProbabilisticV2vHighwayChannelConditionModel
    : public ThreeGppChannelConditionModel {
public:
  static TypeId GetTypeId();

  ProbabilisticV2vHighwayChannelConditionModel();

  ~ProbabilisticV2vHighwayChannelConditionModel() override;

private:
  double ComputePlos(Ptr<const MobilityModel> a,
                     Ptr<const MobilityModel> b) const override;

  double ComputePnlos(Ptr<const MobilityModel> a,
                      Ptr<const MobilityModel> b) const override;

  VehicleDensity m_densityHighway{VehicleDensity::INVALID};
};

} // namespace ns3

#endif
