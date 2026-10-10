#include "rectangle.h"

#include "ns3/assert.h"
#include "ns3/fatal-error.h"
#include "ns3/vector.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <sstream>

namespace ns3 {

Rectangle::Rectangle(double _xMin, double _xMax, double _yMin, double _yMax)
    : xMin(_xMin), xMax(_xMax), yMin(_yMin), yMax(_yMax) {}

Rectangle::Rectangle() : xMin(0.0), xMax(0.0), yMin(0.0), yMax(0.0) {}

bool Rectangle::IsInside(const Vector &position) const {
  return position.x <= this->xMax && position.x >= this->xMin &&
         position.y <= this->yMax && position.y >= this->yMin;
}

bool Rectangle::IsOnTheBorder(const Vector &position) const {
  return position.x == this->xMax || position.x == this->xMin ||
         position.y == this->yMax || position.y == this->yMin;
}

Rectangle::Side
Rectangle::GetClosestSideOrCorner(const Vector &position) const {
  std::array<double, 4> distanceFromBorders{
      std::abs(position.x - this->xMin),
      std::abs(this->xMax - position.x),
      std::abs(position.y - this->yMin),
      std::abs(this->yMax - position.y),
  };
  uint8_t flags = 0;
  double minDist = std::numeric_limits<double>::max();
  for (int i = 0; i < 4; i++) {
    if (distanceFromBorders[i] > minDist) {
      continue;
    }
    if (distanceFromBorders[i] < minDist) {
      minDist = distanceFromBorders[i];
      flags = 0;
    }
    flags |= (0b1000 >> i);
  }
  NS_ASSERT(minDist != std::numeric_limits<double>::max());
  Rectangle::Side side;
  switch (flags) {
  case 0b1111:
    side = TOPSIDE;
    break;
  case 0b0011:
    side = TOPSIDE;
    if (!IsInside(position)) {
      side = (distanceFromBorders[0] > distanceFromBorders[1]) ? RIGHTSIDE
                                                               : LEFTSIDE;
    }
    break;
  case 0b1100:
    side = RIGHTSIDE;
    if (!IsInside(position)) {
      side = (distanceFromBorders[2] > distanceFromBorders[3]) ? TOPSIDE
                                                               : BOTTOMSIDE;
    }
    break;
  case 0b0001:
  case 0b1101:
    side = TOPSIDE;
    break;
  case 0b0010:
  case 0b1110:
    side = BOTTOMSIDE;
    break;
  case 0b0100:
  case 0b0111:
    side = RIGHTSIDE;
    break;
  case 0b0101:
    side = TOPRIGHTCORNER;
    break;
  case 0b0110:
    side = BOTTOMRIGHTCORNER;
    break;
  case 0b1000:
  case 0b1011:
    side = LEFTSIDE;
    break;
  case 0b1001:
    side = TOPLEFTCORNER;
    break;
  case 0b1010:
    side = BOTTOMLEFTCORNER;
    break;
  default:
    NS_FATAL_ERROR("Impossible case");
    break;
  }
  return side;
}

Vector Rectangle::CalculateIntersection(const Vector &current,
                                        const Vector &speed) const {
  NS_ASSERT(IsInside(current));
  double xMaxY = current.y + (this->xMax - current.x) / speed.x * speed.y;
  double xMinY = current.y + (this->xMin - current.x) / speed.x * speed.y;
  double yMaxX = current.x + (this->yMax - current.y) / speed.y * speed.x;
  double yMinX = current.x + (this->yMin - current.y) / speed.y * speed.x;
  bool xMaxYOk = (xMaxY <= this->yMax && xMaxY >= this->yMin);
  bool xMinYOk = (xMinY <= this->yMax && xMinY >= this->yMin);
  bool yMaxXOk = (yMaxX <= this->xMax && yMaxX >= this->xMin);
  bool yMinXOk = (yMinX <= this->xMax && yMinX >= this->xMin);
  if (xMaxYOk && speed.x >= 0) {
    return Vector(this->xMax, xMaxY, 0.0);
  } else if (xMinYOk && speed.x <= 0) {
    return Vector(this->xMin, xMinY, 0.0);
  } else if (yMaxXOk && speed.y >= 0) {
    return Vector(yMaxX, this->yMax, 0.0);
  } else if (yMinXOk && speed.y <= 0) {
    return Vector(yMinX, this->yMin, 0.0);
  } else {
    NS_ASSERT(false);
    return Vector(0.0, 0.0, 0.0);
  }
}

ATTRIBUTE_HELPER_CPP(Rectangle);

std::ostream &operator<<(std::ostream &os, const Rectangle &rectangle) {
  os << rectangle.xMin << "|" << rectangle.xMax << "|" << rectangle.yMin << "|"
     << rectangle.yMax;
  return os;
}

std::istream &operator>>(std::istream &is, Rectangle &rectangle) {
  char c1;
  char c2;
  char c3;
  is >> rectangle.xMin >> c1 >> rectangle.xMax >> c2 >> rectangle.yMin >> c3 >>
      rectangle.yMax;
  if (c1 != '|' || c2 != '|' || c3 != '|') {
    is.setstate(std::ios_base::failbit);
  }
  return is;
}

std::ostream &operator<<(std::ostream &os, const Rectangle::Side &side) {
  switch (side) {
  case Rectangle::RIGHTSIDE:
    os << "RIGHTSIDE";
    break;
  case Rectangle::LEFTSIDE:
    os << "LEFTSIDE";
    break;
  case Rectangle::TOPSIDE:
    os << "TOPSIDE";
    break;
  case Rectangle::BOTTOMSIDE:
    os << "BOTTOMSIDE";
    break;
  case Rectangle::TOPRIGHTCORNER:
    os << "TOPRIGHTCORNER";
    break;
  case Rectangle::TOPLEFTCORNER:
    os << "TOPLEFTCORNER";
    break;
  case Rectangle::BOTTOMRIGHTCORNER:
    os << "BOTTOMRIGHTCORNER";
    break;
  case Rectangle::BOTTOMLEFTCORNER:
    os << "BOTTOMLEFTCORNER";
    break;
  }
  return os;
}

} // namespace ns3
