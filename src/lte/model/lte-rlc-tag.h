
#ifndef RLC_TAG_H
#define RLC_TAG_H

#include "ns3/nstime.h"
#include "ns3/packet.h"

namespace ns3 {

class Tag;

class RlcTag : public Tag {
public:
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;

  RlcTag();
  RlcTag(Time senderTimestamp);

  void Serialize(TagBuffer i) const override;
  void Deserialize(TagBuffer i) override;
  uint32_t GetSerializedSize() const override;
  void Print(std::ostream &os) const override;

  Time GetSenderTimestamp() const { return m_senderTimestamp; }

  void SetSenderTimestamp(Time senderTimestamp) {
    this->m_senderTimestamp = senderTimestamp;
  }

private:
  Time m_senderTimestamp;
};

} // namespace ns3

#endif
