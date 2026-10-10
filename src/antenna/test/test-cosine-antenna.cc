
#include <ns3/cosine-antenna-model.h>
#include <ns3/double.h>
#include <ns3/log.h>
#include <ns3/simulator.h>
#include <ns3/test.h>

#include <cmath>
#include <iostream>
#include <sstream>
#include <string>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TestCosineAntennaModel");

enum CosineAntennaModelGainTestCondition { EQUAL = 0, LESSTHAN = 1 };

class CosineAntennaModelTestCase : public TestCase {
public:
  static std::string BuildNameString(Angles a, double b, double o, double g);
  CosineAntennaModelTestCase(Angles a, double b, double o, double g,
                             double expectedGainDb,
                             CosineAntennaModelGainTestCondition cond);

private:
  void DoRun() override;
  Angles m_a;
  double m_b;
  double m_o;
  double m_g;
  double m_expectedGain;
  CosineAntennaModelGainTestCondition m_cond;
};

std::string CosineAntennaModelTestCase::BuildNameString(Angles a, double b,
                                                        double o, double g) {
  std::ostringstream oss;
  oss << "theta=" << a.GetInclination() << " , phi=" << a.GetAzimuth()
      << ", beamdwidth=" << b << "deg"
      << ", orientation=" << o << ", maxGain=" << g << " dB";
  return oss.str();
}

CosineAntennaModelTestCase::CosineAntennaModelTestCase(
    Angles a, double b, double o, double g, double expectedGainDb,
    CosineAntennaModelGainTestCondition cond)
    : TestCase(BuildNameString(a, b, o, g)), m_a(a), m_b(b), m_o(o), m_g(g),
      m_expectedGain(expectedGainDb), m_cond(cond) {}

