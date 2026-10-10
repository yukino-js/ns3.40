
#ifndef IPV4_QUEUE_DISC_ITEM_H
#define IPV4_QUEUE_DISC_ITEM_H

#include "ipv4-header.h"

#include "ns3/packet.h"
#include "ns3/queue-item.h"

namespace ns3 {

class Ipv4QueueDiscItem : public QueueDiscItem {
public:
  Ipv4QueueDiscItem(Ptr<Packet> p, const Address &addr, uint16_t protocol,
                    const Ipv4Header &header);

  ~Ipv4QueueDiscItem() override;

  Ipv4QueueDiscItem() = delete;
  Ipv4QueueDiscItem(const Ipv4QueueDiscItem &) = delete;
  Ipv4QueueDiscItem &operator=(const Ipv4QueueDiscItem &) = delete;

  uint32_t GetSize() const override;

  const Ipv4Header &GetHeader() const;

  void AddHeader() override;

  void Print(std::ostream &os) const override;

  bool GetUint8Value(Uint8Values field, uint8_t &value) const override;

  bool Mark() override;

  uint32_t Hash(uint32_t perturbation) const override;

private:
  Ipv4Header m_header;
  bool m_headerAdded;
};

} // namespace ns3

#endif
