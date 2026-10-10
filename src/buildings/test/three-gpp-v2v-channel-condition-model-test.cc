
#include "ns3/abort.h"
#include "ns3/boolean.h"
#include "ns3/buildings-channel-condition-model.h"
#include "ns3/buildings-module.h"
#include "ns3/channel-condition-model.h"
#include "ns3/config.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/core-module.h"
#include "ns3/double.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/test.h"
#include "ns3/three-gpp-v2v-channel-condition-model.h"
#include "ns3/three-gpp-v2v-propagation-loss-model.h"
#include "ns3/uinteger.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ThreeGppV2vChannelConditionModelsTest");

class ThreeGppV2vBuildingsChCondModelTestCase : public TestCase {
public:
  ThreeGppV2vBuildingsChCondModelTestCase();

  ~ThreeGppV2vBuildingsChCondModelTestCase() override;

private:
  void DoRun() override;

  struct TestVector {
    Vector m_positionA;
    Vector m_positionB;
    ChannelCondition::LosConditionValue m_losCond;
    TypeId m_typeId;
  };

  TestVectors<TestVector> m_testVectors;
};

ThreeGppV2vBuildingsChCondModelTestCase::
    ThreeGppV2vBuildingsChCondModelTestCase()
    : TestCase("Test case for the ThreeGppV2vUrban and ThreeGppV2vHighway "
               "ChannelConditionModel "
               "with building"),
      m_testVectors() {}

ThreeGppV2vBuildingsChCondModelTestCase::
    ~ThreeGppV2vBuildingsChCondModelTestCase() {}

void ThreeGppV2vBuildingsChCondModelTestCase::DoRun() {
  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(1);

  TestVector testVector;
  testVector.m_positionA = Vector(-5.0, 5.0, 1.5);
  testVector.m_positionB = Vector(20.0, 5.0, 1.5);
  testVector.m_losCond = ChannelCondition::LosConditionValue::NLOS;
  testVector.m_typeId = ThreeGppV2vUrbanChannelConditionModel::GetTypeId();
  m_testVectors.Add(testVector);

  testVector.m_positionA = Vector(0.0, 11.0, 1.5);
  testVector.m_positionB = Vector(4.0, 11.0, 1.5);
  testVector.m_losCond = ChannelCondition::LosConditionValue::LOS;
  testVector.m_typeId = ThreeGppV2vUrbanChannelConditionModel::GetTypeId();
  m_testVectors.Add(testVector);

  testVector.m_positionA = Vector(0.0, 11.0, 1.5);
  testVector.m_positionB = Vector(1000.0, 11.0, 1.5);
  testVector.m_losCond = ChannelCondition::LosConditionValue::NLOSv;
  testVector.m_typeId = ThreeGppV2vUrbanChannelConditionModel::GetTypeId();
  m_testVectors.Add(testVector);

  testVector.m_positionA = Vector(-5.0, 5.0, 1.5);
  testVector.m_positionB = Vector(20.0, 5.0, 1.5);
  testVector.m_losCond = ChannelCondition::LosConditionValue::NLOS;
  testVector.m_typeId = ThreeGppV2vHighwayChannelConditionModel::GetTypeId();
  m_testVectors.Add(testVector);

  testVector.m_positionA = Vector(0.0, 11.0, 1.5);
  testVector.m_positionB = Vector(4.0, 11.0, 1.5);
  testVector.m_losCond = ChannelCondition::LosConditionValue::LOS;
  testVector.m_typeId = ThreeGppV2vHighwayChannelConditionModel::GetTypeId();
  m_testVectors.Add(testVector);

  testVector.m_positionA = Vector(0.0, 11.0, 1.5);
  testVector.m_positionB = Vector(1000.0, 11.0, 1.5);
  testVector.m_losCond = ChannelCondition::LosConditionValue::NLOSv;
  testVector.m_typeId = ThreeGppV2vHighwayChannelConditionModel::GetTypeId();
  m_testVectors.Add(testVector);

  ObjectFactory condModelFactory;

  NodeContainer nodes;
  nodes.Create(2);

  Ptr<MobilityModel> a = CreateObject<ConstantPositionMobilityModel>();
  nodes.Get(0)->AggregateObject(a);

  Ptr<MobilityModel> b = CreateObject<ConstantPositionMobilityModel>();
  nodes.Get(1)->AggregateObject(b);

  Ptr<Building> building = Create<Building>();
  building->SetNRoomsX(1);
  building->SetNRoomsY(1);
  building->SetNFloors(1);
  building->SetBoundaries(Box(0.0, 10.0, 0.0, 10.0, 0.0, 5.0));

  BuildingsHelper::Install(nodes);

  for (uint32_t i = 0; i < m_testVectors.GetN(); ++i) {
    testVector = m_testVectors.Get(i);
    condModelFactory.SetTypeId(testVector.m_typeId);
    Ptr<ChannelConditionModel> condModel =
        DynamicCast<ChannelConditionModel>(condModelFactory.Create());
    condModel->AssignStreams(1);

    a->SetPosition(testVector.m_positionA);
    b->SetPosition(testVector.m_positionB);
    Ptr<MobilityBuildingInfo> buildingInfoA =
        a->GetObject<MobilityBuildingInfo>();
    buildingInfoA->MakeConsistent(a);
    Ptr<MobilityBuildingInfo> buildingInfoB =
        b->GetObject<MobilityBuildingInfo>();
    buildingInfoB->MakeConsistent(b);
    Ptr<ChannelCondition> cond;
    cond = condModel->GetChannelCondition(a, b);

    NS_LOG_DEBUG("Got " << cond->GetLosCondition() << " expected condition "
                        << testVector.m_losCond);
    NS_TEST_ASSERT_MSG_EQ(cond->GetLosCondition(), testVector.m_losCond,
                          "Got unexpected channel condition");
  }

  Simulator::Destroy();
}

