#ifndef EVENT_ID_H
#define EVENT_ID_H

#include "event-impl.h"
#include "ptr.h"

#include <stdint.h>

namespace ns3 {

class EventImpl;

class EventId {
public:
  enum UID { INVALID = 0, NOW = 1, DESTROY = 2, RESERVED = 3, VALID = 4 };

  EventId();
  EventId(const Ptr<EventImpl> &impl, uint64_t ts, uint32_t context,
          uint32_t uid);
  void Cancel();
  void Remove();
  bool IsExpired() const;
  bool IsRunning() const;

public:
  EventImpl *PeekEventImpl() const;
  uint64_t GetTs() const;
  uint32_t GetContext() const;
  uint32_t GetUid() const;

  friend bool operator==(const EventId &a, const EventId &b);
  friend bool operator!=(const EventId &a, const EventId &b);
  friend bool operator<(const EventId &a, const EventId &b);

private:
  Ptr<EventImpl> m_eventImpl;
  uint64_t m_ts;
  uint32_t m_context;
  uint32_t m_uid;
};

inline bool operator==(const EventId &a, const EventId &b) {
  return a.m_uid == b.m_uid && a.m_context == b.m_context && a.m_ts == b.m_ts &&
         a.m_eventImpl == b.m_eventImpl;
}

inline bool operator!=(const EventId &a, const EventId &b) { return !(a == b); }

inline bool operator<(const EventId &a, const EventId &b) {
  return (a.GetTs() < b.GetTs());
}

} // namespace ns3

#endif
