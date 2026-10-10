
#ifndef PRIO_QUEUE_DISC_H
#define PRIO_QUEUE_DISC_H

#include "queue-disc.h"

#include <array>

namespace ns3 {

typedef std::array<uint16_t, 16> Priomap;

class PrioQueueDisc : public QueueDisc {
public:
  static TypeId GetTypeId();
  PrioQueueDisc();

  ~PrioQueueDisc() override;

  void SetBandForPriority(uint8_t prio, uint16_t band);

  uint16_t GetBandForPriority(uint8_t prio) const;

private:
  bool DoEnqueue(Ptr<QueueDiscItem> item) override;
  Ptr<QueueDiscItem> DoDequeue() override;
  Ptr<const QueueDiscItem> DoPeek() override;
  bool CheckConfig() override;
  void InitializeParams() override;

  Priomap m_prio2band;
};

std::ostream &operator<<(std::ostream &os, const Priomap &priomap);

std::istream &operator>>(std::istream &is, Priomap &priomap);

ATTRIBUTE_HELPER_HEADER(Priomap);

} // namespace ns3

#endif
