
#include "ns3/abort.h"
#include "ns3/boolean.h"
#include "ns3/channel-condition-model.h"
#include "ns3/config.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/constant-velocity-mobility-model.h"
#include "ns3/double.h"
#include "ns3/log.h"
#include "ns3/mobility-helper.h"
#include "ns3/simulator.h"
#include "ns3/test.h"
#include "ns3/three-gpp-propagation-loss-model.h"
#include "ns3/three-gpp-v2v-propagation-loss-model.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ThreeGppPropagationLossModelsTest");

class ThreeGppRmaPropagationLossModelTestCase : public TestCase {
public:
  ThreeGppRmaPropagationLossModelTestCase();

  ~ThreeGppRmaPropagationLossModelTestCase() override;

private:
  void DoRun() override;

  struct TestVector {
    double m_distance;
    bool m_isLos;
    double m_frequency;
    double m_pt;
    double m_pr;
  };

  TestVectors<TestVector> m_testVectors;
  double m_tolerance;
};

ThreeGppRmaPropagationLossModelTestCase::
    ThreeGppRmaPropagationLossModelTestCase()
    : TestCase("Test for the ThreeGppRmaPropagationLossModel class"),
      m_testVectors(), m_tolerance(5e-2) {}

ThreeGppRmaPropagationLossModelTestCase::
    ~ThreeGppRmaPropagationLossModelTestCase() {}

void ThreeGppRmaPropagationLossModelTestCase::DoRun() {
  TestVector testVector;

  testVector.m_distance = 10.0;
  testVector.m_isLos = true;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -77.3784;
  m_testVectors.Add(testVector);

  testVector.m_distance = 100.0;
  testVector.m_isLos = true;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -87.2965;
  m_testVectors.Add(testVector);

  testVector.m_distance = 1000.0;
  testVector.m_isLos = true;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -108.5577;
  m_testVectors.Add(testVector);

  testVector.m_distance = 10000.0;
  testVector.m_isLos = true;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -140.3896;
  m_testVectors.Add(testVector);

  testVector.m_distance = 10.0;
  testVector.m_isLos = false;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -77.3784;
  m_testVectors.Add(testVector);

  testVector.m_distance = 100.0;
  testVector.m_isLos = false;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -95.7718;
  m_testVectors.Add(testVector);

  testVector.m_distance = 1000.0;
  testVector.m_isLos = false;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -133.5223;
  m_testVectors.Add(testVector);

  testVector.m_distance = 5000.0;
  testVector.m_isLos = false;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -160.5169;
  m_testVectors.Add(testVector);

  NodeContainer nodes;
  nodes.Create(2);

  Ptr<MobilityModel> a = CreateObject<ConstantPositionMobilityModel>();
  nodes.Get(0)->AggregateObject(a);
  Ptr<MobilityModel> b = CreateObject<ConstantPositionMobilityModel>();
  nodes.Get(1)->AggregateObject(b);

  Ptr<ChannelConditionModel> losCondModel =
      CreateObject<AlwaysLosChannelConditionModel>();
  Ptr<ChannelConditionModel> nlosCondModel =
      CreateObject<NeverLosChannelConditionModel>();

  Ptr<ThreeGppRmaPropagationLossModel> lossModel =
      CreateObject<ThreeGppRmaPropagationLossModel>();
  lossModel->SetAttribute("ShadowingEnabled", BooleanValue(false));

  for (std::size_t i = 0; i < m_testVectors.GetN(); i++) {
    TestVector testVector = m_testVectors.Get(i);

    Vector posBs = Vector(0.0, 0.0, 35.0);
    Vector posUt = Vector(testVector.m_distance, 0.0, 1.5);

    if (testVector.m_isLos) {
      lossModel->SetChannelConditionModel(losCondModel);
    } else {
      lossModel->SetChannelConditionModel(nlosCondModel);
    }

    a->SetPosition(posBs);
    b->SetPosition(posUt);

    lossModel->SetAttribute("Frequency", DoubleValue(testVector.m_frequency));
    NS_TEST_EXPECT_MSG_EQ_TOL(lossModel->CalcRxPower(testVector.m_pt, a, b),
                              testVector.m_pr, m_tolerance,
                              "Got unexpected rcv power");
  }

  Simulator::Destroy();
}

