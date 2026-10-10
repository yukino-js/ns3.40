
#ifndef EPS_GTPU_V1_H
#define EPS_GTPU_V1_H

#include <ns3/header.h>
#include <ns3/ipv4-header.h>
#include <ns3/ptr.h>

namespace ns3 {

class Packet;

class GtpuHeader : public Header {
public:
  static TypeId GetTypeId();
  GtpuHeader();
  ~GtpuHeader() override;
  TypeId GetInstanceTypeId() const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  void Print(std::ostream &os) const override;

  bool GetExtensionHeaderFlag() const;
  uint16_t GetLength() const;
  uint8_t GetMessageType() const;
  uint8_t GetNPduNumber() const;
  bool GetNPduNumberFlag() const;
  uint8_t GetNextExtensionType() const;
  bool GetProtocolType() const;
  uint16_t GetSequenceNumber() const;
  bool GetSequenceNumberFlag() const;
  uint32_t GetTeid() const;
  uint8_t GetVersion() const;
  void SetExtensionHeaderFlag(bool extensionHeaderFlag);
  void SetLength(uint16_t length);
  void SetMessageType(uint8_t messageType);
  void SetNPduNumber(uint8_t nPduNumber);
  void SetNPduNumberFlag(bool nPduNumberFlag);
  void SetNextExtensionType(uint8_t nextExtensionType);
  void SetProtocolType(bool protocolType);
  void SetSequenceNumber(uint16_t sequenceNumber);
  void SetSequenceNumberFlag(bool sequenceNumberFlag);
  void SetTeid(uint32_t teid);
  void SetVersion(uint8_t version);

  bool operator==(const GtpuHeader &b) const;

private:
  uint8_t m_version;

  bool m_protocolType;

  bool m_extensionHeaderFlag;

  bool m_sequenceNumberFlag;
  bool m_nPduNumberFlag;
  uint8_t m_messageType;
  uint16_t m_length;

  uint32_t m_teid;
  uint16_t m_sequenceNumber;
  uint8_t m_nPduNumber;
  uint8_t m_nextExtensionType;
};

} // namespace ns3

#endif
