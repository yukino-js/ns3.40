
#ifndef THREE_GPP_HTTP_HEADER_H
#define THREE_GPP_HTTP_HEADER_H

#include <ns3/header.h>
#include <ns3/nstime.h>

namespace ns3 {

class Packet;

class ThreeGppHttpHeader : public Header {
public:
  ThreeGppHttpHeader();

  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  void Print(std::ostream &os) const override;

  std::string ToString() const;

  enum ContentType_t { NOT_SET, MAIN_OBJECT, EMBEDDED_OBJECT };

  void SetContentType(ContentType_t contentType);

  ContentType_t GetContentType() const;

  void SetContentLength(uint32_t contentLength);

  uint32_t GetContentLength() const;

  void SetClientTs(Time clientTs);

  Time GetClientTs() const;

  void SetServerTs(Time serverTs);

  Time GetServerTs() const;

private:
  uint16_t m_contentType;
  uint32_t m_contentLength;
  uint64_t m_clientTs;
  uint64_t m_serverTs;
};

} // namespace ns3

#endif