class ThreeGppUmaPropagationLossModelTestCase : public TestCase {
public:
  ThreeGppUmaPropagationLossModelTestCase();

  ~ThreeGppUmaPropagationLossModelTestCase() override;

private:
  void DoRun() override;

  struct TestVector {
    double m_distance;
    bool m_isLos;
    double m_frequency;
    double m_pt;
    double m_pr;
  };

  TestVectors<TestVector> m_testVectors;
  double m_tolerance;
};

ThreeGppUmaPropagationLossModelTestCase::
    ThreeGppUmaPropagationLossModelTestCase()
    : TestCase("Test for the ThreeGppUmaPropagationLossModel class"),
      m_testVectors(), m_tolerance(5e-2) {}

ThreeGppUmaPropagationLossModelTestCase::
    ~ThreeGppUmaPropagationLossModelTestCase() {}

void ThreeGppUmaPropagationLossModelTestCase::DoRun() {
  TestVector testVector;

  testVector.m_distance = 10.0;
  testVector.m_isLos = true;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -72.9380;
  m_testVectors.Add(testVector);

  testVector.m_distance = 100.0;
  testVector.m_isLos = true;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -86.2362;
  m_testVectors.Add(testVector);

  testVector.m_distance = 1000.0;
  testVector.m_isLos = true;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -109.7252;
  m_testVectors.Add(testVector);

  testVector.m_distance = 5000.0;
  testVector.m_isLos = true;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -137.6794;
  m_testVectors.Add(testVector);

  testVector.m_distance = 10.0;
  testVector.m_isLos = false;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -82.5131;
  m_testVectors.Add(testVector);

  testVector.m_distance = 100.0;
  testVector.m_isLos = false;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -106.1356;
  m_testVectors.Add(testVector);

  testVector.m_distance = 1000.0;
  testVector.m_isLos = false;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -144.7641;
  m_testVectors.Add(testVector);

  testVector.m_distance = 5000.0;
  testVector.m_isLos = false;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -172.0753;
  m_testVectors.Add(testVector);

  NodeContainer nodes;
  nodes.Create(2);

  Ptr<MobilityModel> a = CreateObject<ConstantPositionMobilityModel>();
  nodes.Get(0)->AggregateObject(a);
  Ptr<MobilityModel> b = CreateObject<ConstantPositionMobilityModel>();
  nodes.Get(1)->AggregateObject(b);

  Ptr<ChannelConditionModel> losCondModel =
      CreateObject<AlwaysLosChannelConditionModel>();
  Ptr<ChannelConditionModel> nlosCondModel =
      CreateObject<NeverLosChannelConditionModel>();

  Ptr<ThreeGppUmaPropagationLossModel> lossModel =
      CreateObject<ThreeGppUmaPropagationLossModel>();
  lossModel->SetAttribute("ShadowingEnabled", BooleanValue(false));

  for (std::size_t i = 0; i < m_testVectors.GetN(); i++) {
    TestVector testVector = m_testVectors.Get(i);

    Vector posBs = Vector(0.0, 0.0, 25.0);
    Vector posUt = Vector(testVector.m_distance, 0.0, 1.5);

    if (testVector.m_isLos) {
      lossModel->SetChannelConditionModel(losCondModel);
    } else {
      lossModel->SetChannelConditionModel(nlosCondModel);
    }

    a->SetPosition(posBs);
    b->SetPosition(posUt);

    lossModel->SetAttribute("Frequency", DoubleValue(testVector.m_frequency));
    NS_TEST_EXPECT_MSG_EQ_TOL(lossModel->CalcRxPower(testVector.m_pt, a, b),
                              testVector.m_pr, m_tolerance,
                              "Got unexpected rcv power");
  }

  Simulator::Destroy();
}

class ThreeGppUmiPropagationLossModelTestCase : public TestCase {
public:
  ThreeGppUmiPropagationLossModelTestCase();

