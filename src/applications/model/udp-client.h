
#ifndef UDP_CLIENT_H
#define UDP_CLIENT_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ipv4-address.h"
#include "ns3/ptr.h"
#include <ns3/traced-callback.h>

namespace ns3 {

class Socket;
class Packet;

class UdpClient : public Application {
public:
  static TypeId GetTypeId();

  UdpClient();

  ~UdpClient() override;

  void SetRemote(Address ip, uint16_t port);
  void SetRemote(Address addr);

  uint64_t GetTotalTx() const;

protected:
  void DoDispose() override;

private:
  void StartApplication() override;
  void StopApplication() override;

  void Send();

  TracedCallback<Ptr<const Packet>> m_txTrace;

  TracedCallback<Ptr<const Packet>, const Address &, const Address &>
      m_txTraceWithAddresses;

  uint32_t m_count;
  Time m_interval;
  uint32_t m_size;

  uint32_t m_sent;
  uint64_t m_totalTx;
  Ptr<Socket> m_socket;
  Address m_peerAddress;
  uint16_t m_peerPort;
  EventId m_sendEvent;

#ifdef NS3_LOG_ENABLE
  std::string m_peerAddressString;
#endif
};

} // namespace ns3

#endif
