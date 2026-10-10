#ifndef WAYPOINT_MOBILITY_MODEL_H
#define WAYPOINT_MOBILITY_MODEL_H

#include "mobility-model.h"
#include "waypoint.h"

#include "ns3/vector.h"

#include <deque>
#include <stdint.h>

class WaypointMobilityModelNotifyTest;

namespace ns3 {

class WaypointMobilityModel : public MobilityModel {
public:
  static TypeId GetTypeId();

  WaypointMobilityModel();
  ~WaypointMobilityModel() override;

  void AddWaypoint(const Waypoint &waypoint);

  Waypoint GetNextWaypoint() const;

  uint32_t WaypointsLeft() const;

  void EndMobility();

private:
  friend class ::WaypointMobilityModelNotifyTest;

  virtual void Update() const;
  void DoDispose() override;
  Vector DoGetPosition() const override;
  void DoSetPosition(const Vector &position) override;
  Vector DoGetVelocity() const override;

protected:
  bool m_first;
  bool m_lazyNotify;
  bool m_initialPositionIsWaypoint;
  mutable std::deque<Waypoint> m_waypoints;
  mutable Waypoint m_current;
  mutable Waypoint m_next;
  mutable Vector m_velocity;
};

} // namespace ns3

#endif
