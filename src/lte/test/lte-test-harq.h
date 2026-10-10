
#ifndef LENA_TEST_HARQ_H
#define LENA_TEST_HARQ_H

#include "ns3/simulator.h"
#include "ns3/test.h"

using namespace ns3;

class LenaHarqTestCase : public TestCase {
public:
  LenaHarqTestCase(uint16_t nUser, uint16_t dist, uint16_t tbSize,
                   double amcBer, double thrRef);
  ~LenaHarqTestCase() override;

private:
  void DoRun() override;
  static std::string BuildNameString(uint16_t nUser, uint16_t dist,
                                     uint16_t tbSize);
  uint16_t m_nUser;
  uint16_t m_dist;
  double m_amcBer;
  double m_throughputRef;
};

class LenaTestHarqSuite : public TestSuite {
public:
  LenaTestHarqSuite();
};

#endif
