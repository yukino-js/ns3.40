
#ifndef LENA_TEST_FDBET_FF_MAC_SCHEDULER_H
#define LENA_TEST_FDBET_FF_MAC_SCHEDULER_H

#include "ns3/simulator.h"
#include "ns3/test.h"

using namespace ns3;

class LenaFdBetFfMacSchedulerTestCase1 : public TestCase {
public:
  LenaFdBetFfMacSchedulerTestCase1(uint16_t nUser, double dist, double thrRefDl,
                                   double thrRefUl, bool errorModelEnabled);
  ~LenaFdBetFfMacSchedulerTestCase1() override;

private:
  static std::string BuildNameString(uint16_t nUser, double dist);
  void DoRun() override;
  uint16_t m_nUser;
  double m_dist;
  double m_thrRefDl;
  double m_thrRefUl;
  bool m_errorModelEnabled;
};

class LenaFdBetFfMacSchedulerTestCase2 : public TestCase {
public:
  LenaFdBetFfMacSchedulerTestCase2(std::vector<double> dist,
                                   std::vector<uint32_t> achievableRateDl,
                                   std::vector<uint32_t> estThrFdBetUl,
                                   bool errorModelEnabled);
  ~LenaFdBetFfMacSchedulerTestCase2() override;

private:
  static std::string BuildNameString(uint16_t nUser, std::vector<double> dist);
  void DoRun() override;
  uint16_t m_nUser;
  std::vector<double> m_dist;
  std::vector<uint32_t> m_achievableRateDl;
  std::vector<uint32_t> m_estThrFdBetUl;
  bool m_errorModelEnabled;
};

class LenaTestFdBetFfMacSchedulerSuite : public TestSuite {
public:
  LenaTestFdBetFfMacSchedulerSuite();
};

#endif
