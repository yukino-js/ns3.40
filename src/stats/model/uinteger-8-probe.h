
#ifndef UINTEGER_8_PROBE_H
#define UINTEGER_8_PROBE_H

#include "probe.h"

#include "ns3/callback.h"
#include "ns3/traced-value.h"

namespace ns3 {

class Uinteger8Probe : public Probe {
public:
  static TypeId GetTypeId();
  Uinteger8Probe();
  ~Uinteger8Probe() override;

  uint8_t GetValue() const;

  void SetValue(uint8_t value);

  static void SetValueByPath(std::string path, uint8_t value);

  bool ConnectByObject(std::string traceSource, Ptr<Object> obj) override;

  void ConnectByPath(std::string path) override;

private:
  void TraceSink(uint8_t oldData, uint8_t newData);

  TracedValue<uint8_t> m_output;
};

} // namespace ns3

#endif
