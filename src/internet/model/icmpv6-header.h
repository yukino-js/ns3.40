
#ifndef ICMPV6_HEADER_H
#define ICMPV6_HEADER_H

#include "ns3/header.h"
#include "ns3/ipv6-address.h"
#include "ns3/packet.h"

namespace ns3 {

class Icmpv6Header : public Header {
public:
  enum Type_e {
    ICMPV6_ERROR_DESTINATION_UNREACHABLE = 1,
    ICMPV6_ERROR_PACKET_TOO_BIG,
    ICMPV6_ERROR_TIME_EXCEEDED,
    ICMPV6_ERROR_PARAMETER_ERROR,
    ICMPV6_ECHO_REQUEST = 128,
    ICMPV6_ECHO_REPLY,
    ICMPV6_SUBSCRIBE_REQUEST,
    ICMPV6_SUBSCRIBE_REPORT,
    ICMPV6_SUBSCRIVE_END,
    ICMPV6_ND_ROUTER_SOLICITATION,
    ICMPV6_ND_ROUTER_ADVERTISEMENT,
    ICMPV6_ND_NEIGHBOR_SOLICITATION,
    ICMPV6_ND_NEIGHBOR_ADVERTISEMENT,
    ICMPV6_ND_REDIRECTION,
    ICMPV6_ROUTER_RENUMBER,
    ICMPV6_INFORMATION_REQUEST,
    ICMPV6_INFORMATION_RESPONSE,
    ICMPV6_INVERSE_ND_SOLICITATION,
    ICMPV6_INVERSE_ND_ADVERSTISEMENT,
    ICMPV6_MLDV2_SUBSCRIBE_REPORT,
    ICMPV6_MOBILITY_HA_DISCOVER_REQUEST,
    ICMPV6_MOBILITY_HA_DISCOVER_RESPONSE,
    ICMPV6_MOBILITY_MOBILE_PREFIX_SOLICITATION,
    ICMPV6_SECURE_ND_CERTIFICATE_PATH_SOLICITATION,
    ICMPV6_SECURE_ND_CERTIFICATE_PATH_ADVERTISEMENT,
    ICMPV6_EXPERIMENTAL_MOBILITY
  };

  enum OptionType_e {
    ICMPV6_OPT_LINK_LAYER_SOURCE = 1,
    ICMPV6_OPT_LINK_LAYER_TARGET,
    ICMPV6_OPT_PREFIX,
    ICMPV6_OPT_REDIRECTED,
    ICMPV6_OPT_MTU
  };

  enum ErrorDestinationUnreachable_e {
    ICMPV6_NO_ROUTE = 0,
    ICMPV6_ADM_PROHIBITED,
    ICMPV6_NOT_NEIGHBOUR,
    ICMPV6_ADDR_UNREACHABLE,
    ICMPV6_PORT_UNREACHABLE
  };

  enum ErrorTimeExceeded_e { ICMPV6_HOPLIMIT = 0, ICMPV6_FRAGTIME };

  enum ErrorParameterError_e {
    ICMPV6_MALFORMED_HEADER = 0,
    ICMPV6_UNKNOWN_NEXT_HEADER,
    ICMPV6_UNKNOWN_OPTION
  };

  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  Icmpv6Header();

  ~Icmpv6Header() override;

  uint8_t GetType() const;

  void SetType(uint8_t type);

  uint8_t GetCode() const;

  void SetCode(uint8_t code);

  uint16_t GetChecksum() const;

  void SetChecksum(uint16_t checksum);

  void Print(std::ostream &os) const override;

  uint32_t GetSerializedSize() const override;

  void Serialize(Buffer::Iterator start) const override;

  uint32_t Deserialize(Buffer::Iterator start) override;

  void CalculatePseudoHeaderChecksum(Ipv6Address src, Ipv6Address dst,
                                     uint16_t length, uint8_t protocol);

protected:
  bool m_calcChecksum;

  uint16_t m_checksum;

private:
  uint8_t m_type;

  uint8_t m_code;
};

class Icmpv6OptionHeader : public Header {
public:
  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  Icmpv6OptionHeader();

  ~Icmpv6OptionHeader() override;

  uint8_t GetType() const;

  void SetType(uint8_t type);

  uint8_t GetLength() const;

  void SetLength(uint8_t len);

  void Print(std::ostream &os) const override;

  uint32_t GetSerializedSize() const override;

  void Serialize(Buffer::Iterator start) const override;

  uint32_t Deserialize(Buffer::Iterator start) override;

private:
  uint8_t m_type;

  uint8_t m_len;
};

class Icmpv6NS : public Icmpv6Header {
public:
  Icmpv6NS(Ipv6Address target);

  Icmpv6NS();

  ~Icmpv6NS() override;

  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  uint32_t GetReserved() const;

  void SetReserved(uint32_t reserved);

