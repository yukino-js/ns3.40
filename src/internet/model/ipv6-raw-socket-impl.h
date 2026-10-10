
#ifndef IPV6_RAW_SOCKET_IMPL_H
#define IPV6_RAW_SOCKET_IMPL_H

#include "ipv6-header.h"

#include "ns3/ipv6-address.h"
#include "ns3/socket.h"

#include <list>

namespace ns3 {

class NetDevice;
class Node;

class Ipv6RawSocketImpl : public Socket {
public:
  static TypeId GetTypeId();

  Ipv6RawSocketImpl();
  ~Ipv6RawSocketImpl() override;

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
  uint32_t GetRxAvailable() const override;
  int Send(Ptr<Packet> p, uint32_t flags) override;
  int SendTo(Ptr<Packet> p, uint32_t flags, const Address &toAddress) override;
  Ptr<Packet> Recv(uint32_t maxSize, uint32_t flags) override;
  Ptr<Packet> RecvFrom(uint32_t maxSize, uint32_t flags,
                       Address &fromAddress) override;
  void Ipv6JoinGroup(Ipv6Address address,
                     Socket::Ipv6MulticastFilterMode filterMode,
                     std::vector<Ipv6Address> sourceAddresses) override;

  void SetProtocol(uint16_t protocol);

  bool ForwardUp(Ptr<const Packet> p, Ipv6Header hdr, Ptr<NetDevice> device);

  bool SetAllowBroadcast(bool allowBroadcast) override;
  bool GetAllowBroadcast() const override;

  void Icmpv6FilterSetPassAll();

  void Icmpv6FilterSetBlockAll();

  void Icmpv6FilterSetPass(uint8_t type);

  void Icmpv6FilterSetBlock(uint8_t type);

  bool Icmpv6FilterWillPass(uint8_t type);

  bool Icmpv6FilterWillBlock(uint8_t type);

private:
  struct Data {
    Ptr<Packet> packet;
    Ipv6Address fromIp;
    uint16_t fromProtocol;
  };

  void DoDispose() override;

  mutable Socket::SocketErrno m_err;

  Ptr<Node> m_node;

  Ipv6Address m_src;

  Ipv6Address m_dst;

  uint16_t m_protocol;

  std::list<Data> m_data;

  bool m_shutdownSend;

  bool m_shutdownRecv;

  struct Icmpv6Filter {
    uint32_t icmpv6Filt[8];
  };

  Icmpv6Filter m_icmpFilter;
};

} // namespace ns3

#endif
