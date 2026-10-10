

#ifndef QUEUE_H
#define QUEUE_H

#include "queue-fwd.h"
#include "queue-item.h"
#include "queue-size.h"

#include "ns3/log.h"
#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/traced-callback.h"
#include "ns3/traced-value.h"

#include <sstream>
#include <string>
#include <type_traits>

namespace ns3 {

class QueueBase : public Object {
public:
  static TypeId GetTypeId();

  QueueBase();
  ~QueueBase() override;

  static void AppendItemTypeIfNotPresent(std::string &typeId,
                                         const std::string &itemType);

  bool IsEmpty() const;

  uint32_t GetNPackets() const;

  uint32_t GetNBytes() const;

  QueueSize GetCurrentSize() const;

  uint32_t GetTotalReceivedBytes() const;

  uint32_t GetTotalReceivedPackets() const;

  uint32_t GetTotalDroppedBytes() const;

  uint32_t GetTotalDroppedBytesBeforeEnqueue() const;

  uint32_t GetTotalDroppedBytesAfterDequeue() const;

  uint32_t GetTotalDroppedPackets() const;

  uint32_t GetTotalDroppedPacketsBeforeEnqueue() const;

  uint32_t GetTotalDroppedPacketsAfterDequeue() const;

  void ResetStatistics();

  void SetMaxSize(QueueSize size);

  QueueSize GetMaxSize() const;

  bool WouldOverflow(uint32_t nPackets, uint32_t nBytes) const;

#if 0
  void EnableRunningAverage(Time averageWindow);
  void DisableRunningAverage();
  double GetQueueSizeAverage();
  double GetReceivedBytesPerSecondAverage();
  double GetReceivedPacketsPerSecondAverage();
  double GetDroppedBytesPerSecondAverage();
  double GetDroppedPacketsPerSecondAverage();
  double GetQueueSizeVariance();
  double GetReceivedBytesPerSecondVariance();
  double GetReceivedPacketsPerSecondVariance();
  double GetDroppedBytesPerSecondVariance();
  double GetDroppedPacketsPerSecondVariance();
#endif

protected:
  TracedValue<uint32_t> m_nBytes;
  uint32_t m_nTotalReceivedBytes;
  TracedValue<uint32_t> m_nPackets;
  uint32_t m_nTotalReceivedPackets;
  uint32_t m_nTotalDroppedBytes;
  uint32_t m_nTotalDroppedBytesBeforeEnqueue;
  uint32_t m_nTotalDroppedBytesAfterDequeue;
  uint32_t m_nTotalDroppedPackets;
  uint32_t m_nTotalDroppedPacketsBeforeEnqueue;
  uint32_t m_nTotalDroppedPacketsAfterDequeue;

