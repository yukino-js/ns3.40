
#include <ns3/isotropic-antenna-model.h>
#include <ns3/log.h>
#include <ns3/test.h>

#include <cmath>
#include <iostream>
#include <sstream>
#include <string>

using namespace ns3;

class IsotropicAntennaModelTestCase : public TestCase {
public:
  static std::string BuildNameString(Angles a);
  IsotropicAntennaModelTestCase(Angles a, double expectedGainDb);

private:
  void DoRun() override;

  Angles m_a;
  double m_expectedGain;
};

std::string IsotropicAntennaModelTestCase::BuildNameString(Angles a) {
  std::ostringstream oss;
  oss << "theta=" << a.GetInclination() << " , phi=" << a.GetAzimuth();
  return oss.str();
}

IsotropicAntennaModelTestCase::IsotropicAntennaModelTestCase(
    Angles a, double expectedGainDb)
    : TestCase(BuildNameString(a)), m_a(a), m_expectedGain(expectedGainDb) {}

void IsotropicAntennaModelTestCase::DoRun() {
  Ptr<IsotropicAntennaModel> a = CreateObject<IsotropicAntennaModel>();
  double actualGain = a->GetGainDb(m_a);
  NS_TEST_EXPECT_MSG_EQ_TOL(actualGain, m_expectedGain, 0.01,
                            "wrong value of the radiation pattern");
}

class IsotropicAntennaModelTestSuite : public TestSuite {
public:
  IsotropicAntennaModelTestSuite();
};

IsotropicAntennaModelTestSuite::IsotropicAntennaModelTestSuite()
    : TestSuite("isotropic-antenna-model", UNIT) {
  AddTestCase(new IsotropicAntennaModelTestCase(Angles(0, 0), 0.0),
              TestCase::QUICK);
  AddTestCase(new IsotropicAntennaModelTestCase(Angles(0, M_PI), 0.0),
              TestCase::QUICK);
  AddTestCase(new IsotropicAntennaModelTestCase(Angles(0, M_PI_2), 0.0),
              TestCase::QUICK);
  AddTestCase(new IsotropicAntennaModelTestCase(Angles(M_PI, 0), 0.0),
              TestCase::QUICK);
  AddTestCase(new IsotropicAntennaModelTestCase(Angles(M_PI, M_PI), 0.0),
              TestCase::QUICK);
  AddTestCase(new IsotropicAntennaModelTestCase(Angles(M_PI, M_PI_2), 0.0),
              TestCase::QUICK);
  AddTestCase(new IsotropicAntennaModelTestCase(Angles(M_PI_2, 0), 0.0),
              TestCase::QUICK);
  AddTestCase(new IsotropicAntennaModelTestCase(Angles(M_PI_2, M_PI), 0.0),
              TestCase::QUICK);
  AddTestCase(new IsotropicAntennaModelTestCase(Angles(M_PI_2, M_PI_2), 0.0),
              TestCase::QUICK);
};

static IsotropicAntennaModelTestSuite
    g_staticIsotropicAntennaModelTestSuiteInstance;
