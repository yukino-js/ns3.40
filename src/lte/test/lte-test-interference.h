
#ifndef LTE_TEST_INTERFERENCE_H
#define LTE_TEST_INTERFERENCE_H

#include "ns3/lte-common.h"
#include "ns3/test.h"

using namespace ns3;

class LteInterferenceTestSuite : public TestSuite {
public:
  LteInterferenceTestSuite();
};

class LteInterferenceTestCase : public TestCase {
public:
  LteInterferenceTestCase(std::string name, double d1, double d2, double dlSinr,
                          double ulSinr, double dlSe, double ulSe,
                          uint16_t dlMcs, uint16_t ulMcs);
  ~LteInterferenceTestCase() override;

  void DlScheduling(DlSchedulingCallbackInfo dlInfo);

  void UlScheduling(uint32_t frameNo, uint32_t subframeNo, uint16_t rnti,
                    uint8_t mcs, uint16_t sizeTb);

private:
  void DoRun() override;

  double m_d1;
  double m_d2;
  double m_expectedDlSinrDb;
  double m_expectedUlSinrDb;
  uint16_t m_dlMcs;
  uint16_t m_ulMcs;
};

#endif