  QueueSize m_maxSize;
};

template <typename Item, typename Container> class Queue : public QueueBase {
public:
  static TypeId GetTypeId();

  Queue();
  ~Queue() override;

  virtual bool Enqueue(Ptr<Item> item) = 0;

  virtual Ptr<Item> Dequeue() = 0;

  virtual Ptr<Item> Remove() = 0;

  virtual Ptr<const Item> Peek() const = 0;

  void Flush();

  typedef Item ItemType;

protected:
  typedef typename Container::const_iterator ConstIterator;
  typedef typename Container::iterator Iterator;

  const Container &GetContainer() const;

  bool DoEnqueue(ConstIterator pos, Ptr<Item> item);

  bool DoEnqueue(ConstIterator pos, Ptr<Item> item, Iterator &ret);

  Ptr<Item> DoDequeue(ConstIterator pos);

  Ptr<Item> DoRemove(ConstIterator pos);

  Ptr<const Item> DoPeek(ConstIterator pos) const;

  void DropBeforeEnqueue(Ptr<Item> item);

  void DropAfterDequeue(Ptr<Item> item);

  void DoDispose() override;

private:
  template <class, class = void> struct MakeGetItem {
    static Ptr<Item> GetItem(const Container &, const ConstIterator it) {
      return *it;
    }
  };

  template <class T>
  struct MakeGetItem<T, std::void_t<decltype(std::declval<T>().GetItem(
                            std::declval<ConstIterator>()))>> {
    static Ptr<Item> GetItem(const Container &container,
                             const ConstIterator it) {
      return container.GetItem(it);
    }
  };

  Container m_packets;
  NS_LOG_TEMPLATE_DECLARE;

  TracedCallback<Ptr<const Item>> m_traceEnqueue;
  TracedCallback<Ptr<const Item>> m_traceDequeue;
  TracedCallback<Ptr<const Item>> m_traceDrop;
  TracedCallback<Ptr<const Item>> m_traceDropBeforeEnqueue;
  TracedCallback<Ptr<const Item>> m_traceDropAfterDequeue;
};

template <typename Item, typename Container>
TypeId Queue<Item, Container>::GetTypeId() {
  std::string name = GetTemplateClassName<Queue<Item, Container>>();
  auto startPos = name.find('<') + 1;
  auto endPos = name.find_first_of(",>", startPos);
  std::string tcbName =
      "ns3::" + name.substr(startPos, endPos - startPos) + "::TracedCallback";

  static TypeId tid =
      TypeId(name)
          .SetParent<QueueBase>()
          .SetGroupName("Network")
          .AddTraceSource(
              "Enqueue", "Enqueue a packet in the queue.",
              MakeTraceSourceAccessor(&Queue<Item, Container>::m_traceEnqueue),
              tcbName)
          .AddTraceSource(
              "Dequeue", "Dequeue a packet from the queue.",
              MakeTraceSourceAccessor(&Queue<Item, Container>::m_traceDequeue),
              tcbName)
          .AddTraceSource(
              "Drop", "Drop a packet (for whatever reason).",
              MakeTraceSourceAccessor(&Queue<Item, Container>::m_traceDrop),
              tcbName)
          .AddTraceSource(
              "DropBeforeEnqueue", "Drop a packet before enqueue.",
              MakeTraceSourceAccessor(
                  &Queue<Item, Container>::m_traceDropBeforeEnqueue),
              tcbName)
          .AddTraceSource("DropAfterDequeue", "Drop a packet after dequeue.",
                          MakeTraceSourceAccessor(
                              &Queue<Item, Container>::m_traceDropAfterDequeue),
                          tcbName);
  return tid;
}

template <typename Item, typename Container>
Queue<Item, Container>::Queue() : NS_LOG_TEMPLATE_DEFINE("Queue") {}

template <typename Item, typename Container> Queue<Item, Container>::~Queue() {}

template <typename Item, typename Container>
const Container &Queue<Item, Container>::GetContainer() const {
  return m_packets;
}

template <typename Item, typename Container>
bool Queue<Item, Container>::DoEnqueue(ConstIterator pos, Ptr<Item> item) {
  Iterator ret;
  return DoEnqueue(pos, item, ret);
}

template <typename Item, typename Container>
bool Queue<Item, Container>::DoEnqueue(ConstIterator pos, Ptr<Item> item,
                                       Iterator &ret) {
  NS_LOG_FUNCTION(this << item);

  if (GetCurrentSize() + item > GetMaxSize()) {
    NS_LOG_LOGIC("Queue full -- dropping pkt");
    DropBeforeEnqueue(item);
    return false;
  }

  ret = m_packets.insert(pos, item);

  uint32_t size = item->GetSize();
  m_nBytes += size;
  m_nTotalReceivedBytes += size;

  m_nPackets++;
  m_nTotalReceivedPackets++;

  NS_LOG_LOGIC("m_traceEnqueue (p)");
  m_traceEnqueue(item);

  return true;
}

template <typename Item, typename Container>
Ptr<Item> Queue<Item, Container>::DoDequeue(ConstIterator pos) {
  NS_LOG_FUNCTION(this);

  if (m_nPackets.Get() == 0) {
    NS_LOG_LOGIC("Queue empty");
    return nullptr;
  }

  Ptr<Item> item = MakeGetItem<Container>::GetItem(m_packets, pos);

  if (item) {
    m_packets.erase(pos);
    NS_ASSERT(m_nBytes.Get() >= item->GetSize());
    NS_ASSERT(m_nPackets.Get() > 0);

    m_nBytes -= item->GetSize();
    m_nPackets--;

    NS_LOG_LOGIC("m_traceDequeue (p)");
    m_traceDequeue(item);
  }
  return item;
}

template <typename Item, typename Container>
Ptr<Item> Queue<Item, Container>::DoRemove(ConstIterator pos) {
  NS_LOG_FUNCTION(this);

  if (m_nPackets.Get() == 0) {
    NS_LOG_LOGIC("Queue empty");
    return nullptr;
  }

  Ptr<Item> item = MakeGetItem<Container>::GetItem(m_packets, pos);

  if (item) {
    m_packets.erase(pos);
    NS_ASSERT(m_nBytes.Get() >= item->GetSize());
    NS_ASSERT(m_nPackets.Get() > 0);

    m_nBytes -= item->GetSize();
    m_nPackets--;

    NS_LOG_LOGIC("m_traceDequeue (p)");
    m_traceDequeue(item);

    DropAfterDequeue(item);
  }
  return item;
}

template <typename Item, typename Container>
void Queue<Item, Container>::Flush() {
  NS_LOG_FUNCTION(this);
  while (!IsEmpty()) {
    Remove();
  }
}

template <typename Item, typename Container>
void Queue<Item, Container>::DoDispose() {
  NS_LOG_FUNCTION(this);
  m_packets.clear();
  Object::DoDispose();
}

template <typename Item, typename Container>
Ptr<const Item> Queue<Item, Container>::DoPeek(ConstIterator pos) const {
  NS_LOG_FUNCTION(this);

  if (m_nPackets.Get() == 0) {
    NS_LOG_LOGIC("Queue empty");
    return nullptr;
  }

  return MakeGetItem<Container>::GetItem(m_packets, pos);
}

template <typename Item, typename Container>
void Queue<Item, Container>::DropBeforeEnqueue(Ptr<Item> item) {
  NS_LOG_FUNCTION(this << item);

  m_nTotalDroppedPackets++;
  m_nTotalDroppedPacketsBeforeEnqueue++;
  m_nTotalDroppedBytes += item->GetSize();
  m_nTotalDroppedBytesBeforeEnqueue += item->GetSize();

  NS_LOG_LOGIC("m_traceDropBeforeEnqueue (p)");
  m_traceDrop(item);
  m_traceDropBeforeEnqueue(item);
}

template <typename Item, typename Container>
void Queue<Item, Container>::DropAfterDequeue(Ptr<Item> item) {
  NS_LOG_FUNCTION(this << item);

  m_nTotalDroppedPackets++;
  m_nTotalDroppedPacketsAfterDequeue++;
  m_nTotalDroppedBytes += item->GetSize();
  m_nTotalDroppedBytesAfterDequeue += item->GetSize();

  NS_LOG_LOGIC("m_traceDropAfterDequeue (p)");
  m_traceDrop(item);
  m_traceDropAfterDequeue(item);
}

extern template class Queue<Packet>;
extern template class Queue<QueueDiscItem>;

} // namespace ns3

#endif
