
#ifndef UPLINK_SCHEDULER_MBQOS_H
#define UPLINK_SCHEDULER_MBQOS_H

#include "bs-uplink-scheduler.h"
#include "service-flow-record.h"
#include "service-flow.h"
#include "ul-job.h"
#include "ul-mac-messages.h"
#include "wimax-phy.h"

#include "ns3/nstime.h"
#include "ns3/object.h"

#include <stdint.h>

namespace ns3 {

class BaseStationNetDevice;
class SSRecord;
class ServiceFlow;
class ServiceFlowRecord;
class UlJob;

class UplinkSchedulerMBQoS : public UplinkScheduler {
public:
  UplinkSchedulerMBQoS();
  UplinkSchedulerMBQoS(Time time);
  ~UplinkSchedulerMBQoS() override;

  static TypeId GetTypeId();

  std::list<OfdmUlMapIe> GetUplinkAllocations() const override;

  void GetChannelDescriptorsToUpdate(bool &updateDcd, bool &updateUcd,
                                     bool &sendDcd, bool &sendUcd) override;
  uint32_t CalculateAllocationStartTime() override;
  void AddUplinkAllocation(OfdmUlMapIe &ulMapIe, const uint32_t &allocationSize,
                           uint32_t &symbolsToAllocation,
                           uint32_t &availableSymbols) override;

  void Schedule() override;
  void ServiceUnsolicitedGrants(const SSRecord *ssRecord,
                                ServiceFlow::SchedulingType schedulingType,
                                OfdmUlMapIe &ulMapIe,
                                const WimaxPhy::ModulationType modulationType,
                                uint32_t &symbolsToAllocation,
                                uint32_t &availableSymbols) override;
  void ServiceBandwidthRequests(const SSRecord *ssRecord,
                                ServiceFlow::SchedulingType schedulingType,
                                OfdmUlMapIe &ulMapIe,
                                const WimaxPhy::ModulationType modulationType,
                                uint32_t &symbolsToAllocation,
                                uint32_t &availableSymbols) override;
  bool ServiceBandwidthRequests(ServiceFlow *serviceFlow,
                                ServiceFlow::SchedulingType schedulingType,
                                OfdmUlMapIe &ulMapIe,
                                const WimaxPhy::ModulationType modulationType,
                                uint32_t &symbolsToAllocation,
                                uint32_t &availableSymbols) override;
  void AllocateInitialRangingInterval(uint32_t &symbolsToAllocation,
                                      uint32_t &availableSymbols) override;
  void SetupServiceFlow(SSRecord *ssRecord, ServiceFlow *serviceFlow) override;

  void CheckDeadline(uint32_t &availableSymbols);

  void CheckMinimumBandwidth(uint32_t &availableSymbols);

  void UplinkSchedWindowTimer();

  void EnqueueJob(UlJob::JobPriority priority, Ptr<UlJob> job);

  Ptr<UlJob> DequeueJob(UlJob::JobPriority priority);

  void
  ProcessBandwidthRequest(const BandwidthRequestHeader &bwRequestHdr) override;

  Time DetermineDeadline(ServiceFlow *serviceFlow);

  void InitOnce() override;

  uint32_t CountSymbolsQueue(std::list<Ptr<UlJob>> jobs);

  uint32_t CountSymbolsJobs(Ptr<UlJob> job);

  void OnSetRequestedBandwidth(ServiceFlowRecord *sfr) override;

  Ptr<UlJob> CreateUlJob(SSRecord *ssRecord,
                         ServiceFlow::SchedulingType schedType,
                         ReqType reqType);

  uint32_t GetPendingSize(ServiceFlow *serviceFlow);

  bool ServiceBandwidthRequestsBytes(
      ServiceFlow *serviceFlow, ServiceFlow::SchedulingType schedulingType,
      OfdmUlMapIe &ulMapIe, const WimaxPhy::ModulationType modulationType,
      uint32_t &symbolsToAllocation, uint32_t &availableSymbols,
      uint32_t allocationSizeBytes);

private:
  std::list<OfdmUlMapIe> m_uplinkAllocations;

  std::list<Ptr<UlJob>> m_uplinkJobs_high;
  std::list<Ptr<UlJob>> m_uplinkJobs_inter;
  std::list<Ptr<UlJob>> m_uplinkJobs_low;

  Time m_windowInterval;
};

} // namespace ns3

#endif
