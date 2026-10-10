
#include <ns3/double.h>
#include <ns3/log.h>
#include <ns3/parabolic-antenna-model.h>
#include <ns3/simulator.h>
#include <ns3/test.h>

#include <cmath>
#include <iostream>
#include <sstream>
#include <string>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TestParabolicAntennaModel");

enum ParabolicAntennaModelGainTestCondition { EQUAL = 0, LESSTHAN = 1 };

class ParabolicAntennaModelTestCase : public TestCase {
public:
  static std::string BuildNameString(Angles a, double b, double o, double g);
  ParabolicAntennaModelTestCase(Angles a, double b, double o, double g,
                                double expectedGainDb,
                                ParabolicAntennaModelGainTestCondition cond);

private:
  void DoRun() override;

  Angles m_a;
  double m_b;
  double m_o;
  double m_g;
  double m_expectedGain;
  ParabolicAntennaModelGainTestCondition m_cond;
};

std::string ParabolicAntennaModelTestCase::BuildNameString(Angles a, double b,
                                                           double o, double g) {
  std::ostringstream oss;
  oss << "theta=" << a.GetInclination() << " , phi=" << a.GetAzimuth()
      << ", beamdwidth=" << b << "deg"
      << ", orientation=" << o << ", maxAttenuation=" << g << " dB";
  return oss.str();
}

ParabolicAntennaModelTestCase::ParabolicAntennaModelTestCase(
    Angles a, double b, double o, double g, double expectedGainDb,
    ParabolicAntennaModelGainTestCondition cond)
    : TestCase(BuildNameString(a, b, o, g)), m_a(a), m_b(b), m_o(o), m_g(g),
      m_expectedGain(expectedGainDb), m_cond(cond) {}

void ParabolicAntennaModelTestCase::DoRun() {
  NS_LOG_FUNCTION(this << BuildNameString(m_a, m_b, m_o, m_g));

  Ptr<ParabolicAntennaModel> a = CreateObject<ParabolicAntennaModel>();
  a->SetAttribute("Beamwidth", DoubleValue(m_b));
  a->SetAttribute("Orientation", DoubleValue(m_o));
  a->SetAttribute("MaxAttenuation", DoubleValue(m_g));
  double actualGain = a->GetGainDb(m_a);
  switch (m_cond) {
  case EQUAL:
    NS_TEST_EXPECT_MSG_EQ_TOL(actualGain, m_expectedGain, 0.001,
                              "wrong value of the radiation pattern");
    break;
  case LESSTHAN:
    NS_TEST_EXPECT_MSG_LT(actualGain, m_expectedGain,
                          "gain higher than expected");
    break;
  default:
    break;
  }
}

class ParabolicAntennaModelTestSuite : public TestSuite {
public:
  ParabolicAntennaModelTestSuite();
};

