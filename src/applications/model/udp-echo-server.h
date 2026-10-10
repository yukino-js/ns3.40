
#ifndef UDP_ECHO_SERVER_H
#define UDP_ECHO_SERVER_H

#include "ns3/address.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"

namespace ns3 {

class Socket;
class Packet;

class UdpEchoServer : public Application {
public:
  static TypeId GetTypeId();
  UdpEchoServer();
  ~UdpEchoServer() override;

protected:
  void DoDispose() override;

private:
  void StartApplication() override;
  void StopApplication() override;

  void HandleRead(Ptr<Socket> socket);

  uint16_t m_port;
  Ptr<Socket> m_socket;
  Ptr<Socket> m_socket6;
  Address m_local;

  TracedCallback<Ptr<const Packet>> m_rxTrace;

  TracedCallback<Ptr<const Packet>, const Address &, const Address &>
      m_rxTraceWithAddresses;
};

} // namespace ns3

#endif
