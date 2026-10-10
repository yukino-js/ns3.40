
#ifndef WIFI_MAC_QUEUE_SCHEDULER_IMPL_H
#define WIFI_MAC_QUEUE_SCHEDULER_IMPL_H

#include "wifi-mac-queue-scheduler.h"
#include "wifi-mac-queue.h"
#include "wifi-mac.h"

#include <algorithm>
#include <functional>
#include <list>
#include <map>
#include <numeric>
#include <unordered_map>
#include <vector>

class WifiMacQueueDropOldestTest;

namespace ns3 {

class WifiMpdu;
class WifiMacQueue;

template <class Priority, class Compare = std::less<Priority>>
class WifiMacQueueSchedulerImpl : public WifiMacQueueScheduler {
public:
  friend class ::WifiMacQueueDropOldestTest;

  static TypeId GetTypeId();

  WifiMacQueueSchedulerImpl();

  void SetWifiMac(Ptr<WifiMac> mac) final;
  std::optional<WifiContainerQueueId>
  GetNext(AcIndex ac, std::optional<uint8_t> linkId) final;
  std::optional<WifiContainerQueueId>
  GetNext(AcIndex ac, std::optional<uint8_t> linkId,
          const WifiContainerQueueId &prevQueueId) final;
  std::list<uint8_t> GetLinkIds(AcIndex ac, Ptr<const WifiMpdu> mpdu) final;
  void BlockQueues(WifiQueueBlockedReason reason, AcIndex ac,
                   const std::list<WifiContainerQueueType> &types,
                   const Mac48Address &rxAddress, const Mac48Address &txAddress,
                   const std::set<uint8_t> &tids,
                   const std::set<uint8_t> &linkIds) final;
  void UnblockQueues(WifiQueueBlockedReason reason, AcIndex ac,
                     const std::list<WifiContainerQueueType> &types,
                     const Mac48Address &rxAddress,
                     const Mac48Address &txAddress,
                     const std::set<uint8_t> &tids,
                     const std::set<uint8_t> &linkIds) final;
  std::optional<Mask> GetQueueLinkMask(AcIndex ac,
                                       const WifiContainerQueueId &queueId,
                                       uint8_t linkId) final;
  Ptr<WifiMpdu> HasToDropBeforeEnqueue(AcIndex ac, Ptr<WifiMpdu> mpdu) final;
  void NotifyEnqueue(AcIndex ac, Ptr<WifiMpdu> mpdu) final;
  void NotifyDequeue(AcIndex ac, const std::list<Ptr<WifiMpdu>> &mpdus) final;
  void NotifyRemove(AcIndex ac, const std::list<Ptr<WifiMpdu>> &mpdus) final;

protected:
  void DoDispose() override;

  void SetPriority(AcIndex ac, const WifiContainerQueueId &queueId,
                   const Priority &priority);

  struct QueueInfo;

  using QueueInfoMap = std::unordered_map<WifiContainerQueueId, QueueInfo>;

  using QueueInfoPair = std::pair<const WifiContainerQueueId, QueueInfo>;

  using SortedQueues =
      std::multimap<Priority, std::reference_wrapper<QueueInfoPair>, Compare>;

  struct QueueInfo {
    std::optional<typename SortedQueues::iterator> priorityIt;
    std::map<uint8_t, Mask> linkIds;
  };

  struct PerAcInfo {
    SortedQueues sortedQueues;
    QueueInfoMap queueInfoMap;
    Ptr<WifiMacQueue> wifiMacQueue;
  };

  const SortedQueues &GetSortedQueues(AcIndex ac) const;

  Ptr<WifiMacQueue> GetWifiMacQueue(AcIndex ac) const;

private:
  typename QueueInfoMap::iterator InitQueueInfo(AcIndex ac,
                                                Ptr<const WifiMpdu> mpdu);

  std::optional<WifiContainerQueueId>
  DoGetNext(AcIndex ac, std::optional<uint8_t> linkId,
            typename SortedQueues::iterator sortedQueuesIt);

  virtual Ptr<WifiMpdu> HasToDropBeforeEnqueuePriv(AcIndex ac,
                                                   Ptr<WifiMpdu> mpdu) = 0;
  virtual void DoNotifyEnqueue(AcIndex ac, Ptr<WifiMpdu> mpdu) = 0;
  virtual void DoNotifyDequeue(AcIndex ac,
                               const std::list<Ptr<WifiMpdu>> &mpdus) = 0;
  virtual void DoNotifyRemove(AcIndex ac,
                              const std::list<Ptr<WifiMpdu>> &mpdus) = 0;

