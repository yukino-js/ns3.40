
#ifndef LTE_TEST_LINK_ADAPTATION_H
#define LTE_TEST_LINK_ADAPTATION_H

#include "ns3/lte-common.h"
#include "ns3/test.h"

using namespace ns3;

class LteLinkAdaptationTestSuite : public TestSuite {
public:
  LteLinkAdaptationTestSuite();
};

class LteLinkAdaptationTestCase : public TestCase {
public:
  LteLinkAdaptationTestCase(std::string name, double snrDb, double loss,
                            uint16_t mcsIndex);
  LteLinkAdaptationTestCase();
  ~LteLinkAdaptationTestCase() override;

  void DlScheduling(DlSchedulingCallbackInfo dlInfo);

private:
  void DoRun() override;

  double m_snrDb;
  double m_loss;
  uint16_t m_mcsIndex;
};

#endif
