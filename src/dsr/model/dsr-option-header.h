
#ifndef DSR_OPTION_HEADER_H
#define DSR_OPTION_HEADER_H

#include "ns3/header.h"
#include "ns3/ipv4-address.h"
#include "ns3/simulator.h"

#include <algorithm>
#include <ostream>

namespace ns3 {

class Time;

namespace dsr {
class DsrOptionHeader : public Header {
public:
  struct Alignment {
    uint8_t factor;
    uint8_t offset;
  };

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  DsrOptionHeader();
  ~DsrOptionHeader() override;
  void SetType(uint8_t type);
  uint8_t GetType() const;
  void SetLength(uint8_t length);
  uint8_t GetLength() const;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  virtual Alignment GetAlignment() const;

private:
  uint8_t m_type;
  uint8_t m_length;
  Buffer m_data;
};

class DsrOptionPad1Header : public DsrOptionHeader {
public:
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  DsrOptionPad1Header();
  ~DsrOptionPad1Header() override;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
};

class DsrOptionPadnHeader : public DsrOptionHeader {
public:
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  DsrOptionPadnHeader(uint32_t pad = 2);
  ~DsrOptionPadnHeader() override;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
};

class DsrOptionRreqHeader : public DsrOptionHeader {
public:
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  DsrOptionRreqHeader();
  ~DsrOptionRreqHeader() override;
  void SetNumberAddress(uint8_t n);
  Ipv4Address GetTarget();
  void SetTarget(Ipv4Address target);
  void SetNodesAddress(std::vector<Ipv4Address> ipv4Address);
  std::vector<Ipv4Address> GetNodesAddresses() const;
  uint32_t GetNodesNumber() const;
  void AddNodeAddress(Ipv4Address ipv4);
  void SetNodeAddress(uint8_t index, Ipv4Address addr);
  Ipv4Address GetNodeAddress(uint8_t index) const;
  void SetId(uint16_t identification);
  uint16_t GetId() const;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  Alignment GetAlignment() const override;

private:
  uint16_t m_identification;
  Ipv4Address m_target;
  Ipv4Address m_address;
  typedef std::vector<Ipv4Address> VectorIpv4Address_t;
  VectorIpv4Address_t m_ipv4Address;
};

class DsrOptionRrepHeader : public DsrOptionHeader {
public:
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  DsrOptionRrepHeader();
  ~DsrOptionRrepHeader() override;
  void SetNumberAddress(uint8_t n);
  void SetNodesAddress(std::vector<Ipv4Address> ipv4Address);
  std::vector<Ipv4Address> GetNodesAddress() const;
  Ipv4Address GetTargetAddress(std::vector<Ipv4Address> ipv4Address) const;
  void SetNodeAddress(uint8_t index, Ipv4Address addr);
  Ipv4Address GetNodeAddress(uint8_t index) const;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  Alignment GetAlignment() const override;

private:
  Ipv4Address m_address;
  typedef std::vector<Ipv4Address> VectorIpv4Address_t;
  VectorIpv4Address_t m_ipv4Address;
};

class DsrOptionSRHeader : public DsrOptionHeader {
public:
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  DsrOptionSRHeader();
  ~DsrOptionSRHeader() override;
  void SetSegmentsLeft(uint8_t segmentsLeft);
  uint8_t GetSegmentsLeft() const;
  void SetNumberAddress(uint8_t n);
  void SetNodesAddress(std::vector<Ipv4Address> ipv4Address);
  std::vector<Ipv4Address> GetNodesAddress() const;
  uint8_t GetNodeListSize() const;
  void SetNodeAddress(uint8_t index, Ipv4Address addr);
  Ipv4Address GetNodeAddress(uint8_t index) const;
  void SetSalvage(uint8_t salvage);
  uint8_t GetSalvage() const;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  Alignment GetAlignment() const override;

