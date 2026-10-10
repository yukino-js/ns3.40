#ifndef JAKES_STATIONARY_LOSS_MODEL_H
#define JAKES_STATIONARY_LOSS_MODEL_H

#include "jakes-process.h"
#include "propagation-cache.h"
#include "propagation-loss-model.h"

namespace ns3 {

class JakesPropagationLossModel : public PropagationLossModel {
public:
  static TypeId GetTypeId();
  JakesPropagationLossModel();
  ~JakesPropagationLossModel() override;

  JakesPropagationLossModel(const JakesPropagationLossModel &) = delete;
  JakesPropagationLossModel &
  operator=(const JakesPropagationLossModel &) = delete;

protected:
  void DoDispose() override;

private:
  friend class JakesProcess;

  double DoCalcRxPower(double txPowerDbm, Ptr<MobilityModel> a,
                       Ptr<MobilityModel> b) const override;

  int64_t DoAssignStreams(int64_t stream) override;

  Ptr<UniformRandomVariable> GetUniformRandomVariable() const;

  Ptr<UniformRandomVariable> m_uniformVariable;
  mutable PropagationCache<JakesProcess> m_propagationCache;
};

} // namespace ns3

#endif
