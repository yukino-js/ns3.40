
#ifndef TCP_L4_PROTOCOL_H
#define TCP_L4_PROTOCOL_H

#include "ip-l4-protocol.h"

#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/sequence-number.h"

#include <stdint.h>
#include <unordered_map>

namespace ns3 {

class Node;
class Socket;
class TcpHeader;
class Ipv4EndPointDemux;
class Ipv6EndPointDemux;
class Ipv4Interface;
class TcpSocketBase;
class Ipv4EndPoint;
class Ipv6EndPoint;
class NetDevice;

class TcpL4Protocol : public IpL4Protocol {
public:
  static TypeId GetTypeId();
  static const uint8_t PROT_NUMBER;

  TcpL4Protocol();
  ~TcpL4Protocol() override;

  TcpL4Protocol(const TcpL4Protocol &) = delete;
  TcpL4Protocol &operator=(const TcpL4Protocol &) = delete;

  void SetNode(Ptr<Node> node);

  Ptr<Socket> CreateSocket();

  Ptr<Socket> CreateSocket(TypeId congestionTypeId, TypeId recoveryTypeId);

  Ptr<Socket> CreateSocket(TypeId congestionTypeId);

  Ipv4EndPoint *Allocate();
  Ipv4EndPoint *Allocate(Ipv4Address address);
  Ipv4EndPoint *Allocate(Ptr<NetDevice> boundNetDevice, uint16_t port);
  Ipv4EndPoint *Allocate(Ptr<NetDevice> boundNetDevice, Ipv4Address address,
                         uint16_t port);
  Ipv4EndPoint *Allocate(Ptr<NetDevice> boundNetDevice,
                         Ipv4Address localAddress, uint16_t localPort,
                         Ipv4Address peerAddress, uint16_t peerPort);
  Ipv6EndPoint *Allocate6();
  Ipv6EndPoint *Allocate6(Ipv6Address address);
  Ipv6EndPoint *Allocate6(Ptr<NetDevice> boundNetDevice, uint16_t port);
  Ipv6EndPoint *Allocate6(Ptr<NetDevice> boundNetDevice, Ipv6Address address,
                          uint16_t port);
  Ipv6EndPoint *Allocate6(Ptr<NetDevice> boundNetDevice,
                          Ipv6Address localAddress, uint16_t localPort,
                          Ipv6Address peerAddress, uint16_t peerPort);

  void SendPacket(Ptr<Packet> pkt, const TcpHeader &outgoing,
                  const Address &saddr, const Address &daddr,
                  Ptr<NetDevice> oif = nullptr) const;

  void AddSocket(Ptr<TcpSocketBase> socket);

  bool RemoveSocket(Ptr<TcpSocketBase> socket);

  void DeAllocate(Ipv4EndPoint *endPoint);
  void DeAllocate(Ipv6EndPoint *endPoint);

  IpL4Protocol::RxStatus Receive(Ptr<Packet> p,
                                 const Ipv4Header &incomingIpHeader,
                                 Ptr<Ipv4Interface> incomingInterface) override;
  IpL4Protocol::RxStatus Receive(Ptr<Packet> p,
                                 const Ipv6Header &incomingIpHeader,
                                 Ptr<Ipv6Interface> incomingInterface) override;

  void ReceiveIcmp(Ipv4Address icmpSource, uint8_t icmpTtl, uint8_t icmpType,
                   uint8_t icmpCode, uint32_t icmpInfo,
                   Ipv4Address payloadSource, Ipv4Address payloadDestination,
                   const uint8_t payload[8]) override;
  void ReceiveIcmp(Ipv6Address icmpSource, uint8_t icmpTtl, uint8_t icmpType,
                   uint8_t icmpCode, uint32_t icmpInfo,
                   Ipv6Address payloadSource, Ipv6Address payloadDestination,
                   const uint8_t payload[8]) override;

  void SetDownTarget(IpL4Protocol::DownTargetCallback cb) override;
  void SetDownTarget6(IpL4Protocol::DownTargetCallback6 cb) override;
  int GetProtocolNumber() const override;
  IpL4Protocol::DownTargetCallback GetDownTarget() const override;
  IpL4Protocol::DownTargetCallback6 GetDownTarget6() const override;

protected:
  void DoDispose() override;

  void NotifyNewAggregate() override;

  IpL4Protocol::RxStatus PacketReceived(Ptr<Packet> packet,
                                        TcpHeader &incomingTcpHeader,
                                        const Address &source,
                                        const Address &destination);

  void NoEndPointsFound(const TcpHeader &incomingHeader,
                        const Address &incomingSAddr,
                        const Address &incomingDAddr);

private:
  Ptr<Node> m_node;
  Ipv4EndPointDemux *m_endPoints;
  Ipv6EndPointDemux *m_endPoints6;
  TypeId m_rttTypeId;
  TypeId m_congestionTypeId;
  TypeId m_recoveryTypeId;
  std::unordered_map<uint64_t, Ptr<TcpSocketBase>> m_sockets;
  uint64_t m_socketIndex{0};
  IpL4Protocol::DownTargetCallback m_downTarget;
  IpL4Protocol::DownTargetCallback6 m_downTarget6;

  void SendPacketV4(Ptr<Packet> pkt, const TcpHeader &outgoing,
                    const Ipv4Address &saddr, const Ipv4Address &daddr,
                    Ptr<NetDevice> oif = nullptr) const;

  void SendPacketV6(Ptr<Packet> pkt, const TcpHeader &outgoing,
                    const Ipv6Address &saddr, const Ipv6Address &daddr,
                    Ptr<NetDevice> oif = nullptr) const;
};

} // namespace ns3

#endif
