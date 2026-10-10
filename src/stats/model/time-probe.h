
#ifndef TIME_PROBE_H
#define TIME_PROBE_H

#include "probe.h"

#include "ns3/boolean.h"
#include "ns3/callback.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/simulator.h"
#include "ns3/traced-value.h"

namespace ns3 {

class TimeProbe : public Probe {
public:
  static TypeId GetTypeId();
  TimeProbe();
  ~TimeProbe() override;

  double GetValue() const;

  void SetValue(Time value);

  static void SetValueByPath(std::string path, Time value);

  bool ConnectByObject(std::string traceSource, Ptr<Object> obj) override;

  void ConnectByPath(std::string path) override;

private:
  void TraceSink(Time oldData, Time newData);

  TracedValue<double> m_output;
};

} // namespace ns3

#endif
