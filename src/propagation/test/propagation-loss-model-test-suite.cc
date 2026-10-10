
#include "ns3/abort.h"
#include "ns3/config.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/double.h"
#include "ns3/log.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/simulator.h"
#include "ns3/test.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("PropagationLossModelsTest");

class FriisPropagationLossModelTestCase : public TestCase {
public:
  FriisPropagationLossModelTestCase();
  ~FriisPropagationLossModelTestCase() override;

private:
  void DoRun() override;

  struct TestVector {
    Vector m_position;
    double m_pt;
    double m_pr;
    double m_tolerance;
  };

  TestVectors<TestVector> m_testVectors;
};

FriisPropagationLossModelTestCase::FriisPropagationLossModelTestCase()
    : TestCase("Check to see that the ns-3 Friis propagation loss model "
               "provides correct received "
               "power"),
      m_testVectors() {}

FriisPropagationLossModelTestCase::~FriisPropagationLossModelTestCase() {}

void FriisPropagationLossModelTestCase::DoRun() {

  Config::SetDefault("ns3::FriisPropagationLossModel::Frequency",
                     DoubleValue(2398339664.0));
  Config::SetDefault("ns3::FriisPropagationLossModel::SystemLoss",
                     DoubleValue(1.0));

  double txPowerW = 0.05035702;
  double txPowerdBm = 10 * std::log10(txPowerW) + 30;

  TestVector testVector;

  testVector.m_position = Vector(100, 0, 0);
  testVector.m_pt = txPowerdBm;
  testVector.m_pr = 4.98265e-10;
  testVector.m_tolerance = 5e-16;
  m_testVectors.Add(testVector);

  testVector.m_position = Vector(500, 0, 0);
  testVector.m_pt = txPowerdBm;
  testVector.m_pr = 1.99306e-11;
  testVector.m_tolerance = 5e-17;
  m_testVectors.Add(testVector);

  testVector.m_position = Vector(1000, 0, 0);
  testVector.m_pt = txPowerdBm;
  testVector.m_pr = 4.98265e-12;
  testVector.m_tolerance = 5e-18;
  m_testVectors.Add(testVector);

  testVector.m_position = Vector(2000, 0, 0);
  testVector.m_pt = txPowerdBm;
  testVector.m_pr = 1.24566e-12;
  testVector.m_tolerance = 5e-18;
  m_testVectors.Add(testVector);

  Ptr<MobilityModel> a = CreateObject<ConstantPositionMobilityModel>();
  a->SetPosition(Vector(0, 0, 0));
  Ptr<MobilityModel> b = CreateObject<ConstantPositionMobilityModel>();

  Ptr<FriisPropagationLossModel> lossModel =
      CreateObject<FriisPropagationLossModel>();
  for (uint32_t i = 0; i < m_testVectors.GetN(); ++i) {
    testVector = m_testVectors.Get(i);
    b->SetPosition(testVector.m_position);
    double resultdBm = lossModel->CalcRxPower(testVector.m_pt, a, b);
    double resultW = std::pow(10.0, resultdBm / 10.0) / 1000;
    NS_TEST_EXPECT_MSG_EQ_TOL(resultW, testVector.m_pr, testVector.m_tolerance,
                              "Got unexpected rcv power");
  }
}

class TwoRayGroundPropagationLossModelTestCase : public TestCase {
public:
  TwoRayGroundPropagationLossModelTestCase();
  ~TwoRayGroundPropagationLossModelTestCase() override;

private:
  void DoRun() override;

  struct TestVector {
    Vector m_position;
    double m_pt;
    double m_pr;
    double m_tolerance;
  };

  TestVectors<TestVector> m_testVectors;
};

TwoRayGroundPropagationLossModelTestCase::
    TwoRayGroundPropagationLossModelTestCase()
    : TestCase("Check to see that the ns-3 TwoRayGround propagation loss model "
               "provides correct "
               "received power"),
      m_testVectors() {}

TwoRayGroundPropagationLossModelTestCase::
    ~TwoRayGroundPropagationLossModelTestCase() {}

