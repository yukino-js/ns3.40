
#include "ns3/abort.h"
#include "ns3/channel-condition-model.h"
#include "ns3/config.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/double.h"
#include "ns3/log.h"
#include "ns3/node-container.h"
#include "ns3/simulator.h"
#include "ns3/test.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ChannelConditionModelsTest");

class ThreeGppChannelConditionModelTestCase : public TestCase {
public:
  ThreeGppChannelConditionModelTestCase();

  ~ThreeGppChannelConditionModelTestCase() override;

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
  Ptr<ThreeGppChannelConditionModel> m_condModel;
  uint64_t m_numLos;
  double m_tolerance;
};

ThreeGppChannelConditionModelTestCase::ThreeGppChannelConditionModelTestCase()
    : TestCase(
          "Test case for the child classes of ThreeGppChannelConditionModel"),
      m_testVectors(), m_tolerance(2e-3) {}

ThreeGppChannelConditionModelTestCase::
    ~ThreeGppChannelConditionModelTestCase() {}

void ThreeGppChannelConditionModelTestCase::EvaluateChannelCondition(
    Ptr<MobilityModel> a, Ptr<MobilityModel> b) {
  Ptr<ChannelCondition> cond = m_condModel->GetChannelCondition(a, b);
  if (cond->GetLosCondition() == ChannelCondition::LosConditionValue::LOS) {
    m_numLos++;
  }
}

