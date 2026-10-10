
#ifndef RIPNG_HEADER_H
#define RIPNG_HEADER_H

#include "ipv6-header.h"

#include "ns3/header.h"
#include "ns3/ipv6-address.h"
#include "ns3/packet.h"

#include <list>

namespace ns3 {

class RipNgRte : public Header {
public:
  RipNgRte();

  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  void Print(std::ostream &os) const override;

  uint32_t GetSerializedSize() const override;

  void Serialize(Buffer::Iterator start) const override;

  uint32_t Deserialize(Buffer::Iterator start) override;

  void SetPrefix(Ipv6Address prefix);

  Ipv6Address GetPrefix() const;

  void SetPrefixLen(uint8_t prefixLen);

  uint8_t GetPrefixLen() const;

  void SetRouteTag(uint16_t routeTag);

  uint16_t GetRouteTag() const;

  void SetRouteMetric(uint8_t routeMetric);

  uint8_t GetRouteMetric() const;

private:
  Ipv6Address m_prefix;
  uint16_t m_tag;
  uint8_t m_prefixLen;
  uint8_t m_metric;
};

std::ostream &operator<<(std::ostream &os, const RipNgRte &h);

class RipNgHeader : public Header {
public:
  RipNgHeader();

  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  void Print(std::ostream &os) const override;

  uint32_t GetSerializedSize() const override;

  void Serialize(Buffer::Iterator start) const override;

  uint32_t Deserialize(Buffer::Iterator start) override;

  enum Command_e {
    REQUEST = 0x1,
    RESPONSE = 0x2,
  };

  void SetCommand(Command_e command);

  Command_e GetCommand() const;

  void AddRte(RipNgRte rte);

  void ClearRtes();

  uint16_t GetRteNumber() const;

  std::list<RipNgRte> GetRteList() const;

private:
  uint8_t m_command;
  std::list<RipNgRte> m_rteList;
};

std::ostream &operator<<(std::ostream &os, const RipNgHeader &h);

} // namespace ns3

#endif