void TwoRayGroundPropagationLossModelTestCase::DoRun() {
  Config::SetDefault("ns3::TwoRayGroundPropagationLossModel::Frequency",
                     DoubleValue(2398339664.0));
  Config::SetDefault("ns3::TwoRayGroundPropagationLossModel::SystemLoss",
                     DoubleValue(1.0));

  Config::SetDefault("ns3::TwoRayGroundPropagationLossModel::HeightAboveZ",
                     DoubleValue(1.5));

  double txPowerW = 0.05035702;
  double txPowerdBm = 10 * std::log10(txPowerW) + 30;

  TestVector testVector;

  testVector.m_position = Vector(100, 0, 0);
  testVector.m_pt = txPowerdBm;
  testVector.m_pr = 4.98265e-10;
  testVector.m_tolerance = 5e-16;
  m_testVectors.Add(testVector);

  testVector.m_position = Vector(500, 0, 0);
  testVector.m_pt = txPowerdBm;
  testVector.m_pr = 4.07891862e-12;
  testVector.m_tolerance = 5e-16;
  m_testVectors.Add(testVector);

  testVector.m_position = Vector(1000, 0, 0);
  testVector.m_pt = txPowerdBm;
  testVector.m_pr = 2.5493241375e-13;
  testVector.m_tolerance = 5e-16;
  m_testVectors.Add(testVector);

  testVector.m_position = Vector(2000, 0, 0);
  testVector.m_pt = txPowerdBm;
  testVector.m_pr = 1.593327585938e-14;
  testVector.m_tolerance = 5e-16;
  m_testVectors.Add(testVector);

  testVector.m_position = Vector(500, 0, 1);
  testVector.m_pt = txPowerdBm;
  testVector.m_pr = 1.13303295e-11;
  testVector.m_tolerance = 5e-16;
  m_testVectors.Add(testVector);

  testVector.m_position = Vector(1000, 0, 4);
  testVector.m_pt = txPowerdBm;
  testVector.m_pr = 3.42742467375e-12;
  testVector.m_tolerance = 5e-16;
  m_testVectors.Add(testVector);

  testVector.m_position = Vector(2000, 0, 10);
  testVector.m_pt = txPowerdBm;
  testVector.m_pr = 9.36522547734e-13;
  testVector.m_tolerance = 5e-16;
  m_testVectors.Add(testVector);

  Ptr<MobilityModel> a = CreateObject<ConstantPositionMobilityModel>();
  a->SetPosition(Vector(0, 0, 0));
  Ptr<MobilityModel> b = CreateObject<ConstantPositionMobilityModel>();

  Ptr<TwoRayGroundPropagationLossModel> lossModel =
      CreateObject<TwoRayGroundPropagationLossModel>();
  for (uint32_t i = 0; i < m_testVectors.GetN(); ++i) {
    testVector = m_testVectors.Get(i);
    b->SetPosition(testVector.m_position);
    double resultdBm = lossModel->CalcRxPower(testVector.m_pt, a, b);
    double resultW = std::pow(10.0, resultdBm / 10.0) / 1000;
    NS_TEST_EXPECT_MSG_EQ_TOL(resultW, testVector.m_pr, testVector.m_tolerance,
                              "Got unexpected rcv power");
  }
}

class LogDistancePropagationLossModelTestCase : public TestCase {
public:
  LogDistancePropagationLossModelTestCase();
  ~LogDistancePropagationLossModelTestCase() override;

private:
  void DoRun() override;

  struct TestVector {
    Vector m_position;
    double m_pt;
    double m_pr;
    double m_tolerance;
  };

  TestVectors<TestVector> m_testVectors;
};

LogDistancePropagationLossModelTestCase::
    LogDistancePropagationLossModelTestCase()
    : TestCase("Check to see that the ns-3 Log Distance propagation loss model "
               "provides correct "
               "received power"),
      m_testVectors() {}

LogDistancePropagationLossModelTestCase::
    ~LogDistancePropagationLossModelTestCase() {}

void LogDistancePropagationLossModelTestCase::DoRun() {
  Config::SetDefault("ns3::LogDistancePropagationLossModel::ReferenceLoss",
                     DoubleValue(40.045997));
  Config::SetDefault("ns3::LogDistancePropagationLossModel::Exponent",
                     DoubleValue(3));

  double txPowerW = 0.05035702;
  double txPowerdBm = 10 * std::log10(txPowerW) + 30;

  TestVector testVector;

  testVector.m_position = Vector(10, 0, 0);
  testVector.m_pt = txPowerdBm;
  testVector.m_pr = 4.98265e-9;
  testVector.m_tolerance = 5e-15;
  m_testVectors.Add(testVector);

  testVector.m_position = Vector(20, 0, 0);
  testVector.m_pt = txPowerdBm;
  testVector.m_pr = 6.22831e-10;
  testVector.m_tolerance = 5e-16;
  m_testVectors.Add(testVector);

  testVector.m_position = Vector(40, 0, 0);
  testVector.m_pt = txPowerdBm;
  testVector.m_pr = 7.78539e-11;
  testVector.m_tolerance = 5e-17;
  m_testVectors.Add(testVector);

  testVector.m_position = Vector(80, 0, 0);
  testVector.m_pt = txPowerdBm;
  testVector.m_pr = 9.73173e-12;
  testVector.m_tolerance = 5e-17;
  m_testVectors.Add(testVector);

  Ptr<MobilityModel> a = CreateObject<ConstantPositionMobilityModel>();
  a->SetPosition(Vector(0, 0, 0));
  Ptr<MobilityModel> b = CreateObject<ConstantPositionMobilityModel>();

  Ptr<LogDistancePropagationLossModel> lossModel =
      CreateObject<LogDistancePropagationLossModel>();
  for (uint32_t i = 0; i < m_testVectors.GetN(); ++i) {
    testVector = m_testVectors.Get(i);
    b->SetPosition(testVector.m_position);
    double resultdBm = lossModel->CalcRxPower(testVector.m_pt, a, b);
    double resultW = std::pow(10.0, resultdBm / 10.0) / 1000;
    NS_TEST_EXPECT_MSG_EQ_TOL(resultW, testVector.m_pr, testVector.m_tolerance,
                              "Got unexpected rcv power");
  }
}

