
#ifndef LINK_MANAGER_H
#define LINK_MANAGER_H

#include "bs-net-device.h"
#include "cid.h"
#include "mac-messages.h"
#include "wimax-net-device.h"

#include "ns3/event-id.h"

#include <stdint.h>

namespace ns3 {

class BSLinkManager : public Object {
public:
  static TypeId GetTypeId();
  BSLinkManager(Ptr<BaseStationNetDevice> bs);
  ~BSLinkManager() override;

  BSLinkManager(const BSLinkManager &) = delete;
  BSLinkManager &operator=(const BSLinkManager &) = delete;

  uint8_t CalculateRangingOppsToAllocate();
  uint64_t SelectDlChannel();

  void ProcessRangingRequest(Cid cid, RngReq rngreq);
  void VerifyInvitedRanging(Cid cid, uint8_t uiuc);

private:
  void PerformRanging(Cid cid, RngReq rngreq);
  void PerformInitialRanging(Cid cid, RngReq *rngreq, RngRsp *rngrsp);
  void PerformInvitedRanging(Cid cid, RngRsp *rngrsp);

  void SetParametersToAdjust(RngRsp *rngrsp);
  void AbortRanging(Cid cid, RngRsp *rngrsp, SSRecord *ssRecord, bool isNewSS);
  void AcceptRanging(Cid cid, RngRsp *rngrsp, SSRecord *ssRecord);
  void ContinueRanging(Cid cid, RngRsp *rngrsp, SSRecord *ssRecord);
  void ScheduleRngRspMessage(Cid cid, RngRsp *rngrsp);
  void DeallocateCids(Cid cid);

  bool ChangeDlChannel();
  uint32_t GetNewDlChannel();
  uint8_t GetSignalQuality();
  bool IsRangingAcceptable();

  Ptr<BaseStationNetDevice> m_bs;

  uint32_t m_signalQuality;
  uint8_t m_signalQualityThreshold;
  int tries;
};

} // namespace ns3

#endif
