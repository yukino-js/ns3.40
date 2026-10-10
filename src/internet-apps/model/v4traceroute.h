
#ifndef V4TRACEROUTE_H
#define V4TRACEROUTE_H

#include "ns3/application.h"
#include "ns3/average.h"
#include "ns3/nstime.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/simulator.h"
#include "ns3/traced-callback.h"

#include <map>

namespace ns3 {

class Socket;

class V4TraceRoute : public Application {
public:
  static TypeId GetTypeId();
  V4TraceRoute();
  ~V4TraceRoute() override;
  void Print(Ptr<OutputStreamWrapper> stream);

private:
  void StartApplication() override;
  void StopApplication() override;
  void DoDispose() override;
  uint32_t GetApplicationId() const;
  void Receive(Ptr<Socket> socket);

  void Send();

  void StartWaitReplyTimer();

  void HandleWaitReplyTimeout();

  Ipv4Address m_remote;

  Time m_interval;
  uint32_t m_size;
  Ptr<Socket> m_socket;
  uint16_t m_seq;
  bool m_verbose;
  Time m_started;
  EventId m_next;
  uint32_t m_probeCount;
  uint16_t m_maxProbes;
  uint16_t m_ttl;
  uint32_t m_maxTtl;
  Time m_waitIcmpReplyTimeout;
  EventId m_waitIcmpReplyTimer;
  std::map<uint16_t, Time> m_sent;

  std::ostringstream m_osRoute;
  std::ostringstream m_routeIpv4;
  Ptr<OutputStreamWrapper> m_printStream;
};

} // namespace ns3

#endif
