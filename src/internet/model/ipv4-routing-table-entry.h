#ifndef IPV4_ROUTING_TABLE_ENTRY_H
#define IPV4_ROUTING_TABLE_ENTRY_H

#include "ns3/ipv4-address.h"

#include <list>
#include <ostream>
#include <vector>

namespace ns3 {

class Ipv4RoutingTableEntry {
public:
  Ipv4RoutingTableEntry();
  Ipv4RoutingTableEntry(const Ipv4RoutingTableEntry &route);
  Ipv4RoutingTableEntry(const Ipv4RoutingTableEntry *route);
  bool IsHost() const;
  bool IsNetwork() const;
  bool IsDefault() const;
  bool IsGateway() const;
  Ipv4Address GetGateway() const;
  Ipv4Address GetDest() const;
  Ipv4Address GetDestNetwork() const;
  Ipv4Mask GetDestNetworkMask() const;
  uint32_t GetInterface() const;
  static Ipv4RoutingTableEntry
  CreateHostRouteTo(Ipv4Address dest, Ipv4Address nextHop, uint32_t interface);
  static Ipv4RoutingTableEntry CreateHostRouteTo(Ipv4Address dest,
                                                 uint32_t interface);
  static Ipv4RoutingTableEntry CreateNetworkRouteTo(Ipv4Address network,
                                                    Ipv4Mask networkMask,
                                                    Ipv4Address nextHop,
                                                    uint32_t interface);
  static Ipv4RoutingTableEntry CreateNetworkRouteTo(Ipv4Address network,
                                                    Ipv4Mask networkMask,
                                                    uint32_t interface);
  static Ipv4RoutingTableEntry CreateDefaultRoute(Ipv4Address nextHop,
                                                  uint32_t interface);

private:
  Ipv4RoutingTableEntry(Ipv4Address network, Ipv4Mask mask, Ipv4Address gateway,
                        uint32_t interface);
  Ipv4RoutingTableEntry(Ipv4Address dest, Ipv4Mask mask, uint32_t interface);
  Ipv4RoutingTableEntry(Ipv4Address dest, Ipv4Address gateway,
                        uint32_t interface);
  Ipv4RoutingTableEntry(Ipv4Address dest, uint32_t interface);

  Ipv4Address m_dest;
  Ipv4Mask m_destNetworkMask;
  Ipv4Address m_gateway;
  uint32_t m_interface;
};

std::ostream &operator<<(std::ostream &os, const Ipv4RoutingTableEntry &route);

bool operator==(const Ipv4RoutingTableEntry a, const Ipv4RoutingTableEntry b);

class Ipv4MulticastRoutingTableEntry {
public:
  Ipv4MulticastRoutingTableEntry();

  Ipv4MulticastRoutingTableEntry(const Ipv4MulticastRoutingTableEntry &route);
  Ipv4MulticastRoutingTableEntry(const Ipv4MulticastRoutingTableEntry *route);
  Ipv4Address GetOrigin() const;
  Ipv4Address GetGroup() const;
  uint32_t GetInputInterface() const;
  uint32_t GetNOutputInterfaces() const;
  uint32_t GetOutputInterface(uint32_t n) const;
  std::vector<uint32_t> GetOutputInterfaces() const;
  static Ipv4MulticastRoutingTableEntry
  CreateMulticastRoute(Ipv4Address origin, Ipv4Address group,
                       uint32_t inputInterface,
                       std::vector<uint32_t> outputInterfaces);

private:
  Ipv4MulticastRoutingTableEntry(Ipv4Address origin, Ipv4Address group,
                                 uint32_t inputInterface,
                                 std::vector<uint32_t> outputInterfaces);

  Ipv4Address m_origin;
  Ipv4Address m_group;
  uint32_t m_inputInterface;
  std::vector<uint32_t> m_outputInterfaces;
};

std::ostream &operator<<(std::ostream &os,
                         const Ipv4MulticastRoutingTableEntry &route);

bool operator==(const Ipv4MulticastRoutingTableEntry a,
                const Ipv4MulticastRoutingTableEntry b);

} // namespace ns3

#endif
