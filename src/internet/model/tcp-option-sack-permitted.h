
#ifndef TCP_OPTION_SACK_PERMITTED_H
#define TCP_OPTION_SACK_PERMITTED_H

#include "tcp-option.h"

namespace ns3 {

class TcpOptionSackPermitted : public TcpOption {
public:
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;

  TcpOptionSackPermitted();
  ~TcpOptionSackPermitted() override;

  void Print(std::ostream &os) const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

  uint8_t GetKind() const override;
  uint32_t GetSerializedSize() const override;
};

} // namespace ns3

#endif