  ~ThreeGppUmiPropagationLossModelTestCase() override;

private:
  void DoRun() override;

  struct TestVector {
    double m_distance;
    bool m_isLos;
    double m_frequency;
    double m_pt;
    double m_pr;
  };

  TestVectors<TestVector> m_testVectors;
  double m_tolerance;
};

ThreeGppUmiPropagationLossModelTestCase::
    ThreeGppUmiPropagationLossModelTestCase()
    : TestCase("Test for the ThreeGppUmiPropagationLossModel class"),
      m_testVectors(), m_tolerance(5e-2) {}

ThreeGppUmiPropagationLossModelTestCase::
    ~ThreeGppUmiPropagationLossModelTestCase() {}

void ThreeGppUmiPropagationLossModelTestCase::DoRun() {
  TestVector testVector;

  testVector.m_distance = 10.0;
  testVector.m_isLos = true;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -69.8591;
  m_testVectors.Add(testVector);

  testVector.m_distance = 100.0;
  testVector.m_isLos = true;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -88.4122;
  m_testVectors.Add(testVector);

  testVector.m_distance = 1000.0;
  testVector.m_isLos = true;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -119.3114;

  testVector.m_distance = 5000.0;
  testVector.m_isLos = true;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -147.2696;

  testVector.m_distance = 10.0;
  testVector.m_isLos = false;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -76.7563;

  testVector.m_distance = 100.0;
  testVector.m_isLos = false;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -107.9432;

  testVector.m_distance = 1000.0;
  testVector.m_isLos = false;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -143.1886;

  testVector.m_distance = 5000.0;
  testVector.m_isLos = false;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -167.8617;

  NodeContainer nodes;
  nodes.Create(2);

  Ptr<MobilityModel> a = CreateObject<ConstantPositionMobilityModel>();
  nodes.Get(0)->AggregateObject(a);
  Ptr<MobilityModel> b = CreateObject<ConstantPositionMobilityModel>();
  nodes.Get(1)->AggregateObject(b);

  Ptr<ChannelConditionModel> losCondModel =
      CreateObject<AlwaysLosChannelConditionModel>();
  Ptr<ChannelConditionModel> nlosCondModel =
      CreateObject<NeverLosChannelConditionModel>();

  Ptr<ThreeGppUmiStreetCanyonPropagationLossModel> lossModel =
      CreateObject<ThreeGppUmiStreetCanyonPropagationLossModel>();
  lossModel->SetAttribute("ShadowingEnabled", BooleanValue(false));

  for (std::size_t i = 0; i < m_testVectors.GetN(); i++) {
    TestVector testVector = m_testVectors.Get(i);

    Vector posBs = Vector(0.0, 0.0, 10.0);
    Vector posUt = Vector(testVector.m_distance, 0.0, 1.5);

    if (testVector.m_isLos) {
      lossModel->SetChannelConditionModel(losCondModel);
    } else {
      lossModel->SetChannelConditionModel(nlosCondModel);
    }

    a->SetPosition(posBs);
    b->SetPosition(posUt);

    lossModel->SetAttribute("Frequency", DoubleValue(testVector.m_frequency));
    NS_TEST_EXPECT_MSG_EQ_TOL(lossModel->CalcRxPower(testVector.m_pt, a, b),
                              testVector.m_pr, m_tolerance,
                              "Got unexpected rcv power");
  }

  Simulator::Destroy();
}

class ThreeGppIndoorOfficePropagationLossModelTestCase : public TestCase {
public:
  ThreeGppIndoorOfficePropagationLossModelTestCase();

  ~ThreeGppIndoorOfficePropagationLossModelTestCase() override;

private:
  void DoRun() override;

  struct TestVector {
    double m_distance;
    bool m_isLos;
    double m_frequency;
    double m_pt;
    double m_pr;
  };

  TestVectors<TestVector> m_testVectors;
  double m_tolerance;
};

