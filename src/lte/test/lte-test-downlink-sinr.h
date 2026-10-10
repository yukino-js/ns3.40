
#ifndef LTE_TEST_DOWNLINK_SINR_H
#define LTE_TEST_DOWNLINK_SINR_H

#include "ns3/spectrum-value.h"
#include "ns3/test.h"

using namespace ns3;

class LteDownlinkSinrTestSuite : public TestSuite {
public:
  LteDownlinkSinrTestSuite();
};

class LteDownlinkDataSinrTestCase : public TestCase {
public:
  LteDownlinkDataSinrTestCase(Ptr<SpectrumValue> sv, Ptr<SpectrumValue> sinr,
                              std::string name);
  ~LteDownlinkDataSinrTestCase() override;

private:
  void DoRun() override;

  Ptr<SpectrumValue> m_sv;
  Ptr<const SpectrumModel> m_sm;
  Ptr<SpectrumValue> m_expectedSinr;
};

class LteDownlinkCtrlSinrTestCase : public TestCase {
public:
  LteDownlinkCtrlSinrTestCase(Ptr<SpectrumValue> sv, Ptr<SpectrumValue> sinr,
                              std::string name);
  ~LteDownlinkCtrlSinrTestCase() override;

private:
  void DoRun() override;

  Ptr<SpectrumValue> m_sv;
  Ptr<const SpectrumModel> m_sm;
  Ptr<SpectrumValue> m_expectedSinr;
};

#endif
