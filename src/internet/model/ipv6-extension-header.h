
#ifndef IPV6_EXTENSION_HEADER_H
#define IPV6_EXTENSION_HEADER_H

#include "ipv6-option-header.h"

#include "ns3/header.h"
#include "ns3/ipv6-address.h"

#include <list>
#include <ostream>
#include <vector>

namespace ns3 {

class Ipv6ExtensionHeader : public Header {
public:
  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  Ipv6ExtensionHeader();

  ~Ipv6ExtensionHeader() override;

  void SetNextHeader(uint8_t nextHeader);

  uint8_t GetNextHeader() const;

  void SetLength(uint16_t length);

  uint16_t GetLength() const;

  void Print(std::ostream &os) const override;

  uint32_t GetSerializedSize() const override;

  void Serialize(Buffer::Iterator start) const override;

  uint32_t Deserialize(Buffer::Iterator start) override;

protected:
  uint8_t m_length;

private:
  uint8_t m_nextHeader;

  Buffer m_data;
};

class OptionField {
public:
  OptionField(uint32_t optionsOffset);

  ~OptionField();

  uint32_t GetSerializedSize() const;

  void Serialize(Buffer::Iterator start) const;

  uint32_t Deserialize(Buffer::Iterator start, uint32_t length);

  void AddOption(const Ipv6OptionHeader &option);

  uint32_t GetOptionsOffset() const;

  Buffer GetOptionBuffer();

private:
  uint32_t CalculatePad(Ipv6OptionHeader::Alignment alignment) const;

  Buffer m_optionData;

  uint32_t m_optionsOffset;
};

class Ipv6ExtensionHopByHopHeader : public Ipv6ExtensionHeader,
                                    public OptionField {
public:
  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  Ipv6ExtensionHopByHopHeader();

  ~Ipv6ExtensionHopByHopHeader() override;

  void Print(std::ostream &os) const override;

  uint32_t GetSerializedSize() const override;

  void Serialize(Buffer::Iterator start) const override;

  uint32_t Deserialize(Buffer::Iterator start) override;
};

class Ipv6ExtensionDestinationHeader : public Ipv6ExtensionHeader,
                                       public OptionField {
public:
  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  Ipv6ExtensionDestinationHeader();

  ~Ipv6ExtensionDestinationHeader() override;

  void Print(std::ostream &os) const override;

  uint32_t GetSerializedSize() const override;

  void Serialize(Buffer::Iterator start) const override;

  uint32_t Deserialize(Buffer::Iterator start) override;
};

class Ipv6ExtensionFragmentHeader : public Ipv6ExtensionHeader {
public:
  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  Ipv6ExtensionFragmentHeader();

  ~Ipv6ExtensionFragmentHeader() override;

  void SetOffset(uint16_t offset);

  uint16_t GetOffset() const;

  void SetMoreFragment(bool moreFragment);

  bool GetMoreFragment() const;

  void SetIdentification(uint32_t identification);

  uint32_t GetIdentification() const;

  void Print(std::ostream &os) const override;

  uint32_t GetSerializedSize() const override;

  void Serialize(Buffer::Iterator start) const override;

  uint32_t Deserialize(Buffer::Iterator start) override;

private:
  uint16_t m_offset;

  uint32_t m_identification;
};

class Ipv6ExtensionRoutingHeader : public Ipv6ExtensionHeader {
public:
  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  Ipv6ExtensionRoutingHeader();

  ~Ipv6ExtensionRoutingHeader() override;

  void SetTypeRouting(uint8_t typeRouting);

  uint8_t GetTypeRouting() const;

  void SetSegmentsLeft(uint8_t segmentsLeft);

  uint8_t GetSegmentsLeft() const;

  void Print(std::ostream &os) const override;

  uint32_t GetSerializedSize() const override;

  void Serialize(Buffer::Iterator start) const override;

  uint32_t Deserialize(Buffer::Iterator start) override;

private:
  uint8_t m_typeRouting;

  uint8_t m_segmentsLeft;
};

class Ipv6ExtensionLooseRoutingHeader : public Ipv6ExtensionRoutingHeader {
public:
  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  Ipv6ExtensionLooseRoutingHeader();

  ~Ipv6ExtensionLooseRoutingHeader() override;

  void SetNumberAddress(uint8_t n);

  void SetRoutersAddress(std::vector<Ipv6Address> routersAddress);

  std::vector<Ipv6Address> GetRoutersAddress() const;

  void SetRouterAddress(uint8_t index, Ipv6Address addr);

  Ipv6Address GetRouterAddress(uint8_t index) const;

  void Print(std::ostream &os) const override;

  uint32_t GetSerializedSize() const override;

  void Serialize(Buffer::Iterator start) const override;

  uint32_t Deserialize(Buffer::Iterator start) override;

private:
  typedef std::vector<Ipv6Address> VectorIpv6Address_t;

  VectorIpv6Address_t m_routersAddress;
};

class Ipv6ExtensionESPHeader : public Ipv6ExtensionHeader {
public:
  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  Ipv6ExtensionESPHeader();

  ~Ipv6ExtensionESPHeader() override;

  void Print(std::ostream &os) const override;

  uint32_t GetSerializedSize() const override;

  void Serialize(Buffer::Iterator start) const override;

  uint32_t Deserialize(Buffer::Iterator start) override;
};

class Ipv6ExtensionAHHeader : public Ipv6ExtensionHeader {
public:
  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  Ipv6ExtensionAHHeader();

  ~Ipv6ExtensionAHHeader() override;

  void Print(std::ostream &os) const override;

  uint32_t GetSerializedSize() const override;

  void Serialize(Buffer::Iterator start) const override;

  uint32_t Deserialize(Buffer::Iterator start) override;
};

} // namespace ns3

#endif
