

#ifndef PIE_QUEUE_DISC_H
#define PIE_QUEUE_DISC_H

#include "queue-disc.h"

#include "ns3/boolean.h"
#include "ns3/data-rate.h"
#include "ns3/event-id.h"
#include "ns3/nstime.h"
#include "ns3/random-variable-stream.h"
#include "ns3/timer.h"

#define BURST_RESET_TIMEOUT 1.5

class PieQueueDiscTestCase;

namespace ns3 {

class TraceContainer;
class UniformRandomVariable;

class PieQueueDisc : public QueueDisc {
public:
  static TypeId GetTypeId();

  PieQueueDisc();

  ~PieQueueDisc() override;

  enum BurstStateT {
    NO_BURST,
    IN_BURST,
    IN_BURST_PROTECTING,
  };

  Time GetQueueDelay();
  int64_t AssignStreams(int64_t stream);

  static constexpr const char *UNFORCED_DROP = "Unforced drop";
  static constexpr const char *FORCED_DROP = "Forced drop";
  static constexpr const char *UNFORCED_MARK = "Unforced mark";
  static constexpr const char *CE_THRESHOLD_EXCEEDED_MARK =
      "CE threshold exceeded mark";

protected:
  void DoDispose() override;

private:
  friend class ::PieQueueDiscTestCase;
  bool DoEnqueue(Ptr<QueueDiscItem> item) override;
  Ptr<QueueDiscItem> DoDequeue() override;
  bool CheckConfig() override;

  void InitializeParams() override;

  bool DropEarly(Ptr<QueueDiscItem> item, uint32_t qSize);

  void CalculateP();

  static const uint64_t DQCOUNT_INVALID = std::numeric_limits<uint64_t>::max();

  Time m_sUpdate;
  Time m_tUpdate;
  Time m_qDelayRef;
  uint32_t m_meanPktSize;
  Time m_maxBurst;
  double m_a;
  double m_b;
  uint32_t m_dqThreshold;
  bool m_useDqRateEstimator;
  bool m_isCapDropAdjustment;
  bool m_useEcn;
  bool m_useDerandomization;
  double m_markEcnTh;
  Time m_activeThreshold;
  Time m_ceThreshold;
  bool m_useL4s;

  double m_dropProb;
  Time m_qDelayOld;
  Time m_qDelay;
  Time m_burstAllowance;
  uint32_t m_burstReset;
  BurstStateT m_burstState;
  bool m_inMeasurement;
  double m_avgDqRate;
  Time m_dqStart;
  uint64_t m_dqCount;
  EventId m_rtrsEvent;
  Ptr<UniformRandomVariable> m_uv;
  double m_accuProb;
  bool m_active;
};

}; // namespace ns3

#endif
