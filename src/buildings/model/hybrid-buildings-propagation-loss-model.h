
#ifndef HYBRID_BUILDINGS_PROPAGATION_LOSS_MODEL_H_
#define HYBRID_BUILDINGS_PROPAGATION_LOSS_MODEL_H_

#include "buildings-propagation-loss-model.h"

#include <ns3/propagation-environment.h>

namespace ns3 {

class OkumuraHataPropagationLossModel;
class ItuR1411LosPropagationLossModel;
class ItuR1411NlosOverRooftopPropagationLossModel;
class ItuR1238PropagationLossModel;
class Kun2600MhzPropagationLossModel;

class HybridBuildingsPropagationLossModel
    : public BuildingsPropagationLossModel {
public:
  static TypeId GetTypeId();
  HybridBuildingsPropagationLossModel();
  ~HybridBuildingsPropagationLossModel() override;

  void SetEnvironment(EnvironmentType env);

  void SetCitySize(CitySize size);

  void SetFrequency(double freq);

  void SetRooftopHeight(double rooftopHeight);

  double GetLoss(Ptr<MobilityModel> a, Ptr<MobilityModel> b) const override;

private:
  double OkumuraHata(Ptr<MobilityModel> a, Ptr<MobilityModel> b) const;
  double ItuR1411(Ptr<MobilityModel> a, Ptr<MobilityModel> b) const;
  double ItuR1238(Ptr<MobilityModel> a, Ptr<MobilityModel> b) const;

  Ptr<OkumuraHataPropagationLossModel> m_okumuraHata;
  Ptr<ItuR1411LosPropagationLossModel> m_ituR1411Los;
  Ptr<ItuR1411NlosOverRooftopPropagationLossModel> m_ituR1411NlosOverRooftop;
  Ptr<ItuR1238PropagationLossModel> m_ituR1238;
  Ptr<Kun2600MhzPropagationLossModel> m_kun2600Mhz;

  double m_itu1411NlosThreshold;
  double m_rooftopHeight;
  double m_frequency;
};

} // namespace ns3

#endif
