#ifndef IPV4_ROUTING_PROTOCOL_H
#define IPV4_ROUTING_PROTOCOL_H

#include "ipv4-header.h"
#include "ipv4-interface-address.h"
#include "ipv4.h"

#include "ns3/callback.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/packet.h"
#include "ns3/socket.h"

namespace ns3 {

class Ipv4MulticastRoute;
class Ipv4Route;
class NetDevice;

class Ipv4RoutingProtocol : public Object {
public:
  static TypeId GetTypeId();

  typedef Callback<void, Ptr<Ipv4Route>, Ptr<const Packet>, const Ipv4Header &>
      UnicastForwardCallback;

  typedef Callback<void, Ptr<Ipv4MulticastRoute>, Ptr<const Packet>,
                   const Ipv4Header &>
      MulticastForwardCallback;

  typedef Callback<void, Ptr<const Packet>, const Ipv4Header &, uint32_t>
      LocalDeliverCallback;

  typedef Callback<void, Ptr<const Packet>, const Ipv4Header &,
                   Socket::SocketErrno>
      ErrorCallback;

  virtual Ptr<Ipv4Route> RouteOutput(Ptr<Packet> p, const Ipv4Header &header,
                                     Ptr<NetDevice> oif,
                                     Socket::SocketErrno &sockerr) = 0;

  virtual bool RouteInput(Ptr<const Packet> p, const Ipv4Header &header,
                          Ptr<const NetDevice> idev,
                          const UnicastForwardCallback &ucb,
                          const MulticastForwardCallback &mcb,
                          const LocalDeliverCallback &lcb,
                          const ErrorCallback &ecb) = 0;

  virtual void NotifyInterfaceUp(uint32_t interface) = 0;
  virtual void NotifyInterfaceDown(uint32_t interface) = 0;

  virtual void NotifyAddAddress(uint32_t interface,
                                Ipv4InterfaceAddress address) = 0;

  virtual void NotifyRemoveAddress(uint32_t interface,
                                   Ipv4InterfaceAddress address) = 0;

  virtual void SetIpv4(Ptr<Ipv4> ipv4) = 0;

  virtual void PrintRoutingTable(Ptr<OutputStreamWrapper> stream,
                                 Time::Unit unit = Time::S) const = 0;
};

} // namespace ns3

#endif
