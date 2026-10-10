
#ifndef UDP_L4_PROTOCOL_H
#define UDP_L4_PROTOCOL_H

#include "ip-l4-protocol.h"

#include "ns3/packet.h"
#include "ns3/ptr.h"

#include <stdint.h>
#include <unordered_map>

namespace ns3 {

class Node;
class Socket;
class Ipv4EndPointDemux;
class Ipv4EndPoint;
class Ipv6EndPointDemux;
class Ipv6EndPoint;
class UdpSocketImpl;
class NetDevice;

class UdpL4Protocol : public IpL4Protocol {
public:
  static TypeId GetTypeId();
  static const uint8_t PROT_NUMBER;

  UdpL4Protocol();
  ~UdpL4Protocol() override;

  UdpL4Protocol(const UdpL4Protocol &) = delete;
  UdpL4Protocol &operator=(const UdpL4Protocol &) = delete;

  void SetNode(Ptr<Node> node);

  int GetProtocolNumber() const override;

  Ptr<Socket> CreateSocket();

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

  void DeAllocate(Ipv4EndPoint *endPoint);
  void DeAllocate(Ipv6EndPoint *endPoint);

  bool RemoveSocket(Ptr<UdpSocketImpl> socket);

  void Send(Ptr<Packet> packet, Ipv4Address saddr, Ipv4Address daddr,
            uint16_t sport, uint16_t dport);
  void Send(Ptr<Packet> packet, Ipv4Address saddr, Ipv4Address daddr,
            uint16_t sport, uint16_t dport, Ptr<Ipv4Route> route);
  void Send(Ptr<Packet> packet, Ipv6Address saddr, Ipv6Address daddr,
            uint16_t sport, uint16_t dport);
  void Send(Ptr<Packet> packet, Ipv6Address saddr, Ipv6Address daddr,
            uint16_t sport, uint16_t dport, Ptr<Ipv6Route> route);

  IpL4Protocol::RxStatus Receive(Ptr<Packet> p, const Ipv4Header &header,
                                 Ptr<Ipv4Interface> interface) override;
  IpL4Protocol::RxStatus Receive(Ptr<Packet> p, const Ipv6Header &header,
                                 Ptr<Ipv6Interface> interface) override;

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
  IpL4Protocol::DownTargetCallback GetDownTarget() const override;
  IpL4Protocol::DownTargetCallback6 GetDownTarget6() const override;

protected:
  void DoDispose() override;
  void NotifyNewAggregate() override;

private:
  Ptr<Node> m_node;
  Ipv4EndPointDemux *m_endPoints;
  Ipv6EndPointDemux *m_endPoints6;

  std::unordered_map<uint64_t, Ptr<UdpSocketImpl>> m_sockets;
  uint64_t m_socketIndex{0};
  IpL4Protocol::DownTargetCallback m_downTarget;
  IpL4Protocol::DownTargetCallback6 m_downTarget6;
};

} // namespace ns3

#endif
