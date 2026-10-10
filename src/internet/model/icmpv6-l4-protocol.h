
#ifndef ICMPV6_L4_PROTOCOL_H
#define ICMPV6_L4_PROTOCOL_H

#include "icmpv6-header.h"
#include "ip-l4-protocol.h"
#include "ndisc-cache.h"

#include "ns3/ipv6-address.h"
#include "ns3/random-variable-stream.h"

#include <list>

namespace ns3 {

class NetDevice;
class Node;
class Packet;
class TraceContext;

class Icmpv6L4Protocol : public IpL4Protocol {
public:
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;

  static const uint8_t PROT_NUMBER;

  uint8_t GetMaxMulticastSolicit() const;

  uint8_t GetMaxUnicastSolicit() const;

  Time GetReachableTime() const;

  Time GetRetransmissionTime() const;

  Time GetDelayFirstProbe() const;

  static uint16_t GetStaticProtocolNumber();

  Icmpv6L4Protocol();

  ~Icmpv6L4Protocol() override;

  void SetNode(Ptr<Node> node);

  Ptr<Node> GetNode();

  void NotifyNewAggregate() override;

  int GetProtocolNumber() const override;

  virtual int GetVersion() const;

  void SendMessage(Ptr<Packet> packet, Ipv6Address src, Ipv6Address dst,
                   uint8_t ttl);

  void DelayedSendMessage(Ptr<Packet> packet, Ipv6Address src, Ipv6Address dst,
                          uint8_t ttl);

  void SendMessage(Ptr<Packet> packet, Ipv6Address dst, Icmpv6Header &icmpv6Hdr,
                   uint8_t ttl);

  void DoDAD(Ipv6Address target, Ptr<Ipv6Interface> interface);

  void SendNA(Ipv6Address src, Ipv6Address dst, Address *hardwareAddress,
              uint8_t flags);

  void SendEchoReply(Ipv6Address src, Ipv6Address dst, uint16_t id,
                     uint16_t seq, Ptr<Packet> data);

  virtual void SendNS(Ipv6Address src, Ipv6Address dst, Ipv6Address target,
                      Address hardwareAddress);

  void SendErrorDestinationUnreachable(Ptr<Packet> malformedPacket,
                                       Ipv6Address dst, uint8_t code);

  void SendErrorTooBig(Ptr<Packet> malformedPacket, Ipv6Address dst,
                       uint32_t mtu);

  void SendErrorTimeExceeded(Ptr<Packet> malformedPacket, Ipv6Address dst,
                             uint8_t code);

  void SendErrorParameterError(Ptr<Packet> malformedPacket, Ipv6Address dst,
                               uint8_t code, uint32_t ptr);

  void SendRedirection(Ptr<Packet> redirectedPacket, Ipv6Address src,
                       Ipv6Address dst, Ipv6Address redirTarget,
                       Ipv6Address redirDestination,
                       Address redirHardwareTarget);

  NdiscCache::Ipv6PayloadHeaderPair ForgeNS(Ipv6Address src, Ipv6Address dst,
                                            Ipv6Address target,
                                            Address hardwareAddress);

  NdiscCache::Ipv6PayloadHeaderPair ForgeNA(Ipv6Address src, Ipv6Address dst,
                                            Address *hardwareAddress,
                                            uint8_t flags);

  NdiscCache::Ipv6PayloadHeaderPair ForgeRS(Ipv6Address src, Ipv6Address dst,
                                            Address hardwareAddress);

  NdiscCache::Ipv6PayloadHeaderPair ForgeEchoRequest(Ipv6Address src,
                                                     Ipv6Address dst,
                                                     uint16_t id, uint16_t seq,
                                                     Ptr<Packet> data);

  IpL4Protocol::RxStatus Receive(Ptr<Packet> p, const Ipv4Header &header,
                                 Ptr<Ipv4Interface> interface) override;

  IpL4Protocol::RxStatus Receive(Ptr<Packet> p, const Ipv6Header &header,
                                 Ptr<Ipv6Interface> interface) override;

  virtual void FunctionDadTimeout(Ipv6Interface *interface, Ipv6Address addr);

  virtual bool Lookup(Ipv6Address dst, Ptr<NetDevice> device,
                      Ptr<NdiscCache> cache, Address *hardwareDestination);

