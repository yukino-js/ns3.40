
#ifndef DSDV_ROUTING_PROTOCOL_H
#define DSDV_ROUTING_PROTOCOL_H

#include "dsdv-packet-queue.h"
#include "dsdv-packet.h"
#include "dsdv-rtable.h"

#include "ns3/ipv4-interface.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/node.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/random-variable-stream.h"

namespace ns3 {
namespace dsdv {

class RoutingProtocol : public Ipv4RoutingProtocol {
public:
  static TypeId GetTypeId();
  static const uint32_t DSDV_PORT;

  RoutingProtocol();

  ~RoutingProtocol() override;
  void DoDispose() override;

  Ptr<Ipv4Route> RouteOutput(Ptr<Packet> p, const Ipv4Header &header,
                             Ptr<NetDevice> oif,
                             Socket::SocketErrno &sockerr) override;
  bool RouteInput(Ptr<const Packet> p, const Ipv4Header &header,
                  Ptr<const NetDevice> idev, const UnicastForwardCallback &ucb,
                  const MulticastForwardCallback &mcb,
                  const LocalDeliverCallback &lcb,
                  const ErrorCallback &ecb) override;
  void PrintRoutingTable(Ptr<OutputStreamWrapper> stream,
                         Time::Unit unit = Time::S) const override;
  void NotifyInterfaceUp(uint32_t interface) override;
  void NotifyInterfaceDown(uint32_t interface) override;
  void NotifyAddAddress(uint32_t interface,
                        Ipv4InterfaceAddress address) override;
  void NotifyRemoveAddress(uint32_t interface,
                           Ipv4InterfaceAddress address) override;
  void SetIpv4(Ptr<Ipv4> ipv4) override;

  void SetEnableBufferFlag(bool f);
  bool GetEnableBufferFlag() const;
  void SetWSTFlag(bool f);
  bool GetWSTFlag() const;
  void SetEnableRAFlag(bool f);
  bool GetEnableRAFlag() const;

  int64_t AssignStreams(int64_t stream);

private:
  uint32_t Holdtimes;
  Time m_periodicUpdateInterval;
  Time m_settlingTime;
  Ipv4Address m_mainAddress;
  Ptr<Ipv4> m_ipv4;
  std::map<Ptr<Socket>, Ipv4InterfaceAddress> m_socketAddresses;
  Ptr<NetDevice> m_lo;
  RoutingTable m_routingTable;
  RoutingTable m_advRoutingTable;
  uint32_t m_maxQueueLen;
  uint32_t m_maxQueuedPacketsPerDst;
  Time m_maxQueueTime;
  PacketQueue m_queue;
  bool EnableBuffering;
  bool EnableWST;
  double m_weightedFactor;
  bool EnableRouteAggregation;
  Time m_routeAggregationTime;
  UnicastForwardCallback m_scb;
  ErrorCallback m_ecb;

private:
  void Start();
  void DeferredRouteOutput(Ptr<const Packet> p, const Ipv4Header &header,
                           UnicastForwardCallback ucb, ErrorCallback ecb);
  void LookForQueuedPackets();
  void SendPacketFromQueue(Ipv4Address dst, Ptr<Ipv4Route> route);
  Ptr<Socket> FindSocketWithInterfaceAddress(Ipv4InterfaceAddress iface) const;

  void RecvDsdv(Ptr<Socket> socket);
  void Send(Ptr<Ipv4Route> route, Ptr<const Packet> packet,
            const Ipv4Header &header);

  Ptr<Ipv4Route> LoopbackRoute(const Ipv4Header &header,
                               Ptr<NetDevice> oif) const;
  Time GetSettlingTime(Ipv4Address dst);
  void SendTriggeredUpdate();
  void SendPeriodicUpdate();
  void MergeTriggerPeriodicUpdates();
  void Drop(Ptr<const Packet> packet, const Ipv4Header &header,
            Socket::SocketErrno err);
  Timer m_periodicUpdateTimer;
  Timer m_triggeredExpireTimer;

  Ptr<UniformRandomVariable> m_uniformRandomVariable;
};

} // namespace dsdv
} // namespace ns3

#endif
