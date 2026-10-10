
#ifndef PROBE_H
#define PROBE_H

#include "data-collection-object.h"

#include "ns3/nstime.h"

namespace ns3 {

class Probe : public DataCollectionObject {
public:
  static TypeId GetTypeId();
  Probe();
  ~Probe() override;

  bool IsEnabled() const override;

  virtual bool ConnectByObject(std::string traceSource, Ptr<Object> obj) = 0;

  virtual void ConnectByPath(std::string path) = 0;

protected:
  Time m_start;

  Time m_stop;
};

} // namespace ns3

#endif
