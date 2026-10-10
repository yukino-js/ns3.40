
#ifndef PFIFO_FAST_H
#define PFIFO_FAST_H

#include "queue-disc.h"

namespace ns3 {

class PfifoFastQueueDisc : public QueueDisc {
public:
  static TypeId GetTypeId();
  PfifoFastQueueDisc();

  ~PfifoFastQueueDisc() override;

  static constexpr const char *LIMIT_EXCEEDED_DROP =
      "Queue disc limit exceeded";

private:
  static const uint32_t prio2band[16];

  bool DoEnqueue(Ptr<QueueDiscItem> item) override;
  Ptr<QueueDiscItem> DoDequeue() override;
  Ptr<const QueueDiscItem> DoPeek() override;
  bool CheckConfig() override;
  void InitializeParams() override;
};

} // namespace ns3

#endif
