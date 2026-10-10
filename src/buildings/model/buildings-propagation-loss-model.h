
#ifndef BUILDINGS_PROPAGATION_LOSS_MODEL_H_
#define BUILDINGS_PROPAGATION_LOSS_MODEL_H_

#include "building.h"
#include "mobility-building-info.h"

#include "ns3/nstime.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/random-variable-stream.h"

namespace ns3 {

class ShadowingLossModel;
class JakesFadingLossModel;

class BuildingsPropagationLossModel : public PropagationLossModel {
public:
  static TypeId GetTypeId();

  BuildingsPropagationLossModel();
  virtual double GetLoss(Ptr<MobilityModel> a, Ptr<MobilityModel> b) const = 0;

  double DoCalcRxPower(double txPowerDbm, Ptr<MobilityModel> a,
                       Ptr<MobilityModel> b) const override;

protected:
  double ExternalWallLoss(Ptr<MobilityBuildingInfo> a) const;
  double HeightLoss(Ptr<MobilityBuildingInfo> n) const;
  double InternalWallsLoss(Ptr<MobilityBuildingInfo> a,
                           Ptr<MobilityBuildingInfo> b) const;

  double GetShadowing(Ptr<MobilityModel> a, Ptr<MobilityModel> b) const;

  double m_lossInternalWall;

  class ShadowingLoss {
  public:
    ShadowingLoss();
    ShadowingLoss(double shadowingValue, Ptr<MobilityModel> receiver);
    double GetLoss() const;
    Ptr<MobilityModel> GetReceiver() const;

  protected:
    double m_shadowingValue;
    Ptr<MobilityModel> m_receiver;
  };

  mutable std::map<Ptr<MobilityModel>,
                   std::map<Ptr<MobilityModel>, ShadowingLoss>>
      m_shadowingLossMap;
  double EvaluateSigma(Ptr<MobilityBuildingInfo> a,
                       Ptr<MobilityBuildingInfo> b) const;

  double m_shadowingSigmaExtWalls;
  double m_shadowingSigmaOutdoor;
  double m_shadowingSigmaIndoor;
  Ptr<NormalRandomVariable> m_randVariable;

  int64_t DoAssignStreams(int64_t stream) override;
};

} // namespace ns3

#endif
