#ifndef RECTANGLE_H
#define RECTANGLE_H

#include "ns3/attribute-helper.h"
#include "ns3/attribute.h"
#include "ns3/vector.h"

namespace ns3 {

class Rectangle {
public:
  enum Side {
    RIGHTSIDE = 0,
    LEFTSIDE,
    TOPSIDE,
    BOTTOMSIDE,
    TOPRIGHTCORNER,
    TOPLEFTCORNER,
    BOTTOMRIGHTCORNER,
    BOTTOMLEFTCORNER
  };

  Rectangle(double _xMin, double _xMax, double _yMin, double _yMax);
  Rectangle();
  bool IsInside(const Vector &position) const;
  bool IsOnTheBorder(const Vector &position) const;
  Side GetClosestSideOrCorner(const Vector &position) const;
  Vector CalculateIntersection(const Vector &current,
                               const Vector &speed) const;

  double xMin;
  double xMax;
  double yMin;
  double yMax;
};

std::ostream &operator<<(std::ostream &os, const Rectangle &rectangle);
std::istream &operator>>(std::istream &is, Rectangle &rectangle);
std::ostream &operator<<(std::ostream &os, const Rectangle::Side &side);

ATTRIBUTE_HELPER_HEADER(Rectangle);

} // namespace ns3

#endif
