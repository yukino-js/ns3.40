
#include "buildings-pathloss-test.h"

#include <ns3/building.h>
#include <ns3/buildings-helper.h>
#include <ns3/constant-position-mobility-model.h>
#include <ns3/double.h>
#include <ns3/enum.h>
#include <ns3/log.h>
#include <ns3/mobility-building-info.h>
#include <ns3/simulator.h>
#include <ns3/string.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("BuildingsPathlossTest");

BuildingsPathlossTestSuite::BuildingsPathlossTestSuite()
    : TestSuite("buildings-pathloss-test", SYSTEM) {
  LogComponentEnable("BuildingsPathlossTest", LOG_LEVEL_ALL);

  double freq = 869e6;

  AddTestCase(new BuildingsPathlossTestCase(freq, 1, 2, UrbanEnvironment,
                                            LargeCity, 137.93,
                                            "OH Urban Large city"),
              TestCase::QUICK);

  AddTestCase(new BuildingsPathlossTestCase(freq, 1, 2, UrbanEnvironment,
                                            SmallCity, 137.88,
                                            "OH Urban small city"),
              TestCase::QUICK);

  AddTestCase(new BuildingsPathlossTestCase(freq, 1, 2, SubUrbanEnvironment,
                                            LargeCity, 128.03,
                                            "loss OH SubUrban"),
              TestCase::QUICK);

  AddTestCase(new BuildingsPathlossTestCase(freq, 1, 2, OpenAreasEnvironment,
                                            LargeCity, 110.21,
                                            "loss OH OpenAreas"),
              TestCase::QUICK);

  freq = 2.1140e9;

  AddTestCase(new BuildingsPathlossTestCase(freq, 1, 2, UrbanEnvironment,
                                            LargeCity, 148.55,
                                            "COST231 Urban Large city"),
              TestCase::QUICK);

  AddTestCase(new BuildingsPathlossTestCase(
                  freq, 1, 2, UrbanEnvironment, SmallCity, 150.64,
                  "COST231 Urban small city and suburban"),
              TestCase::QUICK);

  freq = 2.620e9;

  AddTestCase(new BuildingsPathlossTestCase(freq, 1, 2, UrbanEnvironment,
                                            SmallCity, 121.83, "2.6GHz model"),
              TestCase::QUICK);

  freq = 2.1140e9;
  AddTestCase(new BuildingsPathlossTestCase(freq, 1, 3, UrbanEnvironment,
                                            LargeCity, 81.00, "ITU1411 LOS"),
              TestCase::QUICK);

  freq = 2.1140e9;

  AddTestCase(new BuildingsPathlossTestCase(freq, 1, 4, UrbanEnvironment,
                                            LargeCity, 143.69, "ITU1411 NLOS"),
              TestCase::QUICK);

  freq = 2.1140e9;
  AddTestCase(new BuildingsPathlossTestCase(freq, 5, 6, UrbanEnvironment,
                                            LargeCity, 88.3855, "ITUP1238"),
              TestCase::QUICK);

  freq = 2.1140e9;
  AddTestCase(new BuildingsPathlossTestCase(freq, 1, 7, UrbanEnvironment,
                                            LargeCity, 155.55,
                                            "Okumura Hata Outdoor -> Indoor"),
              TestCase::QUICK);

  freq = 2.1140e9;
  AddTestCase(new BuildingsPathlossTestCase(freq, 1, 8, UrbanEnvironment,
                                            LargeCity, 88.000,
                                            "ITU1411 LOS Outdoor -> Indoor"),
              TestCase::QUICK);

  freq = 2.1140e9;
  AddTestCase(new BuildingsPathlossTestCase(freq, 9, 10, UrbanEnvironment,
                                            LargeCity, 84.838,
                                            "ITU1411 LOS Indoor -> Outdoor"),
              TestCase::QUICK);

  freq = 2.1140e9;
  AddTestCase(new BuildingsPathlossTestCase(freq, 9, 11, UrbanEnvironment,
                                            LargeCity, 183.90,
                                            "ITU1411 NLOS Indoor -> Outdoor"),
              TestCase::QUICK);
}

static BuildingsPathlossTestSuite buildingsPathlossTestSuite;

BuildingsPathlossTestCase::BuildingsPathlossTestCase(
    double freq, uint16_t m1, uint16_t m2, EnvironmentType env, CitySize city,
    double refValue, std::string name)
    : TestCase("LOSS calculation: " + name), m_freq(freq),
      m_mobilityModelIndex1(m1), m_mobilityModelIndex2(m2), m_env(env),
      m_city(city), m_lossRef(refValue) {}

BuildingsPathlossTestCase::~BuildingsPathlossTestCase() {}

void BuildingsPathlossTestCase::DoRun() {
  NS_LOG_FUNCTION(this);

  Ptr<Building> building1 = CreateObject<Building>();
  building1->SetBoundaries(Box(-3000, -1, -4000, 4000.0, 0.0, 12));
  building1->SetBuildingType(Building::Residential);
  building1->SetExtWallsType(Building::ConcreteWithWindows);
  building1->SetNFloors(3);

  Ptr<MobilityModel> mma = CreateMobilityModel(m_mobilityModelIndex1);
  Ptr<MobilityModel> mmb = CreateMobilityModel(m_mobilityModelIndex2);

  Ptr<HybridBuildingsPropagationLossModel> propagationLossModel =
      CreateObject<HybridBuildingsPropagationLossModel>();
  propagationLossModel->SetAttribute("Frequency", DoubleValue(m_freq));
  propagationLossModel->SetAttribute("Environment", EnumValue(m_env));
  propagationLossModel->SetAttribute("CitySize", EnumValue(m_city));
  propagationLossModel->SetAttribute("ShadowSigmaOutdoor", DoubleValue(0.0));
  propagationLossModel->SetAttribute("ShadowSigmaIndoor", DoubleValue(0.0));
  propagationLossModel->SetAttribute("ShadowSigmaExtWalls", DoubleValue(0.0));

  double loss = propagationLossModel->GetLoss(mma, mmb);

  NS_LOG_INFO("Calculated loss: " << loss);
  NS_LOG_INFO("Theoretical loss: " << m_lossRef);

  NS_TEST_ASSERT_MSG_EQ_TOL(loss, m_lossRef, 0.1, "Wrong loss !");
  Simulator::Destroy();
}

Ptr<MobilityModel>
BuildingsPathlossTestCase::CreateMobilityModel(uint16_t index) {

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
