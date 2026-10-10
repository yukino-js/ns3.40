
#ifndef IPV4_STATIC_ROUTING_H
#define IPV4_STATIC_ROUTING_H

#include "ipv4-header.h"
#include "ipv4-routing-protocol.h"
#include "ipv4.h"

#include "ns3/ipv4-address.h"
#include "ns3/ptr.h"
#include "ns3/socket.h"

#include <list>
#include <stdint.h>
#include <utility>

namespace ns3 {

class Packet;
class NetDevice;
class Ipv4Interface;
class Ipv4Address;
class Ipv4Header;
class Ipv4RoutingTableEntry;
class Ipv4MulticastRoutingTableEntry;
class Node;

class Ipv4StaticRouting : public Ipv4RoutingProtocol {
public:
  static TypeId GetTypeId();

  Ipv4StaticRouting();
  ~Ipv4StaticRouting() override;

  Ptr<Ipv4Route> RouteOutput(Ptr<Packet> p, const Ipv4Header &header,
                             Ptr<NetDevice> oif,
                             Socket::SocketErrno &sockerr) override;

  bool RouteInput(Ptr<const Packet> p, const Ipv4Header &header,
                  Ptr<const NetDevice> idev, const UnicastForwardCallback &ucb,
                  const MulticastForwardCallback &mcb,
                  const LocalDeliverCallback &lcb,
                  const ErrorCallback &ecb) override;

  void NotifyInterfaceUp(uint32_t interface) override;
  void NotifyInterfaceDown(uint32_t interface) override;
  void NotifyAddAddress(uint32_t interface,
                        Ipv4InterfaceAddress address) override;
  void NotifyRemoveAddress(uint32_t interface,
                           Ipv4InterfaceAddress address) override;
  void SetIpv4(Ptr<Ipv4> ipv4) override;
  void PrintRoutingTable(Ptr<OutputStreamWrapper> stream,
                         Time::Unit unit = Time::S) const override;

  void AddNetworkRouteTo(Ipv4Address network, Ipv4Mask networkMask,
                         Ipv4Address nextHop, uint32_t interface,
                         uint32_t metric = 0);

  void AddNetworkRouteTo(Ipv4Address network, Ipv4Mask networkMask,
                         uint32_t interface, uint32_t metric = 0);

  void AddHostRouteTo(Ipv4Address dest, Ipv4Address nextHop, uint32_t interface,
                      uint32_t metric = 0);
  void AddHostRouteTo(Ipv4Address dest, uint32_t interface,
                      uint32_t metric = 0);
  void SetDefaultRoute(Ipv4Address nextHop, uint32_t interface,
                       uint32_t metric = 0);

  uint32_t GetNRoutes() const;

  Ipv4RoutingTableEntry GetDefaultRoute();

  Ipv4RoutingTableEntry GetRoute(uint32_t i) const;

  uint32_t GetMetric(uint32_t index) const;

  void RemoveRoute(uint32_t i);

  void AddMulticastRoute(Ipv4Address origin, Ipv4Address group,
                         uint32_t inputInterface,
                         std::vector<uint32_t> outputInterfaces);

  void SetDefaultMulticastRoute(uint32_t outputInterface);

  uint32_t GetNMulticastRoutes() const;

  Ipv4MulticastRoutingTableEntry GetMulticastRoute(uint32_t i) const;

  bool RemoveMulticastRoute(Ipv4Address origin, Ipv4Address group,
                            uint32_t inputInterface);

  void RemoveMulticastRoute(uint32_t index);

protected:
  void DoDispose() override;

private:
  typedef std::list<std::pair<Ipv4RoutingTableEntry *, uint32_t>> NetworkRoutes;

  typedef std::list<std::pair<Ipv4RoutingTableEntry *,
                              uint32_t>>::const_iterator NetworkRoutesCI;

  typedef std::list<std::pair<Ipv4RoutingTableEntry *, uint32_t>>::iterator
      NetworkRoutesI;

  typedef std::list<Ipv4MulticastRoutingTableEntry *> MulticastRoutes;

  typedef std::list<Ipv4MulticastRoutingTableEntry *>::const_iterator
      MulticastRoutesCI;

  typedef std::list<Ipv4MulticastRoutingTableEntry *>::iterator
      MulticastRoutesI;

  bool LookupRoute(const Ipv4RoutingTableEntry &route, uint32_t metric);

  Ptr<Ipv4Route> LookupStatic(Ipv4Address dest, Ptr<NetDevice> oif = nullptr);

  Ptr<Ipv4MulticastRoute> LookupStatic(Ipv4Address origin, Ipv4Address group,
                                       uint32_t interface);

  NetworkRoutes m_networkRoutes;

  MulticastRoutes m_multicastRoutes;

  Ptr<Ipv4> m_ipv4;
};

} // namespace ns3

#endif