void ThreeGppChannelConditionModelTestCase::DoRun() {
  TestVector testVector;

  testVector.m_positionA = Vector(0, 0, 35.0);
  testVector.m_positionB = Vector(10, 0, 1.5);
  testVector.m_pLos = 1;
  testVector.m_typeId = ThreeGppRmaChannelConditionModel::GetTypeId();
  m_testVectors.Add(testVector);

  testVector.m_positionA = Vector(0, 0, 35.0);
  testVector.m_positionB = Vector(100, 0, 1.5);
  testVector.m_pLos = exp(-(100.0 - 10.0) / 1000.0);
  testVector.m_typeId = ThreeGppRmaChannelConditionModel::GetTypeId();
  m_testVectors.Add(testVector);

  testVector.m_positionA = Vector(0, 0, 35.0);
  testVector.m_positionB = Vector(1000, 0, 1.5);
  testVector.m_pLos = exp(-(1000.0 - 10.0) / 1000.0);
  testVector.m_typeId = ThreeGppRmaChannelConditionModel::GetTypeId();
  m_testVectors.Add(testVector);

  testVector.m_positionA = Vector(0, 0, 25.0);
  testVector.m_positionB = Vector(18, 0, 1.5);
  testVector.m_pLos = 1;
  testVector.m_typeId = ThreeGppUmaChannelConditionModel::GetTypeId();
  m_testVectors.Add(testVector);

  testVector.m_positionA = Vector(0, 0, 25.0);
  testVector.m_positionB = Vector(50, 0, 1.5);
  testVector.m_pLos =
      (18.0 / 50.0 + exp(-50.0 / 63.0) * (1.0 - 18.0 / 50.0)) * (1.0 + 0);
  testVector.m_typeId = ThreeGppUmaChannelConditionModel::GetTypeId();
  m_testVectors.Add(testVector);

  testVector.m_positionA = Vector(0, 0, 25.0);
  testVector.m_positionB = Vector(50, 0, 15);
  testVector.m_pLos = (18.0 / 50.0 + exp(-50.0 / 63.0) * (1.0 - 18.0 / 50.0)) *
                      (1.0 + pow(2.0 / 10.0, 1.5) * 5.0 / 4.0 *
                                 pow(50.0 / 100.0, 3) * exp(-50.0 / 150.0));
  testVector.m_typeId = ThreeGppUmaChannelConditionModel::GetTypeId();
  m_testVectors.Add(testVector);

  testVector.m_positionA = Vector(0, 0, 25.0);
  testVector.m_positionB = Vector(100, 0, 1.5);
  testVector.m_pLos =
      (18.0 / 100.0 + exp(-100.0 / 63.0) * (1.0 - 18.0 / 100.0)) * (1.0 + 0);
  testVector.m_typeId = ThreeGppUmaChannelConditionModel::GetTypeId();
  m_testVectors.Add(testVector);

  testVector.m_positionA = Vector(0, 0, 25.0);
  testVector.m_positionB = Vector(100, 0, 15);
  testVector.m_pLos =
      (18.0 / 100.0 + exp(-100.0 / 63.0) * (1.0 - 18.0 / 100.0)) *
      (1.0 + pow(2.0 / 10.0, 1.5) * 5.0 / 4.0 * 1.0 * exp(-100.0 / 150.0));
  testVector.m_typeId = ThreeGppUmaChannelConditionModel::GetTypeId();
  m_testVectors.Add(testVector);

  testVector.m_positionA = Vector(0, 0, 10.0);
  testVector.m_positionB = Vector(18, 0, 1.5);
  testVector.m_pLos = 1;
  testVector.m_typeId =
      ThreeGppUmiStreetCanyonChannelConditionModel::GetTypeId();
  m_testVectors.Add(testVector);

  testVector.m_positionA = Vector(0, 0, 10.0);
  testVector.m_positionB = Vector(50, 0, 1.5);
  testVector.m_pLos = (18.0 / 50.0 + exp(-50.0 / 36.0) * (1.0 - 18.0 / 50.0));
  testVector.m_typeId =
      ThreeGppUmiStreetCanyonChannelConditionModel::GetTypeId();
  m_testVectors.Add(testVector);

  m_testVectors.Add(testVector);
  testVector.m_positionA = Vector(0, 0, 10.0);
  testVector.m_positionB = Vector(100, 0, 15);
  testVector.m_pLos =
      (18.0 / 100.0 + exp(-100.0 / 36.0) * (1.0 - 18.0 / 100.0));
  testVector.m_typeId =
      ThreeGppUmiStreetCanyonChannelConditionModel::GetTypeId();
  m_testVectors.Add(testVector);

  testVector.m_positionA = Vector(0, 0, 2.0);
  testVector.m_positionB = Vector(1.2, 0, 1.5);
  testVector.m_pLos = 1;
  testVector.m_typeId =
      ThreeGppIndoorMixedOfficeChannelConditionModel::GetTypeId();
  m_testVectors.Add(testVector);

  testVector.m_positionA = Vector(0, 0, 2.0);
  testVector.m_positionB = Vector(5, 0, 1.5);
  testVector.m_pLos = exp(-(5.0 - 1.2) / 4.7);
  testVector.m_typeId =
      ThreeGppIndoorMixedOfficeChannelConditionModel::GetTypeId();
  m_testVectors.Add(testVector);

  testVector.m_positionA = Vector(0, 0, 2.0);
  testVector.m_positionB = Vector(10, 0, 1.5);
  testVector.m_pLos = exp(-(10.0 - 6.5) / 32.6) * 0.32;
  testVector.m_typeId =
      ThreeGppIndoorMixedOfficeChannelConditionModel::GetTypeId();
  m_testVectors.Add(testVector);

  testVector.m_positionA = Vector(0, 0, 3.0);
  testVector.m_positionB = Vector(5, 0, 1.5);
  testVector.m_pLos = 1;
  testVector.m_typeId =
      ThreeGppIndoorOpenOfficeChannelConditionModel::GetTypeId();
  m_testVectors.Add(testVector);

  testVector.m_positionA = Vector(0, 0, 3.0);
  testVector.m_positionB = Vector(30, 0, 1.5);
  testVector.m_pLos = exp(-(30.0 - 5.0) / 70.8);
  testVector.m_typeId =
      ThreeGppIndoorOpenOfficeChannelConditionModel::GetTypeId();
  m_testVectors.Add(testVector);

  testVector.m_positionA = Vector(0, 0, 3.0);
  testVector.m_positionB = Vector(100, 0, 1.5);
  testVector.m_pLos = exp(-(100.0 - 49.0) / 211.7) * 0.54;
  testVector.m_typeId =
      ThreeGppIndoorOpenOfficeChannelConditionModel::GetTypeId();
  m_testVectors.Add(testVector);

  ObjectFactory condModelFactory;

  NodeContainer nodes;
  nodes.Create(2);

  Ptr<MobilityModel> a = CreateObject<ConstantPositionMobilityModel>();
  Ptr<MobilityModel> b = CreateObject<ConstantPositionMobilityModel>();

  nodes.Get(0)->AggregateObject(a);
  nodes.Get(1)->AggregateObject(b);

  uint32_t numberOfReps = 500000;
  for (uint32_t i = 0; i < m_testVectors.GetN(); ++i) {
    testVector = m_testVectors.Get(i);

    a->SetPosition(testVector.m_positionA);
    b->SetPosition(testVector.m_positionB);

    condModelFactory.SetTypeId(testVector.m_typeId);
    m_condModel = condModelFactory.Create<ThreeGppChannelConditionModel>();
    m_condModel->SetAttribute("UpdatePeriod", TimeValue(MilliSeconds(9)));

    m_numLos = 0;
    for (uint32_t j = 0; j < numberOfReps; j++) {
      Simulator::Schedule(
          MilliSeconds(10 * j),
          &ThreeGppChannelConditionModelTestCase::EvaluateChannelCondition,
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

class ChannelConditionModelsTestSuite : public TestSuite {
public:
  ChannelConditionModelsTestSuite();
};

ChannelConditionModelsTestSuite::ChannelConditionModelsTestSuite()
    : TestSuite("propagation-channel-condition-model", UNIT) {
  AddTestCase(new ThreeGppChannelConditionModelTestCase, TestCase::QUICK);
}

static ChannelConditionModelsTestSuite g_channelConditionModelsTestSuite;
