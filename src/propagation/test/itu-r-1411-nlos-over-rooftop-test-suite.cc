
#include <ns3/constant-position-mobility-model.h>
#include <ns3/double.h>
#include <ns3/enum.h>
#include <ns3/itu-r-1411-nlos-over-rooftop-propagation-loss-model.h>
#include <ns3/log.h>
#include <ns3/string.h>
#include <ns3/test.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ItuR1411NlosOverRooftopPropagationLossModelTest");

class ItuR1411NlosOverRooftopPropagationLossModelTestCase : public TestCase {
public:
  ItuR1411NlosOverRooftopPropagationLossModelTestCase(
      double freq, double dist, double hb, double hm, EnvironmentType env,
      CitySize city, double refValue, std::string name);
  ~ItuR1411NlosOverRooftopPropagationLossModelTestCase() override;

private:
  void DoRun() override;

  Ptr<MobilityModel> CreateMobilityModel(uint16_t index);

  double m_freq;
  double m_dist;
  double m_hb;
  double m_hm;
  EnvironmentType m_env;
  CitySize m_city;
  double m_lossRef;
};

ItuR1411NlosOverRooftopPropagationLossModelTestCase::
    ItuR1411NlosOverRooftopPropagationLossModelTestCase(
        double freq, double dist, double hb, double hm, EnvironmentType env,
        CitySize city, double refValue, std::string name)
    : TestCase(name), m_freq(freq), m_dist(dist), m_hb(hb), m_hm(hm),
      m_env(env), m_city(city), m_lossRef(refValue) {}

ItuR1411NlosOverRooftopPropagationLossModelTestCase::
    ~ItuR1411NlosOverRooftopPropagationLossModelTestCase() {}

void ItuR1411NlosOverRooftopPropagationLossModelTestCase::DoRun() {
  NS_LOG_FUNCTION(this);

  Ptr<MobilityModel> mma = CreateObject<ConstantPositionMobilityModel>();
  mma->SetPosition(Vector(0.0, 0.0, m_hb));

  Ptr<MobilityModel> mmb = CreateObject<ConstantPositionMobilityModel>();
  mmb->SetPosition(Vector(m_dist, 0.0, m_hm));

  Ptr<ItuR1411NlosOverRooftopPropagationLossModel> propagationLossModel =
      CreateObject<ItuR1411NlosOverRooftopPropagationLossModel>();
  propagationLossModel->SetAttribute("Frequency", DoubleValue(m_freq));
  propagationLossModel->SetAttribute("Environment", EnumValue(m_env));
  propagationLossModel->SetAttribute("CitySize", EnumValue(m_city));

  double loss = propagationLossModel->GetLoss(mma, mmb);

  NS_LOG_INFO("Calculated loss: " << loss);
  NS_LOG_INFO("Theoretical loss: " << m_lossRef);

  NS_TEST_ASSERT_MSG_EQ_TOL(loss, m_lossRef, 0.1, "Wrong loss!");
}

class ItuR1411NlosOverRooftopPropagationLossModelTestSuite : public TestSuite {
public:
  ItuR1411NlosOverRooftopPropagationLossModelTestSuite();
};

ItuR1411NlosOverRooftopPropagationLossModelTestSuite::
    ItuR1411NlosOverRooftopPropagationLossModelTestSuite()
    : TestSuite("itu-r-1411-nlos-over-rooftop", SYSTEM) {
  LogComponentEnable("ItuR1411NlosOverRooftopPropagationLossModelTest",
                     LOG_LEVEL_ALL);

  AddTestCase(new ItuR1411NlosOverRooftopPropagationLossModelTestCase(
                  2.1140e9, 900, 30, 1, UrbanEnvironment, LargeCity, 143.68,
                  "f=2114Mhz, dist=900, urban large city"),
              TestCase::QUICK);
  AddTestCase(new ItuR1411NlosOverRooftopPropagationLossModelTestCase(
                  1.865e9, 500, 30, 1, UrbanEnvironment, LargeCity, 132.84,
                  "f=2114Mhz, dist=900, urban large city"),
              TestCase::QUICK);
}

static ItuR1411NlosOverRooftopPropagationLossModelTestSuite
    g_ituR1411NlosOverRooftopTestSuite;
