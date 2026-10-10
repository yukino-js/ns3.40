
#ifndef TEST_CARRIER_AGGREGATION_H
#define TEST_CARRIER_AGGREGATION_H

#include "fcntl.h"

#include "ns3/lte-common.h"
#include "ns3/simulator.h"
#include "ns3/test.h"

#include <map>

using namespace ns3;

class CarrierAggregationTestCase : public TestCase {
public:
  static bool s_writeResults;

  CarrierAggregationTestCase(uint16_t nUser, uint16_t dist,
                             uint32_t dlbandwidth, uint32_t ulBandwidth,
                             uint32_t numberOfComponentCarriers);
  ~CarrierAggregationTestCase() override;
  void DlScheduling(DlSchedulingCallbackInfo dlInfo);
  void UlScheduling(uint32_t frameNo, uint32_t subframeNo, uint16_t rnti,
                    uint8_t mcs, uint16_t sizeTb, uint8_t componentCarrierId);
  void WriteResultToFile() const;

private:
  void DoRun() override;
  static std::string BuildNameString(uint16_t nUser, uint16_t dist,
                                     uint32_t dlBandwidth, uint32_t ulBandwidth,
                                     uint32_t numberOfComponentCarriers);

  uint16_t m_nUser;
  uint16_t m_dist;
  uint16_t m_dlBandwidth;
  uint16_t m_ulBandwidth;
  uint32_t m_numberOfComponentCarriers;

  std::map<uint8_t, uint32_t> m_ccDownlinkTraffic;
  std::map<uint8_t, uint32_t> m_ccUplinkTraffic;
  uint64_t m_dlThroughput;
  uint64_t m_ulThroughput;
  double m_statsDuration;
};

class TestCarrierAggregationSuite : public TestSuite {
public:
  TestCarrierAggregationSuite();
};

#endif
