
#include <ns3/antenna-model.h>
#include <ns3/log.h>
#include <ns3/test.h>

#include <cmath>
#include <iostream>
#include <sstream>
#include <string>

using namespace ns3;

class DegreesToRadiansTestCase : public TestCase {
public:
  static std::string BuildNameString(double a);
  DegreesToRadiansTestCase(double a, double b);

private:
  void DoRun() override;

  double m_a;
  double m_b;
};

std::string DegreesToRadiansTestCase::BuildNameString(double a) {
  std::ostringstream oss;
  oss << "angle = " << a << " degrees";
  return oss.str();
}

DegreesToRadiansTestCase::DegreesToRadiansTestCase(double a, double b)
    : TestCase(BuildNameString(a)), m_a(a), m_b(b) {}

void DegreesToRadiansTestCase::DoRun() {
  NS_TEST_EXPECT_MSG_EQ_TOL(DegreesToRadians(m_a), m_b, 1e-10,
                            "wrong conversion");
}

class RadiansToDegreesTestCase : public TestCase {
public:
  static std::string BuildNameString(double a);
  RadiansToDegreesTestCase(double a, double b);

private:
  void DoRun() override;

  double m_a;
  double m_b;
};

std::string RadiansToDegreesTestCase::BuildNameString(double a) {
  std::ostringstream oss;
  oss << "angle = " << a << " degrees";
  return oss.str();
}

RadiansToDegreesTestCase::RadiansToDegreesTestCase(double a, double b)
    : TestCase(BuildNameString(a)), m_a(a), m_b(b) {}

void RadiansToDegreesTestCase::DoRun() {
  NS_TEST_EXPECT_MSG_EQ_TOL(RadiansToDegrees(m_a), m_b, 1e-10,
                            "wrong conversion");
}

class DegreesRadiansTestSuite : public TestSuite {
public:
  DegreesRadiansTestSuite();
};

DegreesRadiansTestSuite::DegreesRadiansTestSuite()
    : TestSuite("degrees-radians", UNIT) {
  AddTestCase(new DegreesToRadiansTestCase(0, 0), TestCase::QUICK);
  AddTestCase(new DegreesToRadiansTestCase(90, M_PI_2), TestCase::QUICK);
  AddTestCase(new DegreesToRadiansTestCase(180, M_PI), TestCase::QUICK);
  AddTestCase(new DegreesToRadiansTestCase(270, M_PI + M_PI_2),
              TestCase::QUICK);
  AddTestCase(new DegreesToRadiansTestCase(360, M_PI + M_PI), TestCase::QUICK);
  AddTestCase(new DegreesToRadiansTestCase(-90, -M_PI_2), TestCase::QUICK);
  AddTestCase(new DegreesToRadiansTestCase(810, 4.5 * M_PI), TestCase::QUICK);

  AddTestCase(new RadiansToDegreesTestCase(0, 0), TestCase::QUICK);
  AddTestCase(new RadiansToDegreesTestCase(M_PI_2, 90), TestCase::QUICK);
  AddTestCase(new RadiansToDegreesTestCase(M_PI, 180), TestCase::QUICK);
  AddTestCase(new RadiansToDegreesTestCase(M_PI + M_PI_2, 270),
              TestCase::QUICK);
  AddTestCase(new RadiansToDegreesTestCase(M_PI + M_PI, 360), TestCase::QUICK);
  AddTestCase(new RadiansToDegreesTestCase(-M_PI_2, -90), TestCase::QUICK);
  AddTestCase(new RadiansToDegreesTestCase(4.5 * M_PI, 810), TestCase::QUICK);
};

static DegreesRadiansTestSuite g_staticDegreesRadiansTestSuiteInstance;
