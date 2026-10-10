
#ifndef FIFO_QUEUE_DISC_H
#define FIFO_QUEUE_DISC_H

#include "queue-disc.h"

namespace ns3 {

class FifoQueueDisc : public QueueDisc {
public:
  static TypeId GetTypeId();
  FifoQueueDisc();

  ~FifoQueueDisc() override;

  static constexpr const char *LIMIT_EXCEEDED_DROP =
      "Queue disc limit exceeded";

private:
  bool DoEnqueue(Ptr<QueueDiscItem> item) override;
  Ptr<QueueDiscItem> DoDequeue() override;
  Ptr<const QueueDiscItem> DoPeek() override;
  bool CheckConfig() override;
  void InitializeParams() override;
};

} // namespace ns3

#endif
