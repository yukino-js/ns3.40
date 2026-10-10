
#ifndef IPV6_OPTION_HEADER_H
#define IPV6_OPTION_HEADER_H

#include "ns3/header.h"

#include <ostream>

namespace ns3 {

class Ipv6OptionHeader : public Header {
public:
  struct Alignment {
    uint8_t factor;
    uint8_t offset;
  };

  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  Ipv6OptionHeader();

  ~Ipv6OptionHeader() override;

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

class Ipv6OptionPad1Header : public Ipv6OptionHeader {
public:
  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  Ipv6OptionPad1Header();

  ~Ipv6OptionPad1Header() override;

  void Print(std::ostream &os) const override;

  uint32_t GetSerializedSize() const override;

  void Serialize(Buffer::Iterator start) const override;

  uint32_t Deserialize(Buffer::Iterator start) override;
};

class Ipv6OptionPadnHeader : public Ipv6OptionHeader {
public:
  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  Ipv6OptionPadnHeader(uint32_t pad = 2);

  ~Ipv6OptionPadnHeader() override;

  void Print(std::ostream &os) const override;

  uint32_t GetSerializedSize() const override;

  void Serialize(Buffer::Iterator start) const override;

  uint32_t Deserialize(Buffer::Iterator start) override;
};

class Ipv6OptionJumbogramHeader : public Ipv6OptionHeader {
public:
  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  Ipv6OptionJumbogramHeader();

  ~Ipv6OptionJumbogramHeader() override;

  void SetDataLength(uint32_t dataLength);

  uint32_t GetDataLength() const;

  void Print(std::ostream &os) const override;

  uint32_t GetSerializedSize() const override;

  void Serialize(Buffer::Iterator start) const override;

  uint32_t Deserialize(Buffer::Iterator start) override;

  Alignment GetAlignment() const override;

private:
  uint32_t m_dataLength;
};

class Ipv6OptionRouterAlertHeader : public Ipv6OptionHeader {
public:
  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  Ipv6OptionRouterAlertHeader();

  ~Ipv6OptionRouterAlertHeader() override;

  void SetValue(uint16_t value);

  uint16_t GetValue() const;

  void Print(std::ostream &os) const override;

  uint32_t GetSerializedSize() const override;

  void Serialize(Buffer::Iterator start) const override;

  uint32_t Deserialize(Buffer::Iterator start) override;

  Alignment GetAlignment() const override;

private:
  uint16_t m_value;
};

} // namespace ns3

#endif
