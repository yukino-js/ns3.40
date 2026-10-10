#ifndef TCP_RATE_OPS_H
#define TCP_RATE_OPS_H

#include "tcp-tx-item.h"

#include "ns3/data-rate.h"
#include "ns3/object.h"
#include "ns3/traced-callback.h"
#include "ns3/traced-value.h"

namespace ns3 {

class TcpRateOps : public Object {
public:
  struct TcpRateSample;
  struct TcpRateConnection;

  static TypeId GetTypeId();
  virtual void SkbSent(TcpTxItem *skb, bool isStartOfTransmission) = 0;

  virtual void SkbDelivered(TcpTxItem *skb) = 0;

  virtual void CalculateAppLimited(uint32_t cWnd, uint32_t in_flight,
                                   uint32_t segmentSize,
                                   const SequenceNumber32 &tailSeq,
                                   const SequenceNumber32 &nextTx,
                                   const uint32_t lostOut,
                                   const uint32_t retransOut) = 0;

  virtual const TcpRateSample &GenerateSample(uint32_t delivered, uint32_t lost,
                                              bool is_sack_reneg,
                                              uint32_t priorInFlight,
                                              const Time &minRtt) = 0;

  virtual const TcpRateConnection &GetConnectionRate() = 0;

  struct TcpRateSample {
    DataRate m_deliveryRate{DataRate("0bps")};
    bool m_isAppLimited{false};
    Time m_interval{Seconds(0.0)};
    int32_t m_delivered{0};
    uint32_t m_priorDelivered{0};
    Time m_priorTime{Seconds(0.0)};
    Time m_sendElapsed{Seconds(0.0)};
    Time m_ackElapsed{Seconds(0.0)};
    uint32_t m_bytesLoss{0};
    uint32_t m_priorInFlight{0};
    uint32_t m_ackedSacked{0};

    bool IsValid() const {
      return (m_priorTime != Seconds(0.0) || m_interval != Seconds(0.0));
    }
  };

  struct TcpRateConnection {
    uint64_t m_delivered{0};
    Time m_deliveredTime{Seconds(0)};
    Time m_firstSentTime{Seconds(0)};
    uint32_t m_appLimited{0};
    uint32_t m_txItemDelivered{0};
    int32_t m_rateDelivered{0};
    Time m_rateInterval{Seconds(0)};
    bool m_rateAppLimited{false};
  };
};

class TcpRateLinux : public TcpRateOps {
public:
  static TypeId GetTypeId();

  ~TcpRateLinux() override {}

  void SkbSent(TcpTxItem *skb, bool isStartOfTransmission) override;
  void SkbDelivered(TcpTxItem *skb) override;
  void CalculateAppLimited(uint32_t cWnd, uint32_t in_flight,
                           uint32_t segmentSize,
                           const SequenceNumber32 &tailSeq,
                           const SequenceNumber32 &nextTx,
                           const uint32_t lostOut,
                           const uint32_t retransOut) override;
  const TcpRateSample &GenerateSample(uint32_t delivered, uint32_t lost,
                                      bool is_sack_reneg,
                                      uint32_t priorInFlight,
                                      const Time &minRtt) override;

  const TcpRateConnection &GetConnectionRate() override { return m_rate; }

  typedef void (*TcpRateUpdated)(const TcpRateConnection &rate);

  typedef void (*TcpRateSampleUpdated)(const TcpRateSample &sample);

private:
  TcpRateConnection m_rate;
  TcpRateSample m_rateSample;

  TracedCallback<const TcpRateConnection &> m_rateTrace;
  TracedCallback<const TcpRateSample &> m_rateSampleTrace;
};

std::ostream &operator<<(std::ostream &os,
                         const TcpRateOps::TcpRateSample &sample);

std::ostream &operator<<(std::ostream &os,
                         const TcpRateOps::TcpRateConnection &rate);

bool operator==(const TcpRateLinux::TcpRateSample &lhs,
                const TcpRateLinux::TcpRateSample &rhs);

bool operator==(const TcpRateLinux::TcpRateConnection &lhs,
                const TcpRateLinux::TcpRateConnection &rhs);

} // namespace ns3

#endif
