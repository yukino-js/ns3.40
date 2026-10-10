
#ifndef RANDOM_WALK_2D_OUTDOOR_MOBILITY_MODEL_H
#define RANDOM_WALK_2D_OUTDOOR_MOBILITY_MODEL_H

#include "building.h"

#include "ns3/constant-velocity-helper.h"
#include "ns3/event-id.h"
#include "ns3/mobility-model.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/random-variable-stream.h"
#include "ns3/rectangle.h"

namespace ns3 {

class RandomWalk2dOutdoorMobilityModel : public MobilityModel {
public:
  static TypeId GetTypeId();

  enum Mode { MODE_DISTANCE, MODE_TIME };

private:
  void Rebound(Time timeLeft);
  void AvoidBuilding(Time delayLeft, Vector intersectPosition);
  void DoWalk(Time delayLeft);
  void DoInitializePrivate();
  std::pair<bool, Ptr<Building>>
  IsLineClearOfBuildings(Vector currentPosition, Vector nextPosition) const;
  Vector CalculateIntersectionFromOutside(const Vector &current,
                                          const Vector &next,
                                          const Box boundaries) const;

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
  double m_epsilon;
  uint32_t m_maxIter;
  Vector m_prevPosition;
};

} // namespace ns3

#endif
