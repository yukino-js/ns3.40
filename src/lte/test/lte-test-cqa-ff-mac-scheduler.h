
#ifndef LENA_TEST_CQA_FF_MAC_SCHEDULER_H
#define LENA_TEST_CQA_FF_MAC_SCHEDULER_H

#include "ns3/simulator.h"
#include "ns3/test.h"

using namespace ns3;

class LenaCqaFfMacSchedulerTestCase1 : public TestCase {
public:
  LenaCqaFfMacSchedulerTestCase1(uint16_t nUser, double dist, double thrRefDl,
                                 double thrRefUl, uint16_t packetSize,
                                 uint16_t interval, bool errorModelEnabled);
  ~LenaCqaFfMacSchedulerTestCase1() override;

private:
  static std::string BuildNameString(uint16_t nUser, double dist);
  void DoRun() override;
  uint16_t m_nUser;
  double m_dist;
  uint16_t m_packetSize;
  uint16_t m_interval;
  double m_thrRefDl;
  double m_thrRefUl;
  bool m_errorModelEnabled;
};

class LenaCqaFfMacSchedulerTestCase2 : public TestCase {
public:
  LenaCqaFfMacSchedulerTestCase2(std::vector<double> dist,
                                 std::vector<uint32_t> estThrCqaDl,
                                 std::vector<uint16_t> packetSize,
                                 uint16_t interval, bool errorModelEnabled);
  ~LenaCqaFfMacSchedulerTestCase2() override;

private:
  static std::string BuildNameString(uint16_t nUser, std::vector<double> dist);
  void DoRun() override;
  uint16_t m_nUser;
  std::vector<double> m_dist;
  std::vector<uint16_t> m_packetSize;
  uint16_t m_interval;
  std::vector<uint32_t> m_estThrCqaDl;
  bool m_errorModelEnabled;
};

class LenaTestCqaFfMacSchedulerSuite : public TestSuite {
public:
  LenaTestCqaFfMacSchedulerSuite();
};

#endif
