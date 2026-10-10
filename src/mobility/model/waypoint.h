#ifndef WAYPOINT_H
#define WAYPOINT_H

#include "ns3/attribute-helper.h"
#include "ns3/attribute.h"
#include "ns3/nstime.h"
#include "ns3/vector.h"

namespace ns3 {

class Waypoint {
public:
  Waypoint(const Time &waypointTime, const Vector &waypointPosition);

  Waypoint();
  Time time;
  Vector position;
};

ATTRIBUTE_HELPER_HEADER(Waypoint);

std::ostream &operator<<(std::ostream &os, const Waypoint &waypoint);
std::istream &operator>>(std::istream &is, Waypoint &waypoint);

} // namespace ns3

#endif