class MatrixPropagationLossModelTestCase : public TestCase {
public:
  MatrixPropagationLossModelTestCase();
  ~MatrixPropagationLossModelTestCase() override;

private:
  void DoRun() override;
};

MatrixPropagationLossModelTestCase::MatrixPropagationLossModelTestCase()
    : TestCase("Test MatrixPropagationLossModel") {}

MatrixPropagationLossModelTestCase::~MatrixPropagationLossModelTestCase() {}

void MatrixPropagationLossModelTestCase::DoRun() {
  Ptr<MobilityModel> m[3];
  for (int i = 0; i < 3; ++i) {
    m[i] = CreateObject<ConstantPositionMobilityModel>();
  }

  MatrixPropagationLossModel loss;
  loss.SetDefaultLoss(0);
  loss.SetLoss(m[0], m[1], 10);
  loss.SetLoss(m[0], m[2], 30, false);
  loss.SetLoss(m[2], m[0], 100, false);

  NS_TEST_ASSERT_MSG_EQ(loss.CalcRxPower(0, m[0], m[1]), -10,
                        "Loss 0 -> 1 incorrect");
  NS_TEST_ASSERT_MSG_EQ(loss.CalcRxPower(0, m[1], m[0]), -10,
                        "Loss 1 -> 0 incorrect");
  NS_TEST_ASSERT_MSG_EQ(loss.CalcRxPower(0, m[0], m[2]), -30,
                        "Loss 0 -> 2 incorrect");
  NS_TEST_ASSERT_MSG_EQ(loss.CalcRxPower(0, m[2], m[0]), -100,
                        "Loss 2 -> 0 incorrect");
  NS_TEST_ASSERT_MSG_EQ(loss.CalcRxPower(0, m[1], m[2]), 0,
                        "Loss 1 -> 2 incorrect");
  NS_TEST_ASSERT_MSG_EQ(loss.CalcRxPower(0, m[2], m[1]), 0,
                        "Loss 2 -> 1 incorrect");

  Simulator::Destroy();
}

class RangePropagationLossModelTestCase : public TestCase {
public:
  RangePropagationLossModelTestCase();
  ~RangePropagationLossModelTestCase() override;

private:
  void DoRun() override;
};

RangePropagationLossModelTestCase::RangePropagationLossModelTestCase()
    : TestCase("Test RangePropagationLossModel") {}

RangePropagationLossModelTestCase::~RangePropagationLossModelTestCase() {}

void RangePropagationLossModelTestCase::DoRun() {
  Config::SetDefault("ns3::RangePropagationLossModel::MaxRange",
                     DoubleValue(127.2));
  Ptr<MobilityModel> a = CreateObject<ConstantPositionMobilityModel>();
  a->SetPosition(Vector(0, 0, 0));
  Ptr<MobilityModel> b = CreateObject<ConstantPositionMobilityModel>();
  b->SetPosition(Vector(127.1, 0, 0));

  Ptr<RangePropagationLossModel> lossModel =
      CreateObject<RangePropagationLossModel>();

  double txPwrdBm = -80.0;
  double tolerance = 1e-6;
  double resultdBm = lossModel->CalcRxPower(txPwrdBm, a, b);
  NS_TEST_EXPECT_MSG_EQ_TOL(resultdBm, txPwrdBm, tolerance,
                            "Got unexpected rcv power");
  b->SetPosition(Vector(127.25, 0, 0));
  resultdBm = lossModel->CalcRxPower(txPwrdBm, a, b);
  NS_TEST_EXPECT_MSG_EQ_TOL(resultdBm, -1000.0, tolerance,
                            "Got unexpected rcv power");
  Simulator::Destroy();
}

class PropagationLossModelsTestSuite : public TestSuite {
public:
  PropagationLossModelsTestSuite();
};

PropagationLossModelsTestSuite::PropagationLossModelsTestSuite()
    : TestSuite("propagation-loss-model", UNIT) {
  AddTestCase(new FriisPropagationLossModelTestCase, TestCase::QUICK);
  AddTestCase(new TwoRayGroundPropagationLossModelTestCase, TestCase::QUICK);
  AddTestCase(new LogDistancePropagationLossModelTestCase, TestCase::QUICK);
  AddTestCase(new MatrixPropagationLossModelTestCase, TestCase::QUICK);
  AddTestCase(new RangePropagationLossModelTestCase, TestCase::QUICK);
}

static PropagationLossModelsTestSuite g_propagationLossModelsTestSuite;
