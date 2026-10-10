
#ifndef MAP_SCHEDULER_H
#define MAP_SCHEDULER_H

#include "scheduler.h"

#include <map>
#include <stdint.h>
#include <utility>

namespace ns3 {

class MapScheduler : public Scheduler {
public:
  static TypeId GetTypeId();

  MapScheduler();
  ~MapScheduler() override;

  void Insert(const Scheduler::Event &ev) override;
  bool IsEmpty() const override;
  Scheduler::Event PeekNext() const override;
  Scheduler::Event RemoveNext() override;
  void Remove(const Scheduler::Event &ev) override;

private:
  typedef std::map<Scheduler::EventKey, EventImpl *> EventMap;
  typedef std::map<Scheduler::EventKey, EventImpl *>::iterator EventMapI;
  typedef std::map<Scheduler::EventKey, EventImpl *>::const_iterator EventMapCI;

  EventMap m_list;
};

} // namespace ns3

#endif
