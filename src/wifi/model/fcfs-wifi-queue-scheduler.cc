
#include "fcfs-wifi-queue-scheduler.h"

#include "wifi-mac-queue.h"

#include "ns3/enum.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("FcfsWifiQueueScheduler");

bool operator==(const FcfsPrio &lhs, const FcfsPrio &rhs) {
  return lhs.priority == rhs.priority && lhs.type == rhs.type;
}

bool operator<(const FcfsPrio &lhs, const FcfsPrio &rhs) {
  if (lhs.type == WIFI_CTL_QUEUE && rhs.type != WIFI_CTL_QUEUE) {
    return true;
  }
  if (lhs.type != WIFI_CTL_QUEUE && rhs.type == WIFI_CTL_QUEUE) {
    return false;
  }
  if (lhs.type == WIFI_MGT_QUEUE && rhs.type != WIFI_MGT_QUEUE) {
    return true;
  }
  if (lhs.type != WIFI_MGT_QUEUE && rhs.type == WIFI_MGT_QUEUE) {
    return false;
  }
  return lhs.priority < rhs.priority;
}

NS_OBJECT_ENSURE_REGISTERED(FcfsWifiQueueScheduler);

TypeId FcfsWifiQueueScheduler::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::FcfsWifiQueueScheduler")
          .SetParent<WifiMacQueueSchedulerImpl<Time>>()
          .SetGroupName("Wifi")
          .AddConstructor<FcfsWifiQueueScheduler>()
          .AddAttribute(
              "DropPolicy",
              "Upon enqueue with full queue, drop oldest (DropOldest) "
              "or newest (DropNewest) packet",
              EnumValue(DROP_NEWEST),
              MakeEnumAccessor(&FcfsWifiQueueScheduler::m_dropPolicy),
              MakeEnumChecker(FcfsWifiQueueScheduler::DROP_OLDEST, "DropOldest",
                              FcfsWifiQueueScheduler::DROP_NEWEST,
                              "DropNewest"));
  return tid;
}

FcfsWifiQueueScheduler::FcfsWifiQueueScheduler()
    : NS_LOG_TEMPLATE_DEFINE("FcfsWifiQueueScheduler") {}

Ptr<WifiMpdu>
FcfsWifiQueueScheduler::HasToDropBeforeEnqueuePriv(AcIndex ac,
                                                   Ptr<WifiMpdu> mpdu) {
  auto queue = GetWifiMacQueue(ac);
  if (queue->QueueBase::GetNPackets() < queue->GetMaxSize().GetValue()) {
    return nullptr;
  }

  if (m_dropPolicy == DROP_OLDEST || mpdu->GetHeader().IsCtl() ||
      mpdu->GetHeader().IsMgt()) {
    for (const auto &[priority, queueInfo] : GetSortedQueues(ac)) {
      if (std::get<WifiContainerQueueType>(queueInfo.get().first) ==
              WIFI_MGT_QUEUE ||
          std::get<WifiContainerQueueType>(queueInfo.get().first) ==
              WIFI_CTL_QUEUE) {
        continue;
      }

      Ptr<WifiMpdu> item;
      while ((item = queue->PeekByQueueId(queueInfo.get().first, item))) {
        if (!item->IsInFlight() && !item->GetHeader().IsRetry()) {
          NS_LOG_DEBUG("Dropping " << *item);
          return item;
        }
      }
    }
  }
  NS_LOG_DEBUG("Dropping received MPDU: " << *mpdu);
  return mpdu;
}

void FcfsWifiQueueScheduler::DoNotifyEnqueue(AcIndex ac, Ptr<WifiMpdu> mpdu) {
  NS_LOG_FUNCTION(this << +ac << *mpdu);

  const auto queueId = WifiMacQueueContainer::GetQueueId(mpdu);

  auto item = GetWifiMacQueue(ac)->PeekByQueueId(queueId);
  NS_ASSERT(item);

  SetPriority(
      ac, queueId,
      {item->GetTimestamp(), std::get<WifiContainerQueueType>(queueId)});
}

void FcfsWifiQueueScheduler::DoNotifyDequeue(
    AcIndex ac, const std::list<Ptr<WifiMpdu>> &mpdus) {
  NS_LOG_FUNCTION(this << +ac << mpdus.size());

  std::set<WifiContainerQueueId> queueIds;

  for (const auto &mpdu : mpdus) {
    queueIds.insert(WifiMacQueueContainer::GetQueueId(mpdu));
  }

  for (const auto &queueId : queueIds) {
    if (auto item = GetWifiMacQueue(ac)->PeekByQueueId(queueId)) {
      SetPriority(
          ac, queueId,
          {item->GetTimestamp(), std::get<WifiContainerQueueType>(queueId)});
    }
  }
}

void FcfsWifiQueueScheduler::DoNotifyRemove(
    AcIndex ac, const std::list<Ptr<WifiMpdu>> &mpdus) {
  NS_LOG_FUNCTION(this << +ac << mpdus.size());

  std::set<WifiContainerQueueId> queueIds;

  for (const auto &mpdu : mpdus) {
    queueIds.insert(WifiMacQueueContainer::GetQueueId(mpdu));
  }

  for (const auto &queueId : queueIds) {
    if (auto item = GetWifiMacQueue(ac)->PeekByQueueId(queueId)) {
      SetPriority(
          ac, queueId,
          {item->GetTimestamp(), std::get<WifiContainerQueueType>(queueId)});
    }
  }
}

} // namespace ns3
