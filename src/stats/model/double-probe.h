
#ifndef DOUBLE_PROBE_H
#define DOUBLE_PROBE_H

#include "probe.h"

#include "ns3/boolean.h"
#include "ns3/callback.h"
#include "ns3/object.h"
#include "ns3/simulator.h"
#include "ns3/traced-value.h"

namespace ns3 {

class DoubleProbe : public Probe {
public:
  static TypeId GetTypeId();
  DoubleProbe();
  ~DoubleProbe() override;

  double GetValue() const;

  void SetValue(double value);

  static void SetValueByPath(std::string path, double value);

  bool ConnectByObject(std::string traceSource, Ptr<Object> obj) override;

  void ConnectByPath(std::string path) override;

private:
  void TraceSink(double oldData, double newData);

  TracedValue<double> m_output;
};

} // namespace ns3

#endif
