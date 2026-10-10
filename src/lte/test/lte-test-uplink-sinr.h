
#ifndef LTE_TEST_UPLINK_SINR_H
#define LTE_TEST_UPLINK_SINR_H

#include "ns3/spectrum-value.h"
#include "ns3/test.h"

using namespace ns3;

class LteUplinkSinrTestSuite : public TestSuite {
public:
  LteUplinkSinrTestSuite();
};

class LteUplinkDataSinrTestCase : public TestCase {
public:
  LteUplinkDataSinrTestCase(Ptr<SpectrumValue> sv1, Ptr<SpectrumValue> sv2,
                            Ptr<SpectrumValue> sinr, std::string name);
  ~LteUplinkDataSinrTestCase() override;

private:
  void DoRun() override;

  Ptr<SpectrumValue> m_sv1;
  Ptr<SpectrumValue> m_sv2;
  Ptr<const SpectrumModel> m_sm;
  Ptr<SpectrumValue> m_expectedSinr;
};

class LteUplinkSrsSinrTestCase : public TestCase {
public:
  LteUplinkSrsSinrTestCase(Ptr<SpectrumValue> sv1, Ptr<SpectrumValue> sv2,
                           Ptr<SpectrumValue> sinr, std::string name);
  ~LteUplinkSrsSinrTestCase() override;

  void ReportSinr(const SpectrumValue &sinr);

private:
  void DoRun() override;

  Ptr<SpectrumValue> m_sv1;
  Ptr<SpectrumValue> m_sv2;
  Ptr<const SpectrumModel> m_sm;

  Ptr<SpectrumValue> m_expectedSinr;
  Ptr<SpectrumValue> m_actualSinr;
};

#endif
