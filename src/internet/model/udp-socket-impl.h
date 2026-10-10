#ifndef UDP_SOCKET_IMPL_H
#define UDP_SOCKET_IMPL_H

#include "icmpv4.h"
#include "ipv4-interface.h"
#include "udp-socket.h"

#include "ns3/callback.h"
#include "ns3/ipv4-address.h"
#include "ns3/ptr.h"
#include "ns3/socket.h"
#include "ns3/traced-callback.h"

#include <queue>
#include <stdint.h>

namespace ns3 {

class Ipv4EndPoint;
class Ipv6EndPoint;
class Node;
class Packet;
class UdpL4Protocol;
class Ipv6Header;
class Ipv6Interface;

class UdpSocketImpl : public UdpSocket {
public:
  static TypeId GetTypeId();
  UdpSocketImpl();
  ~UdpSocketImpl() override;

  void SetNode(Ptr<Node> node);
  void SetUdp(Ptr<UdpL4Protocol> udp);

  SocketErrno GetErrno() const override;
  SocketType GetSocketType() const override;
  Ptr<Node> GetNode() const override;
  int Bind() override;
  int Bind6() override;
  int Bind(const Address &address) override;
  int Close() override;
  int ShutdownSend() override;
  int ShutdownRecv() override;
  int Connect(const Address &address) override;
  int Listen() override;
  uint32_t GetTxAvailable() const override;
  int Send(Ptr<Packet> p, uint32_t flags) override;
  int SendTo(Ptr<Packet> p, uint32_t flags, const Address &address) override;
  uint32_t GetRxAvailable() const override;
  Ptr<Packet> Recv(uint32_t maxSize, uint32_t flags) override;
  Ptr<Packet> RecvFrom(uint32_t maxSize, uint32_t flags,
                       Address &fromAddress) override;
  int GetSockName(Address &address) const override;
  int GetPeerName(Address &address) const override;
  int MulticastJoinGroup(uint32_t interfaceIndex,
                         const Address &groupAddress) override;
  int MulticastLeaveGroup(uint32_t interfaceIndex,
                          const Address &groupAddress) override;
  void BindToNetDevice(Ptr<NetDevice> netdevice) override;
  bool SetAllowBroadcast(bool allowBroadcast) override;
  bool GetAllowBroadcast() const override;
  void Ipv6JoinGroup(Ipv6Address address,
                     Socket::Ipv6MulticastFilterMode filterMode,
                     std::vector<Ipv6Address> sourceAddresses) override;

private:
  void SetRcvBufSize(uint32_t size) override;
  uint32_t GetRcvBufSize() const override;
  void SetIpMulticastTtl(uint8_t ipTtl) override;
  uint8_t GetIpMulticastTtl() const override;
  void SetIpMulticastIf(int32_t ipIf) override;
  int32_t GetIpMulticastIf() const override;
  void SetIpMulticastLoop(bool loop) override;
  bool GetIpMulticastLoop() const override;
  void SetMtuDiscover(bool discover) override;
  bool GetMtuDiscover() const override;

  friend class UdpSocketFactory;

  int FinishBind();

  void ForwardUp(Ptr<Packet> packet, Ipv4Header header, uint16_t port,
                 Ptr<Ipv4Interface> incomingInterface);

  void ForwardUp6(Ptr<Packet> packet, Ipv6Header header, uint16_t port,
                  Ptr<Ipv6Interface> incomingInterface);

  void Destroy();

  void Destroy6();

  void DeallocateEndPoint();

  int DoSend(Ptr<Packet> p);
  int DoSendTo(Ptr<Packet> p, Ipv4Address daddr, uint16_t dport, uint8_t tos);
  int DoSendTo(Ptr<Packet> p, Ipv6Address daddr, uint16_t dport);

  void ForwardIcmp(Ipv4Address icmpSource, uint8_t icmpTtl, uint8_t icmpType,
                   uint8_t icmpCode, uint32_t icmpInfo);

  void ForwardIcmp6(Ipv6Address icmpSource, uint8_t icmpTtl, uint8_t icmpType,
                    uint8_t icmpCode, uint32_t icmpInfo);

  Ipv4EndPoint *m_endPoint;
  Ipv6EndPoint *m_endPoint6;
  Ptr<Node> m_node;
  Ptr<UdpL4Protocol> m_udp;
  Callback<void, Ipv4Address, uint8_t, uint8_t, uint8_t, uint32_t>
      m_icmpCallback;
  Callback<void, Ipv6Address, uint8_t, uint8_t, uint8_t, uint32_t>
      m_icmpCallback6;

  Address m_defaultAddress;
  uint16_t m_defaultPort;
  TracedCallback<Ptr<const Packet>> m_dropTrace;

  mutable SocketErrno m_errno;
  bool m_shutdownSend;
  bool m_shutdownRecv;
  bool m_connected;
  bool m_allowBroadcast;

  std::queue<std::pair<Ptr<Packet>, Address>> m_deliveryQueue;
  uint32_t m_rxAvailable;

  uint32_t m_rcvBufSize;
  uint8_t m_ipMulticastTtl;
  int32_t m_ipMulticastIf;
  bool m_ipMulticastLoop;
  bool m_mtuDiscover;
};

} // namespace ns3

#endif
