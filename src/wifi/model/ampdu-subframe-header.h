
#ifndef AMPDU_SUBFRAME_HEADER_H
#define AMPDU_SUBFRAME_HEADER_H

#include "ns3/header.h"

namespace ns3 {

class AmpduSubframeHeader : public Header {
public:
  AmpduSubframeHeader();
  ~AmpduSubframeHeader() override;

  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

  void SetLength(uint16_t length);
  void SetEof(bool eof);
  uint16_t GetLength() const;
  bool GetEof() const;
  bool IsSignatureValid() const;

private:
  uint16_t m_length;
  bool m_eof;
  uint8_t m_signature;
};

} // namespace ns3

#endif
