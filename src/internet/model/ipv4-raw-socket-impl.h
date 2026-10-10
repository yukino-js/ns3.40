
#ifndef IPV4_RAW_SOCKET_IMPL_H
#define IPV4_RAW_SOCKET_IMPL_H

#include "ipv4-header.h"
#include "ipv4-interface.h"
#include "ipv4-route.h"

#include "ns3/socket.h"

#include <list>

namespace ns3 {

class NetDevice;
class Node;

class Ipv4RawSocketImpl : public Socket {
public:
  static TypeId GetTypeId();

  Ipv4RawSocketImpl();

  void SetNode(Ptr<Node> node);

  Socket::SocketErrno GetErrno() const override;

  Socket::SocketType GetSocketType() const override;

  Ptr<Node> GetNode() const override;
  int Bind(const Address &address) override;
  int Bind() override;
  int Bind6() override;
  int GetSockName(Address &address) const override;
  int GetPeerName(Address &address) const override;
  int Close() override;
  int ShutdownSend() override;
  int ShutdownRecv() override;
  int Connect(const Address &address) override;
  int Listen() override;
  uint32_t GetTxAvailable() const override;
  int Send(Ptr<Packet> p, uint32_t flags) override;
  int SendTo(Ptr<Packet> p, uint32_t flags, const Address &toAddress) override;
  uint32_t GetRxAvailable() const override;
  Ptr<Packet> Recv(uint32_t maxSize, uint32_t flags) override;
  Ptr<Packet> RecvFrom(uint32_t maxSize, uint32_t flags,
                       Address &fromAddress) override;

  void SetProtocol(uint16_t protocol);

  bool ForwardUp(Ptr<const Packet> p, Ipv4Header ipHeader,
                 Ptr<Ipv4Interface> incomingInterface);
  bool SetAllowBroadcast(bool allowBroadcast) override;
  bool GetAllowBroadcast() const override;

private:
  void DoDispose() override;

  struct Data {
    Ptr<Packet> packet;
    Ipv4Address fromIp;
    uint16_t fromProtocol;
  };

  mutable Socket::SocketErrno m_err;
  Ptr<Node> m_node;
  Ipv4Address m_src;
  Ipv4Address m_dst;
  uint16_t m_protocol;
  std::list<Data> m_recv;
  bool m_shutdownSend;
  bool m_shutdownRecv;
  uint32_t m_icmpFilter;
  bool m_iphdrincl;
};

} // namespace ns3

#endif
