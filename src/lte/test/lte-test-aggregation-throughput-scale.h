
#ifndef LTE_AGGREGATION_THROUGHPUT_SCALE_H
#define LTE_AGGREGATION_THROUGHPUT_SCALE_H

#include <ns3/test.h>

using namespace ns3;

class LteAggregationThroughputScaleTestSuite : public TestSuite {
public:
  LteAggregationThroughputScaleTestSuite();
};

class LteAggregationThroughputScaleTestCase : public TestCase {
public:
  LteAggregationThroughputScaleTestCase(std::string name);

  ~LteAggregationThroughputScaleTestCase() override;

private:
  void DoRun() override;

  double GetThroughput(uint8_t numberOfComponentCarriers);

  uint16_t m_expectedCellId;
  uint16_t m_actualCellId;
};

#endif
