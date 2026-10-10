
#include "ns3/abort.h"
#include "ns3/buildings-channel-condition-model.h"
#include "ns3/buildings-module.h"
#include "ns3/config.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/test.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("BuildingsChannelConditionModelsTest");

class BuildingsChannelConditionModelTestCase : public TestCase {
public:
  BuildingsChannelConditionModelTestCase();

  ~BuildingsChannelConditionModelTestCase() override;

private:
  void DoRun() override;

  struct TestVector {
    Vector m_positionA;
    Vector m_positionB;
    ChannelCondition::LosConditionValue m_losCond;
  };

  TestVectors<TestVector> m_testVectors;
};

BuildingsChannelConditionModelTestCase::BuildingsChannelConditionModelTestCase()
    : TestCase("Test case for the BuildingsChannelConditionModel"),
      m_testVectors() {}

BuildingsChannelConditionModelTestCase::
    ~BuildingsChannelConditionModelTestCase() {}

void BuildingsChannelConditionModelTestCase::DoRun() {
  TestVector testVector;

  testVector.m_positionA = Vector(0.0, 5.0, 1.5);
  testVector.m_positionB = Vector(20.0, 5.0, 1.5);
  testVector.m_losCond = ChannelCondition::LosConditionValue::NLOS;
  m_testVectors.Add(testVector);

  testVector.m_positionA = Vector(0.0, 11.0, 1.5);
  testVector.m_positionB = Vector(20.0, 11.0, 1.5);
  testVector.m_losCond = ChannelCondition::LosConditionValue::LOS;
  m_testVectors.Add(testVector);

  testVector.m_positionA = Vector(5.0, 5.0, 1.5);
  testVector.m_positionB = Vector(20.0, 5.0, 1.5);
  testVector.m_losCond = ChannelCondition::LosConditionValue::NLOS;
  m_testVectors.Add(testVector);

  testVector.m_positionA = Vector(4.0, 5.0, 1.5);
  testVector.m_positionB = Vector(5.0, 5.0, 1.5);
  testVector.m_losCond = ChannelCondition::LosConditionValue::LOS;
  m_testVectors.Add(testVector);

  NodeContainer nodes;
  nodes.Create(2);

  Ptr<MobilityModel> a = CreateObject<ConstantPositionMobilityModel>();
  nodes.Get(0)->AggregateObject(a);

  Ptr<MobilityModel> b = CreateObject<ConstantPositionMobilityModel>();
  nodes.Get(1)->AggregateObject(b);

  Ptr<BuildingsChannelConditionModel> condModel =
      CreateObject<BuildingsChannelConditionModel>();

  Ptr<Building> building = Create<Building>();
  building->SetNRoomsX(1);
  building->SetNRoomsY(1);
  building->SetNFloors(1);
  building->SetBoundaries(Box(0.0, 10.0, 0.0, 10.0, 0.0, 5.0));
  building->SetExtWallsType(Building::ExtWallsType_t::Wood);

  BuildingsHelper::Install(nodes);

  for (uint32_t i = 0; i < m_testVectors.GetN(); ++i) {
    testVector = m_testVectors.Get(i);
    a->SetPosition(testVector.m_positionA);
    b->SetPosition(testVector.m_positionB);
    Ptr<MobilityBuildingInfo> buildingInfoA =
        a->GetObject<MobilityBuildingInfo>();
    buildingInfoA->MakeConsistent(a);
    Ptr<MobilityBuildingInfo> buildingInfoB =
        b->GetObject<MobilityBuildingInfo>();
    buildingInfoA->MakeConsistent(b);
    Ptr<ChannelCondition> cond = condModel->GetChannelCondition(a, b);

    NS_LOG_DEBUG("Got " << cond->GetLosCondition() << " expected condition "
                        << testVector.m_losCond);
    NS_TEST_ASSERT_MSG_EQ(cond->GetLosCondition(), testVector.m_losCond,
                          " Got unexpected channel condition");
  }

  Simulator::Destroy();
}

class BuildingsChannelConditionModelsTestSuite : public TestSuite {
public:
  BuildingsChannelConditionModelsTestSuite();
};

BuildingsChannelConditionModelsTestSuite::
    BuildingsChannelConditionModelsTestSuite()
    : TestSuite("buildings-channel-condition-model", UNIT) {
  AddTestCase(new BuildingsChannelConditionModelTestCase, TestCase::QUICK);
}

static BuildingsChannelConditionModelsTestSuite
    BuildingsChannelConditionModelsTestSuite;
