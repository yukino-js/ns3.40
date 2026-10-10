
#include "cmath"
#include "iostream"
#include "sstream"
#include "string"

#include "ns3/double.h"
#include "ns3/isotropic-antenna-model.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/simulator.h"
#include "ns3/test.h"
#include "ns3/three-gpp-antenna-model.h"
#include "ns3/uinteger.h"
#include "ns3/uniform-planar-array.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TestUniformPlanarArray");

class UniformPlanarArrayTestCase : public TestCase {
public:
  static std::string BuildNameString(Ptr<AntennaModel> element, uint32_t rows,
                                     uint32_t cols, double rowSpace,
                                     double colSpace, double alpha, double beta,
                                     Angles direction);
  UniformPlanarArrayTestCase(Ptr<AntennaModel> element, uint32_t rows,
                             uint32_t cols, double rowSpace, double colSpace,
                             double alpha, double beta, Angles direction,
                             double expectedGainDb);

private:
  void DoRun() override;
  double ComputeGain(Ptr<UniformPlanarArray> a);

  Ptr<AntennaModel> m_element;
  uint32_t m_rows;
  uint32_t m_cols;
  double m_rowSpace;
  double m_colSpace;
  double m_alpha;
  double m_beta;
  Angles m_direction;
  double m_expectedGain;
};

std::string UniformPlanarArrayTestCase::BuildNameString(
    Ptr<AntennaModel> element, uint32_t rows, uint32_t cols, double rowSpace,
    double colSpace, double alpha, double beta, Angles direction) {
  std::ostringstream oss;
  oss << "UPA=" << rows << "x" << cols << ", row spacing=" << rowSpace
      << "*lambda"
      << ", col spacing=" << colSpace << "*lambda"
      << ", bearing=" << RadiansToDegrees(alpha) << " deg"
      << ", tilting=" << RadiansToDegrees(beta) << " deg"
      << ", element=" << element->GetInstanceTypeId().GetName()
      << ", direction=" << direction;
  return oss.str();
}

UniformPlanarArrayTestCase::UniformPlanarArrayTestCase(
    Ptr<AntennaModel> element, uint32_t rows, uint32_t cols, double rowSpace,
    double colSpace, double alpha, double beta, Angles direction,
    double expectedGainDb)
    : TestCase(BuildNameString(element, rows, cols, rowSpace, colSpace, alpha,
                               beta, direction)),
      m_element(element), m_rows(rows), m_cols(cols), m_rowSpace(rowSpace),
      m_colSpace(colSpace), m_alpha(alpha), m_beta(beta),
      m_direction(direction), m_expectedGain(expectedGainDb) {}

double UniformPlanarArrayTestCase::ComputeGain(Ptr<UniformPlanarArray> a) {
  PhasedArrayModel::ComplexVector sv = a->GetSteeringVector(m_direction);
  NS_TEST_EXPECT_MSG_EQ(sv.GetSize(), a->GetNumberOfElements(),
                        "steering vector of wrong size");
  PhasedArrayModel::ComplexVector bf = a->GetBeamformingVector(m_direction);
  NS_TEST_EXPECT_MSG_EQ(bf.GetSize(), a->GetNumberOfElements(),
                        "beamforming vector of wrong size");
  std::pair<double, double> fp = a->GetElementFieldPattern(m_direction);

  std::complex<double> prod{0};
  for (size_t i = 0; i < sv.GetSize(); i++) {
    prod += sv[i] * bf[i];
  }
  double bfGain = std::pow(std::abs(prod), 2);
  double bfGainDb = 10 * std::log10(bfGain);

  double elementPowerGain =
      std::pow(std::get<0>(fp), 2) + std::pow(std::get<1>(fp), 2);
  double elementPowerGainDb = 10 * std::log10(elementPowerGain);

  return bfGainDb + elementPowerGainDb;
}

void UniformPlanarArrayTestCase::DoRun() {
  NS_LOG_FUNCTION(this << BuildNameString(m_element, m_rows, m_cols, m_rowSpace,
                                          m_colSpace, m_alpha, m_beta,
                                          m_direction));

  Ptr<UniformPlanarArray> a = CreateObject<UniformPlanarArray>();
  a->SetAttribute("AntennaElement", PointerValue(m_element));
  a->SetAttribute("NumRows", UintegerValue(m_rows));
  a->SetAttribute("NumColumns", UintegerValue(m_cols));
  a->SetAttribute("AntennaVerticalSpacing", DoubleValue(m_rowSpace));
  a->SetAttribute("AntennaHorizontalSpacing", DoubleValue(m_colSpace));
  a->SetAttribute("BearingAngle", DoubleValue(m_alpha));
  a->SetAttribute("DowntiltAngle", DoubleValue(m_beta));

  double actualGainDb = ComputeGain(a);
  NS_TEST_EXPECT_MSG_EQ_TOL(actualGainDb, m_expectedGain, 0.001,
                            "wrong value of the radiation pattern");
}

class UniformPlanarArrayTestSuite : public TestSuite {
public:
  UniformPlanarArrayTestSuite();
};

