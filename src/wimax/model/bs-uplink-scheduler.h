
#ifndef UPLINK_SCHEDULER_H
#define UPLINK_SCHEDULER_H

#include "service-flow-record.h"
#include "service-flow.h"
#include "ul-mac-messages.h"
#include "wimax-phy.h"

#include "ns3/nstime.h"

#include <stdint.h>

namespace ns3 {

class BaseStationNetDevice;
class SSRecord;
class ServiceFlow;
class ServiceFlowRecord;

class UplinkScheduler : public Object {
public:
  UplinkScheduler();
  UplinkScheduler(Ptr<BaseStationNetDevice> bs);
  ~UplinkScheduler() override;

  static TypeId GetTypeId();

  virtual uint8_t GetNrIrOppsAllocated() const;
  virtual void SetNrIrOppsAllocated(uint8_t nrIrOppsAllocated);

  virtual bool GetIsIrIntrvlAllocated() const;
  virtual void SetIsIrIntrvlAllocated(bool isIrIntrvlAllocated);

  virtual bool GetIsInvIrIntrvlAllocated() const;
  virtual void SetIsInvIrIntrvlAllocated(bool isInvIrIntrvlAllocated);

  virtual std::list<OfdmUlMapIe> GetUplinkAllocations() const;

  virtual Time GetTimeStampIrInterval();
  virtual void SetTimeStampIrInterval(Time timeStampIrInterval);

  virtual Time GetDcdTimeStamp() const;
  virtual void SetDcdTimeStamp(Time dcdTimeStamp);

  virtual Time GetUcdTimeStamp() const;
  virtual void SetUcdTimeStamp(Time ucdTimeStamp);

  virtual Ptr<BaseStationNetDevice> GetBs();
  virtual void SetBs(Ptr<BaseStationNetDevice> bs);
  virtual void GetChannelDescriptorsToUpdate(bool &, bool &, bool &,
                                             bool &) = 0;
  virtual uint32_t CalculateAllocationStartTime() = 0;
  virtual void AddUplinkAllocation(OfdmUlMapIe &ulMapIe,
                                   const uint32_t &allocationSize,
                                   uint32_t &symbolsToAllocation,
                                   uint32_t &availableSymbols) = 0;
  virtual void Schedule() = 0;
  virtual void ServiceUnsolicitedGrants(
      const SSRecord *ssRecord, ServiceFlow::SchedulingType schedulingType,
      OfdmUlMapIe &ulMapIe, const WimaxPhy::ModulationType modulationType,
      uint32_t &symbolsToAllocation, uint32_t &availableSymbols) = 0;
  virtual void ServiceBandwidthRequests(
      const SSRecord *ssRecord, ServiceFlow::SchedulingType schedulingType,
      OfdmUlMapIe &ulMapIe, const WimaxPhy::ModulationType modulationType,
      uint32_t &symbolsToAllocation, uint32_t &availableSymbols) = 0;
  virtual bool ServiceBandwidthRequests(
      ServiceFlow *serviceFlow, ServiceFlow::SchedulingType schedulingType,
      OfdmUlMapIe &ulMapIe, const WimaxPhy::ModulationType modulationType,
      uint32_t &symbolsToAllocation, uint32_t &availableSymbols) = 0;
  virtual void AllocateInitialRangingInterval(uint32_t &symbolsToAllocation,
                                              uint32_t &availableSymbols) = 0;
  virtual void SetupServiceFlow(SSRecord *ssRecord,
                                ServiceFlow *serviceFlow) = 0;
  virtual void
  ProcessBandwidthRequest(const BandwidthRequestHeader &bwRequestHdr) = 0;

  virtual void InitOnce() = 0;

  virtual void OnSetRequestedBandwidth(ServiceFlowRecord *sfr) = 0;

private:
  Ptr<BaseStationNetDevice> m_bs;
  std::list<OfdmUlMapIe> m_uplinkAllocations;
  Time m_timeStampIrInterval;
  uint8_t m_nrIrOppsAllocated;
  bool m_isIrIntrvlAllocated;
  bool m_isInvIrIntrvlAllocated;
  Time m_dcdTimeStamp;
  Time m_ucdTimeStamp;
};

} // namespace ns3

#endif