  Ipv6Address GetIpv6Target() const;

  void SetIpv6Target(Ipv6Address target);

  void Print(std::ostream &os) const override;

  uint32_t GetSerializedSize() const override;

  void Serialize(Buffer::Iterator start) const override;

  uint32_t Deserialize(Buffer::Iterator start) override;

private:
  uint32_t m_reserved;

  Ipv6Address m_target;
};

class Icmpv6NA : public Icmpv6Header {
public:
  Icmpv6NA();

  ~Icmpv6NA() override;

  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  uint32_t GetReserved() const;

  void SetReserved(uint32_t reserved);

  Ipv6Address GetIpv6Target() const;

  void SetIpv6Target(Ipv6Address target);

  bool GetFlagR() const;

  void SetFlagR(bool r);

  bool GetFlagS() const;

  void SetFlagS(bool s);

  bool GetFlagO() const;

  void SetFlagO(bool o);

  void Print(std::ostream &os) const override;

  uint32_t GetSerializedSize() const override;

  void Serialize(Buffer::Iterator start) const override;

  uint32_t Deserialize(Buffer::Iterator start) override;

private:
  bool m_flagR;

  bool m_flagS;

  bool m_flagO;

  uint32_t m_reserved;

  Ipv6Address m_target;
};

class Icmpv6RA : public Icmpv6Header {
public:
  Icmpv6RA();

  ~Icmpv6RA() override;

  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  void SetCurHopLimit(uint8_t m);

  uint8_t GetCurHopLimit() const;

  void SetLifeTime(uint16_t l);

  uint16_t GetLifeTime() const;

  void SetReachableTime(uint32_t r);

  uint32_t GetReachableTime() const;

  void SetRetransmissionTime(uint32_t r);

  uint32_t GetRetransmissionTime() const;

  bool GetFlagM() const;

  void SetFlagM(bool m);

  bool GetFlagO() const;

  void SetFlagO(bool o);

  bool GetFlagH() const;

  void SetFlagH(bool h);

  void Print(std::ostream &os) const override;

  uint32_t GetSerializedSize() const override;

  void Serialize(Buffer::Iterator start) const override;

  uint32_t Deserialize(Buffer::Iterator start) override;

private:
  bool m_flagM;

  bool m_flagO;

  bool m_flagH;

  uint16_t m_LifeTime;

  uint32_t m_ReachableTime;

  uint32_t m_RetransmissionTimer;

  uint8_t m_curHopLimit;
};

class Icmpv6RS : public Icmpv6Header {
public:
  Icmpv6RS();

  ~Icmpv6RS() override;

  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  uint32_t GetReserved() const;

  void SetReserved(uint32_t reserved);

  void Print(std::ostream &os) const override;

  uint32_t GetSerializedSize() const override;

  void Serialize(Buffer::Iterator start) const override;

  uint32_t Deserialize(Buffer::Iterator start) override;

private:
  uint32_t m_reserved;
};

class Icmpv6Redirection : public Icmpv6Header {
public:
  Icmpv6Redirection();

  ~Icmpv6Redirection() override;

  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  Ipv6Address GetTarget() const;

  void SetTarget(Ipv6Address target);

  Ipv6Address GetDestination() const;

  void SetDestination(Ipv6Address destination);

  void Print(std::ostream &os) const override;

  uint32_t GetSerializedSize() const override;

  void Serialize(Buffer::Iterator start) const override;

  uint32_t Deserialize(Buffer::Iterator start) override;

  uint32_t GetReserved() const;

  void SetReserved(uint32_t reserved);

private:
  Ipv6Address m_target;

  Ipv6Address m_destination;

  uint32_t m_reserved;
};

class Icmpv6Echo : public Icmpv6Header {
public:
  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  Icmpv6Echo();

  Icmpv6Echo(bool request);

  ~Icmpv6Echo() override;

  uint16_t GetId() const;

  void SetId(uint16_t id);

  uint16_t GetSeq() const;

  void SetSeq(uint16_t seq);

  void Print(std::ostream &os) const override;

  uint32_t GetSerializedSize() const override;

  void Serialize(Buffer::Iterator start) const override;

  uint32_t Deserialize(Buffer::Iterator start) override;

private:
  uint16_t m_id;

  uint16_t m_seq;
};

class Icmpv6DestinationUnreachable : public Icmpv6Header {
public:
  Icmpv6DestinationUnreachable();

  ~Icmpv6DestinationUnreachable() override;

  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  void SetPacket(Ptr<Packet> p);

  void Print(std::ostream &os) const override;

  uint32_t GetSerializedSize() const override;

  void Serialize(Buffer::Iterator start) const override;

  uint32_t Deserialize(Buffer::Iterator start) override;

private:
  Ptr<Packet> m_packet;
};

class Icmpv6TooBig : public Icmpv6Header {
public:
  Icmpv6TooBig();

