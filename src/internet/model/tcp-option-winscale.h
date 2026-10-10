
#ifndef TCP_OPTION_WINSCALE_H
#define TCP_OPTION_WINSCALE_H

#include "tcp-option.h"

namespace ns3 {

class TcpOptionWinScale : public TcpOption {
public:
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;

  TcpOptionWinScale();
  ~TcpOptionWinScale() override;

  void Print(std::ostream &os) const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

  uint8_t GetKind() const override;
  uint32_t GetSerializedSize() const override;

  uint8_t GetScale() const;

  void SetScale(uint8_t scale);

protected:
  uint8_t m_scale;
};

} // namespace ns3

#endif