class ThreeGppV2vUrbanLosNlosvChCondModelTestCase : public TestCase {
public:
  ThreeGppV2vUrbanLosNlosvChCondModelTestCase();

  ~ThreeGppV2vUrbanLosNlosvChCondModelTestCase() override;

private:
  void DoRun() override;

  void EvaluateChannelCondition(Ptr<MobilityModel> a, Ptr<MobilityModel> b);

  struct TestVector {
    Vector m_positionA;
    Vector m_positionB;
    double m_pLos;
    TypeId m_typeId;
  };

  TestVectors<TestVector> m_testVectors;
  Ptr<ThreeGppV2vUrbanChannelConditionModel> m_condModel;
  uint64_t m_numLos{0};
  double m_tolerance;
};

ThreeGppV2vUrbanLosNlosvChCondModelTestCase::
    ThreeGppV2vUrbanLosNlosvChCondModelTestCase()
    : TestCase("Test case for the class ThreeGppV2vUrbanChannelConditionModel"),
      m_testVectors(), m_tolerance(2e-3) {}

ThreeGppV2vUrbanLosNlosvChCondModelTestCase::
    ~ThreeGppV2vUrbanLosNlosvChCondModelTestCase() {}

void ThreeGppV2vUrbanLosNlosvChCondModelTestCase::EvaluateChannelCondition(
    Ptr<MobilityModel> a, Ptr<MobilityModel> b) {
  Ptr<ChannelCondition> cond = m_condModel->GetChannelCondition(a, b);
  if (cond->GetLosCondition() == ChannelCondition::LosConditionValue::LOS) {
    m_numLos++;
  }
}