  ~Icmpv6TooBig() override;

  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  void SetPacket(Ptr<Packet> p);

  uint32_t GetMtu() const;

  void SetMtu(uint32_t mtu);

  void Print(std::ostream &os) const override;

  uint32_t GetSerializedSize() const override;

  void Serialize(Buffer::Iterator start) const override;

  uint32_t Deserialize(Buffer::Iterator start) override;

private:
  Ptr<Packet> m_packet;

  uint32_t m_mtu;
};

class Icmpv6TimeExceeded : public Icmpv6Header {
public:
  Icmpv6TimeExceeded();

  ~Icmpv6TimeExceeded() override;

  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  void SetPacket(Ptr<Packet> p);

  void Print(std::ostream &os) const override;

  uint32_t GetSerializedSize() const override;

  void Serialize(Buffer::Iterator start) const override;

  uint32_t Deserialize(Buffer::Iterator start) override;

private:
  Ptr<Packet> m_packet;
};

class Icmpv6ParameterError : public Icmpv6Header {
public:
  Icmpv6ParameterError();

  ~Icmpv6ParameterError() override;

  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  void SetPacket(Ptr<Packet> p);

  uint32_t GetPtr() const;

  void SetPtr(uint32_t ptr);

  void Print(std::ostream &os) const override;

  uint32_t GetSerializedSize() const override;

  void Serialize(Buffer::Iterator start) const override;

  uint32_t Deserialize(Buffer::Iterator start) override;

private:
  Ptr<Packet> m_packet;

  uint32_t m_ptr;
};

class Icmpv6OptionMtu : public Icmpv6OptionHeader {
public:
  Icmpv6OptionMtu();

  Icmpv6OptionMtu(uint32_t mtu);

  ~Icmpv6OptionMtu() override;

  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  uint16_t GetReserved() const;

  void SetReserved(uint16_t reserved);

  uint32_t GetMtu() const;

  void SetMtu(uint32_t mtu);

  void Print(std::ostream &os) const override;

  uint32_t GetSerializedSize() const override;

  void Serialize(Buffer::Iterator start) const override;

  uint32_t Deserialize(Buffer::Iterator start) override;

private:
  uint16_t m_reserved;

  uint32_t m_mtu;
};

class Icmpv6OptionPrefixInformation : public Icmpv6OptionHeader {
public:
  Icmpv6OptionPrefixInformation();

  Icmpv6OptionPrefixInformation(Ipv6Address network, uint8_t prefixlen);

  ~Icmpv6OptionPrefixInformation() override;

  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  enum Flags_t { NONE = 0, ROUTERADDR = 32, AUTADDRCONF = 64, ONLINK = 128 };

  uint8_t GetPrefixLength() const;

  void SetPrefixLength(uint8_t prefixLength);

  uint8_t GetFlags() const;

  void SetFlags(uint8_t flags);

  uint32_t GetValidTime() const;

  void SetValidTime(uint32_t validTime);

  uint32_t GetPreferredTime() const;

  void SetPreferredTime(uint32_t preferredTime);

  uint32_t GetReserved() const;

  void SetReserved(uint32_t reserved);

  Ipv6Address GetPrefix() const;

  void SetPrefix(Ipv6Address prefix);

  void Print(std::ostream &os) const override;

  uint32_t GetSerializedSize() const override;

  void Serialize(Buffer::Iterator start) const override;

  uint32_t Deserialize(Buffer::Iterator start) override;

private:
  Ipv6Address m_prefix;

  uint8_t m_prefixLength;

  uint8_t m_flags;

  uint32_t m_validTime;

  uint32_t m_preferredTime;

  uint32_t m_reserved;
};

class Icmpv6OptionLinkLayerAddress : public Icmpv6OptionHeader {
public:
  Icmpv6OptionLinkLayerAddress(bool source);

  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  Icmpv6OptionLinkLayerAddress(bool source, Address addr);

  Icmpv6OptionLinkLayerAddress();

  ~Icmpv6OptionLinkLayerAddress() override;

  Address GetAddress() const;

  void SetAddress(Address addr);

  void Print(std::ostream &os) const override;

  uint32_t GetSerializedSize() const override;

  void Serialize(Buffer::Iterator start) const override;

  uint32_t Deserialize(Buffer::Iterator start) override;

private:
  Address m_addr;
};

class Icmpv6OptionRedirected : public Icmpv6OptionHeader {
public:
  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  Icmpv6OptionRedirected();

  ~Icmpv6OptionRedirected() override;

  void SetPacket(Ptr<Packet> packet);

  void Print(std::ostream &os) const override;

  uint32_t GetSerializedSize() const override;

  void Serialize(Buffer::Iterator start) const override;

  uint32_t Deserialize(Buffer::Iterator start) override;

private:
  Ptr<Packet> m_packet;
};

} // namespace ns3

#endif
