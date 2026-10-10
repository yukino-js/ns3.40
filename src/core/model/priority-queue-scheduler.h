
#ifndef PRIORITY_QUEUE_SCHEDULER_H
#define PRIORITY_QUEUE_SCHEDULER_H

#include "scheduler.h"

#include <algorithm>
#include <functional>
#include <queue>
#include <stdint.h>
#include <utility>

namespace ns3 {

class PriorityQueueScheduler : public Scheduler {
public:
  static TypeId GetTypeId();

  PriorityQueueScheduler();
  ~PriorityQueueScheduler() override;

  void Insert(const Scheduler::Event &ev) override;
  bool IsEmpty() const override;
  Scheduler::Event PeekNext() const override;
  Scheduler::Event RemoveNext() override;
  void Remove(const Scheduler::Event &ev) override;

private:
  class EventPriorityQueue
      : public std::priority_queue<
            Scheduler::Event, std::vector<Scheduler::Event>, std::greater<>> {
  public:
    bool remove(const Scheduler::Event &ev);
  };

  EventPriorityQueue m_queue;
};

} // namespace ns3

#endif
