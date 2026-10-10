
#ifndef TIME_DATA_CALCULATORS_H
#define TIME_DATA_CALCULATORS_H

#include "data-calculator.h"
#include "data-output-interface.h"

#include "ns3/nstime.h"

namespace ns3 {

class TimeMinMaxAvgTotalCalculator : public DataCalculator {
public:
  TimeMinMaxAvgTotalCalculator();
  ~TimeMinMaxAvgTotalCalculator() override;

  static TypeId GetTypeId();

  void Update(const Time i);

  void Output(DataOutputCallback &callback) const override;

protected:
  void DoDispose() override;

  uint32_t m_count;
  Time m_total;
  Time m_min;
  Time m_max;
};

}; // namespace ns3

#endif
