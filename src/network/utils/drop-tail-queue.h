
#ifndef DROPTAIL_H
#define DROPTAIL_H

#include "queue.h"

namespace ns3 {

template <typename Item> class DropTailQueue : public Queue<Item> {
public:
  static TypeId GetTypeId();
  DropTailQueue();

  ~DropTailQueue() override;

  bool Enqueue(Ptr<Item> item) override;
  Ptr<Item> Dequeue() override;
  Ptr<Item> Remove() override;
  Ptr<const Item> Peek() const override;

private:
  using Queue<Item>::GetContainer;
  using Queue<Item>::DoEnqueue;
  using Queue<Item>::DoDequeue;
  using Queue<Item>::DoRemove;
  using Queue<Item>::DoPeek;

  NS_LOG_TEMPLATE_DECLARE;
};

template <typename Item> TypeId DropTailQueue<Item>::GetTypeId() {
  static TypeId tid =
      TypeId(GetTemplateClassName<DropTailQueue<Item>>())
          .SetParent<Queue<Item>>()
          .SetGroupName("Network")
          .template AddConstructor<DropTailQueue<Item>>()
          .AddAttribute("MaxSize", "The max queue size",
                        QueueSizeValue(QueueSize("100p")),
                        MakeQueueSizeAccessor(&QueueBase::SetMaxSize,
                                              &QueueBase::GetMaxSize),
                        MakeQueueSizeChecker());
  return tid;
}

template <typename Item>
DropTailQueue<Item>::DropTailQueue()
    : Queue<Item>(), NS_LOG_TEMPLATE_DEFINE("DropTailQueue") {
  NS_LOG_FUNCTION(this);
}

template <typename Item> DropTailQueue<Item>::~DropTailQueue() {
  NS_LOG_FUNCTION(this);
}

template <typename Item> bool DropTailQueue<Item>::Enqueue(Ptr<Item> item) {
  NS_LOG_FUNCTION(this << item);

  return DoEnqueue(GetContainer().end(), item);
}

template <typename Item> Ptr<Item> DropTailQueue<Item>::Dequeue() {
  NS_LOG_FUNCTION(this);

  Ptr<Item> item = DoDequeue(GetContainer().begin());

  NS_LOG_LOGIC("Popped " << item);

  return item;
}

template <typename Item> Ptr<Item> DropTailQueue<Item>::Remove() {
  NS_LOG_FUNCTION(this);

  Ptr<Item> item = DoRemove(GetContainer().begin());

  NS_LOG_LOGIC("Removed " << item);

  return item;
}

template <typename Item> Ptr<const Item> DropTailQueue<Item>::Peek() const {
  NS_LOG_FUNCTION(this);

  return DoPeek(GetContainer().begin());
}

extern template class DropTailQueue<Packet>;
extern template class DropTailQueue<QueueDiscItem>;

} // namespace ns3

#endif
