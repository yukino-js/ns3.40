#ifndef BOX_H
#define BOX_H

#include "ns3/attribute-helper.h"
#include "ns3/attribute.h"
#include "ns3/vector.h"

namespace ns3 {

class Box {
public:
  enum Side { RIGHT, LEFT, TOP, BOTTOM, UP, DOWN };

  Box(double _xMin, double _xMax, double _yMin, double _yMax, double _zMin,
      double _zMax);
  Box();
  bool IsInside(const Vector &position) const;
  Side GetClosestSide(const Vector &position) const;
  Vector CalculateIntersection(const Vector &current,
                               const Vector &speed) const;
  bool IsIntersect(const Vector &l1, const Vector &l2) const;

  double xMin;
  double xMax;
  double yMin;
  double yMax;
  double zMin;
  double zMax;
};

std::ostream &operator<<(std::ostream &os, const Box &box);
std::istream &operator>>(std::istream &is, Box &box);

ATTRIBUTE_HELPER_HEADER(Box);

} // namespace ns3

#endif
