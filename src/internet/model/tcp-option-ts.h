
#ifndef TCP_OPTION_TS_H
#define TCP_OPTION_TS_H

#include "tcp-option.h"

#include "ns3/timer.h"

namespace ns3 {

class TcpOptionTS : public TcpOption {
public:
  TcpOptionTS();
  ~TcpOptionTS() override;

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;

  void Print(std::ostream &os) const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

  uint8_t GetKind() const override;
  uint32_t GetSerializedSize() const override;

  uint32_t GetTimestamp() const;
  uint32_t GetEcho() const;
  void SetTimestamp(uint32_t ts);
  void SetEcho(uint32_t ts);

  static uint32_t NowToTsValue();

  static Time ElapsedTimeFromTsValue(uint32_t echoTime);

protected:
  uint32_t m_timestamp;
  uint32_t m_echo;
};

} // namespace ns3

#endif