UniformPlanarArrayTestSuite::UniformPlanarArrayTestSuite()
    : TestSuite("uniform-planar-array-test", UNIT) {
  Ptr<AntennaModel> isotropic = CreateObject<IsotropicAntennaModel>();
  Ptr<AntennaModel> tgpp = CreateObject<ThreeGppAntennaModel>();

  AddTestCase(new UniformPlanarArrayTestCase(
                  isotropic, 1, 1, 0.5, 0.5, DegreesToRadians(0),
                  DegreesToRadians(0),
                  Angles(DegreesToRadians(0), DegreesToRadians(90)), 0.0),
              TestCase::QUICK);
  AddTestCase(new UniformPlanarArrayTestCase(
                  tgpp, 1, 1, 0.5, 0.5, DegreesToRadians(0),
                  DegreesToRadians(0),
                  Angles(DegreesToRadians(0), DegreesToRadians(90)), 8.0),
              TestCase::QUICK);
  AddTestCase(new UniformPlanarArrayTestCase(
                  tgpp, 1, 1, 0.5, 0.5, DegreesToRadians(90),
                  DegreesToRadians(0),
                  Angles(DegreesToRadians(90), DegreesToRadians(90)), 8.0),
              TestCase::QUICK);
  AddTestCase(new UniformPlanarArrayTestCase(
                  tgpp, 1, 1, 0.5, 0.5, DegreesToRadians(-90),
                  DegreesToRadians(0),
                  Angles(DegreesToRadians(-90), DegreesToRadians(90)), 8.0),
              TestCase::QUICK);
  AddTestCase(new UniformPlanarArrayTestCase(
                  tgpp, 1, 1, 0.5, 0.5, DegreesToRadians(180),
                  DegreesToRadians(0),
                  Angles(DegreesToRadians(180), DegreesToRadians(90)), 8.0),
              TestCase::QUICK);
  AddTestCase(new UniformPlanarArrayTestCase(
                  tgpp, 1, 1, 0.5, 0.5, DegreesToRadians(-180),
                  DegreesToRadians(0),
                  Angles(DegreesToRadians(-180), DegreesToRadians(90)), 8.0),
              TestCase::QUICK);
  AddTestCase(new UniformPlanarArrayTestCase(
                  tgpp, 1, 1, 0.5, 0.5, DegreesToRadians(0),
                  DegreesToRadians(45),
                  Angles(DegreesToRadians(0), DegreesToRadians(135)), 8.0),
              TestCase::QUICK);
  AddTestCase(new UniformPlanarArrayTestCase(
                  tgpp, 1, 1, 0.5, 0.5, DegreesToRadians(0),
                  DegreesToRadians(-45),
                  Angles(DegreesToRadians(0), DegreesToRadians(45)), 8.0),
              TestCase::QUICK);
  AddTestCase(new UniformPlanarArrayTestCase(
                  tgpp, 1, 1, 0.5, 0.5, DegreesToRadians(0),
                  DegreesToRadians(90),
                  Angles(DegreesToRadians(0), DegreesToRadians(180)), 8.0),
              TestCase::QUICK);
  AddTestCase(new UniformPlanarArrayTestCase(
                  tgpp, 1, 1, 0.5, 0.5, DegreesToRadians(0),
                  DegreesToRadians(-90),
                  Angles(DegreesToRadians(0), DegreesToRadians(0)), 8.0),
              TestCase::QUICK);

  AddTestCase(new UniformPlanarArrayTestCase(
                  tgpp, 10, 1, 0.5, 0.5, DegreesToRadians(0),
                  DegreesToRadians(0),
                  Angles(DegreesToRadians(0), DegreesToRadians(90)), 18.0),
              TestCase::QUICK);
  AddTestCase(new UniformPlanarArrayTestCase(
                  tgpp, 10, 1, 0.5, 0.5, DegreesToRadians(90),
                  DegreesToRadians(0),
                  Angles(DegreesToRadians(90), DegreesToRadians(90)), 18.0),
              TestCase::QUICK);
  AddTestCase(new UniformPlanarArrayTestCase(
                  tgpp, 10, 1, 0.5, 0.5, DegreesToRadians(0),
                  DegreesToRadians(45),
                  Angles(DegreesToRadians(0), DegreesToRadians(135)), 18.0),
              TestCase::QUICK);

  AddTestCase(new UniformPlanarArrayTestCase(
                  tgpp, 10, 10, 0.5, 0.5, DegreesToRadians(0),
                  DegreesToRadians(0),
                  Angles(DegreesToRadians(0), DegreesToRadians(90)), 28.0),
              TestCase::QUICK);
  AddTestCase(new UniformPlanarArrayTestCase(
                  tgpp, 10, 10, 0.5, 0.5, DegreesToRadians(90),
                  DegreesToRadians(0),
                  Angles(DegreesToRadians(90), DegreesToRadians(90)), 28.0),
              TestCase::QUICK);
  AddTestCase(new UniformPlanarArrayTestCase(
                  tgpp, 10, 10, 0.5, 0.5, DegreesToRadians(0),
                  DegreesToRadians(45),
                  Angles(DegreesToRadians(0), DegreesToRadians(135)), 28.0),
              TestCase::QUICK);
}

static UniformPlanarArrayTestSuite staticUniformPlanarArrayTestSuiteInstance;
