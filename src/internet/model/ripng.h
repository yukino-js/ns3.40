
#ifndef RIPNG_H
#define RIPNG_H

#include "ipv6-interface.h"
#include "ipv6-l3-protocol.h"
#include "ipv6-routing-protocol.h"
#include "ipv6-routing-table-entry.h"
#include "ripng-header.h"

#include "ns3/inet6-socket-address.h"
#include "ns3/random-variable-stream.h"

#include <list>

namespace ns3 {

class RipNgRoutingTableEntry : public Ipv6RoutingTableEntry {
public:
  enum Status_e {
    RIPNG_VALID,
    RIPNG_INVALID,
  };

  RipNgRoutingTableEntry();

  RipNgRoutingTableEntry(Ipv6Address network, Ipv6Prefix networkPrefix,
                         Ipv6Address nextHop, uint32_t interface,
                         Ipv6Address prefixToUse);

  RipNgRoutingTableEntry(Ipv6Address network, Ipv6Prefix networkPrefix,
                         uint32_t interface);

  ~RipNgRoutingTableEntry() override;

  void SetRouteTag(uint16_t routeTag);

  uint16_t GetRouteTag() const;

  void SetRouteMetric(uint8_t routeMetric);

  uint8_t GetRouteMetric() const;

  void SetRouteStatus(Status_e status);

  Status_e GetRouteStatus() const;

  void SetRouteChanged(bool changed);

  bool IsRouteChanged() const;

private:
  uint16_t m_tag;
  uint8_t m_metric;
  Status_e m_status;
  bool m_changed;
};

std::ostream &operator<<(std::ostream &os, const RipNgRoutingTableEntry &route);

class RipNg : public Ipv6RoutingProtocol {
public:
  RipNg();
  ~RipNg() override;

  static TypeId GetTypeId();

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

  enum SplitHorizonType_e {
    NO_SPLIT_HORIZON,
    SPLIT_HORIZON,
    POISON_REVERSE,
  };

  int64_t AssignStreams(int64_t stream);

  std::set<uint32_t> GetInterfaceExclusions() const;

  void SetInterfaceExclusions(std::set<uint32_t> exceptions);

  uint8_t GetInterfaceMetric(uint32_t interface) const;

  void SetInterfaceMetric(uint32_t interface, uint8_t metric);

  void AddDefaultRouteTo(Ipv6Address nextHop, uint32_t interface);

protected:
  void DoDispose() override;

  void DoInitialize() override;

private:
  typedef std::list<std::pair<RipNgRoutingTableEntry *, EventId>> Routes;

  typedef std::list<
      std::pair<RipNgRoutingTableEntry *, EventId>>::const_iterator RoutesCI;

  typedef std::list<std::pair<RipNgRoutingTableEntry *, EventId>>::iterator
      RoutesI;

  void Receive(Ptr<Socket> socket);

  void HandleRequests(RipNgHeader hdr, Ipv6Address senderAddress,
                      uint16_t senderPort, uint32_t incomingInterface,
                      uint8_t hopLimit);

  void HandleResponses(RipNgHeader hdr, Ipv6Address senderAddress,
                       uint32_t incomingInterface, uint8_t hopLimit);

  Ptr<Ipv6Route> Lookup(Ipv6Address dest, bool setSource,
                        Ptr<NetDevice> = nullptr);

  void RecvUnicastRipng(Ptr<Socket> socket);
  void RecvMulticastRipng(Ptr<Socket> socket);

  void AddNetworkRouteTo(Ipv6Address network, Ipv6Prefix networkPrefix,
                         Ipv6Address nextHop, uint32_t interface,
                         Ipv6Address prefixToUse);

  void AddNetworkRouteTo(Ipv6Address network, Ipv6Prefix networkPrefix,
                         uint32_t interface);

  void DoSendRouteUpdate(bool periodic);

  void SendRouteRequest();

  void SendTriggeredRouteUpdate();

  void SendUnsolicitedRouteUpdate();

  void InvalidateRoute(RipNgRoutingTableEntry *route);

  void DeleteRoute(RipNgRoutingTableEntry *route);

  Routes m_routes;
  Ptr<Ipv6> m_ipv6;
  Time m_startupDelay;
  Time m_minTriggeredUpdateDelay;
  Time m_maxTriggeredUpdateDelay;
  Time m_unsolicitedUpdate;
  Time m_timeoutDelay;
  Time m_garbageCollectionDelay;

  typedef std::map<Ptr<Socket>, uint32_t> SocketList;
  typedef std::map<Ptr<Socket>, uint32_t>::iterator SocketListI;
  typedef std::map<Ptr<Socket>, uint32_t>::const_iterator SocketListCI;

  SocketList m_unicastSocketList;
  Ptr<Socket> m_multicastRecvSocket;

  EventId m_nextUnsolicitedUpdate;
  EventId m_nextTriggeredUpdate;

  Ptr<UniformRandomVariable> m_rng;

  std::set<uint32_t> m_interfaceExclusions;
  std::map<uint32_t, uint8_t> m_interfaceMetrics;

  SplitHorizonType_e m_splitHorizonStrategy;

  bool m_initialized;
  uint8_t m_linkDown;
};

} // namespace ns3
#endif
