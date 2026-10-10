#ifndef RANDOM_DIRECTION_MOBILITY_MODEL_H
#define RANDOM_DIRECTION_MOBILITY_MODEL_H

#include "constant-velocity-helper.h"
#include "mobility-model.h"
#include "rectangle.h"

#include "ns3/event-id.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/random-variable-stream.h"

namespace ns3 {

class RandomDirection2dMobilityModel : public MobilityModel {
public:
  static TypeId GetTypeId();
  RandomDirection2dMobilityModel();

private:
  void ResetDirectionAndSpeed();
  void BeginPause();
  void SetDirectionAndSpeed(double direction);
  void DoInitializePrivate();
  void DoDispose() override;
  void DoInitialize() override;
  Vector DoGetPosition() const override;
  void DoSetPosition(const Vector &position) override;
  Vector DoGetVelocity() const override;
  int64_t DoAssignStreams(int64_t) override;

  Ptr<UniformRandomVariable> m_direction;
  Rectangle m_bounds;
  Ptr<RandomVariableStream> m_speed;
  Ptr<RandomVariableStream> m_pause;
  EventId m_event;
  ConstantVelocityHelper m_helper;
};

} // namespace ns3

#endif
