
#ifndef IPV6_ROUTING_TABLE_ENTRY_H
#define IPV6_ROUTING_TABLE_ENTRY_H

#include "ns3/ipv6-address.h"

#include <list>
#include <ostream>
#include <vector>

namespace ns3 {

class Ipv6RoutingTableEntry {
public:
  Ipv6RoutingTableEntry();

  Ipv6RoutingTableEntry(const Ipv6RoutingTableEntry &route);

  Ipv6RoutingTableEntry(const Ipv6RoutingTableEntry *route);

  virtual ~Ipv6RoutingTableEntry();

  bool IsHost() const;

  Ipv6Address GetDest() const;

  Ipv6Address GetPrefixToUse() const;

  void SetPrefixToUse(Ipv6Address prefix);

  bool IsNetwork() const;

  Ipv6Address GetDestNetwork() const;

  Ipv6Prefix GetDestNetworkPrefix() const;

  bool IsDefault() const;

  bool IsGateway() const;

  Ipv6Address GetGateway() const;

  uint32_t GetInterface() const;

  static Ipv6RoutingTableEntry
  CreateHostRouteTo(Ipv6Address dest, Ipv6Address nextHop, uint32_t interface,
                    Ipv6Address prefixToUse = Ipv6Address());

  static Ipv6RoutingTableEntry CreateHostRouteTo(Ipv6Address dest,
                                                 uint32_t interface);

  static Ipv6RoutingTableEntry CreateNetworkRouteTo(Ipv6Address network,
                                                    Ipv6Prefix networkPrefix,
                                                    Ipv6Address nextHop,
                                                    uint32_t interface);

  static Ipv6RoutingTableEntry CreateNetworkRouteTo(Ipv6Address network,
                                                    Ipv6Prefix networkPrefix,
                                                    Ipv6Address nextHop,
                                                    uint32_t interface,
                                                    Ipv6Address prefixToUse);

  static Ipv6RoutingTableEntry CreateNetworkRouteTo(Ipv6Address network,
                                                    Ipv6Prefix networkPrefix,
                                                    uint32_t interface);

  static Ipv6RoutingTableEntry CreateDefaultRoute(Ipv6Address nextHop,
                                                  uint32_t interface);

private:
  Ipv6RoutingTableEntry(Ipv6Address network, Ipv6Prefix prefix,
                        Ipv6Address gateway, uint32_t interface);

  Ipv6RoutingTableEntry(Ipv6Address network, Ipv6Prefix prefix,
                        uint32_t interface, Ipv6Address prefixToUse);

  Ipv6RoutingTableEntry(Ipv6Address network, Ipv6Prefix prefix,
                        Ipv6Address gateway, uint32_t interface,
                        Ipv6Address prefixToUse);

  Ipv6RoutingTableEntry(Ipv6Address dest, Ipv6Prefix prefix,
                        uint32_t interface);

  Ipv6RoutingTableEntry(Ipv6Address dest, Ipv6Address gateway,
                        uint32_t interface);

  Ipv6RoutingTableEntry(Ipv6Address dest, uint32_t interface);

  Ipv6Address m_dest;

  Ipv6Prefix m_destNetworkPrefix;

  Ipv6Address m_gateway;

  uint32_t m_interface;

  Ipv6Address m_prefixToUse;
};

std::ostream &operator<<(std::ostream &os, const Ipv6RoutingTableEntry &route);

class Ipv6MulticastRoutingTableEntry {
public:
  Ipv6MulticastRoutingTableEntry();

  Ipv6MulticastRoutingTableEntry(const Ipv6MulticastRoutingTableEntry &route);

  Ipv6MulticastRoutingTableEntry(const Ipv6MulticastRoutingTableEntry *route);

  Ipv6Address GetOrigin() const;

  Ipv6Address GetGroup() const;

  uint32_t GetInputInterface() const;

  uint32_t GetNOutputInterfaces() const;

  uint32_t GetOutputInterface(uint32_t n) const;

  std::vector<uint32_t> GetOutputInterfaces() const;

  static Ipv6MulticastRoutingTableEntry
  CreateMulticastRoute(Ipv6Address origin, Ipv6Address group,
                       uint32_t inputInterface,
                       std::vector<uint32_t> outputInterfaces);

private:
  Ipv6MulticastRoutingTableEntry(Ipv6Address origin, Ipv6Address group,
                                 uint32_t inputInterface,
                                 std::vector<uint32_t> outputInterfaces);

  Ipv6Address m_origin;

  Ipv6Address m_group;

  uint32_t m_inputInterface;

  std::vector<uint32_t> m_outputInterfaces;
};

std::ostream &operator<<(std::ostream &os,
                         const Ipv6MulticastRoutingTableEntry &route);

} // namespace ns3

#endif
