
#ifndef IPV4_GLOBAL_ROUTING_H
#define IPV4_GLOBAL_ROUTING_H

#include "ipv4-header.h"
#include "ipv4-routing-protocol.h"
#include "ipv4.h"

#include "ns3/ipv4-address.h"
#include "ns3/ptr.h"
#include "ns3/random-variable-stream.h"

#include <list>
#include <stdint.h>

namespace ns3 {

class Packet;
class NetDevice;
class Ipv4Interface;
class Ipv4Address;
class Ipv4Header;
class Ipv4RoutingTableEntry;
class Ipv4MulticastRoutingTableEntry;
class Node;

class Ipv4GlobalRouting : public Ipv4RoutingProtocol {
public:
  static TypeId GetTypeId();
  Ipv4GlobalRouting();
  ~Ipv4GlobalRouting() override;

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

  void AddHostRouteTo(Ipv4Address dest, Ipv4Address nextHop,
                      uint32_t interface);
  void AddHostRouteTo(Ipv4Address dest, uint32_t interface);

  void AddNetworkRouteTo(Ipv4Address network, Ipv4Mask networkMask,
                         Ipv4Address nextHop, uint32_t interface);

  void AddNetworkRouteTo(Ipv4Address network, Ipv4Mask networkMask,
                         uint32_t interface);

  void AddASExternalRouteTo(Ipv4Address network, Ipv4Mask networkMask,
                            Ipv4Address nextHop, uint32_t interface);

  uint32_t GetNRoutes() const;

  Ipv4RoutingTableEntry *GetRoute(uint32_t i) const;

  void RemoveRoute(uint32_t i);

  int64_t AssignStreams(int64_t stream);

protected:
  void DoDispose() override;

private:
  bool m_randomEcmpRouting;
  bool m_flowEcmpRouting;
  bool m_respondToInterfaceEvents;
  Ptr<UniformRandomVariable> m_rand;

  typedef std::list<Ipv4RoutingTableEntry *> HostRoutes;
  typedef std::list<Ipv4RoutingTableEntry *>::const_iterator HostRoutesCI;
  typedef std::list<Ipv4RoutingTableEntry *>::iterator HostRoutesI;

  typedef std::list<Ipv4RoutingTableEntry *> NetworkRoutes;
  typedef std::list<Ipv4RoutingTableEntry *>::const_iterator NetworkRoutesCI;
  typedef std::list<Ipv4RoutingTableEntry *>::iterator NetworkRoutesI;

  typedef std::list<Ipv4RoutingTableEntry *> ASExternalRoutes;
  typedef std::list<Ipv4RoutingTableEntry *>::const_iterator ASExternalRoutesCI;
  typedef std::list<Ipv4RoutingTableEntry *>::iterator ASExternalRoutesI;

  Ptr<Ipv4Route> LookupGlobal(Ipv4Address dest, uint32_t flowHash = 0,
                              Ptr<NetDevice> oif = nullptr);

  HostRoutes m_hostRoutes;
  NetworkRoutes m_networkRoutes;
  ASExternalRoutes m_ASexternalRoutes;

  Ptr<Ipv4> m_ipv4;
};

} // namespace ns3

#endif
