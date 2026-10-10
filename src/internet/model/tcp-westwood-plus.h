
#ifndef TCP_WESTWOOD_H
#define TCP_WESTWOOD_H

#include "tcp-congestion-ops.h"
#include "tcp-recovery-ops.h"

#include "ns3/data-rate.h"
#include "ns3/event-id.h"
#include "ns3/traced-value.h"

namespace ns3 {

class Time;

class TcpWestwoodPlus : public TcpNewReno {
public:
  static TypeId GetTypeId();

  TcpWestwoodPlus();
  TcpWestwoodPlus(const TcpWestwoodPlus &sock);
  ~TcpWestwoodPlus() override;

  enum FilterType { NONE, TUSTIN };

  uint32_t GetSsThresh(Ptr<const TcpSocketState> tcb,
                       uint32_t bytesInFlight) override;

  void PktsAcked(Ptr<TcpSocketState> tcb, uint32_t packetsAcked,
                 const Time &rtt) override;

  Ptr<TcpCongestionOps> Fork() override;

private:
  void UpdateAckedSegments(int acked);

  void EstimateBW(const Time &rtt, Ptr<TcpSocketState> tcb);

protected:
  TracedValue<DataRate> m_currentBW;
  DataRate m_lastSampleBW;
  DataRate m_lastBW;
  FilterType m_fType;

  uint32_t m_ackedSegments;
  bool m_IsCount;
  EventId m_bwEstimateEvent;
  Time m_lastAck;
};

} // namespace ns3

#endif