ThreeGppIndoorOfficePropagationLossModelTestCase::
    ThreeGppIndoorOfficePropagationLossModelTestCase()
    : TestCase("Test for the ThreeGppIndoorOfficePropagationLossModel class"),
      m_testVectors(), m_tolerance(5e-2) {}

ThreeGppIndoorOfficePropagationLossModelTestCase::
    ~ThreeGppIndoorOfficePropagationLossModelTestCase() {}

void ThreeGppIndoorOfficePropagationLossModelTestCase::DoRun() {
  TestVector testVector;

  testVector.m_distance = 1.0;
  testVector.m_isLos = true;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -50.8072;
  m_testVectors.Add(testVector);

  testVector.m_distance = 10.0;
  testVector.m_isLos = true;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -63.7630;
  m_testVectors.Add(testVector);

  testVector.m_distance = 50.0;
  testVector.m_isLos = true;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -75.7750;
  m_testVectors.Add(testVector);

  testVector.m_distance = 100.0;
  testVector.m_isLos = true;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -80.9802;
  m_testVectors.Add(testVector);

  testVector.m_distance = 1.0;
  testVector.m_isLos = false;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -50.8072;
  m_testVectors.Add(testVector);

  testVector.m_distance = 10.0;
  testVector.m_isLos = false;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -73.1894;
  m_testVectors.Add(testVector);

  testVector.m_distance = 50.0;
  testVector.m_isLos = false;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -99.7824;
  m_testVectors.Add(testVector);

  testVector.m_distance = 100.0;
  testVector.m_isLos = false;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -111.3062;
  m_testVectors.Add(testVector);

  NodeContainer nodes;
  nodes.Create(2);

  Ptr<MobilityModel> a = CreateObject<ConstantPositionMobilityModel>();
  nodes.Get(0)->AggregateObject(a);
  Ptr<MobilityModel> b = CreateObject<ConstantPositionMobilityModel>();
  nodes.Get(1)->AggregateObject(b);

  Ptr<ChannelConditionModel> losCondModel =
      CreateObject<AlwaysLosChannelConditionModel>();
  Ptr<ChannelConditionModel> nlosCondModel =
      CreateObject<NeverLosChannelConditionModel>();

  Ptr<ThreeGppIndoorOfficePropagationLossModel> lossModel =
      CreateObject<ThreeGppIndoorOfficePropagationLossModel>();
  lossModel->SetAttribute("ShadowingEnabled", BooleanValue(false));

  for (std::size_t i = 0; i < m_testVectors.GetN(); i++) {
    TestVector testVector = m_testVectors.Get(i);

    Vector posBs = Vector(0.0, 0.0, 3.0);
    Vector posUt = Vector(testVector.m_distance, 0.0, 1.5);

    if (testVector.m_isLos) {
      lossModel->SetChannelConditionModel(losCondModel);
    } else {
      lossModel->SetChannelConditionModel(nlosCondModel);
    }

    a->SetPosition(posBs);
    b->SetPosition(posUt);

    lossModel->SetAttribute("Frequency", DoubleValue(testVector.m_frequency));
    NS_TEST_EXPECT_MSG_EQ_TOL(lossModel->CalcRxPower(testVector.m_pt, a, b),
                              testVector.m_pr, m_tolerance,
                              "Got unexpected rcv power");
  }

  Simulator::Destroy();
}

class ThreeGppV2vUrbanPropagationLossModelTestCase : public TestCase {
public:
  ThreeGppV2vUrbanPropagationLossModelTestCase();

  ~ThreeGppV2vUrbanPropagationLossModelTestCase() override;

private:
  void DoRun() override;

  struct TestVector {
    double m_distance;
    bool m_isLos;
    double m_frequency;
    double m_pt;
    double m_pr;
  };

  TestVectors<TestVector> m_testVectors;
  double m_tolerance;
};

ThreeGppV2vUrbanPropagationLossModelTestCase::
    ThreeGppV2vUrbanPropagationLossModelTestCase()
    : TestCase("Test for the ThreeGppV2vUrbanPropagationLossModel class."),
      m_testVectors(), m_tolerance(5e-2) {}

