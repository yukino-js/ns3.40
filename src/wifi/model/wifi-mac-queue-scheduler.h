
#ifndef WIFI_MAC_QUEUE_SCHEDULER_H
#define WIFI_MAC_QUEUE_SCHEDULER_H

#include "qos-utils.h"
#include "wifi-mac-queue-container.h"

#include "ns3/object.h"

#include <bitset>
#include <optional>

namespace ns3 {

class WifiMpdu;
class WifiMac;

enum class WifiQueueBlockedReason : uint8_t {
  WAITING_ADDBA_RESP = 0,
  POWER_SAVE_MODE,
  USING_OTHER_EMLSR_LINK,
  WAITING_EMLSR_TRANSITION_DELAY,
  TID_NOT_MAPPED,
  REASONS_COUNT
};

inline std::ostream &operator<<(std::ostream &os,
                                WifiQueueBlockedReason reason) {
  switch (reason) {
  case WifiQueueBlockedReason::WAITING_ADDBA_RESP:
    return (os << "WAITING_ADDBA_RESP");
  case WifiQueueBlockedReason::POWER_SAVE_MODE:
    return (os << "POWER_SAVE_MODE");
  case WifiQueueBlockedReason::USING_OTHER_EMLSR_LINK:
    return (os << "USING_OTHER_EMLSR_LINK");
  case WifiQueueBlockedReason::WAITING_EMLSR_TRANSITION_DELAY:
    return (os << "WAITING_EMLSR_TRANSITION_DELAY");
  case WifiQueueBlockedReason::TID_NOT_MAPPED:
    return (os << "TID_NOT_MAPPED");
  case WifiQueueBlockedReason::REASONS_COUNT:
    return (os << "REASONS_COUNT");
  default:
    NS_ABORT_MSG("Unknown queue blocked reason");
    return (os << "unknown");
  }
}

class WifiMacQueueScheduler : public Object {
public:
  static TypeId GetTypeId();

  virtual void SetWifiMac(Ptr<WifiMac> mac);

  virtual std::optional<WifiContainerQueueId>
  GetNext(AcIndex ac, std::optional<uint8_t> linkId) = 0;
  virtual std::optional<WifiContainerQueueId>
  GetNext(AcIndex ac, std::optional<uint8_t> linkId,
          const WifiContainerQueueId &prevQueueId) = 0;

  virtual std::list<uint8_t> GetLinkIds(AcIndex ac,
                                        Ptr<const WifiMpdu> mpdu) = 0;

  virtual void BlockQueues(WifiQueueBlockedReason reason, AcIndex ac,
                           const std::list<WifiContainerQueueType> &types,
                           const Mac48Address &rxAddress,
                           const Mac48Address &txAddress,
                           const std::set<uint8_t> &tids = {},
                           const std::set<uint8_t> &linkIds = {}) = 0;
  virtual void UnblockQueues(WifiQueueBlockedReason reason, AcIndex ac,
                             const std::list<WifiContainerQueueType> &types,
                             const Mac48Address &rxAddress,
                             const Mac48Address &txAddress,
                             const std::set<uint8_t> &tids = {},
                             const std::set<uint8_t> &linkIds = {}) = 0;

  using Mask = std::bitset<static_cast<std::size_t>(
      WifiQueueBlockedReason::REASONS_COUNT)>;

  virtual std::optional<Mask>
  GetQueueLinkMask(AcIndex ac, const WifiContainerQueueId &queueId,
                   uint8_t linkId) = 0;

  virtual Ptr<WifiMpdu> HasToDropBeforeEnqueue(AcIndex ac,
                                               Ptr<WifiMpdu> mpdu) = 0;
  virtual void NotifyEnqueue(AcIndex ac, Ptr<WifiMpdu> mpdu) = 0;
  virtual void NotifyDequeue(AcIndex ac,
                             const std::list<Ptr<WifiMpdu>> &mpdus) = 0;
  virtual void NotifyRemove(AcIndex ac,
                            const std::list<Ptr<WifiMpdu>> &mpdus) = 0;

protected:
  void DoDispose() override;

  Ptr<WifiMac> GetMac() const;

private:
  Ptr<WifiMac> m_mac;
};

} // namespace ns3

#endif
