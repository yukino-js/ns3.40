
#ifndef TCP_LEDBAT_H
#define TCP_LEDBAT_H

#include "tcp-congestion-ops.h"

#include <vector>

namespace ns3 {

class TcpSocketState;

class TcpLedbat : public TcpNewReno {
private:
  enum SlowStartType {
    DO_NOT_SLOWSTART,
    DO_SLOWSTART,
  };

  enum State : uint32_t {
    LEDBAT_VALID_OWD = (1 << 1),
    LEDBAT_CAN_SS = (1 << 3)
  };

public:
  static TypeId GetTypeId();

  TcpLedbat();

  TcpLedbat(const TcpLedbat &sock);

  ~TcpLedbat() override;

  std::string GetName() const override;

  void PktsAcked(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked,
                 const Time &rtt) override;

  Ptr<TcpCongestionOps> Fork() override;

  void IncreaseWindow(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked) override;

  void SetDoSs(SlowStartType doSS);

protected:
  void CongestionAvoidance(Ptr<TcpSocketState> tcb,
                           uint32_t segmentsAcked) override;

private:
  struct OwdCircBuf {
    std::vector<uint32_t> buffer;
    uint32_t min;
  };

  void InitCircBuf(OwdCircBuf &buffer);

  typedef uint32_t (*FilterFunction)(OwdCircBuf &);

  static uint32_t MinCircBuf(OwdCircBuf &b);

  uint32_t CurrentDelay(FilterFunction filter);

  uint32_t BaseDelay();

  void AddDelay(OwdCircBuf &cb, uint32_t owd, uint32_t maxlen);

  void UpdateBaseDelay(uint32_t owd);

  Time m_target;
  double m_gain;
  SlowStartType m_doSs;
  uint32_t m_baseHistoLen;
  uint32_t m_noiseFilterLen;
  uint64_t m_lastRollover;
  int32_t m_sndCwndCnt;
  OwdCircBuf m_baseHistory;
  OwdCircBuf m_noiseFilter;
  uint32_t m_flag;
  uint32_t m_minCwnd;
};

} // namespace ns3

#endif
