
#ifndef BANDWIDTH_MANAGER_H
#define BANDWIDTH_MANAGER_H

#include "bs-uplink-scheduler.h"
#include "cid.h"
#include "ul-job.h"
#include "wimax-net-device.h"

#include <stdint.h>

namespace ns3 {

class SSRecord;
class ServiceFlow;
class UlJob;
class UplinkScheduler;

class BandwidthManager : public Object {
public:
  static TypeId GetTypeId();
  BandwidthManager(Ptr<WimaxNetDevice> device);
  ~BandwidthManager() override;

  BandwidthManager(const BandwidthManager &) = delete;
  BandwidthManager &operator=(const BandwidthManager &) = delete;

  void DoDispose() override;

  uint32_t CalculateAllocationSize(const SSRecord *ssRecord,
                                   const ServiceFlow *serviceFlow);
  ServiceFlow *SelectFlowForRequest(uint32_t &bytesToRequest);
  void SendBandwidthRequest(uint8_t uiuc, uint16_t allocationSize);
  void ProcessBandwidthRequest(const BandwidthRequestHeader &bwRequestHdr);
  void SetSubframeRatio();
  uint32_t GetSymbolsPerFrameAllocated();

private:
  Ptr<WimaxNetDevice> m_device;
  uint16_t m_nrBwReqsSent;
};

} // namespace ns3

#endif
