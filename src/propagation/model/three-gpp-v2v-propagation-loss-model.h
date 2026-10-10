
#ifndef THREE_GPP_V2V_PROPAGATION_LOSS_MODEL_H
#define THREE_GPP_V2V_PROPAGATION_LOSS_MODEL_H

#include "three-gpp-propagation-loss-model.h"

namespace ns3 {

class ThreeGppV2vUrbanPropagationLossModel
    : public ThreeGppPropagationLossModel {
public:
  static TypeId GetTypeId();

  ThreeGppV2vUrbanPropagationLossModel();

  ~ThreeGppV2vUrbanPropagationLossModel() override;

  ThreeGppV2vUrbanPropagationLossModel(
      const ThreeGppV2vUrbanPropagationLossModel &) = delete;
  ThreeGppV2vUrbanPropagationLossModel &
  operator=(const ThreeGppV2vUrbanPropagationLossModel &) = delete;

private:
  double GetLossLos(double distance2D, double distance3D, double hUt,
                    double hBs) const override;

  double GetO2iDistance2dIn() const override;

  double GetLossNlosv(double distance2D, double distance3D, double hUt,
                      double hBs) const override;

  double GetLossNlos(double distance2D, double distance3D, double hUt,
                     double hBs) const override;

  double GetAdditionalNlosvLoss(double distance3D, double hUt,
                                double hBs) const;

  double
  GetShadowingStd(Ptr<MobilityModel> a, Ptr<MobilityModel> b,
                  ChannelCondition::LosConditionValue cond) const override;

  double GetShadowingCorrelationDistance(
      ChannelCondition::LosConditionValue cond) const override;

  int64_t DoAssignStreams(int64_t stream) override;

  double m_percType3Vehicles;
  Ptr<UniformRandomVariable> m_uniformVar;
  Ptr<LogNormalRandomVariable> m_logNorVar;
};

class ThreeGppV2vHighwayPropagationLossModel
    : public ThreeGppV2vUrbanPropagationLossModel {
public:
  static TypeId GetTypeId();

  ThreeGppV2vHighwayPropagationLossModel();

  ~ThreeGppV2vHighwayPropagationLossModel() override;

private:
  double GetLossLos(double distance2D, double distance3D, double hUt,
                    double hBs) const override;
};

} // namespace ns3

#endif
