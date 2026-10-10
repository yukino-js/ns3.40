
#include "buildings-shadowing-test.h"

#include <ns3/building.h>
#include <ns3/buildings-helper.h>
#include <ns3/constant-position-mobility-model.h>
#include <ns3/double.h>
#include <ns3/enum.h>
#include <ns3/hybrid-buildings-propagation-loss-model.h>
#include <ns3/log.h>
#include <ns3/mobility-building-info.h>
#include <ns3/mobility-model.h>
#include <ns3/ptr.h>
#include <ns3/simulator.h>
#include <ns3/string.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("BuildingsShadowingTest");

BuildingsShadowingTestSuite::BuildingsShadowingTestSuite()
    : TestSuite("buildings-shadowing-test", SYSTEM) {
  LogComponentEnable("BuildingsShadowingTest", LOG_LEVEL_ALL);

  AddTestCase(
      new BuildingsShadowingTestCase(1, 2, 148.86, 7.0, "Outdoor Shadowing"),
      TestCase::QUICK);

  AddTestCase(
      new BuildingsShadowingTestCase(5, 6, 88.5724, 8.0, "Indoor Shadowing"),
      TestCase::QUICK);

  AddTestCase(new BuildingsShadowingTestCase(9, 10, 85.0012, 8.6,
                                             "Indoor -> Outdoor Shadowing"),
              TestCase::QUICK);
}

static BuildingsShadowingTestSuite buildingsShadowingTestSuite;

BuildingsShadowingTestCase::BuildingsShadowingTestCase(uint16_t m1, uint16_t m2,
                                                       double refValue,
                                                       double sigmaRef,
                                                       std::string name)
    : TestCase("SHADOWING calculation: " + name), m_mobilityModelIndex1(m1),
      m_mobilityModelIndex2(m2), m_lossRef(refValue), m_sigmaRef(sigmaRef) {}

BuildingsShadowingTestCase::~BuildingsShadowingTestCase() {}

void BuildingsShadowingTestCase::DoRun() {
  NS_LOG_FUNCTION(this);

  Ptr<Building> building1 = CreateObject<Building>();
  building1->SetBoundaries(Box(-3000, -1, -4000, 4000.0, 0.0, 12));
  building1->SetBuildingType(Building::Residential);
  building1->SetExtWallsType(Building::ConcreteWithWindows);
  building1->SetNFloors(3);

  Ptr<HybridBuildingsPropagationLossModel> propagationLossModel =
      CreateObject<HybridBuildingsPropagationLossModel>();

  std::vector<double> loss;
  double sum = 0.0;
  double sumSquared = 0.0;
  int samples = 1000;
  for (int i = 0; i < samples; i++) {
    Ptr<MobilityModel> mma = CreateMobilityModel(m_mobilityModelIndex1);
    Ptr<MobilityModel> mmb = CreateMobilityModel(m_mobilityModelIndex2);
    double shadowingLoss =
        propagationLossModel->DoCalcRxPower(0.0, mma, mmb) + m_lossRef;
    double shadowingLoss2 =
        propagationLossModel->DoCalcRxPower(0.0, mma, mmb) + m_lossRef;
    NS_TEST_ASSERT_MSG_EQ_TOL(
        shadowingLoss, shadowingLoss2, 0.001,
        "Shadowing is not constant for the same mobility model pair!");
    loss.push_back(shadowingLoss);
    sum += shadowingLoss;
    sumSquared += (shadowingLoss * shadowingLoss);
  }
  double sampleMean = sum / samples;
  double sampleVariance = (sumSquared - (sum * sum / samples)) / (samples - 1);
  double sampleStd = std::sqrt(sampleVariance);

  const double zn995 = 2.575829303549;
  double ci = (zn995 * sampleStd) / std::sqrt(samples);
  NS_LOG_INFO("SampleMean from simulation "
              << sampleMean << ", sampleStd " << sampleStd
              << ", reference value " << m_sigmaRef << ", CI(99%) " << ci);
  NS_TEST_ASSERT_MSG_EQ_TOL(std::fabs(sampleMean), 0.0, ci,
                            "Wrong shadowing distribution !");

  double chi2 = (samples - 1) * sampleVariance / (m_sigmaRef * m_sigmaRef);
  const double zchi2_005 = 887.621135217515;
  const double zchi2_995 = 1117.89045267865;
  NS_TEST_ASSERT_MSG_GT(chi2, zchi2_005,
                        "sample variance lesser than expected");
  NS_TEST_ASSERT_MSG_LT(chi2, zchi2_995,
                        "sample variance greater than expected");

  Simulator::Destroy();
}

Ptr<MobilityModel>
BuildingsShadowingTestCase::CreateMobilityModel(uint16_t index) {

  double hm = 1;
  double hb = 30;
  double henbHeight = 10.0;

  Ptr<MobilityModel> mm;

  switch (index) {
  case 1:
    mm = CreateObject<ConstantPositionMobilityModel>();
    mm->SetPosition(Vector(0.0, 0.0, hb));
    break;

  case 2:
    mm = CreateObject<ConstantPositionMobilityModel>();
    mm->SetPosition(Vector(2000, 0.0, hm));
    break;

  case 3:
    mm = CreateObject<ConstantPositionMobilityModel>();
    mm->SetPosition(Vector(100, 0.0, hm));
    break;

  case 4:
    mm = CreateObject<ConstantPositionMobilityModel>();
    mm->SetPosition(Vector(900, 0.0, hm));
    break;

  case 5:
    mm = CreateObject<ConstantPositionMobilityModel>();
    mm->SetPosition(Vector(-5, 0.0, hm));
    break;

  case 6:
    mm = CreateObject<ConstantPositionMobilityModel>();
    mm->SetPosition(Vector(-5, 30, henbHeight));
    break;

  case 7:
    mm = CreateObject<ConstantPositionMobilityModel>();
    mm->SetPosition(Vector(-2000, 0.0, hm));
    break;

  case 8:
    mm = CreateObject<ConstantPositionMobilityModel>();
    mm->SetPosition(Vector(-100, 0.0, hm));
    break;

  case 9:
    mm = CreateObject<ConstantPositionMobilityModel>();
    mm->SetPosition(Vector(0, 0.0, hm));
    break;

  case 10:
    mm = CreateObject<ConstantPositionMobilityModel>();
    mm->SetPosition(Vector(-100, 0.0, henbHeight));
    break;

  case 11:
    mm = CreateObject<ConstantPositionMobilityModel>();
    mm->SetPosition(Vector(-500, 0.0, henbHeight));
    break;

  default:
    mm = nullptr;
    break;
  }
  Ptr<MobilityBuildingInfo> buildingInfo = CreateObject<MobilityBuildingInfo>();
  mm->AggregateObject(buildingInfo);
  buildingInfo->MakeConsistent(mm);
  return mm;
}
