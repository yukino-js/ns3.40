#ifndef GAUSS_MARKOV_MOBILITY_MODEL_H
#define GAUSS_MARKOV_MOBILITY_MODEL_H

#include "box.h"
#include "constant-velocity-helper.h"
#include "mobility-model.h"
#include "position-allocator.h"

#include "ns3/event-id.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/random-variable-stream.h"

namespace ns3 {

class GaussMarkovMobilityModel : public MobilityModel {
public:
  static TypeId GetTypeId();
  GaussMarkovMobilityModel();

private:
  void Start();
  void DoWalk(Time timeLeft);
  void DoDispose() override;
  Vector DoGetPosition() const override;
  void DoSetPosition(const Vector &position) override;
  Vector DoGetVelocity() const override;
  int64_t DoAssignStreams(int64_t) override;
  ConstantVelocityHelper m_helper;
  Time m_timeStep;
  double m_alpha;
  double m_meanVelocity;
  double m_meanDirection;
  double m_meanPitch;
  double m_Velocity;
  double m_Direction;
  double m_Pitch;
  Ptr<RandomVariableStream> m_rndMeanVelocity;
  Ptr<NormalRandomVariable> m_normalVelocity;
  Ptr<RandomVariableStream> m_rndMeanDirection;
  Ptr<NormalRandomVariable> m_normalDirection;
  Ptr<RandomVariableStream> m_rndMeanPitch;
  Ptr<NormalRandomVariable> m_normalPitch;
  EventId m_event;
  Box m_bounds;
};

} // namespace ns3

#endif
