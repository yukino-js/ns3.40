
#ifndef ANGLES_H
#define ANGLES_H

#include <ns3/vector.h>

#include <vector>

namespace ns3 {

double DegreesToRadians(double degrees);

std::vector<double> DegreesToRadians(const std::vector<double> &degrees);

double RadiansToDegrees(double radians);

std::vector<double> RadiansToDegrees(const std::vector<double> &radians);

double WrapTo360(double a);

double WrapTo180(double a);

double WrapTo2Pi(double a);

double WrapToPi(double a);

class Angles {
public:
  Angles(double azimuth, double inclination);

  Angles(Vector v);

  Angles(Vector v, Vector o);

  void SetAzimuth(double azimuth);

  void SetInclination(double inclination);

  double GetAzimuth() const;

  double GetInclination() const;

  friend std::ostream &operator<<(std::ostream &os, const Angles &a);
  friend std::istream &operator>>(std::istream &is, Angles &a);

  static bool m_printDeg;

private:
  Angles();

  void NormalizeAngles();

  void CheckIfValid() const;

  double m_azimuth;
  double m_inclination;
};

} // namespace ns3

#endif
