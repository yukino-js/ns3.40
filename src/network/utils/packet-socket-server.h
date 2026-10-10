
#ifndef PACKET_SOCKET_SERVER_H
#define PACKET_SOCKET_SERVER_H

#include "packet-socket-address.h"

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"

namespace ns3 {

class Socket;
class Packet;

class PacketSocketServer : public Application {
public:
  static TypeId GetTypeId();

  PacketSocketServer();

  ~PacketSocketServer() override;

  void SetLocal(PacketSocketAddress addr);

protected:
  void DoDispose() override;

private:
  void StartApplication() override;
  void StopApplication() override;

  void HandleRead(Ptr<Socket> socket);

  uint32_t m_pktRx;
  uint32_t m_bytesRx;

  Ptr<Socket> m_socket;
  PacketSocketAddress m_localAddress;
  bool m_localAddressSet;

  TracedCallback<Ptr<const Packet>, const Address &> m_rxTrace;
};

} // namespace ns3

#endif
