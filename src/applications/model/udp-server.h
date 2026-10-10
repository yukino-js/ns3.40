
#ifndef UDP_SERVER_H
#define UDP_SERVER_H

#include "packet-loss-counter.h"

#include "ns3/address.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"

namespace ns3 {

class UdpServer : public Application {
public:
  static TypeId GetTypeId();
  UdpServer();
  ~UdpServer() override;
  uint32_t GetLost() const;

  uint64_t GetReceived() const;

  uint16_t GetPacketWindowSize() const;

  void SetPacketWindowSize(uint16_t size);

protected:
  void DoDispose() override;

private:
  void StartApplication() override;
  void StopApplication() override;

  void HandleRead(Ptr<Socket> socket);

  uint16_t m_port;
  Ptr<Socket> m_socket;
  Ptr<Socket> m_socket6;
  uint64_t m_received;
  PacketLossCounter m_lossCounter;

  TracedCallback<Ptr<const Packet>> m_rxTrace;

  TracedCallback<Ptr<const Packet>, const Address &, const Address &>
      m_rxTraceWithAddresses;
};

} // namespace ns3

#endif
