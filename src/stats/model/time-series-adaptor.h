
#ifndef TIME_SERIES_ADAPTOR_H
#define TIME_SERIES_ADAPTOR_H

#include "data-collection-object.h"

#include "ns3/object.h"
#include "ns3/traced-value.h"
#include "ns3/type-id.h"

namespace ns3 {

class TimeSeriesAdaptor : public DataCollectionObject {
public:
  static TypeId GetTypeId();

  TimeSeriesAdaptor();
  ~TimeSeriesAdaptor() override;

  void TraceSinkDouble(double oldData, double newData);

  void TraceSinkBoolean(bool oldData, bool newData);

  void TraceSinkUinteger8(uint8_t oldData, uint8_t newData);

  void TraceSinkUinteger16(uint16_t oldData, uint16_t newData);

  void TraceSinkUinteger32(uint32_t oldData, uint32_t newData);

  typedef void (*OutputTracedCallback)(const double now, const double data);

private:
  TracedCallback<double, double> m_output;
};

} // namespace ns3

#endif
