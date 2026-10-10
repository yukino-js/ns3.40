
#ifndef RIP_HEADER_H
#define RIP_HEADER_H

#include "ipv4-header.h"

#include "ns3/header.h"
#include "ns3/ipv4-address.h"
#include "ns3/packet.h"

#include <list>

namespace ns3 {

class RipRte : public Header {
public:
  RipRte();

  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  void Print(std::ostream &os) const override;

  uint32_t GetSerializedSize() const override;

  void Serialize(Buffer::Iterator start) const override;

  uint32_t Deserialize(Buffer::Iterator start) override;

  void SetPrefix(Ipv4Address prefix);

  Ipv4Address GetPrefix() const;

  void SetSubnetMask(Ipv4Mask subnetMask);

  Ipv4Mask GetSubnetMask() const;

  void SetRouteTag(uint16_t routeTag);

  uint16_t GetRouteTag() const;

  void SetRouteMetric(uint32_t routeMetric);

  uint32_t GetRouteMetric() const;

  void SetNextHop(Ipv4Address nextHop);

  Ipv4Address GetNextHop() const;

private:
  uint16_t m_tag;
  Ipv4Address m_prefix;
  Ipv4Mask m_subnetMask;
  Ipv4Address m_nextHop;
  uint32_t m_metric;
};

std::ostream &operator<<(std::ostream &os, const RipRte &h);

class RipHeader : public Header {
public:
  RipHeader();

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

  void AddRte(RipRte rte);

  void ClearRtes();

  uint16_t GetRteNumber() const;

  std::list<RipRte> GetRteList() const;

private:
  uint8_t m_command;
  std::list<RipRte> m_rteList;
};

std::ostream &operator<<(std::ostream &os, const RipHeader &h);

} // namespace ns3

#endif
