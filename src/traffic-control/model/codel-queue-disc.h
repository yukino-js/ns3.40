
#ifndef CODEL_H
#define CODEL_H

#include "queue-disc.h"

#include "ns3/nstime.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/traced-value.h"

class CoDelQueueDiscNewtonStepTest;
class CoDelQueueDiscControlLawTest;

namespace ns3 {

static const int CODEL_SHIFT = 10;

#define DEFAULT_CODEL_LIMIT 1000
#define REC_INV_SQRT_BITS (8 * sizeof(uint16_t))
#define REC_INV_SQRT_SHIFT (32 - REC_INV_SQRT_BITS)

class TraceContainer;

class CoDelQueueDisc : public QueueDisc {
public:
  static TypeId GetTypeId();

  CoDelQueueDisc();

  ~CoDelQueueDisc() override;

  Time GetTarget();

  Time GetInterval();

  uint32_t GetDropNext();

  static constexpr const char *TARGET_EXCEEDED_DROP = "Target exceeded drop";
  static constexpr const char *OVERLIMIT_DROP = "Overlimit drop";
  static constexpr const char *TARGET_EXCEEDED_MARK = "Target exceeded mark";
  static constexpr const char *CE_THRESHOLD_EXCEEDED_MARK =
      "CE threshold exceeded mark";

private:
  friend class ::CoDelQueueDiscNewtonStepTest;
  friend class ::CoDelQueueDiscControlLawTest;
  bool DoEnqueue(Ptr<QueueDiscItem> item) override;

  Ptr<QueueDiscItem> DoDequeue() override;

  bool CheckConfig() override;

  static uint16_t NewtonStep(uint16_t recInvSqrt, uint32_t count);

  static uint32_t ControlLaw(uint32_t t, uint32_t interval,
                             uint32_t recInvSqrt);

  bool OkToDrop(Ptr<QueueDiscItem> item, uint32_t now);

  bool CoDelTimeAfter(uint32_t a, uint32_t b);
  bool CoDelTimeAfterEq(uint32_t a, uint32_t b);
  bool CoDelTimeBefore(uint32_t a, uint32_t b);
  bool CoDelTimeBeforeEq(uint32_t a, uint32_t b);

  uint32_t Time2CoDel(Time t);

  void InitializeParams() override;

  bool m_useEcn;
  bool m_useL4s;
  uint32_t m_minBytes;
  Time m_interval;
  Time m_target;
  Time m_ceThreshold;
  TracedValue<uint32_t> m_count;
  TracedValue<uint32_t> m_lastCount;
  TracedValue<bool> m_dropping;
  uint16_t m_recInvSqrt;
  uint32_t m_firstAboveTime;
  TracedValue<uint32_t> m_dropNext;
};

} // namespace ns3

#endif
