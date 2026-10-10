
#ifndef UPLINK_SCHEDULER_SIMPLE_H
#define UPLINK_SCHEDULER_SIMPLE_H

#include "bs-uplink-scheduler.h"
#include "service-flow.h"
#include "ul-mac-messages.h"
#include "wimax-phy.h"

#include "ns3/nstime.h"

#include <stdint.h>

namespace ns3 {

class BaseStationNetDevice;
class SSRecord;
class ServiceFlow;

class UplinkSchedulerSimple : public UplinkScheduler {
public:
  UplinkSchedulerSimple();
  UplinkSchedulerSimple(Ptr<BaseStationNetDevice> bs);
  ~UplinkSchedulerSimple() override;

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

  void
  ProcessBandwidthRequest(const BandwidthRequestHeader &bwRequestHdr) override;

  void InitOnce() override;

  void OnSetRequestedBandwidth(ServiceFlowRecord *sfr) override;

private:
  std::list<OfdmUlMapIe> m_uplinkAllocations;
};

} // namespace ns3

#endif
