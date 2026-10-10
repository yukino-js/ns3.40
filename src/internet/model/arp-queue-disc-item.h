
#ifndef ARP_QUEUE_DISC_ITEM_H
#define ARP_QUEUE_DISC_ITEM_H

#include "arp-header.h"

#include "ns3/packet.h"
#include "ns3/queue-item.h"

namespace ns3 {

class ArpQueueDiscItem : public QueueDiscItem {
public:
  ArpQueueDiscItem(Ptr<Packet> p, const Address &addr, uint16_t protocol,
                   const ArpHeader &header);

  ~ArpQueueDiscItem() override;

  ArpQueueDiscItem() = delete;
  ArpQueueDiscItem(const ArpQueueDiscItem &) = delete;
  ArpQueueDiscItem &operator=(const ArpQueueDiscItem &) = delete;

  uint32_t GetSize() const override;

  const ArpHeader &GetHeader() const;

  void AddHeader() override;

  void Print(std::ostream &os) const override;

  bool Mark() override;

  uint32_t Hash(uint32_t perturbation) const override;

private:
  ArpHeader m_header;
  bool m_headerAdded;
};

} // namespace ns3

#endif
