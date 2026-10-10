
#ifndef QUEUE_DISC_H
#define QUEUE_DISC_H

#include "packet-filter.h"

#include "ns3/object.h"
#include "ns3/queue-fwd.h"
#include "ns3/queue-item.h"
#include "ns3/queue-size.h"
#include "ns3/traced-callback.h"
#include "ns3/traced-value.h"

#include <functional>
#include <map>
#include <string>
#include <vector>

namespace ns3 {

class QueueDisc;
class NetDeviceQueueInterface;

class QueueDiscClass : public Object {
public:
  static TypeId GetTypeId();

  QueueDiscClass();
  ~QueueDiscClass() override;

  Ptr<QueueDisc> GetQueueDisc() const;

  void SetQueueDisc(Ptr<QueueDisc> qd);

protected:
  void DoDispose() override;

private:
  Ptr<QueueDisc> m_queueDisc;
};

enum QueueDiscSizePolicy {
  SINGLE_INTERNAL_QUEUE,
  SINGLE_CHILD_QUEUE_DISC,
  MULTIPLE_QUEUES,
  NO_LIMITS
};

class QueueDisc : public Object {
public:
  struct Stats {
    uint32_t nTotalReceivedPackets;
    uint64_t nTotalReceivedBytes;
    uint32_t nTotalSentPackets;
    uint64_t nTotalSentBytes;
    uint32_t nTotalEnqueuedPackets;
    uint64_t nTotalEnqueuedBytes;
    uint32_t nTotalDequeuedPackets;
    uint64_t nTotalDequeuedBytes;
    uint32_t nTotalDroppedPackets;
    uint32_t nTotalDroppedPacketsBeforeEnqueue;
    std::map<std::string, uint32_t, std::less<>> nDroppedPacketsBeforeEnqueue;
    uint32_t nTotalDroppedPacketsAfterDequeue;
    std::map<std::string, uint32_t, std::less<>> nDroppedPacketsAfterDequeue;
    uint64_t nTotalDroppedBytes;
    uint64_t nTotalDroppedBytesBeforeEnqueue;
    std::map<std::string, uint64_t, std::less<>> nDroppedBytesBeforeEnqueue;
    uint64_t nTotalDroppedBytesAfterDequeue;
    std::map<std::string, uint64_t, std::less<>> nDroppedBytesAfterDequeue;
    uint32_t nTotalRequeuedPackets;
    uint64_t nTotalRequeuedBytes;
    uint32_t nTotalMarkedPackets;
    std::map<std::string, uint32_t, std::less<>> nMarkedPackets;
    uint32_t nTotalMarkedBytes;
    std::map<std::string, uint64_t, std::less<>> nMarkedBytes;

    Stats();

    uint32_t GetNDroppedPackets(std::string reason) const;
    uint64_t GetNDroppedBytes(std::string reason) const;
    uint32_t GetNMarkedPackets(std::string reason) const;
    uint64_t GetNMarkedBytes(std::string reason) const;
    void Print(std::ostream &os) const;
  };

  static TypeId GetTypeId();

  QueueDisc(
      QueueDiscSizePolicy policy = QueueDiscSizePolicy::SINGLE_INTERNAL_QUEUE);

  QueueDisc(QueueDiscSizePolicy policy, QueueSizeUnit unit);

  ~QueueDisc() override;

  QueueDisc(const QueueDisc &) = delete;
  QueueDisc &operator=(const QueueDisc &) = delete;

  uint32_t GetNPackets() const;

  uint32_t GetNBytes() const;

  QueueSize GetMaxSize() const;

  bool SetMaxSize(QueueSize size);

  QueueSize GetCurrentSize();

  const Stats &GetStats();

  void SetNetDeviceQueueInterface(Ptr<NetDeviceQueueInterface> ndqi);

  Ptr<NetDeviceQueueInterface> GetNetDeviceQueueInterface() const;

  typedef std::function<void(Ptr<QueueDiscItem>)> SendCallback;

  void SetSendCallback(SendCallback func);

  SendCallback GetSendCallback() const;

  virtual void SetQuota(const uint32_t quota);

  virtual uint32_t GetQuota() const;

  bool Enqueue(Ptr<QueueDiscItem> item);

  Ptr<QueueDiscItem> Dequeue();

  Ptr<const QueueDiscItem> Peek();

  void Run();

  typedef Queue<QueueDiscItem> InternalQueue;

  void AddInternalQueue(Ptr<InternalQueue> queue);

  Ptr<InternalQueue> GetInternalQueue(std::size_t i) const;