  typedef void (*TracedCallback)(const DsrOptionSRHeader &header);

private:
  Ipv4Address m_address;
  uint8_t m_segmentsLeft;
  uint8_t m_salvage;
  typedef std::vector<Ipv4Address> VectorIpv4Address_t;
  VectorIpv4Address_t m_ipv4Address;
};

enum ErrorType {
  NODE_UNREACHABLE = 1,
  FLOW_STATE_NOT_SUPPORTED = 2,
  OPTION_NOT_SUPPORTED = 3,
};

class DsrOptionRerrHeader : public DsrOptionHeader {
public:
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  DsrOptionRerrHeader();
  ~DsrOptionRerrHeader() override;
  void SetErrorType(uint8_t errorType);
  uint8_t GetErrorType() const;
  virtual void SetErrorSrc(Ipv4Address errorSrcAddress);
  virtual Ipv4Address GetErrorSrc() const;
  virtual void SetSalvage(uint8_t salvage);
  virtual uint8_t GetSalvage() const;
  virtual void SetErrorDst(Ipv4Address errorDstAddress);
  virtual Ipv4Address GetErrorDst() const;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  Alignment GetAlignment() const override;

private:
  uint8_t m_errorType;
  uint8_t m_salvage;
  uint16_t m_errorLength;
  Ipv4Address m_errorSrcAddress;
  Ipv4Address m_errorDstAddress;
  Buffer m_errorData;
};

class DsrOptionRerrUnreachHeader : public DsrOptionRerrHeader {
public:
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  DsrOptionRerrUnreachHeader();
  ~DsrOptionRerrUnreachHeader() override;
  void SetErrorSrc(Ipv4Address errorSrcAddress) override;
  Ipv4Address GetErrorSrc() const override;
  void SetSalvage(uint8_t salvage) override;
  uint8_t GetSalvage() const override;
  void SetErrorDst(Ipv4Address errorDstAddress) override;
  Ipv4Address GetErrorDst() const override;
  void SetUnreachNode(Ipv4Address unreachNode);
  Ipv4Address GetUnreachNode() const;
  void SetOriginalDst(Ipv4Address originalDst);
  Ipv4Address GetOriginalDst() const;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  Alignment GetAlignment() const override;

private:
  uint8_t m_errorType;
  uint8_t m_salvage;
  Ipv4Address m_errorSrcAddress;
  Ipv4Address m_errorDstAddress;
  Ipv4Address m_unreachNode;
  Ipv4Address m_originalDst;
};

class DsrOptionRerrUnsupportedHeader : public DsrOptionRerrHeader {
public:
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  DsrOptionRerrUnsupportedHeader();
  ~DsrOptionRerrUnsupportedHeader() override;
  void SetErrorSrc(Ipv4Address errorSrcAddress) override;
  Ipv4Address GetErrorSrc() const override;
  void SetSalvage(uint8_t salvage) override;
  uint8_t GetSalvage() const override;
  void SetErrorDst(Ipv4Address errorDstAddress) override;
  Ipv4Address GetErrorDst() const override;
  void SetUnsupported(uint16_t optionType);
  uint16_t GetUnsupported() const;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  Alignment GetAlignment() const override;

private:
  uint8_t m_errorType;
  uint8_t m_salvage;
  Ipv4Address m_errorSrcAddress;
  Ipv4Address m_errorDstAddress;
  uint16_t m_unsupported;
};

class DsrOptionAckReqHeader : public DsrOptionHeader {
public:
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  DsrOptionAckReqHeader();
  ~DsrOptionAckReqHeader() override;
  void SetAckId(uint16_t identification);
  uint16_t GetAckId() const;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  Alignment GetAlignment() const override;

private:
  uint16_t m_identification;
};

class DsrOptionAckHeader : public DsrOptionHeader {
public:
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  DsrOptionAckHeader();
  ~DsrOptionAckHeader() override;
  void SetAckId(uint16_t identification);
  uint16_t GetAckId() const;
  void SetRealSrc(Ipv4Address realSrcAddress);
  Ipv4Address GetRealSrc() const;
  void SetRealDst(Ipv4Address realDstAddress);
  Ipv4Address GetRealDst() const;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  Alignment GetAlignment() const override;

private:
  uint16_t m_identification;
  Ipv4Address m_realSrcAddress;
  Ipv4Address m_realDstAddress;
};

[[maybe_unused]] static inline std::ostream &
operator<<(std::ostream &os, const DsrOptionSRHeader &sr) {
  sr.Print(os);
  return os;
}

} // namespace dsr
} // namespace ns3

#endif
