
#include "geographic-positions.h"

#include <ns3/log.h>

#include <cmath>

NS_LOG_COMPONENT_DEFINE("GeographicPositions");

namespace ns3 {

static constexpr double EARTH_RADIUS = 6371e3;

static constexpr double EARTH_SEMIMAJOR_AXIS = 6378137;

static constexpr double EARTH_GRS80_ECCENTRICITY = 0.0818191910428158;

static constexpr double EARTH_WGS84_ECCENTRICITY = 0.0818191908426215;

static constexpr double DEG2RAD = M_PI / 180.0;

static constexpr double RAD2DEG = 180.0 * M_1_PI;

Vector GeographicPositions::GeographicToCartesianCoordinates(
    double latitude, double longitude, double altitude,
    EarthSpheroidType sphType) {
  NS_LOG_FUNCTION_NOARGS();
  double latitudeRadians = DEG2RAD * latitude;
  double longitudeRadians = DEG2RAD * longitude;
  double a;
  double e;
  if (sphType == SPHERE) {
    a = EARTH_RADIUS;
    e = 0;
  } else if (sphType == GRS80) {
    a = EARTH_SEMIMAJOR_AXIS;
    e = EARTH_GRS80_ECCENTRICITY;
  } else {
    a = EARTH_SEMIMAJOR_AXIS;
    e = EARTH_WGS84_ECCENTRICITY;
  }

  double Rn = a / (sqrt(1 - pow(e, 2) * pow(sin(latitudeRadians), 2)));
  double x = (Rn + altitude) * cos(latitudeRadians) * cos(longitudeRadians);
  double y = (Rn + altitude) * cos(latitudeRadians) * sin(longitudeRadians);
  double z = ((1 - pow(e, 2)) * Rn + altitude) * sin(latitudeRadians);
  Vector cartesianCoordinates = Vector(x, y, z);
  return cartesianCoordinates;
}

Vector GeographicPositions::CartesianToGeographicCoordinates(
    Vector pos, EarthSpheroidType sphType) {
  NS_LOG_FUNCTION(pos << sphType);

  double a;
  double e;
  if (sphType == SPHERE) {
    a = EARTH_RADIUS;
    e = 0;
  } else if (sphType == GRS80) {
    a = EARTH_SEMIMAJOR_AXIS;
    e = EARTH_GRS80_ECCENTRICITY;
  } else {
    a = EARTH_SEMIMAJOR_AXIS;
    e = EARTH_WGS84_ECCENTRICITY;
  }

  Vector lla;
  Vector tmp;
  lla.y = atan2(pos.y, pos.x);

  double e2 = e * e;
  double p = CalculateDistance(pos, {0, 0, pos.z});
  lla.x = atan2(pos.z, p * (1 - e2));

  do {
    tmp = lla;
    double N = a / sqrt(1 - e2 * sin(tmp.x) * sin(tmp.x));
    double v = p / cos(tmp.x);
    lla.z = v - N;
    lla.x = atan2(pos.z, p * (1 - e2 * N / v));
  } while (fabs(lla.x - tmp.x) > 0.00000926 * DEG2RAD);

  lla.x *= RAD2DEG;
  lla.y *= RAD2DEG;

  if (lla.x > 90.0) {
    lla.x = 180 - lla.x;
    lla.y += lla.y < 0 ? 180 : -180;
  } else if (lla.x < -90.0) {
    lla.x = -180 - lla.x;
    lla.y += lla.y < 0 ? 180 : -180;
  }
  if (lla.y == 180.0) {
    lla.y = -180;
  }

  NS_ASSERT_MSG(-180.0 <= lla.y, "Conversion error: longitude too negative");
  NS_ASSERT_MSG(180.0 > lla.y, "Conversion error: longitude too positive");
  NS_ASSERT_MSG(-90.0 <= lla.x, "Conversion error: latitude too negative");
  NS_ASSERT_MSG(90.0 >= lla.x, "Conversion error: latitude too positive");

  return lla;
}

std::list<Vector> GeographicPositions::RandCartesianPointsAroundGeographicPoint(
    double originLatitude, double originLongitude, double maxAltitude,
    int numPoints, double maxDistFromOrigin,
    Ptr<UniformRandomVariable> uniRand) {
  NS_LOG_FUNCTION_NOARGS();
  if (originLatitude >= 90) {
    NS_LOG_WARN("origin latitude must be less than 90. setting to 89.999");
    originLatitude = 89.999;
  } else if (originLatitude <= -90) {
    NS_LOG_WARN("origin latitude must be greater than -90. setting to -89.999");
    originLatitude = -89.999;
  }

  if (maxAltitude < 0) {
    NS_LOG_WARN(
        "maximum altitude must be greater than or equal to 0. setting to 0");
    maxAltitude = 0;
  }

  double originLatitudeRadians = originLatitude * DEG2RAD;
  double originLongitudeRadians = originLongitude * DEG2RAD;
  double originColatitude = (M_PI_2)-originLatitudeRadians;

  double a = maxDistFromOrigin / EARTH_RADIUS;
  if (a > M_PI) {
    a = M_PI;
  }

  std::list<Vector> generatedPoints;
  for (int i = 0; i < numPoints; i++) {
    double d = uniRand->GetValue(0, EARTH_RADIUS - EARTH_RADIUS * cos(a));
    double phi = uniRand->GetValue(0, M_PI * 2);
    double alpha = acos((EARTH_RADIUS - d) / EARTH_RADIUS);

    double theta = M_PI_2 - alpha;
    double randPointLatitude =
        asin(sin(theta) * cos(originColatitude) +
             cos(theta) * sin(originColatitude) * sin(phi));
    double intermedLong =
        asin((sin(randPointLatitude) * cos(originColatitude) - sin(theta)) /
             (cos(randPointLatitude) * sin(originColatitude)));
    intermedLong = intermedLong + M_PI_2;

    if (phi > (M_PI_2) && phi <= (3 * M_PI_2)) {
      intermedLong = -intermedLong;
    }

    double randPointLongitude = intermedLong + originLongitudeRadians;

    double randAltitude = uniRand->GetValue(0, maxAltitude);

    Vector pointPosition =
        GeographicPositions::GeographicToCartesianCoordinates(
            randPointLatitude * RAD2DEG, randPointLongitude * RAD2DEG,
            randAltitude, SPHERE);

    generatedPoints.push_back(pointPosition);
  }
  return generatedPoints;
}

} // namespace ns3