ThreeGppV2vUrbanPropagationLossModelTestCase::
    ~ThreeGppV2vUrbanPropagationLossModelTestCase() {}

void ThreeGppV2vUrbanPropagationLossModelTestCase::DoRun() {
  TestVector testVector;

  testVector.m_distance = 10.0;
  testVector.m_isLos = true;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -68.1913;
  m_testVectors.Add(testVector);

  testVector.m_distance = 100.0;
  testVector.m_isLos = true;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -84.8913;
  m_testVectors.Add(testVector);

  testVector.m_distance = 1000.0;
  testVector.m_isLos = true;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -101.5913;
  m_testVectors.Add(testVector);

  testVector.m_distance = 10.0;
  testVector.m_isLos = false;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -80.0605;
  m_testVectors.Add(testVector);

  testVector.m_distance = 100.0;
  testVector.m_isLos = false;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -110.0605;
  m_testVectors.Add(testVector);

  testVector.m_distance = 1000.0;
  testVector.m_isLos = false;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -140.0605;
  m_testVectors.Add(testVector);

  NodeContainer nodes;
  nodes.Create(2);

  Ptr<MobilityModel> a = CreateObject<ConstantPositionMobilityModel>();
  nodes.Get(0)->AggregateObject(a);
  Ptr<MobilityModel> b = CreateObject<ConstantPositionMobilityModel>();
  nodes.Get(1)->AggregateObject(b);

  Ptr<ChannelConditionModel> losCondModel =
      CreateObject<AlwaysLosChannelConditionModel>();
  Ptr<ChannelConditionModel> nlosCondModel =
      CreateObject<NeverLosChannelConditionModel>();

  Ptr<ThreeGppPropagationLossModel> lossModel =
      CreateObject<ThreeGppV2vUrbanPropagationLossModel>();
  lossModel->SetAttribute("ShadowingEnabled", BooleanValue(false));

  for (std::size_t i = 0; i < m_testVectors.GetN(); i++) {
    TestVector testVector = m_testVectors.Get(i);

    Vector posUe1 = Vector(0.0, 0.0, 1.6);
    Vector posUe2 = Vector(testVector.m_distance, 0.0, 1.6);

    if (testVector.m_isLos) {
      lossModel->SetChannelConditionModel(losCondModel);
    } else {
      lossModel->SetChannelConditionModel(nlosCondModel);
    }

    a->SetPosition(posUe1);
    b->SetPosition(posUe2);

    lossModel->SetAttribute("Frequency", DoubleValue(testVector.m_frequency));
    NS_TEST_EXPECT_MSG_EQ_TOL(lossModel->CalcRxPower(testVector.m_pt, a, b),
                              testVector.m_pr, m_tolerance,
                              "Got unexpected rcv power");
  }

  Simulator::Destroy();
}

class ThreeGppV2vHighwayPropagationLossModelTestCase : public TestCase {
public:
  ThreeGppV2vHighwayPropagationLossModelTestCase();

  ~ThreeGppV2vHighwayPropagationLossModelTestCase() override;

private:
  void DoRun() override;

  struct TestVector {
    double m_distance;
    bool m_isLos;
    double m_frequency;
    double m_pt;
    double m_pr;
  };

  TestVectors<TestVector> m_testVectors;
  double m_tolerance;
};

ThreeGppV2vHighwayPropagationLossModelTestCase::
    ThreeGppV2vHighwayPropagationLossModelTestCase()
    : TestCase("Test for the ThreeGppV2vHighwayPropagationLossModel"),
      m_testVectors(), m_tolerance(5e-2) {}

ThreeGppV2vHighwayPropagationLossModelTestCase::
    ~ThreeGppV2vHighwayPropagationLossModelTestCase() {}

