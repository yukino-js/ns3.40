
#ifndef LIST_SCHEDULER_H
#define LIST_SCHEDULER_H

#include "scheduler.h"

#include <list>
#include <stdint.h>
#include <utility>

namespace ns3 {

class EventImpl;

class ListScheduler : public Scheduler {
public:
  static TypeId GetTypeId();

  ListScheduler();
  ~ListScheduler() override;

  void Insert(const Scheduler::Event &ev) override;
  bool IsEmpty() const override;
  Scheduler::Event PeekNext() const override;
  Scheduler::Event RemoveNext() override;
  void Remove(const Scheduler::Event &ev) override;

private:
  typedef std::list<Scheduler::Event> Events;
  typedef std::list<Scheduler::Event>::iterator EventsI;

  Events m_events;
};

} // namespace ns3

#endif
