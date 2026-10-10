
#ifndef ICMPV4_H
#define ICMPV4_H

#include "ipv4-header.h"

#include "ns3/header.h"
#include "ns3/ptr.h"

#include <stdint.h>

namespace ns3 {

class Packet;

class Icmpv4Header : public Header {
public:
  enum Type_e {
    ICMPV4_ECHO_REPLY = 0,
    ICMPV4_DEST_UNREACH = 3,
    ICMPV4_ECHO = 8,
    ICMPV4_TIME_EXCEEDED = 11
  };

  void EnableChecksum();

  void SetType(uint8_t type);

  void SetCode(uint8_t code);

  uint8_t GetType() const;
  uint8_t GetCode() const;

  static TypeId GetTypeId();
  Icmpv4Header();
  ~Icmpv4Header() override;

  TypeId GetInstanceTypeId() const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  void Print(std::ostream &os) const override;

private:
  uint8_t m_type;
  uint8_t m_code;
  bool m_calcChecksum;
};

class Icmpv4Echo : public Header {
public:
  void SetIdentifier(uint16_t id);
  void SetSequenceNumber(uint16_t seq);
  void SetData(Ptr<const Packet> data);
  uint16_t GetIdentifier() const;
  uint16_t GetSequenceNumber() const;
  uint32_t GetDataSize() const;
  uint32_t GetData(uint8_t payload[]) const;

  static TypeId GetTypeId();
  Icmpv4Echo();
  ~Icmpv4Echo() override;
  TypeId GetInstanceTypeId() const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  void Print(std::ostream &os) const override;

private:
  uint16_t m_identifier;
  uint16_t m_sequence;
  uint8_t *m_data;
  uint32_t m_dataSize;
};

class Icmpv4DestinationUnreachable : public Header {
public:
  enum ErrorDestinationUnreachable_e {
    ICMPV4_NET_UNREACHABLE = 0,
    ICMPV4_HOST_UNREACHABLE = 1,
    ICMPV4_PROTOCOL_UNREACHABLE = 2,
    ICMPV4_PORT_UNREACHABLE = 3,
    ICMPV4_FRAG_NEEDED = 4,
    ICMPV4_SOURCE_ROUTE_FAILED = 5
  };

  static TypeId GetTypeId();
  Icmpv4DestinationUnreachable();
  ~Icmpv4DestinationUnreachable() override;

  void SetNextHopMtu(uint16_t mtu);
  uint16_t GetNextHopMtu() const;

  void SetData(Ptr<const Packet> data);
  void SetHeader(Ipv4Header header);

  void GetData(uint8_t payload[8]) const;
  Ipv4Header GetHeader() const;

private:
  TypeId GetInstanceTypeId() const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  void Print(std::ostream &os) const override;

private:
  uint16_t m_nextHopMtu;
  Ipv4Header m_header;
  uint8_t m_data[8];
};

class Icmpv4TimeExceeded : public Header {
public:
  enum ErrorTimeExceeded_e {
    ICMPV4_TIME_TO_LIVE = 0,
    ICMPV4_FRAGMENT_REASSEMBLY = 1
  };

  void SetData(Ptr<const Packet> data);
  void SetHeader(Ipv4Header header);

  void GetData(uint8_t payload[8]) const;
  Ipv4Header GetHeader() const;

  static TypeId GetTypeId();
  Icmpv4TimeExceeded();
  ~Icmpv4TimeExceeded() override;
  TypeId GetInstanceTypeId() const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  void Print(std::ostream &os) const override;

private:
  Ipv4Header m_header;
  uint8_t m_data[8];
};

} // namespace ns3

#endif
