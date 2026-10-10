
#include <ns3/constant-position-mobility-model.h>
#include <ns3/double.h>
#include <ns3/enum.h>
#include <ns3/itu-r-1411-los-propagation-loss-model.h>
#include <ns3/log.h>
#include <ns3/string.h>
#include <ns3/test.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ItuR1411LosPropagationLossModelTest");

class ItuR1411LosPropagationLossModelTestCase : public TestCase {
public:
  ItuR1411LosPropagationLossModelTestCase(double freq, double dist, double hb,
                                          double hm, double refValue,
                                          std::string name);
  ~ItuR1411LosPropagationLossModelTestCase() override;

private:
  void DoRun() override;

  Ptr<MobilityModel> CreateMobilityModel(uint16_t index);

  double m_freq;
  double m_dist;
  double m_hb;
  double m_hm;
  double m_lossRef;
};

ItuR1411LosPropagationLossModelTestCase::
    ItuR1411LosPropagationLossModelTestCase(double freq, double dist, double hb,
                                            double hm, double refValue,
                                            std::string name)
    : TestCase(name), m_freq(freq), m_dist(dist), m_hb(hb), m_hm(hm),
      m_lossRef(refValue) {}

ItuR1411LosPropagationLossModelTestCase::
    ~ItuR1411LosPropagationLossModelTestCase() {}

void ItuR1411LosPropagationLossModelTestCase::DoRun() {
  NS_LOG_FUNCTION(this);

  Ptr<MobilityModel> mma = CreateObject<ConstantPositionMobilityModel>();
  mma->SetPosition(Vector(0.0, 0.0, m_hb));

  Ptr<MobilityModel> mmb = CreateObject<ConstantPositionMobilityModel>();
  mmb->SetPosition(Vector(m_dist, 0.0, m_hm));

  Ptr<ItuR1411LosPropagationLossModel> propagationLossModel =
      CreateObject<ItuR1411LosPropagationLossModel>();
  propagationLossModel->SetAttribute("Frequency", DoubleValue(m_freq));

  double loss = propagationLossModel->GetLoss(mma, mmb);

  NS_LOG_INFO("Calculated loss: " << loss);
  NS_LOG_INFO("Theoretical loss: " << m_lossRef);

  NS_TEST_ASSERT_MSG_EQ_TOL(loss, m_lossRef, 0.1, "Wrong loss!");
}

class ItuR1411LosPropagationLossModelTestSuite : public TestSuite {
public:
  ItuR1411LosPropagationLossModelTestSuite();
};

ItuR1411LosPropagationLossModelTestSuite::
    ItuR1411LosPropagationLossModelTestSuite()
    : TestSuite("itu-r-1411-los", SYSTEM) {
  LogComponentEnable("ItuR1411LosPropagationLossModelTest", LOG_LEVEL_ALL);

  AddTestCase(new ItuR1411LosPropagationLossModelTestCase(
                  2.1140e9, 100, 30, 1, 81.005, "freq=2114MHz, dist=100m"),
              TestCase::QUICK);
  AddTestCase(new ItuR1411LosPropagationLossModelTestCase(
                  1999e6, 200, 30, 1, 87.060, "freq=1999MHz, dist=200m"),
              TestCase::QUICK);
}

static ItuR1411LosPropagationLossModelTestSuite g_ituR1411LosTestSuite;
