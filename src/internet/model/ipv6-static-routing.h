
#ifndef IPV6_STATIC_ROUTING_H
#define IPV6_STATIC_ROUTING_H

#include "ipv6-header.h"
#include "ipv6-routing-protocol.h"
#include "ipv6.h"

#include "ns3/ipv6-address.h"
#include "ns3/ptr.h"

#include <list>
#include <stdint.h>

namespace ns3 {

class Packet;
class NetDevice;
class Ipv6Interface;
class Ipv6Route;
class Node;
class Ipv6RoutingTableEntry;
class Ipv6MulticastRoutingTableEntry;

class Ipv6StaticRouting : public Ipv6RoutingProtocol {
public:
  static TypeId GetTypeId();

  Ipv6StaticRouting();
  ~Ipv6StaticRouting() override;

  void AddHostRouteTo(Ipv6Address dest, Ipv6Address nextHop, uint32_t interface,
                      Ipv6Address prefixToUse = Ipv6Address("::"),
                      uint32_t metric = 0);

  void AddHostRouteTo(Ipv6Address dest, uint32_t interface,
                      uint32_t metric = 0);

  void AddNetworkRouteTo(Ipv6Address network, Ipv6Prefix networkPrefix,
                         Ipv6Address nextHop, uint32_t interface,
                         uint32_t metric = 0);

  void AddNetworkRouteTo(Ipv6Address network, Ipv6Prefix networkPrefix,
                         Ipv6Address nextHop, uint32_t interface,
                         Ipv6Address prefixToUse, uint32_t metric = 0);

  void AddNetworkRouteTo(Ipv6Address network, Ipv6Prefix networkPrefix,
                         uint32_t interface, uint32_t metric = 0);

  void SetDefaultRoute(Ipv6Address nextHop, uint32_t interface,
                       Ipv6Address prefixToUse = Ipv6Address("::"),
                       uint32_t metric = 0);

  uint32_t GetNRoutes() const;

  Ipv6RoutingTableEntry GetDefaultRoute();

  Ipv6RoutingTableEntry GetRoute(uint32_t i) const;

  uint32_t GetMetric(uint32_t index) const;

  void RemoveRoute(uint32_t i);

  void RemoveRoute(Ipv6Address network, Ipv6Prefix prefix, uint32_t ifIndex,
                   Ipv6Address prefixToUse);

  void AddMulticastRoute(Ipv6Address origin, Ipv6Address group,
                         uint32_t inputInterface,
                         std::vector<uint32_t> outputInterfaces);

  void SetDefaultMulticastRoute(uint32_t outputInterface);

  uint32_t GetNMulticastRoutes() const;

  Ipv6MulticastRoutingTableEntry GetMulticastRoute(uint32_t i) const;

  bool RemoveMulticastRoute(Ipv6Address origin, Ipv6Address group,
                            uint32_t inputInterface);

  void RemoveMulticastRoute(uint32_t i);

  bool HasNetworkDest(Ipv6Address dest, uint32_t interfaceIndex);

  Ptr<Ipv6Route> RouteOutput(Ptr<Packet> p, const Ipv6Header &header,
                             Ptr<NetDevice> oif,
                             Socket::SocketErrno &sockerr) override;

  bool RouteInput(Ptr<const Packet> p, const Ipv6Header &header,
                  Ptr<const NetDevice> idev, const UnicastForwardCallback &ucb,
                  const MulticastForwardCallback &mcb,
                  const LocalDeliverCallback &lcb,
                  const ErrorCallback &ecb) override;

  void NotifyInterfaceUp(uint32_t interface) override;
  void NotifyInterfaceDown(uint32_t interface) override;
  void NotifyAddAddress(uint32_t interface,
                        Ipv6InterfaceAddress address) override;
  void NotifyRemoveAddress(uint32_t interface,
                           Ipv6InterfaceAddress address) override;
  void
  NotifyAddRoute(Ipv6Address dst, Ipv6Prefix mask, Ipv6Address nextHop,
                 uint32_t interface,
                 Ipv6Address prefixToUse = Ipv6Address::GetZero()) override;
  void
  NotifyRemoveRoute(Ipv6Address dst, Ipv6Prefix mask, Ipv6Address nextHop,
                    uint32_t interface,
                    Ipv6Address prefixToUse = Ipv6Address::GetZero()) override;
  void SetIpv6(Ptr<Ipv6> ipv6) override;
  void PrintRoutingTable(Ptr<OutputStreamWrapper> stream,
                         Time::Unit unit = Time::S) const override;

protected:
  void DoDispose() override;

private:
  typedef std::list<std::pair<Ipv6RoutingTableEntry *, uint32_t>> NetworkRoutes;

  typedef std::list<std::pair<Ipv6RoutingTableEntry *,
                              uint32_t>>::const_iterator NetworkRoutesCI;

  typedef std::list<std::pair<Ipv6RoutingTableEntry *, uint32_t>>::iterator
      NetworkRoutesI;

  typedef std::list<Ipv6MulticastRoutingTableEntry *> MulticastRoutes;

  typedef std::list<Ipv6MulticastRoutingTableEntry *>::const_iterator
      MulticastRoutesCI;

  typedef std::list<Ipv6MulticastRoutingTableEntry *>::iterator
      MulticastRoutesI;

  bool LookupRoute(const Ipv6RoutingTableEntry &route, uint32_t metric);

  Ptr<Ipv6Route> LookupStatic(Ipv6Address dest, Ptr<NetDevice> = nullptr);

  Ptr<Ipv6MulticastRoute> LookupStatic(Ipv6Address origin, Ipv6Address group,
                                       uint32_t ifIndex);

  NetworkRoutes m_networkRoutes;

  MulticastRoutes m_multicastRoutes;

  Ptr<Ipv6> m_ipv6;
};

} // namespace ns3

#endif
