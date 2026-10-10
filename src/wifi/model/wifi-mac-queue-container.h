
#ifndef WIFI_MAC_QUEUE_CONTAINER_H
#define WIFI_MAC_QUEUE_CONTAINER_H

#include "wifi-mac-queue-elem.h"

#include "ns3/mac48-address.h"

#include <list>
#include <optional>
#include <tuple>
#include <unordered_map>

namespace ns3 {

enum WifiContainerQueueType {
  WIFI_CTL_QUEUE = 0,
  WIFI_MGT_QUEUE = 1,
  WIFI_QOSDATA_QUEUE = 2,
  WIFI_DATA_QUEUE = 3
};

enum WifiReceiverAddressType : uint8_t { WIFI_UNICAST = 0, WIFI_BROADCAST };

using WifiContainerQueueId =
    std::tuple<WifiContainerQueueType, WifiReceiverAddressType, Mac48Address,
               std::optional<uint8_t>>;

} // namespace ns3

template <> struct std::hash<ns3::WifiContainerQueueId> {
  std::size_t operator()(ns3::WifiContainerQueueId queueId) const;
};

namespace ns3 {

class WifiMacQueueContainer {
public:
  using ContainerQueue = std::list<WifiMacQueueElem>;
  using iterator = ContainerQueue::iterator;
  using const_iterator = ContainerQueue::const_iterator;

  void clear();

  iterator insert(const_iterator pos, Ptr<WifiMpdu> item);

  iterator erase(const_iterator pos);

  Ptr<WifiMpdu> GetItem(const const_iterator it) const;

  static WifiContainerQueueId GetQueueId(Ptr<const WifiMpdu> mpdu);

  const ContainerQueue &GetQueue(const WifiContainerQueueId &queueId) const;

  uint32_t GetNBytes(const WifiContainerQueueId &queueId) const;

  std::pair<iterator, iterator>
  ExtractExpiredMpdus(const WifiContainerQueueId &queueId) const;
  std::pair<iterator, iterator> ExtractAllExpiredMpdus() const;
  std::pair<iterator, iterator> GetAllExpiredMpdus() const;

private:
  std::pair<iterator, iterator>
  DoExtractExpiredMpdus(ContainerQueue &queue) const;

  mutable std::unordered_map<WifiContainerQueueId, ContainerQueue> m_queues;
  mutable ContainerQueue m_expiredQueue;
  mutable std::unordered_map<WifiContainerQueueId, uint32_t> m_nBytesPerQueue;
};

} // namespace ns3

#endif
