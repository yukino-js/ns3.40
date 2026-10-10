
#ifndef UINTEGER_16_PROBE_H
#define UINTEGER_16_PROBE_H

#include "probe.h"

#include "ns3/callback.h"
#include "ns3/traced-value.h"

namespace ns3 {

class Uinteger16Probe : public Probe {
public:
  static TypeId GetTypeId();
  Uinteger16Probe();
  ~Uinteger16Probe() override;

  uint16_t GetValue() const;

  void SetValue(uint16_t value);

  static void SetValueByPath(std::string path, uint16_t value);

  bool ConnectByObject(std::string traceSource, Ptr<Object> obj) override;

  void ConnectByPath(std::string path) override;

private:
  void TraceSink(uint16_t oldData, uint16_t newData);

  TracedValue<uint16_t> m_output;
};

} // namespace ns3

#endif
