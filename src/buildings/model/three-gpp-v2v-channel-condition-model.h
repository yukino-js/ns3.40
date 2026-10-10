
#ifndef THREE_GPP_V2V_CHANNEL_CONDITION_MODEL
#define THREE_GPP_V2V_CHANNEL_CONDITION_MODEL

#include "buildings-channel-condition-model.h"

#include "ns3/channel-condition-model.h"

#include <functional>

namespace ns3 {

class MobilityModel;

class ThreeGppV2vUrbanChannelConditionModel
    : public ThreeGppChannelConditionModel {
public:
  static TypeId GetTypeId();

  ThreeGppV2vUrbanChannelConditionModel();

  ~ThreeGppV2vUrbanChannelConditionModel() override;

private:
  double ComputePlos(Ptr<const MobilityModel> a,
                     Ptr<const MobilityModel> b) const override;

  double ComputePnlos(Ptr<const MobilityModel> a,
                      Ptr<const MobilityModel> b) const override;

  Ptr<BuildingsChannelConditionModel> m_buildingsCcm;
};

class ThreeGppV2vHighwayChannelConditionModel
    : public ThreeGppChannelConditionModel {
public:
  static TypeId GetTypeId();

  ThreeGppV2vHighwayChannelConditionModel();

  ~ThreeGppV2vHighwayChannelConditionModel() override;

private:
  double ComputePlos(Ptr<const MobilityModel> a,
                     Ptr<const MobilityModel> b) const override;

  double ComputePnlos(Ptr<const MobilityModel> a,
                      Ptr<const MobilityModel> b) const override;

  std::function<Ptr<ChannelCondition>(Ptr<const MobilityModel>,
                                      Ptr<const MobilityModel>)>
      ComputeChCond;

  Ptr<ChannelCondition> GetChCondAndFixCallback(Ptr<const MobilityModel> a,
                                                Ptr<const MobilityModel> b);

  Ptr<ChannelCondition>
  GetChCondWithBuildings(Ptr<const MobilityModel> a,
                         Ptr<const MobilityModel> b) const;

  Ptr<ChannelCondition>
  GetChCondWithNoBuildings(Ptr<const MobilityModel> a,
                           Ptr<const MobilityModel> b) const;

  Ptr<BuildingsChannelConditionModel> m_buildingsCcm;
};

} // namespace ns3

#endif
