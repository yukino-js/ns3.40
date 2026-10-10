
#ifndef HEAP_SCHEDULER_H
#define HEAP_SCHEDULER_H

#include "scheduler.h"

#include <stdint.h>
#include <vector>

namespace ns3 {

class HeapScheduler : public Scheduler {
public:
  static TypeId GetTypeId();

  HeapScheduler();
  ~HeapScheduler() override;

  void Insert(const Scheduler::Event &ev) override;
  bool IsEmpty() const override;
  Scheduler::Event PeekNext() const override;
  Scheduler::Event RemoveNext() override;
  void Remove(const Scheduler::Event &ev) override;

private:
  typedef std::vector<Scheduler::Event> BinaryHeap;

  inline std::size_t Parent(std::size_t id) const;
  std::size_t Sibling(std::size_t id) const;
  inline std::size_t LeftChild(std::size_t id) const;
  inline std::size_t RightChild(std::size_t id) const;
  inline std::size_t Root() const;
  std::size_t Last() const;
  inline bool IsRoot(std::size_t id) const;
  inline bool IsBottom(std::size_t id) const;
  inline bool IsLessStrictly(std::size_t a, std::size_t b) const;
  inline std::size_t Smallest(std::size_t a, std::size_t b) const;
  inline void Exch(std::size_t a, std::size_t b);
  void BottomUp();
  void TopDown(std::size_t start);

  BinaryHeap m_heap;
};

} // namespace ns3

#endif
