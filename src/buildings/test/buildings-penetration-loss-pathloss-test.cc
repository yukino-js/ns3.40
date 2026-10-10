
#include "ns3/abort.h"
#include "ns3/boolean.h"
#include "ns3/building.h"
#include "ns3/buildings-channel-condition-model.h"
#include "ns3/channel-condition-model.h"
#include "ns3/config.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/constant-velocity-mobility-model.h"
#include "ns3/double.h"
#include "ns3/log.h"
#include "ns3/mobility-building-info.h"
#include "ns3/mobility-helper.h"
#include "ns3/simulator.h"
#include "ns3/test.h"
#include "ns3/three-gpp-propagation-loss-model.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("BuildingsPenetrationLossesTest");

class BuildingsPenetrationLossesTestCase : public TestCase {
public:
  BuildingsPenetrationLossesTestCase();

  ~BuildingsPenetrationLossesTestCase() override;

private:
  void DoRun() override;

  struct TestVector {
    Vector m_positionA;
    Vector m_positionB;
    double m_frequency;
    bool m_isLos;
    TypeId m_condModel;
    TypeId m_propModel;
    double m_pt;
    double m_pr;
  };

  TestVectors<TestVector> m_testVectors;
  Ptr<ThreeGppPropagationLossModel> m_propModel;
  double m_tolerance;
};

BuildingsPenetrationLossesTestCase::BuildingsPenetrationLossesTestCase()
    : TestCase("Test case for BuildingsPenetrationLosses"), m_testVectors(),
      m_tolerance(2e-3) {}

BuildingsPenetrationLossesTestCase::~BuildingsPenetrationLossesTestCase() {}

