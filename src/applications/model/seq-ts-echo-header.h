
#ifndef SEQ_TS_ECHO_HEADER_H
#define SEQ_TS_ECHO_HEADER_H

#include "ns3/header.h"
#include "ns3/nstime.h"

namespace ns3 {
class SeqTsEchoHeader : public Header {
public:
  static TypeId GetTypeId();

  SeqTsEchoHeader();

  void SetSeq(uint32_t seq);

  uint32_t GetSeq() const;

  Time GetTsValue() const;

  Time GetTsEchoReply() const;

  void SetTsValue(Time ts);

  void SetTsEchoReply(Time ts);

  TypeId GetInstanceTypeId() const override;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

private:
  uint32_t m_seq;
  Time m_tsValue;
  Time m_tsEchoReply;
};

} // namespace ns3

#endif
