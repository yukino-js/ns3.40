
#ifndef PACKET_SOCKET_CLIENT_H
#define PACKET_SOCKET_CLIENT_H

#include "packet-socket-address.h"

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"

namespace ns3 {

class Socket;
class Packet;

class PacketSocketClient : public Application {
public:
  static TypeId GetTypeId();

  PacketSocketClient();

  ~PacketSocketClient() override;

  void SetRemote(PacketSocketAddress addr);

  uint8_t GetPriority() const;

protected:
  void DoDispose() override;

private:
  void StartApplication() override;
  void StopApplication() override;

  void SetPriority(uint8_t priority);

  void Send();

  uint32_t m_maxPackets;
  Time m_interval;
  uint32_t m_size;
  uint8_t m_priority;

  uint32_t m_sent;
  Ptr<Socket> m_socket;
  PacketSocketAddress m_peerAddress;
  bool m_peerAddressSet;
  EventId m_sendEvent;

  TracedCallback<Ptr<const Packet>, const Address &> m_txTrace;
};

} // namespace ns3

#endif
