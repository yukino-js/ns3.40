
#ifndef MQ_QUEUE_DISC_H
#define MQ_QUEUE_DISC_H

#include "queue-disc.h"

namespace ns3 {

class MqQueueDisc : public QueueDisc {
public:
  static TypeId GetTypeId();
  MqQueueDisc();

  ~MqQueueDisc() override;

  WakeMode GetWakeMode() const override;

private:
  bool DoEnqueue(Ptr<QueueDiscItem> item) override;
  Ptr<QueueDiscItem> DoDequeue() override;
  Ptr<const QueueDiscItem> DoPeek() override;
  bool CheckConfig() override;
  void InitializeParams() override;
};

} // namespace ns3

#endif
