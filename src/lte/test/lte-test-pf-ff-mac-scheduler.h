
#ifndef LENA_TEST_PF_FF_MAC_SCHEDULER_H
#define LENA_TEST_PF_FF_MAC_SCHEDULER_H

#include "ns3/simulator.h"
#include "ns3/test.h"

using namespace ns3;

class LenaPfFfMacSchedulerTestCase1 : public TestCase {
public:
  LenaPfFfMacSchedulerTestCase1(uint16_t nUser, double dist, double thrRefDl,
                                double thrRefUl, bool errorModelEnabled);
  ~LenaPfFfMacSchedulerTestCase1() override;

private:
  static std::string BuildNameString(uint16_t nUser, double dist);
  void DoRun() override;
  uint16_t m_nUser;
  double m_dist;
  double m_thrRefDl;
  double m_thrRefUl;
  bool m_errorModelEnabled;
};

class LenaPfFfMacSchedulerTestCase2 : public TestCase {
public:
  LenaPfFfMacSchedulerTestCase2(std::vector<double> dist,
                                std::vector<uint32_t> estThrPfDl,
                                std::vector<uint32_t> estThrPfUl,
                                bool errorModelEnabled);
  ~LenaPfFfMacSchedulerTestCase2() override;

private:
  static std::string BuildNameString(uint16_t nUser, std::vector<double> dist);
  void DoRun() override;
  uint16_t m_nUser;
  std::vector<double> m_dist;
  std::vector<uint32_t> m_estThrPfDl;
  std::vector<uint32_t> m_estThrPfUl;
  bool m_errorModelEnabled;
};

class LenaTestPfFfMacSchedulerSuite : public TestSuite {
public:
  LenaTestPfFfMacSchedulerSuite();
};

#endif
