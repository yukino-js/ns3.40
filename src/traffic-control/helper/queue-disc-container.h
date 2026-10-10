
#ifndef QUEUE_DISC_CONTAINER_H
#define QUEUE_DISC_CONTAINER_H

#include "ns3/queue-disc.h"

#include <stdint.h>
#include <vector>

namespace ns3 {

class QueueDiscContainer {
public:
  typedef std::vector<Ptr<QueueDisc>>::const_iterator ConstIterator;

  QueueDiscContainer();

  QueueDiscContainer(Ptr<QueueDisc> qDisc);

  ConstIterator Begin() const;

  ConstIterator End() const;

  std::size_t GetN() const;

  Ptr<QueueDisc> Get(std::size_t i) const;

  void Add(QueueDiscContainer other);

  void Add(Ptr<QueueDisc> qDisc);

private:
  std::vector<Ptr<QueueDisc>> m_queueDiscs;
};

} // namespace ns3

#endif
