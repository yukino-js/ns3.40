
#ifndef LTE_TEST_PATHLOSS_MODEL_H
#define LTE_TEST_PATHLOSS_MODEL_H

#include "ns3/lte-common.h"
#include "ns3/spectrum-value.h"
#include "ns3/test.h"
#include <ns3/buildings-propagation-loss-model.h>

using namespace ns3;

class LtePathlossModelTestSuite : public TestSuite {
public:
  LtePathlossModelTestSuite();
};

class LtePathlossModelSystemTestCase : public TestCase {
public:
  LtePathlossModelSystemTestCase(std::string name, double snrDb, double dist,
                                 uint16_t mcsIndex);
  LtePathlossModelSystemTestCase();
  ~LtePathlossModelSystemTestCase() override;

  void DlScheduling(DlSchedulingCallbackInfo dlInfo);

private:
  void DoRun() override;

  double m_snrDb;
  double m_distance;
  uint16_t m_mcsIndex;
};

#endif