ParabolicAntennaModelTestSuite::ParabolicAntennaModelTestSuite()
    : TestSuite("parabolic-antenna-model", UNIT) {
  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(0), DegreesToRadians(90)), 60, 0, 20,
                  0, EQUAL),
              TestCase::QUICK);
  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(30), DegreesToRadians(90)), 60, 0, 20,
                  -3, EQUAL),
              TestCase::QUICK);
  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(-30), DegreesToRadians(90)), 60, 0,
                  20, -3, EQUAL),
              TestCase::QUICK);
  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(-90), DegreesToRadians(90)), 60, 0,
                  20, -20, EQUAL),
              TestCase::QUICK);
  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(90), DegreesToRadians(90)), 60, 0, 20,
                  -20, EQUAL),
              TestCase::QUICK);
  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(100), DegreesToRadians(90)), 60, 0,
                  20, -20, EQUAL),
              TestCase::QUICK);
  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(150), DegreesToRadians(90)), 60, 0,
                  20, -20, EQUAL),
              TestCase::QUICK);
  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(180), DegreesToRadians(90)), 60, 0,
                  20, -20, EQUAL),
              TestCase::QUICK);
  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(-100), DegreesToRadians(90)), 60, 0,
                  20, -20, EQUAL),
              TestCase::QUICK);
  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(-150), DegreesToRadians(90)), 60, 0,
                  20, -20, EQUAL),
              TestCase::QUICK);
  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(-180), DegreesToRadians(90)), 60, 0,
                  20, -20, EQUAL),
              TestCase::QUICK);

  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(60), DegreesToRadians(90)), 60, 60,
                  10, 0, EQUAL),
              TestCase::QUICK);
  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(90), DegreesToRadians(90)), 60, 60,
                  10, -3, EQUAL),
              TestCase::QUICK);
  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(30), DegreesToRadians(90)), 60, 60,
                  10, -3, EQUAL),
              TestCase::QUICK);
  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(-30), DegreesToRadians(90)), 60, 60,
                  10, -10, EQUAL),
              TestCase::QUICK);
  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(150), DegreesToRadians(90)), 60, 60,
                  10, -10, EQUAL),
              TestCase::QUICK);
  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(160), DegreesToRadians(90)), 60, 60,
                  10, -10, EQUAL),
              TestCase::QUICK);
  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(210), DegreesToRadians(90)), 60, 60,
                  10, -10, EQUAL),
              TestCase::QUICK);
  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(240), DegreesToRadians(90)), 60, 60,
                  10, -10, EQUAL),
              TestCase::QUICK);
  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(-40), DegreesToRadians(90)), 60, 60,
                  10, -10, EQUAL),
              TestCase::QUICK);
  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(-90), DegreesToRadians(90)), 60, 60,
                  10, -10, EQUAL),
              TestCase::QUICK);
  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(-120), DegreesToRadians(90)), 60, 60,
                  10, -10, EQUAL),
              TestCase::QUICK);

  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(-150), DegreesToRadians(90)), 80,
                  -150, 10, 0, EQUAL),
              TestCase::QUICK);
  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(-110), DegreesToRadians(90)), 80,
                  -150, 10, -3, EQUAL),
              TestCase::QUICK);
  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(-190), DegreesToRadians(90)), 80,
                  -150, 10, -3, EQUAL),
              TestCase::QUICK);
  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(-70), DegreesToRadians(90)), 80, -150,
                  10, -10, EQUAL),
              TestCase::QUICK);
  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(92), DegreesToRadians(90)), 80, -150,
                  10, -10, EQUAL),
              TestCase::QUICK);
  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(-30), DegreesToRadians(90)), 80, -150,
                  10, -10, EQUAL),
              TestCase::QUICK);
  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(0), DegreesToRadians(90)), 80, -150,
                  10, -10, EQUAL),
              TestCase::QUICK);
  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(60), DegreesToRadians(90)), 80, -150,
                  10, -10, EQUAL),
              TestCase::QUICK);
  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(90), DegreesToRadians(90)), 80, -150,
                  10, -10, EQUAL),
              TestCase::QUICK);
  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(30), DegreesToRadians(90)), 80, -150,
                  10, -10, EQUAL),
              TestCase::QUICK);

  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(0), DegreesToRadians(88)), 60, 0, 20,
                  0, EQUAL),
              TestCase::QUICK);
  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(30), DegreesToRadians(88)), 60, 0, 20,
                  -3, EQUAL),
              TestCase::QUICK);
  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(-30), DegreesToRadians(88)), 60, 0,
                  20, -3, EQUAL),
              TestCase::QUICK);
  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(-90), DegreesToRadians(88)), 60, 0,
                  20, -20, EQUAL),
              TestCase::QUICK);
  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(-180), DegreesToRadians(88)), 60, 0,
                  20, -20, EQUAL),
              TestCase::QUICK);
  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(60), DegreesToRadians(93)), 60, 60,
                  20, 0, EQUAL),
              TestCase::QUICK);
  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(90), DegreesToRadians(93)), 60, 60,
                  20, -3, EQUAL),
              TestCase::QUICK);
  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(30), DegreesToRadians(93)), 60, 60,
                  20, -3, EQUAL),
              TestCase::QUICK);
  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(-120), DegreesToRadians(93)), 60, 60,
                  20, -20, EQUAL),
              TestCase::QUICK);
  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(-150), DegreesToRadians(93)), 100,
                  -150, 10, 0, EQUAL),
              TestCase::QUICK);
  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(-100), DegreesToRadians(93)), 100,
                  -150, 10, -3, EQUAL),
              TestCase::QUICK);
  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(-200), DegreesToRadians(93)), 100,
                  -150, 10, -3, EQUAL),
              TestCase::QUICK);
  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(-30), DegreesToRadians(93)), 100,
                  -150, 10, -10, EQUAL),
              TestCase::QUICK);
  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(90), DegreesToRadians(80.5)), 100,
                  -150, 10, -10, EQUAL),
              TestCase::QUICK);
  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(0), DegreesToRadians(80.5)), 60, 0,
                  20, 0, EQUAL),
              TestCase::QUICK);
  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(30), DegreesToRadians(80.5)), 60, 0,
                  20, -3, EQUAL),
              TestCase::QUICK);
  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(-30), DegreesToRadians(80.5)), 60, 0,
                  20, -3, EQUAL),
              TestCase::QUICK);
  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(100), DegreesToRadians(80.5)), 60, 0,
                  20, -20, EQUAL),
              TestCase::QUICK);
  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(-150), DegreesToRadians(80.5)), 100,
                  -150, 30, 0, EQUAL),
              TestCase::QUICK);
  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(-100), DegreesToRadians(80.5)), 100,
                  -150, 30, -3, EQUAL),
              TestCase::QUICK);
  AddTestCase(new ParabolicAntennaModelTestCase(
                  Angles(DegreesToRadians(-200), DegreesToRadians(80.5)), 100,
                  -150, 30, -3, EQUAL),
              TestCase::QUICK);
};

static ParabolicAntennaModelTestSuite
    g_staticParabolicAntennaModelTestSuiteInstance;
