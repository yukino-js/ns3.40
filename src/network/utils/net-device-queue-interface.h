#ifndef NET_DEVICE_QUEUE_INTERFACE_H
#define NET_DEVICE_QUEUE_INTERFACE_H

#include "ns3/callback.h"
#include "ns3/log.h"
#include "ns3/net-device.h"
#include "ns3/object-factory.h"
#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/simulator.h"

#include <functional>
#include <vector>

namespace ns3 {

class QueueLimits;
class NetDeviceQueueInterface;
class QueueItem;

class NetDeviceQueue : public Object {
public:
  static TypeId GetTypeId();

  NetDeviceQueue();
  ~NetDeviceQueue() override;

  virtual void Start();

  virtual void Stop();

  virtual void Wake();

  virtual bool IsStopped() const;

  void NotifyAggregatedObject(Ptr<NetDeviceQueueInterface> ndqi);

  typedef Callback<void> WakeCallback;

  virtual void SetWakeCallback(WakeCallback cb);

  virtual void NotifyQueuedBytes(uint32_t bytes);

  virtual void NotifyTransmittedBytes(uint32_t bytes);

  void ResetQueueLimits();

  void SetQueueLimits(Ptr<QueueLimits> ql);

  Ptr<QueueLimits> GetQueueLimits();

  template <typename QueueType>
  void PacketEnqueued(QueueType *queue,
                      Ptr<const typename QueueType::ItemType> item);

  template <typename QueueType>
  void PacketDequeued(QueueType *queue,
                      Ptr<const typename QueueType::ItemType> item);

  template <typename QueueType>
  void PacketDiscarded(QueueType *queue,
                       Ptr<const typename QueueType::ItemType> item);

  template <typename QueueType> void ConnectQueueTraces(Ptr<QueueType> queue);

private:
  bool m_stoppedByDevice;
  bool m_stoppedByQueueLimits;
  Ptr<QueueLimits> m_queueLimits;
  WakeCallback m_wakeCallback;
  Ptr<NetDevice> m_device;

  NS_LOG_TEMPLATE_DECLARE;
};

class NetDeviceQueueInterface : public Object {
public:
  static TypeId GetTypeId();

  NetDeviceQueueInterface();
  ~NetDeviceQueueInterface() override;

  Ptr<NetDeviceQueue> GetTxQueue(std::size_t i) const;

  std::size_t GetNTxQueues() const;

  void SetTxQueuesType(TypeId type);

  void SetNTxQueues(std::size_t numTxQueues);

  typedef std::function<std::size_t(Ptr<QueueItem>)> SelectQueueCallback;

  void SetSelectQueueCallback(SelectQueueCallback cb);

  SelectQueueCallback GetSelectQueueCallback() const;

protected:
  void DoDispose() override;
  void NotifyNewAggregate() override;

private:
  ObjectFactory m_txQueues;
  std::vector<Ptr<NetDeviceQueue>> m_txQueuesVector;
  SelectQueueCallback m_selectQueueCallback;
};

template <typename QueueType>
void NetDeviceQueue::ConnectQueueTraces(Ptr<QueueType> queue) {
  NS_ASSERT(queue);

  queue->TraceConnectWithoutContext(
      "Enqueue", MakeCallback(&NetDeviceQueue::PacketEnqueued<QueueType>, this)
                     .Bind(PeekPointer(queue)));
  queue->TraceConnectWithoutContext(
      "Dequeue", MakeCallback(&NetDeviceQueue::PacketDequeued<QueueType>, this)
                     .Bind(PeekPointer(queue)));
  queue->TraceConnectWithoutContext(
      "DropBeforeEnqueue",
      MakeCallback(&NetDeviceQueue::PacketDiscarded<QueueType>, this)
          .Bind(PeekPointer(queue)));
}

template <typename QueueType>
void NetDeviceQueue::PacketEnqueued(
    QueueType *queue, Ptr<const typename QueueType::ItemType> item) {
  NS_LOG_FUNCTION(this << queue << item);

  NotifyQueuedBytes(item->GetSize());

  NS_ASSERT_MSG(m_device, "Aggregated NetDevice not set");

  if (queue->WouldOverflow(1, m_device->GetMtu())) {
    NS_LOG_DEBUG("The device queue is being stopped ("
                 << queue->GetCurrentSize() << " inside)");
    Stop();
  }
}

template <typename QueueType>
void NetDeviceQueue::PacketDequeued(
    QueueType *queue, Ptr<const typename QueueType::ItemType> item) {
  NS_LOG_FUNCTION(this << queue << item);
  NS_ASSERT_MSG(m_device, "Aggregated NetDevice not set");

  Simulator::ScheduleNow([=]() {
    NotifyTransmittedBytes(item->GetSize());

    if (!queue->WouldOverflow(1, m_device->GetMtu())) {
      Wake();
    }
  });
}

template <typename QueueType>
void NetDeviceQueue::PacketDiscarded(
    QueueType *queue, Ptr<const typename QueueType::ItemType> item) {
  NS_LOG_FUNCTION(this << queue << item);

  NS_LOG_ERROR("BUG! No room in the device queue for the received packet! ("
               << queue->GetCurrentSize() << " inside)");

  Stop();
}

} // namespace ns3

#endif
