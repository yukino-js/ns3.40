
#ifndef PING6_H
#define PING6_H

#include "ns3/application.h"
#include "ns3/ipv6-address.h"
#include "ns3/ipv6-l3-protocol.h"

namespace ns3 {

class Packet;
class Socket;

class NS_DEPRECATED_3_38(
    "Use Ping instead - the attributes might have been renamed.") Ping6
    : public Application {
public:
  static TypeId GetTypeId();

  Ping6();

  ~Ping6() override;

  void SetLocal(Ipv6Address ipv6);

  void SetRemote(Ipv6Address ipv6);

  void SetIfIndex(uint32_t ifIndex);

  void SetRouters(std::vector<Ipv6Address> routers);

protected:
  void DoDispose() override;

private:
  void StartApplication() override;

  void StopApplication() override;

  void ScheduleTransmit(Time dt);

  void Send();

  void HandleRead(Ptr<Socket> socket);

  Ipv6Address m_address;

  uint32_t m_count;

  uint32_t m_sent;

  uint32_t m_size;

  Time m_interval;

  Ipv6Address m_localAddress;

  uint32_t m_ipInterfaceIndex;

  Ptr<Ipv6L3Protocol> m_ipv6Protocol;

  Ipv6Address m_peerAddress;

  Ptr<Socket> m_socket;

  uint16_t m_seq;

  EventId m_sendEvent;

  uint32_t m_ifIndex;

  std::vector<Ipv6Address> m_routers;
};

} // namespace ns3

#endif
