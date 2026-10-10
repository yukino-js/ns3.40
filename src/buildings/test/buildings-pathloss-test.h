
#ifndef BUILDINGS_PATHLOSS_TEST_H
#define BUILDINGS_PATHLOSS_TEST_H

#include <ns3/hybrid-buildings-propagation-loss-model.h>
#include <ns3/test.h>

using namespace ns3;

class BuildingsPathlossTestSuite : public TestSuite {
public:
  BuildingsPathlossTestSuite();
};

class BuildingsPathlossTestCase : public TestCase {
public:
  BuildingsPathlossTestCase(double freq, uint16_t m1, uint16_t m2,
                            EnvironmentType env, CitySize city, double refValue,
                            std::string name);
  ~BuildingsPathlossTestCase() override;

private:
  void DoRun() override;
  Ptr<MobilityModel> CreateMobilityModel(uint16_t index);

  double m_freq;
  uint16_t m_mobilityModelIndex1;
  uint16_t m_mobilityModelIndex2;
  EnvironmentType m_env;
  CitySize m_city;
  double m_lossRef;
};

#endif
