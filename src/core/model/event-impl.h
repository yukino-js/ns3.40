#ifndef EVENT_IMPL_H
#define EVENT_IMPL_H

#include "simple-ref-count.h"

#include <stdint.h>

namespace ns3 {

class EventImpl : public SimpleRefCount<EventImpl> {
public:
  EventImpl();
  virtual ~EventImpl() = 0;
  void Invoke();
  void Cancel();
  bool IsCancelled();

protected:
  virtual void Notify() = 0;

private:
  bool m_cancel;
};

} // namespace ns3

#endif