void ThreeGppV2vHighwayPropagationLossModelTestCase::DoRun() {
  TestVector testVector;

  testVector.m_distance = 10.0;
  testVector.m_isLos = true;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -66.3794;
  m_testVectors.Add(testVector);

  testVector.m_distance = 100.0;
  testVector.m_isLos = true;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -86.3794;
  m_testVectors.Add(testVector);

  testVector.m_distance = 1000.0;
  testVector.m_isLos = true;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -106.3794;
  m_testVectors.Add(testVector);

  testVector.m_distance = 10.0;
  testVector.m_isLos = false;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -80.0605;
  m_testVectors.Add(testVector);

  testVector.m_distance = 100.0;
  testVector.m_isLos = false;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -110.0605;
  m_testVectors.Add(testVector);

  testVector.m_distance = 1000.0;
  testVector.m_isLos = false;
  testVector.m_frequency = 5.0e9;
  testVector.m_pt = 0.0;
  testVector.m_pr = -140.0605;
  m_testVectors.Add(testVector);

  NodeContainer nodes;
  nodes.Create(2);

  Ptr<MobilityModel> a = CreateObject<ConstantPositionMobilityModel>();
  nodes.Get(0)->AggregateObject(a);
  Ptr<MobilityModel> b = CreateObject<ConstantPositionMobilityModel>();
  nodes.Get(1)->AggregateObject(b);

  Ptr<ChannelConditionModel> losCondModel =
      CreateObject<AlwaysLosChannelConditionModel>();
  Ptr<ChannelConditionModel> nlosCondModel =
      CreateObject<NeverLosChannelConditionModel>();

  Ptr<ThreeGppPropagationLossModel> lossModel =
      CreateObject<ThreeGppV2vHighwayPropagationLossModel>();
  lossModel->SetAttribute("ShadowingEnabled", BooleanValue(false));

  for (std::size_t i = 0; i < m_testVectors.GetN(); i++) {
    TestVector testVector = m_testVectors.Get(i);

    Vector posUe1 = Vector(0.0, 0.0, 1.6);
    Vector posUe2 = Vector(testVector.m_distance, 0.0, 1.6);

    if (testVector.m_isLos) {
      lossModel->SetChannelConditionModel(losCondModel);
    } else {
      lossModel->SetChannelConditionModel(nlosCondModel);
    }

    a->SetPosition(posUe1);
    b->SetPosition(posUe2);

    lossModel->SetAttribute("Frequency", DoubleValue(testVector.m_frequency));
    NS_TEST_EXPECT_MSG_EQ_TOL(lossModel->CalcRxPower(testVector.m_pt, a, b),
                              testVector.m_pr, m_tolerance,
                              "Got unexpected rcv power");
  }

  Simulator::Destroy();
}

class ThreeGppShadowingTestCase : public TestCase {
public:
  ThreeGppShadowingTestCase();
  ~ThreeGppShadowingTestCase() override;

private:
  void DoRun() override;

  void RunTest(uint16_t testNum, std::string propagationLossModelType,
               double hBs, double hUt, double distance, bool shadowingEnabled);

  void EvaluateLoss(Ptr<MobilityModel> a, Ptr<MobilityModel> b,
                    uint8_t testNum);

  void ChangeChannelCondition(Ptr<ChannelConditionModel> ccm);

  struct TestVector {
    std::string m_propagationLossModelType;
    double m_hBs;
    double m_hUt;
    double m_distance;
    double m_shadowingStdLos;
    double m_shadowingStdNlos;
  };

  TestVectors<TestVector> m_testVectors;
  Ptr<ThreeGppPropagationLossModel> m_lossModel;
  std::map<uint16_t, std::vector<double>> m_results;
};

ThreeGppShadowingTestCase::ThreeGppShadowingTestCase()
    : TestCase("Test to check if the shadow fading is correctly computed") {}

ThreeGppShadowingTestCase::~ThreeGppShadowingTestCase() {}

void ThreeGppShadowingTestCase::EvaluateLoss(Ptr<MobilityModel> a,
                                             Ptr<MobilityModel> b,
                                             uint8_t testNum) {
  double loss = m_lossModel->CalcRxPower(0, a, b);
  m_results.at(testNum).push_back(loss);
}

void ThreeGppShadowingTestCase::ChangeChannelCondition(
    Ptr<ChannelConditionModel> ccm) {
  m_lossModel->SetChannelConditionModel(ccm);
}

