#ifndef EVENT_GARBAGE_COLLECTOR_H
#define EVENT_GARBAGE_COLLECTOR_H

#include "ns3/event-id.h"
#include "ns3/simulator.h"

#include <set>

namespace ns3 {

class EventGarbageCollector {
public:
  EventGarbageCollector();

  void Track(EventId event);

  ~EventGarbageCollector();

private:
  typedef std::multiset<EventId> EventList;

  const typename EventList::size_type CHUNK_INIT_SIZE = 8;
  const typename EventList::size_type CHUNK_MAX_SIZE = 128;

  EventList::size_type m_nextCleanupSize;
  EventList m_events;

  void Cleanup();
  void Grow();
  void Shrink();
};

} // namespace ns3

#endif
