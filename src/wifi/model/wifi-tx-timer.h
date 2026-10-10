
#ifndef WIFI_TX_TIMER_H
#define WIFI_TX_TIMER_H

#include "ns3/event-id.h"
#include "ns3/nstime.h"
#include "ns3/simulator.h"
#include "ns3/traced-callback.h"

#include <functional>
#include <unordered_map>

namespace ns3 {

class WifiMpdu;
class WifiPsdu;
class WifiTxVector;
class Mac48Address;

typedef std::unordered_map<uint16_t, Ptr<WifiPsdu>> WifiPsduMap;

class WifiTxTimer {
public:
  enum Reason : uint8_t {
    NOT_RUNNING = 0,
    WAIT_CTS,
    WAIT_NORMAL_ACK,
    WAIT_BLOCK_ACK,
    WAIT_CTS_AFTER_MU_RTS,
    WAIT_NORMAL_ACK_AFTER_DL_MU_PPDU,
    WAIT_BLOCK_ACKS_IN_TB_PPDU,
    WAIT_TB_PPDU_AFTER_BASIC_TF,
    WAIT_QOS_NULL_AFTER_BSRP_TF,
    WAIT_BLOCK_ACK_AFTER_TB_PPDU,
  };

  WifiTxTimer();

  virtual ~WifiTxTimer();

  template <typename MEM, typename OBJ, typename... Args>
  void Set(Reason reason, const Time &delay, const std::set<Mac48Address> &from,
           MEM mem_ptr, OBJ obj, Args... args);

  void Reschedule(const Time &delay);

  Reason GetReason() const;

  std::string GetReasonString(Reason reason) const;

  bool IsRunning() const;

  void Cancel();

  void GotResponseFrom(const Mac48Address &from);

  const std::set<Mac48Address> &GetStasExpectedToRespond() const;

  Time GetDelayLeft() const;

  typedef Callback<void, uint8_t, Ptr<const WifiMpdu>, const WifiTxVector &>
      MpduResponseTimeout;

  typedef Callback<void, uint8_t, Ptr<const WifiPsdu>, const WifiTxVector &>
      PsduResponseTimeout;

  typedef Callback<void, uint8_t, WifiPsduMap *, const std::set<Mac48Address> *,
                   std::size_t>
      PsduMapResponseTimeout;

  void SetMpduResponseTimeoutCallback(MpduResponseTimeout callback) const;

  void SetPsduResponseTimeoutCallback(PsduResponseTimeout callback) const;

  void SetPsduMapResponseTimeoutCallback(PsduMapResponseTimeout callback) const;

private:
  template <typename MEM, typename OBJ, typename... Args>
  void Timeout(MEM mem_ptr, OBJ obj, Args... args);

  void Expire();

  void FeedTraceSource(Ptr<WifiMpdu> item, WifiTxVector txVector);

  void FeedTraceSource(Ptr<WifiPsdu> psdu, WifiTxVector txVector);

  void FeedTraceSource(WifiPsduMap *psduMap, std::size_t nTotalStations);

  EventId m_timeoutEvent;
  Reason m_reason;
  Ptr<EventImpl> m_impl;
  Time m_end;
  std::set<Mac48Address> m_staExpectResponseFrom;

  mutable MpduResponseTimeout m_mpduResponseTimeoutCallback;
  mutable PsduResponseTimeout m_psduResponseTimeoutCallback;
  mutable PsduMapResponseTimeout m_psduMapResponseTimeoutCallback;
};

} // namespace ns3

namespace ns3 {

template <typename MEM, typename OBJ, typename... Args>
void WifiTxTimer::Set(Reason reason, const Time &delay,
                      const std::set<Mac48Address> &from, MEM mem_ptr, OBJ obj,
                      Args... args) {
  typedef void (WifiTxTimer::*TimeoutType)(MEM, OBJ, Args...);

  m_timeoutEvent = Simulator::Schedule(delay, &WifiTxTimer::Expire, this);
  m_reason = reason;
  m_end = Simulator::Now() + delay;
  m_staExpectResponseFrom = from;

  m_impl = Ptr<EventImpl>(MakeEvent<TimeoutType>(&WifiTxTimer::Timeout, this,
                                                 mem_ptr, obj,
                                                 std::forward<Args>(args)...),
                          false);
}

template <typename MEM, typename OBJ, typename... Args>
void WifiTxTimer::Timeout(MEM mem_ptr, OBJ obj, Args... args) {
  FeedTraceSource(std::forward<Args>(args)...);

  ((*obj).*mem_ptr)(std::forward<Args>(args)...);
}

} // namespace ns3

#endif
