#ifndef STEADY_STATE_RANDOM_WAYPOINT_MOBILITY_MODEL_H
#define STEADY_STATE_RANDOM_WAYPOINT_MOBILITY_MODEL_H

#include "constant-velocity-helper.h"
#include "mobility-model.h"
#include "position-allocator.h"

#include "ns3/ptr.h"
#include "ns3/random-variable-stream.h"

namespace ns3 {

class SteadyStateRandomWaypointMobilityModel : public MobilityModel {
public:
  static TypeId GetTypeId();
  SteadyStateRandomWaypointMobilityModel();

protected:
  void DoInitialize() override;

private:
  void DoInitializePrivate();
  void SteadyStateBeginWalk(const Vector &destination);
  void Start();
  void BeginWalk();
  Vector DoGetPosition() const override;
  void DoSetPosition(const Vector &position) override;
  Vector DoGetVelocity() const override;
  int64_t DoAssignStreams(int64_t) override;

  ConstantVelocityHelper m_helper;
  double m_maxSpeed;
  double m_minSpeed;
  Ptr<UniformRandomVariable> m_speed;
  double m_minX;
  double m_maxX;
  double m_minY;
  double m_maxY;
  double m_z;
  Ptr<RandomBoxPositionAllocator> m_position;
  double m_minPause;
  double m_maxPause;
  Ptr<UniformRandomVariable> m_pause;
  EventId m_event;
  bool alreadyStarted;
  Ptr<UniformRandomVariable> m_x1_r;
  Ptr<UniformRandomVariable> m_y1_r;
  Ptr<UniformRandomVariable> m_x2_r;
  Ptr<UniformRandomVariable> m_y2_r;
  Ptr<UniformRandomVariable> m_u_r;
  Ptr<UniformRandomVariable> m_x;
  Ptr<UniformRandomVariable> m_y;
};

} // namespace ns3

#endif
