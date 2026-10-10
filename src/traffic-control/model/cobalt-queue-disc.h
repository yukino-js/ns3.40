
#ifndef COBALT_H
#define COBALT_H

#include "queue-disc.h"

#include "ns3/boolean.h"
#include "ns3/data-rate.h"
#include "ns3/nstime.h"
#include "ns3/random-variable-stream.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/traced-value.h"

namespace ns3 {

#define REC_INV_SQRT_CACHE (16)
#define DEFAULT_COBALT_LIMIT 1000

class TraceContainer;

class CobaltQueueDisc : public QueueDisc {
public:
  static TypeId GetTypeId();

  CobaltQueueDisc();

  ~CobaltQueueDisc() override;

  Time GetTarget() const;

  Time GetInterval() const;

  int64_t GetDropNext() const;

  static constexpr const char *TARGET_EXCEEDED_DROP = "Target exceeded drop";
  static constexpr const char *OVERLIMIT_DROP = "Overlimit drop";
  static constexpr const char *FORCED_MARK = "forcedMark";
  static constexpr const char *CE_THRESHOLD_EXCEEDED_MARK =
      "CE threshold exceeded mark";

  double GetPdrop() const;

  int64_t AssignStreams(int64_t stream);

  int64_t Time2CoDel(Time t) const;

protected:
  void DoDispose() override;

private:
  bool DoEnqueue(Ptr<QueueDiscItem> item) override;
  Ptr<QueueDiscItem> DoDequeue() override;
  Ptr<const QueueDiscItem> DoPeek() override;
  bool CheckConfig() override;

  void InitializeParams() override;

  void NewtonStep();

  int64_t ControlLaw(int64_t t);

  void InvSqrt();

  void CacheInit();

  bool CoDelTimeAfter(int64_t a, int64_t b);

  bool CoDelTimeAfterEq(int64_t a, int64_t b);

  void CobaltQueueFull(int64_t now);

  void CobaltQueueEmpty(int64_t now);

  bool CobaltShouldDrop(Ptr<QueueDiscItem> item, int64_t now);

  Stats m_stats;

  TracedValue<uint32_t> m_count;
  TracedValue<int64_t> m_dropNext;
  TracedValue<bool> m_dropping;
  uint32_t m_recInvSqrt;
  uint32_t m_recInvSqrtCache[REC_INV_SQRT_CACHE] = {0};

  Time m_interval;
  Time m_target;
  bool m_useEcn;
  Time m_ceThreshold;
  bool m_useL4s;
  Time m_blueThreshold;

  Ptr<UniformRandomVariable> m_uv;
  uint32_t m_lastUpdateTimeBlue;

  double m_increment;
  double m_decrement;
  double m_pDrop;
};

} // namespace ns3

#endif