void ThreeGppShadowingTestCase::RunTest(uint16_t testNum,
                                        std::string propagationLossModelType,
                                        double hBs, double hUt, double distance,
                                        bool shadowingEnabled) {
  m_results[testNum] = std::vector<double>();

  NodeContainer nodes;
  nodes.Create(2);

  Ptr<MobilityModel> a = CreateObject<ConstantPositionMobilityModel>();
  a->SetPosition(Vector(0.0, 0.0, hBs));
  nodes.Get(0)->AggregateObject(a);

  Ptr<ConstantVelocityMobilityModel> b =
      CreateObject<ConstantVelocityMobilityModel>();
  nodes.Get(1)->AggregateObject(b);
  b->SetPosition(Vector(0.0, distance, hUt));
  b->SetVelocity(Vector(1.0, 0.0, 0.0));

  ObjectFactory propagationLossModelFactory =
      ObjectFactory(propagationLossModelType);
  m_lossModel =
      propagationLossModelFactory.Create<ThreeGppPropagationLossModel>();
  m_lossModel->SetAttribute("Frequency", DoubleValue(3.5e9));
  m_lossModel->SetAttribute("ShadowingEnabled", BooleanValue(shadowingEnabled));

  Ptr<ChannelConditionModel> losCondModel =
      CreateObject<AlwaysLosChannelConditionModel>();
  m_lossModel->SetChannelConditionModel(losCondModel);
  Ptr<ChannelConditionModel> nlosCondModel =
      CreateObject<NeverLosChannelConditionModel>();
  Simulator::Schedule(Seconds(99.5),
                      &ThreeGppShadowingTestCase::ChangeChannelCondition, this,
                      nlosCondModel);

  for (int i = 0; i < 200; i++) {
    if (i % 2 == 0) {
      Simulator::Schedule(MilliSeconds(1000 * i),
                          &ThreeGppShadowingTestCase::EvaluateLoss, this, a, b,
                          testNum);
    } else {
      Simulator::Schedule(MilliSeconds(1000 * i),
                          &ThreeGppShadowingTestCase::EvaluateLoss, this, b, a,
                          testNum);
    }
  }

  Simulator::Run();
  Simulator::Destroy();
}