  std::size_t GetNInternalQueues() const;

  void AddPacketFilter(Ptr<PacketFilter> filter);

  Ptr<PacketFilter> GetPacketFilter(std::size_t i) const;

  std::size_t GetNPacketFilters() const;

  void AddQueueDiscClass(Ptr<QueueDiscClass> qdClass);

  Ptr<QueueDiscClass> GetQueueDiscClass(std::size_t i) const;

  std::size_t GetNQueueDiscClasses() const;

  int32_t Classify(Ptr<QueueDiscItem> item);

  enum WakeMode { WAKE_ROOT = 0x00, WAKE_CHILD = 0x01 };

  virtual WakeMode GetWakeMode() const;

  static constexpr const char *INTERNAL_QUEUE_DROP =
      "Dropped by internal queue";
  static constexpr const char *CHILD_QUEUE_DISC_DROP =
      "(Dropped by child queue disc) ";
  static constexpr const char *CHILD_QUEUE_DISC_MARK =
      "(Marked by child queue disc) ";

protected:
  void DoDispose() override;

  void DoInitialize() override;

  void DropBeforeEnqueue(Ptr<const QueueDiscItem> item, const char *reason);

  void DropAfterDequeue(Ptr<const QueueDiscItem> item, const char *reason);

  bool Mark(Ptr<QueueDiscItem> item, const char *reason);

private:
  virtual bool DoEnqueue(Ptr<QueueDiscItem> item) = 0;

  virtual Ptr<QueueDiscItem> DoDequeue() = 0;

  virtual Ptr<const QueueDiscItem> DoPeek();

  virtual bool CheckConfig() = 0;

  virtual void InitializeParams() = 0;

  bool RunBegin();

  void RunEnd();

  bool Restart();

  Ptr<QueueDiscItem> DequeuePacket();

  void Requeue(Ptr<QueueDiscItem> item);

  bool Transmit(Ptr<QueueDiscItem> item);

  void PacketEnqueued(Ptr<const QueueDiscItem> item);

  void PacketDequeued(Ptr<const QueueDiscItem> item);

  static const uint32_t DEFAULT_QUOTA = 64;

  std::vector<Ptr<InternalQueue>> m_queues;
  std::vector<Ptr<PacketFilter>> m_filters;
  std::vector<Ptr<QueueDiscClass>> m_classes;

  TracedValue<uint32_t> m_nPackets;
  TracedValue<uint32_t> m_nBytes;
  TracedCallback<Time> m_sojourn;
  QueueSize m_maxSize;

  Stats m_stats;
  uint32_t m_quota;
  Ptr<NetDeviceQueueInterface> m_devQueueIface;
  SendCallback m_send;
  bool m_running;
  Ptr<QueueDiscItem> m_requeued;
  bool m_peeked;
  std::string m_childQueueDiscDropMsg;
  std::string m_childQueueDiscMarkMsg;
  QueueDiscSizePolicy m_sizePolicy;
  bool m_prohibitChangeMode;

  TracedCallback<Ptr<const QueueDiscItem>> m_traceEnqueue;
  TracedCallback<Ptr<const QueueDiscItem>> m_traceDequeue;
  TracedCallback<Ptr<const QueueDiscItem>> m_traceRequeue;
  TracedCallback<Ptr<const QueueDiscItem>> m_traceDrop;
  TracedCallback<Ptr<const QueueDiscItem>, const char *>
      m_traceDropBeforeEnqueue;
  TracedCallback<Ptr<const QueueDiscItem>, const char *>
      m_traceDropAfterDequeue;
  TracedCallback<Ptr<const QueueDiscItem>, const char *> m_traceMark;

  typedef std::function<void(Ptr<const QueueDiscItem>)>
      InternalQueueDropFunctor;
  typedef std::function<void(Ptr<const QueueDiscItem>, const char *)>
      ChildQueueDiscDropFunctor;
  typedef std::function<void(Ptr<const QueueDiscItem>, const char *)>
      ChildQueueDiscMarkFunctor;

  InternalQueueDropFunctor m_internalQueueDbeFunctor;
  InternalQueueDropFunctor m_internalQueueDadFunctor;
  ChildQueueDiscDropFunctor m_childQueueDiscDbeFunctor;
  ChildQueueDiscDropFunctor m_childQueueDiscDadFunctor;
  ChildQueueDiscMarkFunctor m_childQueueDiscMarkFunctor;
};

std::ostream &operator<<(std::ostream &os, const QueueDisc::Stats &stats);

} // namespace ns3

#endif
