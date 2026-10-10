
#ifndef LENA_TEST_RR_FF_MAC_SCHEDULER_H
#define LENA_TEST_RR_FF_MAC_SCHEDULER_H

#include "ns3/simulator.h"
#include "ns3/test.h"

using namespace ns3;

class LenaRrFfMacSchedulerTestCase : public TestCase {
public:
  LenaRrFfMacSchedulerTestCase(uint16_t nUser, double dist, double thrRefDl,
                               double thrRefUl, bool errorModelEnabled);
  ~LenaRrFfMacSchedulerTestCase() override;

private:
  void DoRun() override;
  static std::string BuildNameString(uint16_t nUser, double dist);
  uint16_t m_nUser;
  double m_dist;
  double m_thrRefDl;
  double m_thrRefUl;
  bool m_errorModelEnabled;
};

class LenaTestRrFfMacSchedulerSuite : public TestSuite {
public:
  LenaTestRrFfMacSchedulerSuite();
};

#endif