void ThreeGppShadowingTestCase::DoRun() {

  TestVector testVector;
  testVector.m_propagationLossModelType =
      "ns3::ThreeGppRmaPropagationLossModel";
  testVector.m_hBs = 25;
  testVector.m_hUt = 1.6;
  testVector.m_distance = 100;
  testVector.m_shadowingStdLos = 4;
  testVector.m_shadowingStdNlos = 8;
  m_testVectors.Add(testVector);

  testVector.m_propagationLossModelType =
      "ns3::ThreeGppRmaPropagationLossModel";
  testVector.m_hBs = 25;
  testVector.m_hUt = 1.6;
  testVector.m_distance = 4000;
  testVector.m_shadowingStdLos = 6;
  testVector.m_shadowingStdNlos = 8;
  m_testVectors.Add(testVector);

  testVector.m_propagationLossModelType =
      "ns3::ThreeGppUmaPropagationLossModel";
  testVector.m_hBs = 25;
  testVector.m_hUt = 1.6;
  testVector.m_distance = 100;
  testVector.m_shadowingStdLos = 4;
  testVector.m_shadowingStdNlos = 6;
  m_testVectors.Add(testVector);

  testVector.m_propagationLossModelType =
      "ns3::ThreeGppUmiStreetCanyonPropagationLossModel";
  testVector.m_hBs = 10;
  testVector.m_hUt = 1.6;
  testVector.m_distance = 100;
  testVector.m_shadowingStdLos = 4;
  testVector.m_shadowingStdNlos = 7.82;
  m_testVectors.Add(testVector);

  testVector.m_propagationLossModelType =
      "ns3::ThreeGppIndoorOfficePropagationLossModel";
  testVector.m_hBs = 3;
  testVector.m_hUt = 1;
  testVector.m_distance = 50;
  testVector.m_shadowingStdLos = 3;
  testVector.m_shadowingStdNlos = 8.03;
  m_testVectors.Add(testVector);

  testVector.m_propagationLossModelType =
      "ns3::ThreeGppV2vUrbanPropagationLossModel";
  testVector.m_hBs = 1.6;
  testVector.m_hUt = 1.6;
  testVector.m_distance = 50;
  testVector.m_shadowingStdLos = 3;
  testVector.m_shadowingStdNlos = 4;
  m_testVectors.Add(testVector);

  testVector.m_propagationLossModelType =
      "ns3::ThreeGppV2vHighwayPropagationLossModel";
  testVector.m_hBs = 1.6;
  testVector.m_hUt = 1.6;
  testVector.m_distance = 50;
  testVector.m_shadowingStdLos = 3;
  testVector.m_shadowingStdNlos = 4;
  m_testVectors.Add(testVector);

  uint16_t numSamples = 250;

  for (std::size_t tvIndex = 0; tvIndex < m_testVectors.GetN(); tvIndex++) {
    TestVector tv = m_testVectors.Get(tvIndex);

    for (uint16_t sampleIndex = 0; sampleIndex < numSamples; sampleIndex++) {
      RunTest(sampleIndex, tv.m_propagationLossModelType, tv.m_hBs, tv.m_hUt,
              tv.m_distance, true);
    }

    std::vector<double> mean_vector;

    uint16_t numPositions = m_results.at(0).size();
    for (uint16_t k = 0; k < numPositions; k++) {
      double mean = 0.0;
      for (auto resIt : m_results) {
        mean += resIt.second.at(k);
      }
      mean /= m_results.size();
      mean_vector.push_back(mean);
    }

    RunTest(numSamples, tv.m_propagationLossModelType, tv.m_hBs, tv.m_hUt,
            tv.m_distance, false);
    std::vector<double> true_mean = m_results.at(numSamples);

    for (std::size_t i = 0; i < mean_vector.size() / 2; i++) {
      double z = (mean_vector.at(i) - true_mean.at(i)) /
                 (tv.m_shadowingStdLos / std::sqrt(mean_vector.size() / 2));
      NS_TEST_EXPECT_MSG_EQ_TOL(z, 0.0, 1.96,
                                "Null hypothesis test (LOS case) for the "
                                "shadowing component rejected");
    }

    for (std::size_t i = mean_vector.size() / 2; i < mean_vector.size(); i++) {
      double z = (mean_vector.at(i) - true_mean.at(i)) /
                 (tv.m_shadowingStdNlos / std::sqrt(mean_vector.size() / 2));
      NS_TEST_EXPECT_MSG_EQ_TOL(z, 0.0, 1.96,
                                "Null hypothesis test (NLOS case) for the "
                                "shadowing component rejected");
    }
  }
}

class ThreeGppPropagationLossModelsTestSuite : public TestSuite {
public:
  ThreeGppPropagationLossModelsTestSuite();
};

ThreeGppPropagationLossModelsTestSuite::ThreeGppPropagationLossModelsTestSuite()
    : TestSuite("three-gpp-propagation-loss-model", UNIT) {
  AddTestCase(new ThreeGppRmaPropagationLossModelTestCase, TestCase::QUICK);
  AddTestCase(new ThreeGppUmaPropagationLossModelTestCase, TestCase::QUICK);
  AddTestCase(new ThreeGppUmiPropagationLossModelTestCase, TestCase::QUICK);
  AddTestCase(new ThreeGppIndoorOfficePropagationLossModelTestCase,
              TestCase::QUICK);
  AddTestCase(new ThreeGppV2vUrbanPropagationLossModelTestCase,
              TestCase::QUICK);
  AddTestCase(new ThreeGppV2vHighwayPropagationLossModelTestCase,
              TestCase::QUICK);
  AddTestCase(new ThreeGppShadowingTestCase, TestCase::QUICK);
}

static ThreeGppPropagationLossModelsTestSuite g_propagationLossModelsTestSuite;
