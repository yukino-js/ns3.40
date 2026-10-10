
#include "ns3/abort.h"
#include "ns3/boolean.h"
#include "ns3/channel-condition-model.h"
#include "ns3/config.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/core-module.h"
#include "ns3/double.h"
#include "ns3/log.h"
#include "ns3/node-container.h"
#include "ns3/probabilistic-v2v-channel-condition-model.h"
#include "ns3/simulator.h"
#include "ns3/test.h"
#include "ns3/three-gpp-v2v-propagation-loss-model.h"
#include "ns3/uinteger.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ProbabilisticV2vChannelConditionModelsTest");

class V2vUrbanProbChCondModelTestCase : public TestCase {
public:
  V2vUrbanProbChCondModelTestCase();

  ~V2vUrbanProbChCondModelTestCase() override;

private:
  void DoRun() override;

  void EvaluateChannelCondition(Ptr<MobilityModel> a, Ptr<MobilityModel> b);

  struct TestVector {
    Vector m_positionA;
    Vector m_positionB;
    double m_pLos{0.0};
    double m_pNlosv{0.0};
    std::string m_density;
    TypeId m_typeId;
  };

  TestVectors<TestVector> m_testVectors;
  Ptr<ProbabilisticV2vUrbanChannelConditionModel> m_condModel;
  uint64_t m_numLos{0};
  uint64_t m_numNlosv{0};
  double m_tolerance;
};

V2vUrbanProbChCondModelTestCase::V2vUrbanProbChCondModelTestCase()
    : TestCase(
          "Test case for the class ProbabilisticV2vUrbanChannelConditionModel"),
      m_testVectors(), m_tolerance(5e-3) {}

V2vUrbanProbChCondModelTestCase::~V2vUrbanProbChCondModelTestCase() {}

void V2vUrbanProbChCondModelTestCase::EvaluateChannelCondition(
    Ptr<MobilityModel> a, Ptr<MobilityModel> b) {
  Ptr<ChannelCondition> cond = m_condModel->GetChannelCondition(a, b);
  if (cond->GetLosCondition() == ChannelCondition::LosConditionValue::LOS) {
    m_numLos++;
  } else if (cond->GetLosCondition() ==
             ChannelCondition::LosConditionValue::NLOSv) {
    m_numNlosv++;
  }
}

