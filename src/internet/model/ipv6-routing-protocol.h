

#ifndef IPV6_ROUTING_PROTOCOL_H
#define IPV6_ROUTING_PROTOCOL_H

#include "ipv6-header.h"
#include "ipv6-interface-address.h"
#include "ipv6.h"

#include "ns3/callback.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/packet.h"
#include "ns3/socket.h"

namespace ns3 {

class Ipv6MulticastRoute;
class Ipv6Route;
class NetDevice;

class Ipv6RoutingProtocol : public Object {
public:
  static TypeId GetTypeId();

  typedef Callback<void, Ptr<const NetDevice>, Ptr<Ipv6Route>,
                   Ptr<const Packet>, const Ipv6Header &>
      UnicastForwardCallback;

  typedef Callback<void, Ptr<const NetDevice>, Ptr<Ipv6MulticastRoute>,
                   Ptr<const Packet>, const Ipv6Header &>
      MulticastForwardCallback;

  typedef Callback<void, Ptr<const Packet>, const Ipv6Header &, uint32_t>
      LocalDeliverCallback;

  typedef Callback<void, Ptr<const Packet>, const Ipv6Header &,
                   Socket::SocketErrno>
      ErrorCallback;

  virtual Ptr<Ipv6Route> RouteOutput(Ptr<Packet> p, const Ipv6Header &header,
                                     Ptr<NetDevice> oif,
                                     Socket::SocketErrno &sockerr) = 0;

  virtual bool RouteInput(Ptr<const Packet> p, const Ipv6Header &header,
                          Ptr<const NetDevice> idev,
                          const UnicastForwardCallback &ucb,
                          const MulticastForwardCallback &mcb,
                          const LocalDeliverCallback &lcb,
                          const ErrorCallback &ecb) = 0;

  virtual void NotifyInterfaceUp(uint32_t interface) = 0;

  virtual void NotifyInterfaceDown(uint32_t interface) = 0;

  virtual void NotifyAddAddress(uint32_t interface,
                                Ipv6InterfaceAddress address) = 0;

  virtual void NotifyRemoveAddress(uint32_t interface,
                                   Ipv6InterfaceAddress address) = 0;

  virtual void
  NotifyAddRoute(Ipv6Address dst, Ipv6Prefix mask, Ipv6Address nextHop,
                 uint32_t interface,
                 Ipv6Address prefixToUse = Ipv6Address::GetZero()) = 0;

  virtual void
  NotifyRemoveRoute(Ipv6Address dst, Ipv6Prefix mask, Ipv6Address nextHop,
                    uint32_t interface,
                    Ipv6Address prefixToUse = Ipv6Address::GetZero()) = 0;

  virtual void SetIpv6(Ptr<Ipv6> ipv6) = 0;

  virtual void PrintRoutingTable(Ptr<OutputStreamWrapper> stream,
                                 Time::Unit unit = Time::S) const = 0;
};

} // namespace ns3

#endif
