#ifndef NS3_VECTOR_H
#define NS3_VECTOR_H

#include "attribute-helper.h"
#include "attribute.h"

namespace ns3 {

class Vector3D {
public:
  Vector3D(double _x, double _y, double _z);
  Vector3D();

  double x;
  double y;
  double z;

  double GetLength() const;

  double GetLengthSquared() const;

  friend double CalculateDistance(const Vector3D &a, const Vector3D &b);

  friend double CalculateDistanceSquared(const Vector3D &a, const Vector3D &b);

  friend std::ostream &operator<<(std::ostream &os, const Vector3D &vector);

  friend std::istream &operator>>(std::istream &is, Vector3D &vector);

  friend bool operator<(const Vector3D &a, const Vector3D &b);

  friend bool operator<=(const Vector3D &a, const Vector3D &b);

  friend bool operator>(const Vector3D &a, const Vector3D &b);

  friend bool operator>=(const Vector3D &a, const Vector3D &b);

  friend bool operator==(const Vector3D &a, const Vector3D &b);

  friend bool operator!=(const Vector3D &a, const Vector3D &b);

  friend Vector3D operator+(const Vector3D &a, const Vector3D &b);

  friend Vector3D operator-(const Vector3D &a, const Vector3D &b);
};

class Vector2D {
public:
  Vector2D(double _x, double _y);
  Vector2D();
  double x;
  double y;

  double GetLength() const;

  double GetLengthSquared() const;

  friend double CalculateDistance(const Vector2D &a, const Vector2D &b);

  friend double CalculateDistanceSquared(const Vector2D &a, const Vector2D &b);

  friend std::ostream &operator<<(std::ostream &os, const Vector2D &vector);

  friend std::istream &operator>>(std::istream &is, Vector2D &vector);

  friend bool operator<(const Vector2D &a, const Vector2D &b);

  friend bool operator<=(const Vector2D &a, const Vector2D &b);

  friend bool operator>(const Vector2D &a, const Vector2D &b);

  friend bool operator>=(const Vector2D &a, const Vector2D &b);

  friend bool operator==(const Vector2D &a, const Vector2D &b);

  friend bool operator!=(const Vector2D &a, const Vector2D &b);

  friend Vector2D operator+(const Vector2D &a, const Vector2D &b);

  friend Vector2D operator-(const Vector2D &a, const Vector2D &b);
};

double CalculateDistance(const Vector3D &a, const Vector3D &b);
double CalculateDistance(const Vector2D &a, const Vector2D &b);
double CalculateDistanceSquared(const Vector3D &a, const Vector3D &b);
double CalculateDistanceSquared(const Vector2D &a, const Vector2D &b);
std::ostream &operator<<(std::ostream &os, const Vector3D &vector);
std::ostream &operator<<(std::ostream &os, const Vector2D &vector);
std::istream &operator>>(std::istream &is, Vector3D &vector);
std::istream &operator>>(std::istream &is, Vector2D &vector);
bool operator<(const Vector3D &a, const Vector3D &b);
bool operator<(const Vector2D &a, const Vector2D &b);

ATTRIBUTE_HELPER_HEADER(Vector3D);
ATTRIBUTE_HELPER_HEADER(Vector2D);

typedef Vector3D Vector;

typedef Vector3DValue VectorValue;

typedef Vector3DChecker VectorChecker;

ATTRIBUTE_ACCESSOR_DEFINE(Vector);

Ptr<const AttributeChecker> MakeVectorChecker();

} // namespace ns3

#endif
