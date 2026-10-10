
#ifndef IPV6_QUEUE_DISC_ITEM_H
#define IPV6_QUEUE_DISC_ITEM_H

#include "ipv6-header.h"

#include "ns3/packet.h"
#include "ns3/queue-item.h"

namespace ns3 {

class Ipv6QueueDiscItem : public QueueDiscItem {
public:
  Ipv6QueueDiscItem(Ptr<Packet> p, const Address &addr, uint16_t protocol,
                    const Ipv6Header &header);

  ~Ipv6QueueDiscItem() override;

  Ipv6QueueDiscItem() = delete;
  Ipv6QueueDiscItem(const Ipv6QueueDiscItem &) = delete;
  Ipv6QueueDiscItem &operator=(const Ipv6QueueDiscItem &) = delete;

  uint32_t GetSize() const override;

  const Ipv6Header &GetHeader() const;

  void AddHeader() override;

  void Print(std::ostream &os) const override;

  bool GetUint8Value(Uint8Values field, uint8_t &value) const override;

  bool Mark() override;

  uint32_t Hash(uint32_t perturbation) const override;

private:
  Ipv6Header m_header;
  bool m_headerAdded;
};

} // namespace ns3

#endif
