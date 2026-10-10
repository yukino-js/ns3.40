
#ifndef UDP_ECHO_CLIENT_H
#define UDP_ECHO_CLIENT_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ipv4-address.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"

namespace ns3 {

class Socket;
class Packet;

class UdpEchoClient : public Application {
public:
  static TypeId GetTypeId();

  UdpEchoClient();

  ~UdpEchoClient() override;

  void SetRemote(Address ip, uint16_t port);
  void SetRemote(Address addr);

  void SetDataSize(uint32_t dataSize);

  uint32_t GetDataSize() const;

  void SetFill(std::string fill);

  void SetFill(uint8_t fill, uint32_t dataSize);

  void SetFill(uint8_t *fill, uint32_t fillSize, uint32_t dataSize);

protected:
  void DoDispose() override;

private:
  void StartApplication() override;
  void StopApplication() override;

  void ScheduleTransmit(Time dt);
  void Send();

  void HandleRead(Ptr<Socket> socket);

  uint32_t m_count;
  Time m_interval;
  uint32_t m_size;

  uint32_t m_dataSize;
  uint8_t *m_data;

  uint32_t m_sent;
  Ptr<Socket> m_socket;
  Address m_peerAddress;
  uint16_t m_peerPort;
  EventId m_sendEvent;

  TracedCallback<Ptr<const Packet>> m_txTrace;

  TracedCallback<Ptr<const Packet>> m_rxTrace;

  TracedCallback<Ptr<const Packet>, const Address &, const Address &>
      m_txTraceWithAddresses;

  TracedCallback<Ptr<const Packet>, const Address &, const Address &>
      m_rxTraceWithAddresses;
};

} // namespace ns3

#endif
