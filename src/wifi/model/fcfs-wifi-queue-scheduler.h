
#ifndef FCFS_WIFI_QUEUE_SCHEDULER_H
#define FCFS_WIFI_QUEUE_SCHEDULER_H

#include "wifi-mac-queue-scheduler-impl.h"

#include "ns3/nstime.h"

namespace ns3 {

class WifiMpdu;

struct FcfsPrio {
  Time priority;
  WifiContainerQueueType type;
};

bool operator==(const FcfsPrio &lhs, const FcfsPrio &rhs);
bool operator<(const FcfsPrio &lhs, const FcfsPrio &rhs);

class FcfsWifiQueueScheduler : public WifiMacQueueSchedulerImpl<FcfsPrio> {
public:
  static TypeId GetTypeId();

  FcfsWifiQueueScheduler();

  enum DropPolicy { DROP_NEWEST, DROP_OLDEST };

private:
  Ptr<WifiMpdu> HasToDropBeforeEnqueuePriv(AcIndex ac,
                                           Ptr<WifiMpdu> mpdu) override;
  void DoNotifyEnqueue(AcIndex ac, Ptr<WifiMpdu> mpdu) override;
  void DoNotifyDequeue(AcIndex ac,
                       const std::list<Ptr<WifiMpdu>> &mpdus) override;
  void DoNotifyRemove(AcIndex ac,
                      const std::list<Ptr<WifiMpdu>> &mpdus) override;

  DropPolicy m_dropPolicy;
  NS_LOG_TEMPLATE_DECLARE;
};

} // namespace ns3

#endif