void CosineAntennaModelTestCase::DoRun() {
  NS_LOG_FUNCTION(this << BuildNameString(m_a, m_b, m_o, m_g));

  Ptr<CosineAntennaModel> a = CreateObject<CosineAntennaModel>();
  a->SetAttribute("HorizontalBeamwidth", DoubleValue(m_b));
  a->SetAttribute("VerticalBeamwidth", DoubleValue(m_b));
  a->SetAttribute("Orientation", DoubleValue(m_o));
  a->SetAttribute("MaxGain", DoubleValue(m_g));
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

class CosineAntennaModelTestSuite : public TestSuite {
public:
  CosineAntennaModelTestSuite();
};

CosineAntennaModelTestSuite::CosineAntennaModelTestSuite()
    : TestSuite("cosine-antenna-model", UNIT) {

  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(0), DegreesToRadians(90)), 60, 0, 0,
                  0, EQUAL),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(30), DegreesToRadians(90)), 60, 0, 0,
                  -3, EQUAL),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(-30), DegreesToRadians(90)), 60, 0, 0,
                  -3, EQUAL),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(-90), DegreesToRadians(90)), 60, 0, 0,
                  -20, LESSTHAN),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(90), DegreesToRadians(90)), 60, 0, 0,
                  -20, LESSTHAN),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(100), DegreesToRadians(90)), 60, 0, 0,
                  -20, LESSTHAN),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(150), DegreesToRadians(90)), 60, 0, 0,
                  -20, LESSTHAN),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(180), DegreesToRadians(90)), 60, 0, 0,
                  -20, LESSTHAN),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(-100), DegreesToRadians(90)), 60, 0,
                  0, -20, LESSTHAN),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(-150), DegreesToRadians(90)), 60, 0,
                  0, -20, LESSTHAN),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(-180), DegreesToRadians(90)), 60, 0,
                  0, -20, LESSTHAN),
              TestCase::QUICK);

  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(60), DegreesToRadians(90)), 60, 60, 0,
                  0, EQUAL),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(90), DegreesToRadians(90)), 60, 60, 0,
                  -3, EQUAL),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(30), DegreesToRadians(90)), 60, 60, 0,
                  -3, EQUAL),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(-30), DegreesToRadians(90)), 60, 60,
                  0, -20, LESSTHAN),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(150), DegreesToRadians(90)), 60, 60,
                  0, -20, LESSTHAN),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(160), DegreesToRadians(90)), 60, 60,
                  0, -20, LESSTHAN),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(210), DegreesToRadians(90)), 60, 60,
                  0, -20, LESSTHAN),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(240), DegreesToRadians(90)), 60, 60,
                  0, -20, LESSTHAN),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(-40), DegreesToRadians(90)), 60, 60,
                  0, -20, LESSTHAN),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(-90), DegreesToRadians(90)), 60, 60,
                  0, -20, LESSTHAN),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(-120), DegreesToRadians(90)), 60, 60,
                  0, -20, LESSTHAN),
              TestCase::QUICK);

  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(-150), DegreesToRadians(90)), 100,
                  -150, 0, 0, EQUAL),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(-100), DegreesToRadians(90)), 100,
                  -150, 0, -3, EQUAL),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(-200), DegreesToRadians(90)), 100,
                  -150, 0, -3, EQUAL),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(-32.531), DegreesToRadians(90)), 100,
                  -150, 0, -20, EQUAL),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(92.531), DegreesToRadians(90)), 100,
                  -150, 0, -20, EQUAL),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(-30), DegreesToRadians(90)), 100,
                  -150, 0, -20, LESSTHAN),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(0), DegreesToRadians(90)), 100, -150,
                  0, -20, LESSTHAN),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(60), DegreesToRadians(90)), 100, -150,
                  0, -20, LESSTHAN),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(90), DegreesToRadians(90)), 100, -150,
                  0, -20, LESSTHAN),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(30), DegreesToRadians(90)), 100, -150,
                  0, -20, LESSTHAN),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(-150), DegreesToRadians(90)), 150,
                  -150, 0, 0, EQUAL),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(135), DegreesToRadians(90)), 150,
                  -150, 0, -3, EQUAL),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(-75), DegreesToRadians(90)), 150,
                  -150, 0, -3, EQUAL),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(85.070), DegreesToRadians(90)), 150,
                  -150, 0, -10, EQUAL),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(-25.070), DegreesToRadians(90)), 150,
                  -150, 0, -10, EQUAL),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(5.3230), DegreesToRadians(90)), 150,
                  -150, 0, -20, EQUAL),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(54.677), DegreesToRadians(90)), 150,
                  -150, 0, -20, EQUAL),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(30), DegreesToRadians(90)), 150, -150,
                  0, -20, LESSTHAN),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(20), DegreesToRadians(90)), 150, -150,
                  0, -20, LESSTHAN),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(0), DegreesToRadians(90)), 360, 0, 0,
                  0, EQUAL),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(180), DegreesToRadians(90)), 360, 0,
                  0, 0, EQUAL),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(-180), DegreesToRadians(90)), 360, 0,
                  0, 0, EQUAL),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(0), DegreesToRadians(0)), 360, 0, 0,
                  0, EQUAL),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(0), DegreesToRadians(180)), 360, 0, 0,
                  0, EQUAL),
              TestCase::QUICK);

  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(0), DegreesToRadians(90)), 60, 0, 10,
                  10, EQUAL),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(30), DegreesToRadians(90)), 60, 0, 22,
                  19, EQUAL),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(-30), DegreesToRadians(90)), 60, 0,
                  -4, -7, EQUAL),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(-90), DegreesToRadians(90)), 60, 0,
                  10, -10, LESSTHAN),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(90), DegreesToRadians(90)), 60, 0,
                  -20, -40, LESSTHAN),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(100), DegreesToRadians(90)), 60, 0,
                  40, 20, LESSTHAN),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(-150), DegreesToRadians(90)), 100,
                  -150, 2, 2, EQUAL),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(-100), DegreesToRadians(90)), 100,
                  -150, 4, 1, EQUAL),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(-200), DegreesToRadians(90)), 100,
                  -150, -1, -4, EQUAL),
              TestCase::QUICK);

  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(0), DegreesToRadians(60)), 60, 0, 0,
                  -3, EQUAL),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(30), DegreesToRadians(60)), 60, 0, 0,
                  -6, EQUAL),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(-30), DegreesToRadians(60)), 60, 0, 0,
                  -6, EQUAL),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(-90), DegreesToRadians(60)), 60, 0, 0,
                  -20, LESSTHAN),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(-180), DegreesToRadians(60)), 60, 0,
                  0, -20, LESSTHAN),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(60), DegreesToRadians(120)), 60, 60,
                  0, -3, EQUAL),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(90), DegreesToRadians(120)), 60, 60,
                  0, -6, EQUAL),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(30), DegreesToRadians(120)), 60, 60,
                  0, -6, EQUAL),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(-120), DegreesToRadians(120)), 60, 60,
                  0, -20, LESSTHAN),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(-150), DegreesToRadians(140)), 100,
                  -150, 0, -3, EQUAL),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(-100), DegreesToRadians(140)), 100,
                  -150, 0, -6, EQUAL),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(-200), DegreesToRadians(140)), 100,
                  -150, 0, -6, EQUAL),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(-30), DegreesToRadians(140)), 100,
                  -150, 0, -20, LESSTHAN),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(0), DegreesToRadians(60)), 60, 0, 10,
                  7, EQUAL),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(30), DegreesToRadians(60)), 60, 0, 22,
                  16, EQUAL),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(-30), DegreesToRadians(60)), 60, 0,
                  -4, -10, EQUAL),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(-90), DegreesToRadians(60)), 60, 0,
                  10, -13, LESSTHAN),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(90), DegreesToRadians(60)), 60, 0,
                  -20, -43, LESSTHAN),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(100), DegreesToRadians(60)), 60, 0,
                  40, 17, LESSTHAN),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(-150), DegreesToRadians(40)), 100,
                  -150, 2, -1, EQUAL),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(-100), DegreesToRadians(40)), 100,
                  -150, 4, -2, EQUAL),
              TestCase::QUICK);
  AddTestCase(new CosineAntennaModelTestCase(
                  Angles(DegreesToRadians(-200), DegreesToRadians(40)), 100,
                  -150, -1, -7, EQUAL),
              TestCase::QUICK);
};

static CosineAntennaModelTestSuite g_staticCosineAntennaModelTestSuiteInstance;
