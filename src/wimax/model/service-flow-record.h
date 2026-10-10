
#ifndef SERVICE_FLOW_RECORD_H
#define SERVICE_FLOW_RECORD_H

#include "bs-net-device.h"
#include "bs-uplink-scheduler.h"

#include "ns3/nstime.h"
#include "ns3/ptr.h"

#include <stdint.h>

namespace ns3 {

class ServiceFlowRecord {
public:
  ServiceFlowRecord();
  ~ServiceFlowRecord();

  void SetGrantSize(uint32_t grantSize);
  uint32_t GetGrantSize() const;
  void SetGrantTimeStamp(Time grantTimeStamp);
  Time GetGrantTimeStamp() const;
  void SetDlTimeStamp(Time dlTimeStamp);
  Time GetDlTimeStamp() const;
  void SetPktsSent(uint32_t pktsSent);
  void UpdatePktsSent(uint32_t pktsSent);
  uint32_t GetPktsSent() const;
  void SetPktsRcvd(uint32_t pktsRcvd);
  void UpdatePktsRcvd(uint32_t pktsRcvd);
  uint32_t GetPktsRcvd() const;
  void SetBytesSent(uint32_t bytesSent);
  void UpdateBytesSent(uint32_t bytesSent);
  uint32_t GetBytesSent() const;
  void SetBytesRcvd(uint32_t bytesRcvd);
  void UpdateBytesRcvd(uint32_t bytesRcvd);
  uint32_t GetBytesRcvd() const;

  void SetRequestedBandwidth(uint32_t requestedBandwidth);
  void UpdateRequestedBandwidth(uint32_t requestedBandwidth);
  uint32_t GetRequestedBandwidth() const;

  void SetGrantedBandwidth(uint32_t grantedBandwidth);
  void UpdateGrantedBandwidth(uint32_t grantedBandwidth);
  uint32_t GetGrantedBandwidth() const;

  void SetGrantedBandwidthTemp(uint32_t grantedBandwidthTemp);
  void UpdateGrantedBandwidthTemp(uint32_t grantedBandwidthTemp);
  uint32_t GetGrantedBandwidthTemp() const;

  void SetBwSinceLastExpiry(uint32_t bwSinceLastExpiry);
  void UpdateBwSinceLastExpiry(uint32_t bwSinceLastExpiry);
  uint32_t GetBwSinceLastExpiry() const;

  void SetLastGrantTime(Time grantTime);
  Time GetLastGrantTime() const;

  void SetBacklogged(uint32_t backlogged);
  void IncreaseBacklogged(uint32_t backlogged);
  uint32_t GetBacklogged() const;

  void SetBackloggedTemp(uint32_t backloggedTemp);
  void IncreaseBackloggedTemp(uint32_t backloggedTemp);
  uint32_t GetBackloggedTemp() const;

private:
  uint32_t m_grantSize;
  Time m_grantTimeStamp;
  Time m_dlTimeStamp;

  uint32_t m_pktsSent;
  uint32_t m_pktsRcvd;

  uint32_t m_bytesSent;
  uint32_t m_bytesRcvd;

  uint32_t m_requestedBandwidth;
  uint32_t m_grantedBandwidth;
  uint32_t m_grantedBandwidthTemp;

  uint32_t m_bwSinceLastExpiry;
  Time m_lastGrantTime;
  int32_t m_backlogged;
  int32_t m_backloggedTemp;
};

} // namespace ns3

#endif
