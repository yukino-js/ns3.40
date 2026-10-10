
#ifndef SS_RECORD_H
#define SS_RECORD_H

#include "service-flow.h"
#include "wimax-connection.h"
#include "wimax-net-device.h"
#include "wimax-phy.h"

#include "ns3/ipv4-address.h"
#include "ns3/mac48-address.h"

#include <ostream>
#include <stdint.h>

namespace ns3 {

class ServiceFlow;

class SSRecord {
public:
  SSRecord();
  SSRecord(Mac48Address macAddress);
  SSRecord(Mac48Address macAddress, Ipv4Address IPaddress);
  ~SSRecord();

  void SetBasicCid(Cid basicCid);
  Cid GetBasicCid() const;

  void SetPrimaryCid(Cid primaryCid);
  Cid GetPrimaryCid() const;

  void SetMacAddress(Mac48Address macAddress);
  Mac48Address GetMacAddress() const;

  uint8_t GetRangingCorrectionRetries() const;
  void ResetRangingCorrectionRetries();
  void IncrementRangingCorrectionRetries();
  uint8_t GetInvitedRangRetries() const;
  void ResetInvitedRangingRetries();
  void IncrementInvitedRangingRetries();
  void SetModulationType(WimaxPhy::ModulationType modulationType);
  WimaxPhy::ModulationType GetModulationType() const;

  void SetRangingStatus(WimaxNetDevice::RangingStatus rangingStatus);
  WimaxNetDevice::RangingStatus GetRangingStatus() const;

  void EnablePollForRanging();
  void DisablePollForRanging();
  bool GetPollForRanging() const;

  bool GetAreServiceFlowsAllocated() const;

  void SetPollMeBit(bool pollMeBit);
  bool GetPollMeBit() const;

  void AddServiceFlow(ServiceFlow *serviceFlow);
  std::vector<ServiceFlow *>
  GetServiceFlows(ServiceFlow::SchedulingType schedulingType) const;
  bool GetHasServiceFlowUgs() const;
  bool GetHasServiceFlowRtps() const;
  bool GetHasServiceFlowNrtps() const;
  bool GetHasServiceFlowBe() const;

  void SetSfTransactionId(uint16_t sfTransactionId);
  uint16_t GetSfTransactionId() const;

  void SetDsaRspRetries(uint8_t dsaRspRetries);
  void IncrementDsaRspRetries();
  uint8_t GetDsaRspRetries() const;

  void SetDsaRsp(DsaRsp dsaRsp);
  DsaRsp GetDsaRsp() const;
  void SetIsBroadcastSS(bool broadcast_enable);
  bool GetIsBroadcastSS() const;

  Ipv4Address GetIPAddress();
  void SetIPAddress(Ipv4Address IPaddress);
  void SetAreServiceFlowsAllocated(bool val);

private:
  void Initialize();

  Mac48Address m_macAddress;
  Ipv4Address m_IPAddress;

  Cid m_basicCid;
  Cid m_primaryCid;

  uint8_t m_rangingCorrectionRetries;
  uint8_t m_invitedRangingRetries;

  WimaxPhy::ModulationType m_modulationType;
  WimaxNetDevice::RangingStatus m_rangingStatus;
  bool m_pollForRanging;
  bool m_areServiceFlowsAllocated;
  bool m_pollMeBit;
  bool m_broadcast;

  std::vector<ServiceFlow *> *m_serviceFlows;

  uint16_t m_sfTransactionId;
  uint8_t m_dsaRspRetries;
  DsaRsp m_dsaRsp;
};

} // namespace ns3

#endif
