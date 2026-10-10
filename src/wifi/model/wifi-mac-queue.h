
#ifndef WIFI_MAC_QUEUE_H
#define WIFI_MAC_QUEUE_H

#include "qos-utils.h"
#include "wifi-mac-queue-container.h"
#include "wifi-mpdu.h"

#include "ns3/queue.h"

#include <functional>
#include <optional>
#include <unordered_map>

namespace ns3 {

class WifiMacQueueScheduler;

extern template class Queue<WifiMpdu, ns3::WifiMacQueueContainer>;

class WifiMacQueue : public Queue<WifiMpdu, ns3::WifiMacQueueContainer> {
public:
  static TypeId GetTypeId();

  WifiMacQueue(AcIndex ac = AC_UNDEF);

  ~WifiMacQueue() override;

  using Queue<WifiMpdu, WifiMacQueueContainer>::ConstIterator;
  using Queue<WifiMpdu, WifiMacQueueContainer>::Iterator;
  using Queue<WifiMpdu, WifiMacQueueContainer>::IsEmpty;
  using Queue<WifiMpdu, WifiMacQueueContainer>::GetNPackets;
  using Queue<WifiMpdu, WifiMacQueueContainer>::GetNBytes;

  AcIndex GetAc() const;

  void SetScheduler(Ptr<WifiMacQueueScheduler> scheduler);

  void SetMaxDelay(Time delay);
  Time GetMaxDelay() const;

  bool Enqueue(Ptr<WifiMpdu> item) override;
  Ptr<WifiMpdu> Dequeue() override;
  void DequeueIfQueued(const std::list<Ptr<const WifiMpdu>> &mpdus);
  Ptr<const WifiMpdu> Peek() const override;
  Ptr<WifiMpdu> Peek(std::optional<uint8_t> linkId) const;
  Ptr<WifiMpdu> PeekByTidAndAddress(uint8_t tid, Mac48Address dest,
                                    Ptr<const WifiMpdu> item = nullptr) const;
  Ptr<WifiMpdu> PeekByQueueId(const WifiContainerQueueId &queueId,
                              Ptr<const WifiMpdu> item = nullptr) const;

  Ptr<WifiMpdu> PeekFirstAvailable(uint8_t linkId,
                                   Ptr<const WifiMpdu> item = nullptr) const;
  Ptr<WifiMpdu> Remove() override;
  Ptr<WifiMpdu> Remove(Ptr<const WifiMpdu> item);

  void Flush();

  void Replace(Ptr<const WifiMpdu> currentItem, Ptr<WifiMpdu> newItem);

  uint32_t GetNPackets(const WifiContainerQueueId &queueId) const;

  uint32_t GetNBytes(const WifiContainerQueueId &queueId) const;

  bool TtlExceeded(Ptr<const WifiMpdu> item, const Time &now);

  void ExtractExpiredMpdus(const WifiContainerQueueId &queueId) const;
  void ExtractAllExpiredMpdus() const;
  void WipeAllExpiredMpdus();

  Ptr<WifiMpdu> GetOriginal(Ptr<WifiMpdu> mpdu);

  Ptr<WifiMpdu> GetAlias(Ptr<const WifiMpdu> mpdu, uint8_t linkId);

protected:
  using Queue<WifiMpdu, WifiMacQueueContainer>::GetContainer;

  void DoDispose() override;

private:
  Iterator GetIt(Ptr<const WifiMpdu> mpdu) const;

  bool Insert(ConstIterator pos, Ptr<WifiMpdu> item);
  bool DoEnqueue(ConstIterator pos, Ptr<WifiMpdu> item);
  void DoDequeue(const std::list<ConstIterator> &iterators);
  Ptr<WifiMpdu> DoRemove(ConstIterator pos);

  Time m_maxDelay;
  AcIndex m_ac;
  Ptr<WifiMacQueueScheduler> m_scheduler;

  TracedCallback<Ptr<const WifiMpdu>> m_traceExpired;

  NS_LOG_TEMPLATE_DECLARE;
};

} // namespace ns3

#endif
