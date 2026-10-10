

#ifndef V4PING_H
#define V4PING_H

#include "ns3/application.h"
#include "ns3/average.h"
#include "ns3/traced-callback.h"

#include <map>

namespace ns3 {

class Socket;

class NS_DEPRECATED_3_38(
    "Use Ping instead - the attributes might have been renamed.") V4Ping
    : public Application {
public:
  static TypeId GetTypeId();

  V4Ping();
  ~V4Ping() override;

private:
  void Write32(uint8_t *buffer, const uint32_t data);
  void Read32(const uint8_t *buffer, uint32_t &data);

  void StartApplication() override;
  void StopApplication() override;
  void DoDispose() override;
  uint32_t GetApplicationId() const;
  void Receive(Ptr<Socket> socket);
  void Send();

  Ipv4Address m_remote;
  Time m_interval;
  uint32_t m_size;
  Ptr<Socket> m_socket;
  uint16_t m_seq;
  TracedCallback<Time> m_traceRtt;
  bool m_verbose;
  uint32_t m_recv;
  Time m_started;
  Average<double> m_avgRtt;
  EventId m_next;
  std::map<uint16_t, Time> m_sent;
};

} // namespace ns3

#endif
