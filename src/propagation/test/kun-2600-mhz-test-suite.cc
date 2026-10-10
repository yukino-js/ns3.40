
#include <ns3/constant-position-mobility-model.h>
#include <ns3/double.h>
#include <ns3/enum.h>
#include <ns3/kun-2600-mhz-propagation-loss-model.h>
#include <ns3/log.h>
#include <ns3/string.h>
#include <ns3/test.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Kun2600MhzPropagationLossModelTest");

class Kun2600MhzPropagationLossModelTestCase : public TestCase {
public:
  Kun2600MhzPropagationLossModelTestCase(double dist, double hb, double hm,
                                         double refValue, std::string name);
  ~Kun2600MhzPropagationLossModelTestCase() override;

private:
  void DoRun() override;

  Ptr<MobilityModel> CreateMobilityModel(uint16_t index);

  double m_dist;
  double m_hb;
  double m_hm;
  double m_lossRef;
};

Kun2600MhzPropagationLossModelTestCase::Kun2600MhzPropagationLossModelTestCase(
    double dist, double hb, double hm, double refValue, std::string name)
    : TestCase(name), m_dist(dist), m_hb(hb), m_hm(hm), m_lossRef(refValue) {}

Kun2600MhzPropagationLossModelTestCase::
    ~Kun2600MhzPropagationLossModelTestCase() {}

void Kun2600MhzPropagationLossModelTestCase::DoRun() {
  NS_LOG_FUNCTION(this);

  Ptr<MobilityModel> mma = CreateObject<ConstantPositionMobilityModel>();
  mma->SetPosition(Vector(0.0, 0.0, m_hb));

  Ptr<MobilityModel> mmb = CreateObject<ConstantPositionMobilityModel>();
  mmb->SetPosition(Vector(m_dist, 0.0, m_hm));

  Ptr<Kun2600MhzPropagationLossModel> propagationLossModel =
      CreateObject<Kun2600MhzPropagationLossModel>();

  double loss = propagationLossModel->GetLoss(mma, mmb);

  NS_LOG_INFO("Calculated loss: " << loss);
  NS_LOG_INFO("Theoretical loss: " << m_lossRef);

  NS_TEST_ASSERT_MSG_EQ_TOL(loss, m_lossRef, 0.1, "Wrong loss!");
}

class Kun2600MhzPropagationLossModelTestSuite : public TestSuite {
public:
  Kun2600MhzPropagationLossModelTestSuite();
};

Kun2600MhzPropagationLossModelTestSuite::
    Kun2600MhzPropagationLossModelTestSuite()
    : TestSuite("kun-2600-mhz", SYSTEM) {
  LogComponentEnable("Kun2600MhzPropagationLossModelTest", LOG_LEVEL_ALL);

  AddTestCase(new Kun2600MhzPropagationLossModelTestCase(2000, 30, 1, 121.83,
                                                         "dist=2000m"),
              TestCase::QUICK);
}

static Kun2600MhzPropagationLossModelTestSuite g_kun2600MhzTestSuite;