void ThreeGppV2vUrbanLosNlosvChCondModelTestCase::DoRun() {
  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(1);

  TestVector testVector;

  testVector.m_positionA = Vector(0, 0, 1.6);
  testVector.m_positionB = Vector(10, 0, 1.6);
  testVector.m_pLos = std::min(1.0, 1.05 * exp(-0.0114 * 10.0));
  testVector.m_typeId = ThreeGppV2vUrbanChannelConditionModel::GetTypeId();
  m_testVectors.Add(testVector);

  testVector.m_positionA = Vector(0, 0, 1.6);
  testVector.m_positionB = Vector(100, 0, 1.6);
  testVector.m_pLos = std::min(1.0, 1.05 * exp(-0.0114 * 100.0));
  testVector.m_typeId = ThreeGppV2vUrbanChannelConditionModel::GetTypeId();
  m_testVectors.Add(testVector);

  testVector.m_positionA = Vector(0, 0, 1.6);
  testVector.m_positionB = Vector(1000, 0, 1.6);
  testVector.m_pLos = std::min(1.0, 1.05 * exp(-0.0114 * 1000.0));
  testVector.m_typeId = ThreeGppV2vUrbanChannelConditionModel::GetTypeId();
  m_testVectors.Add(testVector);

  ObjectFactory condModelFactory;

  NodeContainer nodes;
  nodes.Create(2);

  Ptr<MobilityModel> a = CreateObject<ConstantPositionMobilityModel>();
  Ptr<MobilityModel> b = CreateObject<ConstantPositionMobilityModel>();

  nodes.Get(0)->AggregateObject(a);
  nodes.Get(1)->AggregateObject(b);

  BuildingsHelper::Install(nodes);

  uint32_t numberOfReps = 500000;
  for (uint32_t i = 0; i < m_testVectors.GetN(); ++i) {
    testVector = m_testVectors.Get(i);

    a->SetPosition(testVector.m_positionA);
    b->SetPosition(testVector.m_positionB);
    Ptr<MobilityBuildingInfo> buildingInfoA =
        a->GetObject<MobilityBuildingInfo>();
    buildingInfoA->MakeConsistent(a);
    Ptr<MobilityBuildingInfo> buildingInfoB =
        b->GetObject<MobilityBuildingInfo>();
    buildingInfoB->MakeConsistent(b);

    condModelFactory.SetTypeId(testVector.m_typeId);
    m_condModel =
        condModelFactory.Create<ThreeGppV2vUrbanChannelConditionModel>();
    m_condModel->SetAttribute("UpdatePeriod", TimeValue(MilliSeconds(9)));
    m_condModel->AssignStreams(1);

    m_numLos = 0;
    for (uint32_t j = 0; j < numberOfReps; j++) {
      Simulator::Schedule(MilliSeconds(10 * j),
                          &ThreeGppV2vUrbanLosNlosvChCondModelTestCase::
                              EvaluateChannelCondition,
                          this, a, b);
    }

    Simulator::Run();
    Simulator::Destroy();

    double resultPlos = double(m_numLos) / double(numberOfReps);
    NS_LOG_DEBUG(testVector.m_typeId
                 << "  a pos " << testVector.m_positionA << " b pos "
                 << testVector.m_positionB << " numLos " << m_numLos
                 << " numberOfReps " << numberOfReps << " resultPlos "
                 << resultPlos << " ref " << testVector.m_pLos);
    NS_TEST_EXPECT_MSG_EQ_TOL(resultPlos, testVector.m_pLos, m_tolerance,
                              "Got unexpected LOS probability");
  }
}

class ThreeGppV2vHighwayLosNlosvChCondModelTestCase : public TestCase {
public:
  ThreeGppV2vHighwayLosNlosvChCondModelTestCase();

  ~ThreeGppV2vHighwayLosNlosvChCondModelTestCase() override;

private:
  void DoRun() override;

  void EvaluateChannelCondition(Ptr<MobilityModel> a, Ptr<MobilityModel> b);

  struct TestVector {
    Vector m_positionA;
    Vector m_positionB;
    double m_pLos;
    TypeId m_typeId;
  };

  TestVectors<TestVector> m_testVectors;
  Ptr<ThreeGppV2vHighwayChannelConditionModel> m_condModel;
  uint64_t m_numLos{0};
  double m_tolerance;
};

ThreeGppV2vHighwayLosNlosvChCondModelTestCase::
    ThreeGppV2vHighwayLosNlosvChCondModelTestCase()
    : TestCase(
          "Test case for the class ThreeGppV2vHighwayChannelConditionModel"),
      m_testVectors(), m_tolerance(2e-3) {}

ThreeGppV2vHighwayLosNlosvChCondModelTestCase::
    ~ThreeGppV2vHighwayLosNlosvChCondModelTestCase() {}

void ThreeGppV2vHighwayLosNlosvChCondModelTestCase::EvaluateChannelCondition(
    Ptr<MobilityModel> a, Ptr<MobilityModel> b) {
  Ptr<ChannelCondition> cond = m_condModel->GetChannelCondition(a, b);
  if (cond->GetLosCondition() == ChannelCondition::LosConditionValue::LOS) {
    m_numLos++;
  }
}