  virtual bool Lookup(Ptr<Packet> p, const Ipv6Header &ipHeader,
                      Ipv6Address dst, Ptr<NetDevice> device,
                      Ptr<NdiscCache> cache, Address *hardwareDestination);

  void SendRS(Ipv6Address src, Ipv6Address dst, Address hardwareAddress);

  virtual Ptr<NdiscCache> CreateCache(Ptr<NetDevice> device,
                                      Ptr<Ipv6Interface> interface);

  bool IsAlwaysDad() const;

  int64_t AssignStreams(int64_t stream);

  Time GetDadTimeout() const;

protected:
  void DoDispose() override;

  typedef std::list<Ptr<NdiscCache>> CacheList;

  void Forward(Ipv6Address source, Icmpv6Header icmp, uint32_t info,
               Ipv6Header ipHeader, const uint8_t payload[8]);

  void HandleNS(Ptr<Packet> p, const Ipv6Address &src, const Ipv6Address &dst,
                Ptr<Ipv6Interface> interface);

  void HandleRS(Ptr<Packet> p, const Ipv6Address &src, const Ipv6Address &dst,
                Ptr<Ipv6Interface> interface);

  virtual void HandleRsTimeout(Ipv6Address src, Ipv6Address dst,
                               Address hardwareAddress);

  void HandleRA(Ptr<Packet> p, const Ipv6Address &src, const Ipv6Address &dst,
                Ptr<Ipv6Interface> interface);

  void HandleEchoRequest(Ptr<Packet> p, const Ipv6Address &src,
                         const Ipv6Address &dst, Ptr<Ipv6Interface> interface);

  void HandleNA(Ptr<Packet> p, const Ipv6Address &src, const Ipv6Address &dst,
                Ptr<Ipv6Interface> interface);

  void HandleRedirection(Ptr<Packet> p, const Ipv6Address &src,
                         const Ipv6Address &dst, Ptr<Ipv6Interface> interface);

  void HandleDestinationUnreachable(Ptr<Packet> p, const Ipv6Address &src,
                                    const Ipv6Address &dst,
                                    Ptr<Ipv6Interface> interface);

  void HandleTimeExceeded(Ptr<Packet> p, const Ipv6Address &src,
                          const Ipv6Address &dst, Ptr<Ipv6Interface> interface);

  void HandlePacketTooBig(Ptr<Packet> p, const Ipv6Address &src,
                          const Ipv6Address &dst, Ptr<Ipv6Interface> interface);

  void HandleParameterError(Ptr<Packet> p, const Ipv6Address &src,
                            const Ipv6Address &dst,
                            Ptr<Ipv6Interface> interface);

  void ReceiveLLA(Icmpv6OptionLinkLayerAddress lla, const Ipv6Address &src,
                  const Ipv6Address &dst, Ptr<Ipv6Interface> interface);

  Ptr<NdiscCache> FindCache(Ptr<NetDevice> device);

  void SetDownTarget(IpL4Protocol::DownTargetCallback cb) override;
  void SetDownTarget6(IpL4Protocol::DownTargetCallback6 cb) override;
  IpL4Protocol::DownTargetCallback GetDownTarget() const override;
  IpL4Protocol::DownTargetCallback6 GetDownTarget6() const override;

  bool m_alwaysDad;

  CacheList m_cacheList;

  uint8_t m_maxMulticastSolicit;

  uint8_t m_maxUnicastSolicit;

  Time m_rsInitialRetransmissionTime;

  Time m_rsMaxRetransmissionTime;

  uint32_t m_rsMaxRetransmissionCount;

  Time m_rsMaxRetransmissionDuration;

  uint32_t m_rsRetransmissionCount{0};

  Time m_rsPrevRetransmissionTimeout;

  Time m_rsFirstTransmissionTime;

  Time m_reachableTime;

  Time m_retransmissionTime;

  Time m_delayFirstProbe;

  Ptr<Node> m_node;

  Ptr<RandomVariableStream> m_solicitationJitter;

  Ptr<UniformRandomVariable> m_rsRetransmissionJitter;

  Time m_dadTimeout;

  EventId m_handleRsTimeoutEvent;

  IpL4Protocol::DownTargetCallback6 m_downTarget;
};

} // namespace ns3

#endif
