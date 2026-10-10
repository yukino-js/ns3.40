
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "object.h"

#include <stdint.h>

namespace ns3 {

class EventImpl;

class Scheduler : public Object {
public:
  static TypeId GetTypeId();

  struct EventKey {
    uint64_t m_ts;
    uint32_t m_uid;
    uint32_t m_context;
  };

  struct Event {
    EventImpl *impl;
    EventKey key;
  };

  ~Scheduler() override = 0;

  virtual void Insert(const Event &ev) = 0;
  virtual bool IsEmpty() const = 0;
  virtual Event PeekNext() const = 0;
  virtual Event RemoveNext() = 0;
  virtual void Remove(const Event &ev) = 0;
};

inline bool operator==(const Scheduler::EventKey &a,
                       const Scheduler::EventKey &b) {
  return a.m_uid == b.m_uid;
}

inline bool operator!=(const Scheduler::EventKey &a,
                       const Scheduler::EventKey &b) {
  return a.m_uid != b.m_uid;
}

inline bool operator<(const Scheduler::EventKey &a,
                      const Scheduler::EventKey &b) {
  if (a.m_ts < b.m_ts) {
    return true;
  } else if (a.m_ts == b.m_ts && a.m_uid < b.m_uid) {
    return true;
  } else {
    return false;
  }
}

inline bool operator>(const Scheduler::EventKey &a,
                      const Scheduler::EventKey &b) {
  if (a.m_ts > b.m_ts) {
    return true;
  } else if (a.m_ts == b.m_ts && a.m_uid > b.m_uid) {
    return true;
  } else {
    return false;
  }
}

inline bool operator==(const Scheduler::Event &a, const Scheduler::Event &b) {
  return a.key == b.key;
}

inline bool operator!=(const Scheduler::Event &a, const Scheduler::Event &b) {
  return a.key != b.key;
}

inline bool operator<(const Scheduler::Event &a, const Scheduler::Event &b) {
  return a.key < b.key;
}

inline bool operator>(const Scheduler::Event &a, const Scheduler::Event &b) {
  return a.key > b.key;
}

} // namespace ns3

#endif
