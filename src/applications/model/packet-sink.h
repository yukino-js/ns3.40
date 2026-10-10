
#ifndef PACKET_SINK_H
#define PACKET_SINK_H

#include "seq-ts-size-header.h"

#include "ns3/address.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"

#include <unordered_map>

namespace ns3 {

class Address;
class Socket;
class Packet;

class PacketSink : public Application {
public:
  static TypeId GetTypeId();
  PacketSink();

  ~PacketSink() override;

  uint64_t GetTotalRx() const;

  Ptr<Socket> GetListeningSocket() const;

  std::list<Ptr<Socket>> GetAcceptedSockets() const;

  typedef void (*SeqTsSizeCallback)(Ptr<const Packet> p, const Address &from,
                                    const Address &to,
                                    const SeqTsSizeHeader &header);

protected:
  void DoDispose() override;

private:
  void StartApplication() override;
  void StopApplication() override;

  void HandleRead(Ptr<Socket> socket);
  void HandleAccept(Ptr<Socket> socket, const Address &from);
  void HandlePeerClose(Ptr<Socket> socket);
  void HandlePeerError(Ptr<Socket> socket);

  void PacketReceived(const Ptr<Packet> &p, const Address &from,
                      const Address &localAddress);

  struct AddressHash {
    size_t operator()(const Address &x) const {
      if (InetSocketAddress::IsMatchingType(x)) {
        InetSocketAddress a = InetSocketAddress::ConvertFrom(x);
        return Ipv4AddressHash()(a.GetIpv4());
      } else if (Inet6SocketAddress::IsMatchingType(x)) {
        Inet6SocketAddress a = Inet6SocketAddress::ConvertFrom(x);
        return Ipv6AddressHash()(a.GetIpv6());
      }

      NS_ABORT_MSG(
          "PacketSink: unexpected address type, neither IPv4 nor IPv6");
      return 0;
    }
  };

  std::unordered_map<Address, Ptr<Packet>, AddressHash> m_buffer;

  Ptr<Socket> m_socket;
  std::list<Ptr<Socket>> m_socketList;

  Address m_local;
  uint16_t m_localPort;
  uint64_t m_totalRx;
  TypeId m_tid;

  bool m_enableSeqTsSizeHeader{false};

  TracedCallback<Ptr<const Packet>, const Address &> m_rxTrace;
  TracedCallback<Ptr<const Packet>, const Address &, const Address &>
      m_rxTraceWithAddresses;
  TracedCallback<Ptr<const Packet>, const Address &, const Address &,
                 const SeqTsSizeHeader &>
      m_rxTraceWithSeqTsSize;
};

} // namespace ns3

#endif
