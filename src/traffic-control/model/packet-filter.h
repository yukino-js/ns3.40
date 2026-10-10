
#ifndef PACKET_FILTER_H
#define PACKET_FILTER_H

#include "ns3/object.h"

namespace ns3 {

class QueueDiscItem;

class PacketFilter : public Object {
public:
  static TypeId GetTypeId();

  PacketFilter();
  ~PacketFilter() override;

  static const int PF_NO_MATCH = -1;

  int32_t Classify(Ptr<QueueDiscItem> item) const;

private:
  virtual bool CheckProtocol(Ptr<QueueDiscItem> item) const = 0;

  virtual int32_t DoClassify(Ptr<QueueDiscItem> item) const = 0;
};

} // namespace ns3

#endif