void ThreeGppV2vHighwayLosNlosvChCondModelTestCase::DoRun() {
  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(1);

  TestVector testVector;

  testVector.m_positionA = Vector(0, 0, 1.6);
  testVector.m_positionB = Vector(10, 0, 1.6);
  testVector.m_pLos =
      std::min(1.0, 0.0000021013 * 10.0 * 10.0 - 0.002 * 10.0 + 1.0193);
  testVector.m_typeId = ThreeGppV2vHighwayChannelConditionModel::GetTypeId();
  m_testVectors.Add(testVector);

  testVector.m_positionA = Vector(0, 0, 1.6);
  testVector.m_positionB = Vector(100, 0, 1.6);
  testVector.m_pLos =
      std::min(1.0, 0.0000021013 * 100.0 * 100.0 - 0.002 * 100.0 + 1.0193);
  testVector.m_typeId = ThreeGppV2vHighwayChannelConditionModel::GetTypeId();
  m_testVectors.Add(testVector);

  testVector.m_positionA = Vector(0, 0, 1.6);
  testVector.m_positionB = Vector(1000, 0, 1.6);
  testVector.m_pLos = std::max(0.0, 0.54 - 0.001 * (1000.0 - 475));
  testVector.m_typeId = ThreeGppV2vHighwayChannelConditionModel::GetTypeId();
  m_testVectors.Add(testVector);

  ObjectFactory condModelFactory;

  NodeContainer nodes;
  nodes.Create(2);

  Ptr<MobilityModel> a = CreateObject<ConstantPositionMobilityModel>();
  Ptr<MobilityModel> b = CreateObject<ConstantPositionMobilityModel>();

  nodes.Get(0)->AggregateObject(a);
  nodes.Get(1)->AggregateObject(b);

  BuildingsHelper::Install(nodes);

  uint32_t numberOfReps = 500000;
  for (uint32_t i = 0; i < m_testVectors.GetN(); ++i) {
    testVector = m_testVectors.Get(i);

    a->SetPosition(testVector.m_positionA);
    b->SetPosition(testVector.m_positionB);

    condModelFactory.SetTypeId(testVector.m_typeId);
    m_condModel =
        condModelFactory.Create<ThreeGppV2vHighwayChannelConditionModel>();
    m_condModel->SetAttribute("UpdatePeriod", TimeValue(MilliSeconds(9)));
    m_condModel->AssignStreams(1);

    m_numLos = 0;
    for (uint32_t j = 0; j < numberOfReps; j++) {
      Simulator::Schedule(MilliSeconds(10 * j),
                          &ThreeGppV2vHighwayLosNlosvChCondModelTestCase::
                              EvaluateChannelCondition,
                          this, a, b);
    }

    Simulator::Run();
    Simulator::Destroy();

    double resultPlos =
        static_cast<double>(m_numLos) / static_cast<double>(numberOfReps);
    NS_LOG_DEBUG(testVector.m_typeId
                 << "  a pos " << testVector.m_positionA << " b pos "
                 << testVector.m_positionB << " numLos " << m_numLos
                 << " numberOfReps " << numberOfReps << " resultPlos "
                 << resultPlos << " ref " << testVector.m_pLos);
    NS_TEST_EXPECT_MSG_EQ_TOL(resultPlos, testVector.m_pLos, m_tolerance,
                              "Got unexpected LOS probability");
  }
}

class ThreeGppV2vChCondModelsTestSuite : public TestSuite {
public:
  ThreeGppV2vChCondModelsTestSuite();
};

ThreeGppV2vChCondModelsTestSuite::ThreeGppV2vChCondModelsTestSuite()
    : TestSuite("three-gpp-v2v-channel-condition-model", SYSTEM) {
  AddTestCase(new ThreeGppV2vBuildingsChCondModelTestCase, TestCase::QUICK);
  AddTestCase(new ThreeGppV2vUrbanLosNlosvChCondModelTestCase, TestCase::QUICK);
  AddTestCase(new ThreeGppV2vHighwayLosNlosvChCondModelTestCase,
              TestCase::QUICK);
}

static ThreeGppV2vChCondModelsTestSuite ThreeGppV2vChCondModelsTestSuite;
