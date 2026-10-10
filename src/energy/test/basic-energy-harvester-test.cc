
#include "ns3/basic-energy-harvester.h"
#include "ns3/basic-energy-source.h"
#include "ns3/config.h"
#include "ns3/double.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/test.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("BasicEnergyHarvesterTestSuite");

class BasicEnergyHarvesterTestCase : public TestCase {
public:
  BasicEnergyHarvesterTestCase();
  ~BasicEnergyHarvesterTestCase() override;

  void DoRun() override;

  double m_timeS;
  double m_tolerance;

  ObjectFactory m_energySource;
  ObjectFactory m_energyHarvester;
};

BasicEnergyHarvesterTestCase::BasicEnergyHarvesterTestCase()
    : TestCase("Basic Energy Harvester test case") {
  m_timeS = 15;
  m_tolerance = 1.0e-13;
}

BasicEnergyHarvesterTestCase::~BasicEnergyHarvesterTestCase() {}

void BasicEnergyHarvesterTestCase::DoRun() {
  m_energySource.SetTypeId("ns3::BasicEnergySource");
  m_energyHarvester.SetTypeId("ns3::BasicEnergyHarvester");
  Ptr<Node> node = CreateObject<Node>();

  Ptr<BasicEnergySource> source = m_energySource.Create<BasicEnergySource>();
  node->AggregateObject(source);

  Ptr<BasicEnergyHarvester> harvester =
      m_energyHarvester.Create<BasicEnergyHarvester>();
  harvester->SetHarvestedPowerUpdateInterval(Seconds(m_timeS + 1.0));
  source->ConnectEnergyHarvester(harvester);
  harvester->SetNode(node);
  harvester->SetEnergySource(source);

  Time now = Simulator::Now();

  Simulator::Schedule(Seconds(m_timeS), &BasicEnergySource::UpdateEnergySource,
                      source);

  double timeDelta = 0.000000001;
  Simulator::Stop(Seconds(m_timeS + timeDelta));
  Simulator::Run();

  double estRemainingEnergy = source->GetInitialEnergy();
  estRemainingEnergy += harvester->GetPower() * m_timeS;

  double remainingEnergy = source->GetRemainingEnergy();
  NS_LOG_DEBUG("Remaining energy is " << remainingEnergy);
  NS_LOG_DEBUG("Estimated remaining energy is " << estRemainingEnergy);
  NS_LOG_DEBUG("Difference is " << estRemainingEnergy - remainingEnergy);

  Simulator::Destroy();

  NS_TEST_ASSERT_MSG_EQ_TOL(remainingEnergy, estRemainingEnergy, m_tolerance,
                            "Incorrect Remaining energy!");
}

class BasicEnergyHarvesterTestSuite : public TestSuite {
public:
  BasicEnergyHarvesterTestSuite();
};

BasicEnergyHarvesterTestSuite::BasicEnergyHarvesterTestSuite()
    : TestSuite("basic-energy-harvester", UNIT) {
  AddTestCase(new BasicEnergyHarvesterTestCase, TestCase::QUICK);
}

static BasicEnergyHarvesterTestSuite g_basicEnergyHarvesterTestSuite;
