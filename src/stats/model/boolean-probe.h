
#ifndef BOOL_PROBE_H
#define BOOL_PROBE_H

#include "probe.h"

#include "ns3/boolean.h"
#include "ns3/callback.h"
#include "ns3/object.h"
#include "ns3/simulator.h"
#include "ns3/traced-value.h"

namespace ns3 {

class BooleanProbe : public Probe {
public:
  static TypeId GetTypeId();
  BooleanProbe();
  ~BooleanProbe() override;

  bool GetValue() const;

  void SetValue(bool value);

  static void SetValueByPath(std::string path, bool value);

  bool ConnectByObject(std::string traceSource, Ptr<Object> obj) override;

  void ConnectByPath(std::string path) override;

private:
  void TraceSink(bool oldData, bool newData);

  TracedValue<bool> m_output;
};

} // namespace ns3

#endif