  void DoBlockQueues(bool block, WifiQueueBlockedReason reason, AcIndex ac,
                     const std::list<WifiContainerQueueType> &types,
                     const Mac48Address &rxAddress,
                     const Mac48Address &txAddress,
                     const std::set<uint8_t> &tids,
                     const std::set<uint8_t> &linkIds);

  std::vector<PerAcInfo> m_perAcInfo{AC_UNDEF};
  NS_LOG_TEMPLATE_DECLARE;
};

template <class Priority, class Compare>
WifiMacQueueSchedulerImpl<Priority, Compare>::WifiMacQueueSchedulerImpl()
    : NS_LOG_TEMPLATE_DEFINE("WifiMacQueueScheduler") {}

template <class Priority, class Compare>
TypeId WifiMacQueueSchedulerImpl<Priority, Compare>::GetTypeId() {
  static TypeId tid = TypeId("ns3::WifiMacQueueSchedulerImpl")
                          .SetParent<WifiMacQueueScheduler>()
                          .SetGroupName("Wifi");
  return tid;
}

template <class Priority, class Compare>
void WifiMacQueueSchedulerImpl<Priority, Compare>::DoDispose() {
  m_perAcInfo.clear();
  WifiMacQueueScheduler::DoDispose();
}

template <class Priority, class Compare>
void WifiMacQueueSchedulerImpl<Priority, Compare>::SetWifiMac(
    Ptr<WifiMac> mac) {
  for (auto ac : {AC_BE, AC_BK, AC_VI, AC_VO, AC_BE_NQOS, AC_BEACON}) {
    if (auto queue = mac->GetTxopQueue(ac); queue != nullptr) {
      m_perAcInfo.at(ac).wifiMacQueue = queue;
      queue->SetScheduler(this);
    }
  }
  WifiMacQueueScheduler::SetWifiMac(mac);
}

template <class Priority, class Compare>
Ptr<WifiMacQueue> WifiMacQueueSchedulerImpl<Priority, Compare>::GetWifiMacQueue(
    AcIndex ac) const {
  NS_ASSERT(static_cast<uint8_t>(ac) < AC_UNDEF);
  return m_perAcInfo.at(ac).wifiMacQueue;
}

template <class Priority, class Compare>
const typename WifiMacQueueSchedulerImpl<Priority, Compare>::SortedQueues &
WifiMacQueueSchedulerImpl<Priority, Compare>::GetSortedQueues(
    AcIndex ac) const {
  NS_ASSERT(static_cast<uint8_t>(ac) < AC_UNDEF);
  return m_perAcInfo.at(ac).sortedQueues;
}

template <class Priority, class Compare>
typename WifiMacQueueSchedulerImpl<Priority, Compare>::QueueInfoMap::iterator
WifiMacQueueSchedulerImpl<Priority, Compare>::InitQueueInfo(
    AcIndex ac, Ptr<const WifiMpdu> mpdu) {
  NS_LOG_FUNCTION(this << ac << *mpdu);

  auto queueId = WifiMacQueueContainer::GetQueueId(mpdu);
  auto [queueInfoIt, ret] =
      m_perAcInfo[ac].queueInfoMap.insert({queueId, QueueInfo()});

  if (GetMac() && GetMac()->GetNLinks() > 1 &&
      mpdu->GetHeader().GetAddr2() == GetMac()->GetAddress()) {
    const auto rxAddr = mpdu->GetHeader().GetAddr1();

    NS_ASSERT_MSG(rxAddr.IsGroup() || GetMac()->GetMldAddress(rxAddr) == rxAddr,
                  "Address 1 (" << rxAddr << ") is not an MLD address");

    NS_ASSERT_MSG(GetMac()->CanForwardPacketsTo(rxAddr),
                  "Cannot forward frame to " << rxAddr);
    for (const auto linkId : GetMac()->GetLinkIds()) {
      if (rxAddr.IsGroup() || GetMac()
                                  ->GetWifiRemoteStationManager(linkId)
                                  ->GetAffiliatedStaAddress(rxAddr)) {
        queueInfoIt->second.linkIds.emplace(linkId, Mask{});
      } else {
        queueInfoIt->second.linkIds.erase(linkId);
      }
    }
  } else {
    auto linkId =
        GetMac() ? GetMac()->GetLinkIdByAddress(mpdu->GetHeader().GetAddr2())
                 : SINGLE_LINK_OP_ID;
    NS_ASSERT(linkId.has_value());
    auto &linkIdsMap = queueInfoIt->second.linkIds;
    NS_ASSERT_MSG(
        linkIdsMap.size() <= 1,
        "At most one link can be associated with this container queue");
    if (linkIdsMap.empty() || linkIdsMap.cbegin()->first != *linkId) {
      linkIdsMap = {{*linkId, Mask{}}};
    }
  }

  return queueInfoIt;
}

template <class Priority, class Compare>
void WifiMacQueueSchedulerImpl<Priority, Compare>::SetPriority(
    AcIndex ac, const WifiContainerQueueId &queueId, const Priority &priority) {
  NS_LOG_FUNCTION(this << +ac);
  NS_ASSERT(static_cast<uint8_t>(ac) < AC_UNDEF);

  NS_ABORT_MSG_IF(GetWifiMacQueue(ac)->GetNBytes(queueId) == 0,
                  "Cannot set the priority of an empty queue");

  auto queueInfoIt = m_perAcInfo[ac].queueInfoMap.find(queueId);
  NS_ASSERT_MSG(queueInfoIt != m_perAcInfo[ac].queueInfoMap.end(),
                "No queue info for the given container queue");
  typename SortedQueues::iterator sortedQueuesIt;

  if (queueInfoIt->second.priorityIt.has_value()) {
    if (queueInfoIt->second.priorityIt.value()->first == priority) {
      return;
    }

    auto handle = m_perAcInfo[ac].sortedQueues.extract(
        queueInfoIt->second.priorityIt.value());
    handle.key() = priority;
    sortedQueuesIt = m_perAcInfo[ac].sortedQueues.insert(std::move(handle));
  } else {
    sortedQueuesIt =
        m_perAcInfo[ac].sortedQueues.insert({priority, std::ref(*queueInfoIt)});
  }
  queueInfoIt->second.priorityIt = sortedQueuesIt;
}

template <class Priority, class Compare>
std::list<uint8_t> WifiMacQueueSchedulerImpl<Priority, Compare>::GetLinkIds(
    AcIndex ac, Ptr<const WifiMpdu> mpdu) {
  auto queueInfoIt = InitQueueInfo(ac, mpdu);
  std::list<uint8_t> linkIds;

  for (const auto [linkId, mask] : queueInfoIt->second.linkIds) {
    if (mask.none()) {
      linkIds.emplace_back(linkId);
    }
  }

  return linkIds;
}

template <class Priority, class Compare>
void WifiMacQueueSchedulerImpl<Priority, Compare>::DoBlockQueues(
    bool block, WifiQueueBlockedReason reason, AcIndex ac,
    const std::list<WifiContainerQueueType> &types,
    const Mac48Address &rxAddress, const Mac48Address &txAddress,
    const std::set<uint8_t> &tids, const std::set<uint8_t> &linkIds) {
  NS_LOG_FUNCTION(this << block << reason << ac << rxAddress << txAddress);
  std::list<WifiMacHeader> headers;

  for (const auto queueType : types) {
    switch (queueType) {
    case WIFI_CTL_QUEUE:
      headers.emplace_back(WIFI_MAC_CTL_BACKREQ);
      break;
    case WIFI_MGT_QUEUE:
      headers.emplace_back(WIFI_MAC_MGT_ACTION);
      break;
    case WIFI_QOSDATA_QUEUE:
      NS_ASSERT_MSG(
          !tids.empty(),
          "TID must be specified for queues containing QoS data frames");
      for (const auto tid : tids) {
        headers.emplace_back(WIFI_MAC_QOSDATA);
        headers.back().SetQosTid(tid);
      }
      break;
    case WIFI_DATA_QUEUE:
      headers.emplace_back(WIFI_MAC_DATA);
      break;
    }
  }
  for (auto &hdr : headers) {
    hdr.SetAddr1(rxAddress);
    hdr.SetAddr2(txAddress);

    auto queueInfoIt =
        InitQueueInfo(ac, Create<WifiMpdu>(Create<Packet>(), hdr));
    for (auto &[linkId, mask] : queueInfoIt->second.linkIds) {
      if (linkIds.empty() || linkIds.count(linkId) > 0) {
        mask.set(static_cast<std::size_t>(reason), block);
      }
    }
  }
}

template <class Priority, class Compare>
void WifiMacQueueSchedulerImpl<Priority, Compare>::BlockQueues(
    WifiQueueBlockedReason reason, AcIndex ac,
    const std::list<WifiContainerQueueType> &types,
    const Mac48Address &rxAddress, const Mac48Address &txAddress,
    const std::set<uint8_t> &tids, const std::set<uint8_t> &linkIds) {
  DoBlockQueues(true, reason, ac, types, rxAddress, txAddress, tids, linkIds);
}

template <class Priority, class Compare>
void WifiMacQueueSchedulerImpl<Priority, Compare>::UnblockQueues(
    WifiQueueBlockedReason reason, AcIndex ac,
    const std::list<WifiContainerQueueType> &types,
    const Mac48Address &rxAddress, const Mac48Address &txAddress,
    const std::set<uint8_t> &tids, const std::set<uint8_t> &linkIds) {
  DoBlockQueues(false, reason, ac, types, rxAddress, txAddress, tids, linkIds);
}

template <class Priority, class Compare>
std::optional<WifiMacQueueScheduler::Mask>
WifiMacQueueSchedulerImpl<Priority, Compare>::GetQueueLinkMask(
    AcIndex ac, const WifiContainerQueueId &queueId, uint8_t linkId) {
  NS_LOG_FUNCTION(this << +ac << +linkId);

  const auto queueInfoIt = m_perAcInfo[ac].queueInfoMap.find(queueId);

  if (queueInfoIt == m_perAcInfo[ac].queueInfoMap.cend()) {
    return std::nullopt;
  }

  const auto &linkIds = queueInfoIt->second.linkIds;
  if (const auto linkIt = linkIds.find(linkId); linkIt != linkIds.cend()) {
    return linkIt->second;
  }

  return std::nullopt;
}

template <class Priority, class Compare>
std::optional<WifiContainerQueueId>
WifiMacQueueSchedulerImpl<Priority, Compare>::GetNext(
    AcIndex ac, std::optional<uint8_t> linkId) {
  NS_LOG_FUNCTION(this << +ac << linkId.has_value());
  return DoGetNext(ac, linkId, m_perAcInfo[ac].sortedQueues.begin());
}

template <class Priority, class Compare>
std::optional<WifiContainerQueueId>
WifiMacQueueSchedulerImpl<Priority, Compare>::GetNext(
    AcIndex ac, std::optional<uint8_t> linkId,
    const WifiContainerQueueId &prevQueueId) {
  NS_LOG_FUNCTION(this << +ac << linkId.has_value());

  auto queueInfoIt = m_perAcInfo[ac].queueInfoMap.find(prevQueueId);
  NS_ABORT_IF(queueInfoIt == m_perAcInfo[ac].queueInfoMap.end() ||
              !queueInfoIt->second.priorityIt.has_value());

  auto sortedQueuesIt = queueInfoIt->second.priorityIt.value();
  NS_ABORT_IF(sortedQueuesIt == m_perAcInfo[ac].sortedQueues.end());

  return DoGetNext(ac, linkId, ++sortedQueuesIt);
}

template <class Priority, class Compare>
std::optional<WifiContainerQueueId>
WifiMacQueueSchedulerImpl<Priority, Compare>::DoGetNext(
    AcIndex ac, std::optional<uint8_t> linkId,
    typename SortedQueues::iterator sortedQueuesIt) {
  NS_LOG_FUNCTION(this << +ac << linkId.has_value());
  NS_ASSERT(static_cast<uint8_t>(ac) < AC_UNDEF);

  while (sortedQueuesIt != m_perAcInfo[ac].sortedQueues.end()) {
    const auto &queueInfoPair = sortedQueuesIt->second.get();
    const auto &linkIds = queueInfoPair.second.linkIds;
    typename std::decay_t<decltype(linkIds)>::const_iterator linkIt;

    if (!linkId.has_value() ||
        ((linkIt = linkIds.find(*linkId)) != linkIds.cend() &&
         linkIt->second.none())) {
      std::optional<typename SortedQueues::iterator> prevQueueIt;
      if (sortedQueuesIt != m_perAcInfo[ac].sortedQueues.begin()) {
        prevQueueIt = std::prev(sortedQueuesIt);
      }

      GetWifiMacQueue(ac)->ExtractExpiredMpdus(queueInfoPair.first);

      if (GetWifiMacQueue(ac)->GetNBytes(queueInfoPair.first) == 0) {
        sortedQueuesIt =
            (prevQueueIt.has_value() ? std::next(prevQueueIt.value())
                                     : m_perAcInfo[ac].sortedQueues.begin());
        continue;
      }
      break;
    }

    sortedQueuesIt++;
  }

  std::optional<WifiContainerQueueId> queueId;

  if (sortedQueuesIt != m_perAcInfo[ac].sortedQueues.end()) {
    queueId = sortedQueuesIt->second.get().first;
  }
  return queueId;
}

template <class Priority, class Compare>
Ptr<WifiMpdu>
WifiMacQueueSchedulerImpl<Priority, Compare>::HasToDropBeforeEnqueue(
    AcIndex ac, Ptr<WifiMpdu> mpdu) {
  NS_LOG_FUNCTION(this << +ac << *mpdu);
  return HasToDropBeforeEnqueuePriv(ac, mpdu);
}

template <class Priority, class Compare>
void WifiMacQueueSchedulerImpl<Priority, Compare>::NotifyEnqueue(
    AcIndex ac, Ptr<WifiMpdu> mpdu) {
  NS_LOG_FUNCTION(this << +ac << *mpdu);
  NS_ASSERT(static_cast<uint8_t>(ac) < AC_UNDEF);

  auto queueInfoIt = InitQueueInfo(ac, mpdu);

  DoNotifyEnqueue(ac, mpdu);

  if (!queueInfoIt->second.priorityIt.has_value()) {
    NS_ABORT_MSG("No info for the queue the MPDU was stored into (forgot to "
                 "call SetPriority()?)");
  }
}

template <class Priority, class Compare>
void WifiMacQueueSchedulerImpl<Priority, Compare>::NotifyDequeue(
    AcIndex ac, const std::list<Ptr<WifiMpdu>> &mpdus) {
  NS_LOG_FUNCTION(this << +ac);
  NS_ASSERT(static_cast<uint8_t>(ac) < AC_UNDEF);

  DoNotifyDequeue(ac, mpdus);

  std::list<WifiContainerQueueId> queueIds;

  for (const auto &mpdu : mpdus) {
    queueIds.push_back(WifiMacQueueContainer::GetQueueId(mpdu));
  }

  for (const auto &queueId : queueIds) {
    if (GetWifiMacQueue(ac)->GetNBytes(queueId) == 0) {
      auto queueInfoIt = m_perAcInfo[ac].queueInfoMap.find(queueId);
      NS_ASSERT(queueInfoIt != m_perAcInfo[ac].queueInfoMap.end());
      if (queueInfoIt->second.priorityIt.has_value()) {
        m_perAcInfo[ac].sortedQueues.erase(
            queueInfoIt->second.priorityIt.value());
        queueInfoIt->second.priorityIt.reset();
      }
    }
  }
}

template <class Priority, class Compare>
void WifiMacQueueSchedulerImpl<Priority, Compare>::NotifyRemove(
    AcIndex ac, const std::list<Ptr<WifiMpdu>> &mpdus) {
  NS_LOG_FUNCTION(this << +ac);
  NS_ASSERT(static_cast<uint8_t>(ac) < AC_UNDEF);

  DoNotifyRemove(ac, mpdus);

  std::list<WifiContainerQueueId> queueIds;

  for (const auto &mpdu : mpdus) {
    queueIds.push_back(WifiMacQueueContainer::GetQueueId(mpdu));
  }

  for (const auto &queueId : queueIds) {
    if (GetWifiMacQueue(ac)->GetNBytes(queueId) == 0) {
      auto queueInfoIt = m_perAcInfo[ac].queueInfoMap.find(queueId);
      NS_ASSERT(queueInfoIt != m_perAcInfo[ac].queueInfoMap.end());
      if (queueInfoIt->second.priorityIt.has_value()) {
        m_perAcInfo[ac].sortedQueues.erase(
            queueInfoIt->second.priorityIt.value());
        queueInfoIt->second.priorityIt.reset();
      }
    }
  }
}

} // namespace ns3

#endif
