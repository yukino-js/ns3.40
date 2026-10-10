
#ifndef BUILDINGS_SHADOWING_TEST_H
#define BUILDINGS_SHADOWING_TEST_H

#include "ns3/ptr.h"
#include "ns3/test.h"

namespace ns3 {
class MobilityModel;
}

using namespace ns3;

class BuildingsShadowingTestSuite : public TestSuite {
public:
  BuildingsShadowingTestSuite();
};

class BuildingsShadowingTestCase : public TestCase {
public:
  BuildingsShadowingTestCase(uint16_t m1, uint16_t m2, double refValue,
                             double sigmaRef, std::string name);
  ~BuildingsShadowingTestCase() override;

private:
  void DoRun() override;
  Ptr<MobilityModel> CreateMobilityModel(uint16_t index);

  uint16_t m_mobilityModelIndex1;
  uint16_t m_mobilityModelIndex2;
  double m_lossRef;
  double m_sigmaRef;
};

#endif
