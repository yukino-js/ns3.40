#include "waypoint.h"

namespace ns3 {

ATTRIBUTE_HELPER_CPP(Waypoint);

Waypoint::Waypoint(const Time &waypointTime, const Vector &waypointPosition)
    : time(waypointTime), position(waypointPosition) {}

Waypoint::Waypoint() : time(Seconds(0.0)), position(0, 0, 0) {}

std::ostream &operator<<(std::ostream &os, const Waypoint &waypoint) {
  os << waypoint.time.GetSeconds() << "$" << waypoint.position;
  return os;
}

std::istream &operator>>(std::istream &is, Waypoint &waypoint) {
  char separator;
  is >> waypoint.time >> separator >> waypoint.position;
  if (separator != '$') {
    is.setstate(std::ios_base::failbit);
  }
  return is;
}

} // namespace ns3
