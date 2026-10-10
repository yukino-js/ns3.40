
#ifndef LENA_TEST_MIMO_H
#define LENA_TEST_MIMO_H

#include "ns3/simulator.h"
#include "ns3/test.h"

namespace ns3 {
class RadioBearerStatsCalculator;
}

using namespace ns3;

class LenaMimoTestCase : public TestCase {
public:
  LenaMimoTestCase(uint16_t dist, std::vector<uint32_t> estThrDl,
                   std::string schedulerType, bool useIdealRrc);
  ~LenaMimoTestCase() override;

private:
  void DoRun() override;

  void GetRlcBufferSample(Ptr<RadioBearerStatsCalculator> rlcStats,
                          uint64_t imsi, uint8_t rnti);

  static std::string BuildNameString(uint16_t dist, std::string schedulerType,
                                     bool useIdealRrc);
  uint16_t m_dist;
  std::vector<uint32_t> m_estThrDl;
  std::string m_schedulerType;
  bool m_useIdealRrc;

  std::vector<uint64_t> m_dlDataRxed;
};

class LenaTestMimoSuite : public TestSuite {
public:
  LenaTestMimoSuite();
};

#endif