void V2vUrbanProbChCondModelTestCase::DoRun() {
  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(1);

  TestVector testVector;

  testVector.m_positionA = Vector(0, 0, 1.6);
  testVector.m_positionB = Vector(10, 0, 1.6);
  testVector.m_pLos =
      std::min(1.0, std::max(0.0, 0.8548 * exp(-0.0064 * 10.0)));
  testVector.m_typeId = ProbabilisticV2vUrbanChannelConditionModel::GetTypeId();
  testVector.m_pNlosv =
      std::min(1.0, std::max(0.0, 1 / (0.0396 * 10.0) *
                                      exp(-(log(10.0) - 5.2718) *
                                          (log(10.0) - 5.2718) / 3.4827)));
  testVector.m_density = "Low";
  m_testVectors.Add(testVector);

  testVector.m_positionA = Vector(0, 0, 1.6);
  testVector.m_positionB = Vector(100, 0, 1.6);
  testVector.m_pLos =
      std::min(1.0, std::max(0.0, 0.8548 * exp(-0.0064 * 100.0)));
  testVector.m_typeId = ProbabilisticV2vUrbanChannelConditionModel::GetTypeId();
  testVector.m_pNlosv =
      std::min(1.0, std::max(0.0, 1 / (0.0396 * 100.0) *
                                      exp(-(log(100.0) - 5.2718) *
                                          (log(100.0) - 5.2718) / 3.4827)));
  testVector.m_density = "Low";
  m_testVectors.Add(testVector);

  testVector.m_positionA = Vector(0, 0, 1.6);
  testVector.m_positionB = Vector(10, 0, 1.6);
  testVector.m_pLos =
      std::min(1.0, std::max(0.0, 0.8372 * exp(-0.0114 * 10.0)));
  testVector.m_typeId = ProbabilisticV2vUrbanChannelConditionModel::GetTypeId();
  testVector.m_pNlosv =
      std::min(1.0, std::max(0.0, 1 / (0.0312 * 10.0) *
                                      exp(-(log(10.0) - 5.0063) *
                                          (log(10.0) - 5.0063) / 2.4544)));
  testVector.m_density = "Medium";
  m_testVectors.Add(testVector);

  testVector.m_positionA = Vector(0, 0, 1.6);
  testVector.m_positionB = Vector(100, 0, 1.6);
  testVector.m_pLos =
      std::min(1.0, std::max(0.0, 0.8372 * exp(-0.0114 * 100.0)));
  testVector.m_typeId = ProbabilisticV2vUrbanChannelConditionModel::GetTypeId();
  testVector.m_pNlosv =
      std::min(1.0, std::max(0.0, 1 / (0.0312 * 100.0) *
                                      exp(-(log(100.0) - 5.0063) *
                                          (log(100.0) - 5.0063) / 2.4544)));
  testVector.m_density = "Medium";
  m_testVectors.Add(testVector);

  testVector.m_positionA = Vector(0, 0, 1.6);
  testVector.m_positionB = Vector(10, 0, 1.6);
  testVector.m_pLos = std::min(1.0, std::max(0.0, 0.8962 * exp(-0.017 * 10.0)));
  testVector.m_typeId = ProbabilisticV2vUrbanChannelConditionModel::GetTypeId();
  testVector.m_pNlosv =
      std::min(1.0, std::max(0.0, 1 / (0.0242 * 10.0) *
                                      exp(-(log(10.0) - 5.0115) *
                                          (log(10.0) - 5.0115) / 2.2092)));
  testVector.m_density = "High";
  m_testVectors.Add(testVector);

  testVector.m_positionA = Vector(0, 0, 1.6);
  testVector.m_positionB = Vector(100, 0, 1.6);
  testVector.m_pLos =
      std::min(1.0, std::max(0.0, 0.8962 * exp(-0.017 * 100.0)));
  testVector.m_typeId = ProbabilisticV2vUrbanChannelConditionModel::GetTypeId();
  testVector.m_pNlosv =
      std::min(1.0, std::max(0.0, 1 / (0.0242 * 100.0) *
                                      exp(-(log(100.0) - 5.0115) *
                                          (log(100.0) - 5.0115) / 2.2092)));
  testVector.m_density = "High";
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
    m_condModel =
        condModelFactory.Create<ProbabilisticV2vUrbanChannelConditionModel>();
    m_condModel->SetAttribute("UpdatePeriod", TimeValue(MilliSeconds(9)));
    m_condModel->AssignStreams(1);
    m_condModel->SetAttribute("Density", StringValue(testVector.m_density));

    m_numLos = 0;
    m_numNlosv = 0;
    for (uint32_t j = 0; j < numberOfReps; j++) {
      Simulator::Schedule(
          MilliSeconds(10 * j),
          &V2vUrbanProbChCondModelTestCase::EvaluateChannelCondition, this, a,
          b);
    }

    Simulator::Run();
    Simulator::Destroy();

    double resultPlos = double(m_numLos) / double(numberOfReps);
    double resultPnlosv = double(m_numNlosv) / double(numberOfReps);
    NS_LOG_DEBUG(testVector.m_typeId
                 << "  a pos " << testVector.m_positionA << " b pos "
                 << testVector.m_positionB << " numLos " << m_numLos
                 << " numberOfReps " << numberOfReps << " resultPlos "
                 << resultPlos << " ref " << testVector.m_pLos);
    NS_TEST_EXPECT_MSG_EQ_TOL(resultPlos, testVector.m_pLos, m_tolerance,
                              "Got unexpected LOS probability");
    NS_LOG_DEBUG(testVector.m_typeId
                 << "  a pos " << testVector.m_positionA << " b pos "
                 << testVector.m_positionB << " numNlosv " << m_numNlosv
                 << " numberOfReps " << numberOfReps << " resultPnlosv "
                 << resultPnlosv << " ref " << testVector.m_pNlosv);
    NS_TEST_EXPECT_MSG_EQ_TOL(resultPnlosv, testVector.m_pNlosv, m_tolerance,
                              "Got unexpected NLOSv probability");
  }
}

class V2vHighwayProbChCondModelTestCase : public TestCase {
public:
  V2vHighwayProbChCondModelTestCase();

