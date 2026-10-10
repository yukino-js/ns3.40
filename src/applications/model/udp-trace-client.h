
#ifndef UDP_TRACE_CLIENT_H
#define UDP_TRACE_CLIENT_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ipv4-address.h"
#include "ns3/ptr.h"

#include <vector>

namespace ns3 {

class Socket;
class Packet;

class UdpTraceClient : public Application {
public:
  static TypeId GetTypeId();

  UdpTraceClient();

  UdpTraceClient(Ipv4Address ip, uint16_t port, char *traceFile);
  ~UdpTraceClient() override;

  void SetRemote(Address ip, uint16_t port);
  void SetRemote(Address addr);

  void SetTraceFile(std::string filename);

  uint16_t GetMaxPacketSize();

  void SetMaxPacketSize(uint16_t maxPacketSize);

  void SetTraceLoop(bool traceLoop);

protected:
  void DoDispose() override;

private:
  void LoadTrace(std::string filename);
  void LoadDefaultTrace();
  void StartApplication() override;
  void StopApplication() override;

  void Send();
  void SendPacket(uint32_t size);

  struct TraceEntry {
    uint32_t timeToSend;
    uint32_t packetSize;
    char frameType;
  };

  uint32_t m_sent;
  Ptr<Socket> m_socket;
  Address m_peerAddress;
  uint16_t m_peerPort;
  EventId m_sendEvent;

  std::vector<TraceEntry> m_entries;
  uint32_t m_currentEntry;
  static TraceEntry g_defaultEntries[];
  uint16_t m_maxPacketSize;
  bool m_traceLoop;
};

} // namespace ns3

#endif
