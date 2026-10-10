
#ifndef RIP_H
#define RIP_H

#include "ipv4-interface.h"
#include "ipv4-l3-protocol.h"
#include "ipv4-routing-protocol.h"
#include "ipv4-routing-table-entry.h"
#include "rip-header.h"

#include "ns3/inet-socket-address.h"
#include "ns3/random-variable-stream.h"

#include <list>

namespace ns3 {

class RipRoutingTableEntry : public Ipv4RoutingTableEntry {
public:
  enum Status_e {
    RIP_VALID,
    RIP_INVALID,
  };

  RipRoutingTableEntry();

  RipRoutingTableEntry(Ipv4Address network, Ipv4Mask networkPrefix,
                       Ipv4Address nextHop, uint32_t interface);

  RipRoutingTableEntry(Ipv4Address network, Ipv4Mask networkPrefix,
                       uint32_t interface);

  virtual ~RipRoutingTableEntry();

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

std::ostream &operator<<(std::ostream &os, const RipRoutingTableEntry &route);

class Rip : public Ipv4RoutingProtocol {
public:
  Rip();
  ~Rip() override;

  static TypeId GetTypeId();

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

  void AddDefaultRouteTo(Ipv4Address nextHop, uint32_t interface);

protected:
  void DoDispose() override;

  void DoInitialize() override;

private:
  typedef std::list<std::pair<RipRoutingTableEntry *, EventId>> Routes;

  typedef std::list<std::pair<RipRoutingTableEntry *, EventId>>::const_iterator
      RoutesCI;

  typedef std::list<std::pair<RipRoutingTableEntry *, EventId>>::iterator
      RoutesI;

  void Receive(Ptr<Socket> socket);

  void HandleRequests(RipHeader hdr, Ipv4Address senderAddress,
                      uint16_t senderPort, uint32_t incomingInterface,
                      uint8_t hopLimit);

  void HandleResponses(RipHeader hdr, Ipv4Address senderAddress,
                       uint32_t incomingInterface, uint8_t hopLimit);

  Ptr<Ipv4Route> Lookup(Ipv4Address dest, bool setSource,
                        Ptr<NetDevice> = nullptr);

  void RecvUnicastRip(Ptr<Socket> socket);
  void RecvMulticastRip(Ptr<Socket> socket);

  void AddNetworkRouteTo(Ipv4Address network, Ipv4Mask networkPrefix,
                         Ipv4Address nextHop, uint32_t interface);

  void AddNetworkRouteTo(Ipv4Address network, Ipv4Mask networkPrefix,
                         uint32_t interface);

  void DoSendRouteUpdate(bool periodic);

  void SendRouteRequest();

  void SendTriggeredRouteUpdate();

  void SendUnsolicitedRouteUpdate();

  void InvalidateRoute(RipRoutingTableEntry *route);

  void DeleteRoute(RipRoutingTableEntry *route);

  Routes m_routes;
  Ptr<Ipv4> m_ipv4;
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
  uint32_t m_linkDown;
};

} // namespace ns3
#endif