void BuildingsPenetrationLossesTestCase::DoRun() {
  TestVector testVector;

  testVector.m_positionA = Vector(0, 0, 35.0);
  testVector.m_positionB = Vector(10, 0, 1.5);
  testVector.m_frequency = 5.0e9;
  testVector.m_propModel = ThreeGppRmaPropagationLossModel::GetTypeId();
  testVector.m_pt = 0.0;
  testVector.m_pr = -77.3784;
  m_testVectors.Add(testVector);

  testVector.m_positionA = Vector(0, 0, 35.0);
  testVector.m_positionB = Vector(100, 0, 1.5);
  testVector.m_frequency = 5.0e9;
  testVector.m_propModel = ThreeGppRmaPropagationLossModel::GetTypeId();
  testVector.m_pt = 0.0;
  testVector.m_pr = -87.2965;
  m_testVectors.Add(testVector);

  testVector.m_positionA = Vector(0, 0, 35.0);
  testVector.m_positionB = Vector(1000, 0, 1.5);
  testVector.m_frequency = 5.0e9;
  testVector.m_propModel = ThreeGppRmaPropagationLossModel::GetTypeId();
  testVector.m_pt = 0.0;
  testVector.m_pr = -108.5577;
  m_testVectors.Add(testVector);

  testVector.m_positionA = Vector(0, 0, 25.0);
  testVector.m_positionB = Vector(10, 0, 1.5);
  testVector.m_frequency = 5.0e9;
  testVector.m_propModel = ThreeGppUmaPropagationLossModel::GetTypeId();
  testVector.m_pt = 0.0;
  testVector.m_pr = -72.9380;
  m_testVectors.Add(testVector);

  testVector.m_positionA = Vector(0, 0, 25.0);
  testVector.m_positionB = Vector(100, 0, 1.5);
  testVector.m_frequency = 5.0e9;
  testVector.m_propModel = ThreeGppUmaPropagationLossModel::GetTypeId();
  testVector.m_pt = 0.0;
  testVector.m_pr = -86.2362;
  m_testVectors.Add(testVector);

  testVector.m_positionA = Vector(0, 0, 25.0);
  testVector.m_positionB = Vector(1000, 0, 1.5);
  testVector.m_frequency = 5.0e9;
  testVector.m_propModel = ThreeGppUmaPropagationLossModel::GetTypeId();
  testVector.m_pt = 0.0;
  testVector.m_pr = -109.7252;
  m_testVectors.Add(testVector);

  testVector.m_positionA = Vector(0, 0, 10.0);
  testVector.m_positionB = Vector(10, 0, 1.5);
  testVector.m_frequency = 5.0e9;
  testVector.m_propModel =
      ThreeGppUmiStreetCanyonPropagationLossModel::GetTypeId();
  testVector.m_pt = 0.0;
  testVector.m_pr = -69.8591;
  m_testVectors.Add(testVector);

  testVector.m_positionA = Vector(0, 0, 10.0);
  testVector.m_positionB = Vector(100, 0, 1.5);
  testVector.m_frequency = 5.0e9;
  testVector.m_propModel =
      ThreeGppUmiStreetCanyonPropagationLossModel::GetTypeId();
  testVector.m_pt = 0.0;
  testVector.m_pr = -88.4122;
  m_testVectors.Add(testVector);

  testVector.m_positionA = Vector(0, 0, 10.0);
  testVector.m_positionB = Vector(1000, 0, 1.5);
  testVector.m_frequency = 5.0e9;
  testVector.m_propModel =
      ThreeGppUmiStreetCanyonPropagationLossModel::GetTypeId();
  testVector.m_pt = 0.0;
  testVector.m_pr = -119.3114;
  m_testVectors.Add(testVector);

  ObjectFactory propModelFactory;

  Ptr<Building> building = Create<Building>();
  building->SetExtWallsType(Building::ExtWallsType_t::ConcreteWithWindows);
  building->SetNRoomsX(1);
  building->SetNRoomsY(1);
  building->SetNFloors(2);
  building->SetBoundaries(Box(0.0, 100.0, 0.0, 10.0, 0.0, 5.0));

  NodeContainer nodes;
  nodes.Create(2);

  Ptr<MobilityModel> a = CreateObject<ConstantPositionMobilityModel>();
  Ptr<MobilityModel> b = CreateObject<ConstantPositionMobilityModel>();

  nodes.Get(0)->AggregateObject(a);
  nodes.Get(1)->AggregateObject(b);

  Ptr<MobilityBuildingInfo> buildingInfoA =
      CreateObject<MobilityBuildingInfo>();
  Ptr<MobilityBuildingInfo> buildingInfoB =
      CreateObject<MobilityBuildingInfo>();
  a->AggregateObject(buildingInfoA);
  buildingInfoA->MakeConsistent(a);

  b->AggregateObject(buildingInfoB);
  buildingInfoB->MakeConsistent(b);

  Ptr<ChannelConditionModel> condModel =
      CreateObject<BuildingsChannelConditionModel>();

  for (std::size_t i = 0; i < m_testVectors.GetN(); i++) {
    TestVector testVector = m_testVectors.Get(i);
    a->SetPosition(testVector.m_positionA);
    b->SetPosition(testVector.m_positionB);

    Ptr<ChannelCondition> cond = condModel->GetChannelCondition(a, b);

    propModelFactory.SetTypeId(testVector.m_propModel);
    m_propModel = propModelFactory.Create<ThreeGppPropagationLossModel>();
    m_propModel->SetAttribute("Frequency", DoubleValue(testVector.m_frequency));
    m_propModel->SetAttribute("ShadowingEnabled", BooleanValue(false));
    m_propModel->SetChannelConditionModel(condModel);

    bool isAIndoor = buildingInfoA->IsIndoor();
    bool isBIndoor = buildingInfoB->IsIndoor();

    if (!isAIndoor && !isBIndoor) {
      cond->SetLosCondition(ChannelCondition::LosConditionValue::LOS);
      cond->SetO2iCondition(ChannelCondition::O2iConditionValue::O2O);

      NS_TEST_EXPECT_MSG_EQ_TOL(m_propModel->CalcRxPower(testVector.m_pt, a, b),
                                testVector.m_pr, m_tolerance,
                                "rcv power is not equal expected value");
    } else {
      cond->SetLosCondition(ChannelCondition::LosConditionValue::LOS);
      cond->SetO2iCondition(ChannelCondition::O2iConditionValue::O2I);
      cond->SetO2iLowHighCondition(
          ChannelCondition::O2iLowHighConditionValue::LOW);

      NS_TEST_EXPECT_MSG_LT(m_propModel->CalcRxPower(testVector.m_pt, a, b),
                            testVector.m_pr,
                            "rcv power is not less than calculated value");
    }
    m_propModel = nullptr;
  }
  Simulator::Destroy();
}

class BuildingsPenetrationLossesTestSuite : public TestSuite {
public:
  BuildingsPenetrationLossesTestSuite();
};

BuildingsPenetrationLossesTestSuite::BuildingsPenetrationLossesTestSuite()
    : TestSuite("buildings-penetration-losses", UNIT) {
  AddTestCase(new BuildingsPenetrationLossesTestCase, TestCase::QUICK);
}

static BuildingsPenetrationLossesTestSuite
    g_buildingsPenetrationLossesTestSuite;
