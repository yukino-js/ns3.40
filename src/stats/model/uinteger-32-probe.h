
#ifndef UINTEGER_32_PROBE_H
#define UINTEGER_32_PROBE_H

#include "probe.h"

#include "ns3/callback.h"
#include "ns3/traced-value.h"

namespace ns3 {

class Uinteger32Probe : public Probe {
public:
  static TypeId GetTypeId();
  Uinteger32Probe();
  ~Uinteger32Probe() override;

  uint32_t GetValue() const;

  void SetValue(uint32_t value);

  static void SetValueByPath(std::string path, uint32_t value);

  bool ConnectByObject(std::string traceSource, Ptr<Object> obj) override;

  void ConnectByPath(std::string path) override;

private:
  void TraceSink(uint32_t oldData, uint32_t newData);

  TracedValue<uint32_t> m_output;
};

} // namespace ns3

#endif