  ~V2vHighwayProbChCondModelTestCase() override;

private:
  void DoRun() override;

  void EvaluateChannelCondition(Ptr<MobilityModel> a, Ptr<MobilityModel> b);

  struct TestVector {
    Vector m_positionA;
    Vector m_positionB;
    double m_pLos{0.0};
    double m_pNlos{0.0};
    std::string m_density;
    TypeId m_typeId;
  };

  TestVectors<TestVector> m_testVectors;
  Ptr<ProbabilisticV2vHighwayChannelConditionModel> m_condModel;
  uint64_t m_numLos{0};
  uint64_t m_numNlos{0};
  double m_tolerance;
};

V2vHighwayProbChCondModelTestCase::V2vHighwayProbChCondModelTestCase()
    : TestCase("Test case for the class "
               "ProbabilisticV2vHighwayChannelConditionModel"),
      m_testVectors(), m_tolerance(5e-3) {}

V2vHighwayProbChCondModelTestCase::~V2vHighwayProbChCondModelTestCase() {}

void V2vHighwayProbChCondModelTestCase::EvaluateChannelCondition(
    Ptr<MobilityModel> a, Ptr<MobilityModel> b) {
  Ptr<ChannelCondition> cond = m_condModel->GetChannelCondition(a, b);
  if (cond->GetLosCondition() == ChannelCondition::LosConditionValue::LOS) {
    m_numLos++;
  } else if (cond->GetLosCondition() ==
             ChannelCondition::LosConditionValue::NLOS) {
    m_numNlos++;
  }
}

