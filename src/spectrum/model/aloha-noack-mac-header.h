
#ifndef ALOHA_NOACK_MAC_HEADER_H
#define ALOHA_NOACK_MAC_HEADER_H

#include <ns3/address-utils.h>
#include <ns3/header.h>
#include <ns3/mac48-address.h>

namespace ns3 {

class AlohaNoackMacHeader : public Header {
public:
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  void Print(std::ostream &os) const override;

  void SetSource(Mac48Address source);
  void SetDestination(Mac48Address destination);

  Mac48Address GetSource() const;
  Mac48Address GetDestination() const;

private:
  Mac48Address m_source;
  Mac48Address m_destination;
};

} // namespace ns3

#endif
