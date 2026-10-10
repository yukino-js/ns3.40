
#ifndef CALENDAR_SCHEDULER_H
#define CALENDAR_SCHEDULER_H

#include "scheduler.h"

#include <list>
#include <stdint.h>

namespace ns3 {

class EventImpl;

class CalendarScheduler : public Scheduler {
public:
  static TypeId GetTypeId();

  CalendarScheduler();
  ~CalendarScheduler() override;

  void Insert(const Scheduler::Event &ev) override;
  bool IsEmpty() const override;
  Scheduler::Event PeekNext() const override;
  Scheduler::Event RemoveNext() override;
  void Remove(const Scheduler::Event &ev) override;

private:
  void ResizeUp();
  void ResizeDown();
  void Resize(uint32_t newSize);
  uint64_t CalculateNewWidth();
  void Init(uint32_t nBuckets, uint64_t width, uint64_t startPrio);
  inline uint32_t Hash(uint64_t key) const;
  void PrintInfo();
  void DoResize(uint32_t newSize, uint64_t newWidth);
  Scheduler::Event DoRemoveNext();
  void DoInsert(const Scheduler::Event &ev);

  typedef std::list<Scheduler::Event> Bucket;

  Bucket *m_buckets;
  uint32_t m_nBuckets;
  uint64_t m_width;
  uint32_t m_lastBucket;
  uint64_t m_bucketTop;
  uint64_t m_lastPrio;
  uint32_t m_qSize;

  void SetReverse(bool reverse);
  Scheduler::Event &(*NextEvent)(Bucket &bucket);
  bool (*Order)(const EventKey &newEvent, const EventKey &it);
  void (*Pop)(Bucket &);
  bool m_reverse = false;
};

} // namespace ns3

#endif
