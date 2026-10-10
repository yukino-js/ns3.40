#ifndef RANDOM_WALK_2D_MOBILITY_MODEL_H
#define RANDOM_WALK_2D_MOBILITY_MODEL_H

#include "constant-velocity-helper.h"
#include "mobility-model.h"
#include "rectangle.h"

#include "ns3/event-id.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/random-variable-stream.h"

namespace ns3 {

class RandomWalk2dMobilityModel : public MobilityModel {
public:
  static TypeId GetTypeId();

  enum Mode { MODE_DISTANCE, MODE_TIME };

private:
  void Rebound(Time timeLeft);
  void DoWalk(Time timeLeft);
  void DoInitializePrivate();
  void DoDispose() override;
  void DoInitialize() override;
  Vector DoGetPosition() const override;
  void DoSetPosition(const Vector &position) override;
  Vector DoGetVelocity() const override;
  int64_t DoAssignStreams(int64_t) override;

  ConstantVelocityHelper m_helper;
  EventId m_event;
  Mode m_mode;
  double m_modeDistance;
  Time m_modeTime;
  Ptr<RandomVariableStream> m_speed;
  Ptr<RandomVariableStream> m_direction;
  Rectangle m_bounds;
};

} // namespace ns3

#endif
