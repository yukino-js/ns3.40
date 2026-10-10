
#ifndef QUEUE_LIMITS_H
#define QUEUE_LIMITS_H

#include "ns3/object.h"

namespace ns3 {

class QueueLimits : public Object {
public:
  static TypeId GetTypeId();

  ~QueueLimits() override;

  virtual void Reset() = 0;

  virtual void Completed(uint32_t count) = 0;

  virtual int32_t Available() const = 0;

  virtual void Queued(uint32_t count) = 0;
};

} // namespace ns3

#endif
