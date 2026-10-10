
#ifndef LENA_TEST_TDBET_FF_MAC_SCHEDULER_H
#define LENA_TEST_TDBET_FF_MAC_SCHEDULER_H

#include "ns3/simulator.h"
#include "ns3/test.h"

using namespace ns3;

class LenaTdBetFfMacSchedulerTestCase1 : public TestCase {
public:
  LenaTdBetFfMacSchedulerTestCase1(uint16_t nUser, double dist, double thrRefDl,
                                   double thrRefUl, bool errorModelEnabled);
  ~LenaTdBetFfMacSchedulerTestCase1() override;

private:
  static std::string BuildNameString(uint16_t nUser, double dist);
  void DoRun() override;
  uint16_t m_nUser;
  double m_dist;
  double m_thrRefDl;
  double m_thrRefUl;
  bool m_errorModelEnabled;
};

class LenaTdBetFfMacSchedulerTestCase2 : public TestCase {
public:
  LenaTdBetFfMacSchedulerTestCase2(std::vector<double> dist,
                                   std::vector<uint32_t> achievableRateDl,
                                   std::vector<uint32_t> estThrTdBetUl,
                                   bool errorModelEnabled);
  ~LenaTdBetFfMacSchedulerTestCase2() override;

private:
  static std::string BuildNameString(uint16_t nUser, std::vector<double> dist);
  void DoRun() override;
  uint16_t m_nUser;
  std::vector<double> m_dist;
  std::vector<uint32_t> m_achievableRateDl;
  std::vector<uint32_t> m_estThrTdBetUl;
  bool m_errorModelEnabled;
};

class LenaTestTdBetFfMacSchedulerSuite : public TestSuite {
public:
  LenaTestTdBetFfMacSchedulerSuite();
};

#endif
