
#ifndef LENA_TEST_PHY_ERROR_MODEL_H
#define LENA_TEST_PHY_ERROR_MODEL_H

#include <ns3/nstime.h>
#include <ns3/simulator.h>
#include <ns3/test.h>

using namespace ns3;

class LenaDataPhyErrorModelTestCase : public TestCase {
public:
  LenaDataPhyErrorModelTestCase(uint16_t nUser, uint16_t dist, double blerRef,
                                uint16_t toleranceRxPackets,
                                Time statsStartTime, uint32_t rngRun);
  ~LenaDataPhyErrorModelTestCase() override;

private:
  void DoRun() override;
  static std::string BuildNameString(uint16_t nUser, uint16_t dist,
                                     uint32_t rngRun);
  uint16_t m_nUser;
  double m_dist;
  double m_blerRef;
  uint16_t m_toleranceRxPackets;
  Time m_statsStartTime;
  uint32_t m_rngRun;
};

class LenaDlCtrlPhyErrorModelTestCase : public TestCase {
public:
  LenaDlCtrlPhyErrorModelTestCase(uint16_t nEnb, uint16_t dist, double blerRef,
                                  uint16_t toleranceRxPackets,
                                  Time statsStartTime, uint32_t rngRun);
  ~LenaDlCtrlPhyErrorModelTestCase() override;

private:
  void DoRun() override;
  static std::string BuildNameString(uint16_t nUser, uint16_t dist,
                                     uint32_t rngRun);
  uint16_t m_nEnb;
  double m_dist;
  double m_blerRef;
  uint16_t m_toleranceRxPackets;
  Time m_statsStartTime;
  uint32_t m_rngRun;
};

class LenaTestPhyErrorModelSuite : public TestSuite {
public:
  LenaTestPhyErrorModelSuite();
};

#endif
