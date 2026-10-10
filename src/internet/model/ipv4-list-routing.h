
#ifndef IPV4_LIST_ROUTING_H
#define IPV4_LIST_ROUTING_H

#include "ipv4-routing-protocol.h"

#include "ns3/nstime.h"
#include "ns3/simulator.h"

#include <list>

namespace ns3 {

class Ipv4ListRouting : public Ipv4RoutingProtocol {
public:
  static TypeId GetTypeId();

  Ipv4ListRouting();
  ~Ipv4ListRouting() override;

  virtual void AddRoutingProtocol(Ptr<Ipv4RoutingProtocol> routingProtocol,
                                  int16_t priority);
  virtual uint32_t GetNRoutingProtocols() const;
  virtual Ptr<Ipv4RoutingProtocol> GetRoutingProtocol(uint32_t index,
                                                      int16_t &priority) const;

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

protected:
  void DoDispose() override;
  void DoInitialize() override;

private:
  typedef std::pair<int16_t, Ptr<Ipv4RoutingProtocol>> Ipv4RoutingProtocolEntry;
  typedef std::list<Ipv4RoutingProtocolEntry> Ipv4RoutingProtocolList;
  Ipv4RoutingProtocolList m_routingProtocols;

  static bool Compare(const Ipv4RoutingProtocolEntry &a,
                      const Ipv4RoutingProtocolEntry &b);
  Ptr<Ipv4> m_ipv4;
};

} // namespace ns3

#endif
