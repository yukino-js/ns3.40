
#ifndef IPV6_LIST_ROUTING_H
#define IPV6_LIST_ROUTING_H

#include "ipv6-routing-protocol.h"

#include <list>

namespace ns3 {

class Ipv6ListRouting : public Ipv6RoutingProtocol {
public:
  static TypeId GetTypeId();

  Ipv6ListRouting();

  ~Ipv6ListRouting() override;

  virtual void AddRoutingProtocol(Ptr<Ipv6RoutingProtocol> routingProtocol,
                                  int16_t priority);

  virtual uint32_t GetNRoutingProtocols() const;

  virtual Ptr<Ipv6RoutingProtocol> GetRoutingProtocol(uint32_t index,
                                                      int16_t &priority) const;

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
  typedef std::pair<int16_t, Ptr<Ipv6RoutingProtocol>> Ipv6RoutingProtocolEntry;

  typedef std::list<Ipv6RoutingProtocolEntry> Ipv6RoutingProtocolList;

  static bool Compare(const Ipv6RoutingProtocolEntry &a,
                      const Ipv6RoutingProtocolEntry &b);

  Ipv6RoutingProtocolList m_routingProtocols;
  Ptr<Ipv6> m_ipv6;
};

} // namespace ns3

#endif