void V2vHighwayProbChCondModelTestCase::DoRun() {
  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(1);

  TestVector testVector;

  testVector.m_positionA = Vector(0, 0, 1.6);
  testVector.m_positionB = Vector(10, 0, 1.6);
  double aLos = 1.5e-6;
  double bLos = -0.0015;
  double cLos = 1.0;
  testVector.m_pLos =
      std::min(1.0, std::max(0.0, aLos * 10.0 * 10.0 + bLos * 10.0 + cLos));
  testVector.m_typeId =
      ProbabilisticV2vHighwayChannelConditionModel::GetTypeId();
  double aNlos = -2.9e-7;
  double bNlos = 0.00059;
  double cNlos = 0.0017;
  testVector.m_pNlos =
      std::min(1.0, std::max(0.0, aNlos * 10.0 * 10.0 + bNlos * 10.0 + cNlos));
  testVector.m_density = "Low";
  m_testVectors.Add(testVector);

  testVector.m_positionA = Vector(0, 0, 1.6);
  testVector.m_positionB = Vector(100, 0, 1.6);
  testVector.m_pLos =
      std::min(1.0, std::max(0.0, aLos * 100.0 * 100.0 + bLos * 100.0 + cLos));
  testVector.m_typeId =
      ProbabilisticV2vHighwayChannelConditionModel::GetTypeId();
  testVector.m_pNlos = std::min(
      1.0, std::max(0.0, aNlos * 100.0 * 100.0 + bNlos * 100.0 + cNlos));
  testVector.m_density = "Low";
  m_testVectors.Add(testVector);

  testVector.m_positionA = Vector(0, 0, 1.6);
  testVector.m_positionB = Vector(10, 0, 1.6);
  aLos = 2.7e-6;
  bLos = -0.0025;
  cLos = 1.0;
  testVector.m_pLos =
      std::min(1.0, std::max(0.0, aLos * 10.0 * 10.0 + bLos * 10.0 + cLos));
  testVector.m_typeId =
      ProbabilisticV2vHighwayChannelConditionModel::GetTypeId();
  aNlos = -3.7e-7;
  bNlos = 0.00061;
  cNlos = 0.015;
  testVector.m_pNlos =
      std::min(1.0, std::max(0.0, aNlos * 10.0 * 10.0 + bNlos * 10.0 + cNlos));
  testVector.m_density = "Medium";
  m_testVectors.Add(testVector);

  testVector.m_positionA = Vector(0, 0, 1.6);
  testVector.m_positionB = Vector(100, 0, 1.6);
  testVector.m_pLos =
      std::min(1.0, std::max(0.0, aLos * 100.0 * 100.0 + bLos * 100.0 + cLos));
  testVector.m_typeId =
      ProbabilisticV2vHighwayChannelConditionModel::GetTypeId();
  testVector.m_pNlos = std::min(
      1.0, std::max(0.0, aNlos * 100.0 * 100.0 + bNlos * 100.0 + cNlos));
  testVector.m_density = "Medium";
  m_testVectors.Add(testVector);

  testVector.m_positionA = Vector(0, 0, 1.6);
  testVector.m_positionB = Vector(10, 0, 1.6);
  aLos = 3.2e-6;
  bLos = -0.003;
  cLos = 1.0;
  testVector.m_pLos =
      std::min(1.0, std::max(0.0, aLos * 10.0 * 10.0 + bLos * 10.0 + cLos));
  testVector.m_typeId =
      ProbabilisticV2vHighwayChannelConditionModel::GetTypeId();
  aNlos = -4.1e-7;
  bNlos = 0.00067;
  cNlos = 0.0;
  testVector.m_pNlos =
      std::min(1.0, std::max(0.0, aNlos * 10.0 * 10.0 + bNlos * 10.0 + cNlos));
  testVector.m_density = "High";
  m_testVectors.Add(testVector);

  testVector.m_positionA = Vector(0, 0, 1.6);
  testVector.m_positionB = Vector(100, 0, 1.6);
  testVector.m_pLos =
      std::min(1.0, std::max(0.0, aLos * 100.0 * 100.0 + bLos * 100.0 + cLos));
  testVector.m_typeId =
      ProbabilisticV2vHighwayChannelConditionModel::GetTypeId();
  testVector.m_pNlos = std::min(
      1.0, std::max(0.0, aNlos * 100.0 * 100.0 + bNlos * 100.0 + cNlos));
  testVector.m_density = "High";
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
    m_condModel =
        condModelFactory.Create<ProbabilisticV2vHighwayChannelConditionModel>();
    m_condModel->SetAttribute("UpdatePeriod", TimeValue(MilliSeconds(9)));
    m_condModel->AssignStreams(1);
    m_condModel->SetAttribute("Density", StringValue(testVector.m_density));

    m_numLos = 0;
    m_numNlos = 0;
    for (uint32_t j = 0; j < numberOfReps; j++) {
      Simulator::Schedule(
          MilliSeconds(10 * j),
          &V2vHighwayProbChCondModelTestCase::EvaluateChannelCondition, this, a,
          b);
    }

    Simulator::Run();
    Simulator::Destroy();

    double resultPlos = double(m_numLos) / double(numberOfReps);
    double resultPnlos = double(m_numNlos) / double(numberOfReps);
    NS_LOG_DEBUG(testVector.m_typeId
                 << "  a pos " << testVector.m_positionA << " b pos "
                 << testVector.m_positionB << " numLos " << m_numLos
                 << " numberOfReps " << numberOfReps << " resultPlos "
                 << resultPlos << " ref " << testVector.m_pLos);
    NS_TEST_EXPECT_MSG_EQ_TOL(resultPlos, testVector.m_pLos, m_tolerance,
                              "Got unexpected LOS probability");
    NS_LOG_DEBUG(testVector.m_typeId
                 << "  a pos " << testVector.m_positionA << " b pos "
                 << testVector.m_positionB << " numNlos " << m_numNlos
                 << " numberOfReps " << numberOfReps << " resultPnlos "
                 << resultPnlos << " ref " << testVector.m_pNlos);
    NS_TEST_EXPECT_MSG_EQ_TOL(resultPnlos, testVector.m_pNlos, m_tolerance,
                              "Got unexpected NLOS probability");
  }
}

class ProbabilisticV2vChCondModelsTestSuite : public TestSuite {
public:
  ProbabilisticV2vChCondModelsTestSuite();
};

ProbabilisticV2vChCondModelsTestSuite::ProbabilisticV2vChCondModelsTestSuite()
    : TestSuite("probabilistic-v2v-channel-condition-model", SYSTEM) {
  AddTestCase(new V2vUrbanProbChCondModelTestCase, TestCase::QUICK);
  AddTestCase(new V2vHighwayProbChCondModelTestCase, TestCase::QUICK);
}

static ProbabilisticV2vChCondModelsTestSuite
    g_probabilisticV2vChCondModelsTestSuite;
