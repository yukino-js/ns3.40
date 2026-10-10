
#ifndef PDCP_TAG_H
#define PDCP_TAG_H

#include "ns3/nstime.h"
#include "ns3/packet.h"

namespace ns3 {

class Tag;

class PdcpTag : public Tag {
public:
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;

  PdcpTag();
  PdcpTag(Time senderTimestamp);

  void Serialize(TagBuffer i) const override;
  void Deserialize(TagBuffer i) override;
  uint32_t GetSerializedSize() const override;
  void Print(std::ostream &os) const override;

  Time GetSenderTimestamp() const;

  void SetSenderTimestamp(Time senderTimestamp);

private:
  Time m_senderTimestamp;
};

} // namespace ns3

#endif
