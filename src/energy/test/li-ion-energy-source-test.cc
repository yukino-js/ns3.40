
#include "ns3/li-ion-energy-source.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/simple-device-energy-model.h"
#include "ns3/simulator.h"
#include "ns3/test.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("LiIonEnergySourceTestSuite");

class LiIonEnergyTestCase : public TestCase {
public:
  LiIonEnergyTestCase();
  ~LiIonEnergyTestCase() override;

  void DoRun() override;

  Ptr<Node> m_node;
};

LiIonEnergyTestCase::LiIonEnergyTestCase()
    : TestCase("Li-Ion energy source test case") {}

LiIonEnergyTestCase::~LiIonEnergyTestCase() { m_node = nullptr; }

void LiIonEnergyTestCase::DoRun() {
  m_node = CreateObject<Node>();

  Ptr<SimpleDeviceEnergyModel> sem = CreateObject<SimpleDeviceEnergyModel>();
  Ptr<LiIonEnergySource> es = CreateObject<LiIonEnergySource>();

  es->SetNode(m_node);
  sem->SetEnergySource(es);
  es->AppendDeviceEnergyModel(sem);
  m_node->AggregateObject(es);

  Time now = Simulator::Now();

  sem->SetCurrentA(2.33);
  now += Seconds(1701);

  Simulator::Stop(now);
  Simulator::Run();
  Simulator::Destroy();

  NS_TEST_ASSERT_MSG_EQ_TOL(es->GetSupplyVoltage(), 3.6, 1.0e-3,
                            "Incorrect consumed energy!");
}

class LiIonEnergySourceTestSuite : public TestSuite {
public:
  LiIonEnergySourceTestSuite();
};

LiIonEnergySourceTestSuite::LiIonEnergySourceTestSuite()
    : TestSuite("li-ion-energy-source", UNIT) {
  AddTestCase(new LiIonEnergyTestCase, TestCase::QUICK);
}

static LiIonEnergySourceTestSuite g_liIonEnergySourceTestSuite;
