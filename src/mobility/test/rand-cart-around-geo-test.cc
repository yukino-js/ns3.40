
#include <ns3/geographic-positions.h>
#include <ns3/log.h>
#include <ns3/test.h>

#include <cmath>

NS_LOG_COMPONENT_DEFINE("RandCartAroundGeoTest");

using namespace ns3;

const double TOLERANCE = 0.1;

static const double EARTH_RADIUS = 6371e3;

class RandCartAroundGeoTestCase : public TestCase {
public:
  RandCartAroundGeoTestCase(double originLatitude, double originLongitude,
                            double maxAltitude, int numPoints,
                            double maxDistFromOrigin,
                            Ptr<UniformRandomVariable> uniRand);
  ~RandCartAroundGeoTestCase() override;

private:
  void DoRun() override;
  static std::string Name(double originLatitude, double originLongitude,
                          double maxDistFromOrigin);
  double m_originLatitude;
  double m_originLongitude;
  double m_maxAltitude;
  int m_numPoints;
  double m_maxDistFromOrigin;
  Ptr<UniformRandomVariable> m_uniRand;
};

std::string RandCartAroundGeoTestCase::Name(double originLatitude,
                                            double originLongitude,
                                            double maxDistFromOrigin) {
  std::ostringstream oss;
  oss << "origin latitude = " << originLatitude << " degrees, "
      << "origin longitude = " << originLongitude << " degrees, "
      << "max distance from origin = " << maxDistFromOrigin;
  return oss.str();
}

RandCartAroundGeoTestCase::RandCartAroundGeoTestCase(
    double originLatitude, double originLongitude, double maxAltitude,
    int numPoints, double maxDistFromOrigin, Ptr<UniformRandomVariable> uniRand)
    : TestCase(Name(originLatitude, originLongitude, maxDistFromOrigin)),
      m_originLatitude(originLatitude), m_originLongitude(originLongitude),
      m_maxAltitude(maxAltitude), m_numPoints(numPoints),
      m_maxDistFromOrigin(maxDistFromOrigin), m_uniRand(uniRand) {}

RandCartAroundGeoTestCase::~RandCartAroundGeoTestCase() {}

void RandCartAroundGeoTestCase::DoRun() {
  std::list<Vector> points =
      GeographicPositions::RandCartesianPointsAroundGeographicPoint(
          m_originLatitude, m_originLongitude, m_maxAltitude, m_numPoints,
          m_maxDistFromOrigin, m_uniRand);
  Vector origin = GeographicPositions::GeographicToCartesianCoordinates(
      m_originLatitude, m_originLongitude, m_maxAltitude,
      GeographicPositions::SPHERE);
  Vector randPoint;
  while (!points.empty()) {
    randPoint = points.front();
    points.pop_front();

    double straightDistFromOrigin =
        sqrt(pow(randPoint.x - origin.x, 2) + pow(randPoint.y - origin.y, 2) +
             pow(randPoint.z - origin.z, 2));

    double arcDistFromOrigin =
        2 * EARTH_RADIUS * asin(straightDistFromOrigin / (2 * EARTH_RADIUS));

    NS_TEST_ASSERT_MSG_LT(arcDistFromOrigin, m_maxDistFromOrigin + TOLERANCE,
                          "random point ("
                              << randPoint.x << ", " << randPoint.y << ", "
                              << randPoint.z
                              << ") is outside of max radius from origin");
  }
}

class RandCartAroundGeoTestSuite : public TestSuite {
public:
  RandCartAroundGeoTestSuite();
};

RandCartAroundGeoTestSuite::RandCartAroundGeoTestSuite()
    : TestSuite("rand-cart-around-geo", UNIT) {
  NS_LOG_INFO("creating RandCartAroundGeoTestSuite");
  Ptr<UniformRandomVariable> uniRand = CreateObject<UniformRandomVariable>();
  uniRand->SetStream(5);
  for (double originLatitude = -89.9; originLatitude <= 89.9;
       originLatitude += 35.96) {
    for (double originLongitude = 0; originLongitude <= 360;
         originLongitude += 72) {
      for (double maxDistFromOrigin = 1000; maxDistFromOrigin <= 1000000;
           maxDistFromOrigin *= 10) {
        AddTestCase(new RandCartAroundGeoTestCase(originLatitude,
                                                  originLongitude, 0, 50,
                                                  maxDistFromOrigin, uniRand),
                    TestCase::QUICK);
      }
    }
  }
}

static RandCartAroundGeoTestSuite g_RandCartAroundGeoTestSuite;
