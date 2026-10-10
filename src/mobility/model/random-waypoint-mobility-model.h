#ifndef RANDOM_WAYPOINT_MOBILITY_MODEL_H
#define RANDOM_WAYPOINT_MOBILITY_MODEL_H

#include "constant-velocity-helper.h"
#include "mobility-model.h"
#include "position-allocator.h"

#include "ns3/ptr.h"
#include "ns3/random-variable-stream.h"

namespace ns3 {

class RandomWaypointMobilityModel : public MobilityModel {
public:
  static TypeId GetTypeId();

protected:
  void DoInitialize() override;

private:
  void BeginWalk();
  void DoInitializePrivate();
  Vector DoGetPosition() const override;
  void DoSetPosition(const Vector &position) override;
  Vector DoGetVelocity() const override;
  int64_t DoAssignStreams(int64_t) override;

  ConstantVelocityHelper m_helper;
  Ptr<PositionAllocator> m_position;
  Ptr<RandomVariableStream> m_speed;
  Ptr<RandomVariableStream> m_pause;
  EventId m_event;
};

} // namespace ns3

#endif
