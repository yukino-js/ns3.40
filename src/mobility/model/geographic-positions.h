
#include <ns3/random-variable-stream.h>
#include <ns3/vector.h>

#ifndef GEOGRAPHIC_POSITIONS_H
#define GEOGRAPHIC_POSITIONS_H

namespace ns3 {

class GeographicPositions {
public:
  enum EarthSpheroidType { SPHERE, GRS80, WGS84 };

  static Vector GeographicToCartesianCoordinates(double latitude,
                                                 double longitude,
                                                 double altitude,
                                                 EarthSpheroidType sphType);

  static Vector CartesianToGeographicCoordinates(Vector pos,
                                                 EarthSpheroidType sphType);

  static std::list<Vector> RandCartesianPointsAroundGeographicPoint(
      double originLatitude, double originLongitude, double maxAltitude,
      int numPoints, double maxDistFromOrigin,
      Ptr<UniformRandomVariable> uniRand);
};

} // namespace ns3

#endif
