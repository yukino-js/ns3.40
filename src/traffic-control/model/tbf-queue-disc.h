#ifndef TBF_QUEUE_DISC_H
#define TBF_QUEUE_DISC_H

#include "queue-disc.h"

#include "ns3/boolean.h"
#include "ns3/data-rate.h"
#include "ns3/event-id.h"
#include "ns3/nstime.h"
#include "ns3/random-variable-stream.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/traced-value.h"

namespace ns3 {

class TbfQueueDisc : public QueueDisc {
public:
  static TypeId GetTypeId();

  TbfQueueDisc();

  ~TbfQueueDisc() override;

  void SetBurst(uint32_t burst);

  uint32_t GetBurst() const;

  void SetMtu(uint32_t mtu);

  uint32_t GetMtu() const;

  void SetRate(DataRate rate);

  DataRate GetRate() const;

  void SetPeakRate(DataRate peakRate);

  DataRate GetPeakRate() const;

  uint32_t GetFirstBucketTokens() const;

  uint32_t GetSecondBucketTokens() const;

protected:
  void DoDispose() override;

private:
  bool DoEnqueue(Ptr<QueueDiscItem> item) override;
  Ptr<QueueDiscItem> DoDequeue() override;
  bool CheckConfig() override;
  void InitializeParams() override;

  uint32_t m_burst;
  uint32_t m_mtu;
  DataRate m_rate;
  DataRate m_peakRate;

  TracedValue<uint32_t> m_btokens;
  TracedValue<uint32_t> m_ptokens;
  Time m_timeCheckPoint;
  EventId m_id;
};

} // namespace ns3

#endif
