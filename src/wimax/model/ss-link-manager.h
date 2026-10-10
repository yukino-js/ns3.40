
#ifndef LINK_MANAGER_H
#define LINK_MANAGER_H

#include "cid.h"
#include "mac-messages.h"
#include "ss-net-device.h"
#include "wimax-net-device.h"

#include "ns3/event-id.h"
#include "ns3/nstime.h"

#include <stdint.h>

namespace ns3 {

class SSLinkManager : public Object {
public:
  static TypeId GetTypeId();
  SSLinkManager(Ptr<SubscriberStationNetDevice> ss);
  ~SSLinkManager() override;
  void DoDispose() override;

  void SetBsEirp(uint16_t bs_eirp);
  void SetEirXPIrMax(uint16_t eir_x_p_ir_max);
  void SetRangingIntervalFound(bool rangingIntervalFound);
  bool GetRangingIntervalFound() const;
  void SetNrRangingTransOpps(uint8_t nrRangingTransOpps);
  void SetRangingCW(uint8_t rangingCW);
  void IncrementNrInvitedPollsRecvd();
  EventId GetDlMapSyncTimeoutEvent();

  void PerformRanging(Cid cid, RngRsp rngrsp);
  void StartScanning(SubscriberStationNetDevice::EventType type,
                     bool deleteParameters);
  void SendRangingRequest(uint8_t uiuc, uint16_t allocationSize);
  void StartContentionResolution();
  void PerformBackoff();
  bool IsUlChannelUsable();
  void ScheduleScanningRestart(Time interval,
                               SubscriberStationNetDevice::EventType eventType,
                               bool deleteUlParameters, EventId &eventId);

private:
  SSLinkManager(const SSLinkManager &);
  SSLinkManager &operator=(const SSLinkManager &);

  void EndScanning(bool status, uint64_t frequency);
  void StartSynchronizing();
  bool SearchForDlChannel(uint8_t channel);
  void SelectRandomBackoff();
  void IncreaseRangingRequestCW();
  void ResetRangingRequestCW();
  void DeleteUplinkParameters();
  void AdjustRangingParameters(const RngRsp &rngrsp);
  void NegotiateBasicCapabilities();
  uint16_t CalculateMaxIRSignalStrength();
  uint16_t GetMinTransmitPowerLevel();

  Ptr<SubscriberStationNetDevice> m_ss;

  WimaxNetDevice::RangingStatus m_rangingStatus;
  uint16_t m_bsEirp;
  uint16_t m_eirXPIrMax;
  uint16_t m_pTxIrMax;

  uint8_t m_initRangOppNumber;
  uint8_t m_contentionRangingRetries;
  uint32_t m_rngReqFrameNumber;
  RngReq m_rngreq;

  uint8_t m_dlChnlNr;
  uint64_t m_frequency;
  bool m_rangingIntervalFound;

  uint16_t m_nrRngReqsSent;
  uint16_t m_nrRngRspsRecvd;
  uint16_t m_nrInvitedPollsRecvd;

  uint8_t m_rangingCW;
  uint8_t m_rangingBO;
  uint8_t m_nrRangingTransOpps;
  bool m_isBackoffSet;
  uint8_t m_rangingAnomalies;

  EventId m_waitForRngRspEvent;
  EventId m_dlMapSyncTimeoutEvent;
};

} // namespace ns3

#endif
