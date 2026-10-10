
#ifndef COST231_PROPAGATION_LOSS_MODEL_H
#define COST231_PROPAGATION_LOSS_MODEL_H

#include "propagation-loss-model.h"

#include "ns3/nstime.h"

namespace ns3 {

class Cost231PropagationLossModel : public PropagationLossModel {
public:
  static TypeId GetTypeId();
  Cost231PropagationLossModel();

  Cost231PropagationLossModel(const Cost231PropagationLossModel &) = delete;
  Cost231PropagationLossModel &
  operator=(const Cost231PropagationLossModel &) = delete;

  double GetLoss(Ptr<MobilityModel> a, Ptr<MobilityModel> b) const;

  void SetBSAntennaHeight(double height);
  void SetSSAntennaHeight(double height);

  void SetLambda(double lambda);
  void SetLambda(double frequency, double speed);
  void SetMinDistance(double minDistance);
  double GetBSAntennaHeight() const;
  double GetSSAntennaHeight() const;
  double GetMinDistance() const;
  double GetLambda() const;
  double GetShadowing() const;
  void SetShadowing(double shadowing);

private:
  double DoCalcRxPower(double txPowerDbm, Ptr<MobilityModel> a,
                       Ptr<MobilityModel> b) const override;
  int64_t DoAssignStreams(int64_t stream) override;

  double m_BSAntennaHeight;
  double m_SSAntennaHeight;
  double m_lambda;
  double m_minDistance;
  double m_frequency;
  double m_shadowing;
};

} // namespace ns3

#endif
