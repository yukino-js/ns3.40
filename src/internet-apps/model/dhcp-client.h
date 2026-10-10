
#ifndef DHCP_CLIENT_H
#define DHCP_CLIENT_H

#include "dhcp-header.h"

#include "ns3/application.h"
#include "ns3/traced-value.h"

#include <list>

namespace ns3 {

class Socket;
class Packet;
class RandomVariableStream;

class DhcpClient : public Application {
public:
  static TypeId GetTypeId();

  DhcpClient();
  ~DhcpClient() override;

  DhcpClient(Ptr<NetDevice> netDevice);

  Ptr<NetDevice> GetDhcpClientNetDevice();

  void SetDhcpClientNetDevice(Ptr<NetDevice> netDevice);

  Ipv4Address GetDhcpServer();

  int64_t AssignStreams(int64_t stream);

protected:
  void DoDispose() override;

private:
  enum States { WAIT_OFFER = 1, REFRESH_LEASE = 2, WAIT_ACK = 9 };

  static const int DHCP_PEER_PORT = 67;

  void StartApplication() override;

  void StopApplication() override;

  void LinkStateHandler();

  void NetHandler(Ptr<Socket> socket);

  void Boot();

  void OfferHandler(DhcpHeader header);

  void Select();

  void Request();

  void AcceptAck(DhcpHeader header, Address from);

  void RemoveAndStart();

  uint8_t m_state;
  bool m_firstBoot;
  Ptr<NetDevice> m_device;
  Ptr<Socket> m_socket;
  Ipv4Address m_remoteAddress;
  Ipv4Address m_offeredAddress;
  Ipv4Address m_myAddress;
  Address m_chaddr;
  Ipv4Mask m_myMask;
  Ipv4Address m_server;
  Ipv4Address m_gateway;
  EventId m_requestEvent;
  EventId m_discoverEvent;
  EventId m_refreshEvent;
  EventId m_rebindEvent;
  EventId m_nextOfferEvent;
  EventId m_timeout;
  EventId m_collectEvent;
  Time m_lease;
  Time m_renew;
  Time m_rebind;
  Time m_nextoffer;
  Ptr<RandomVariableStream> m_ran;
  Time m_rtrs;
  Time m_collect;
  bool m_offered;
  std::list<DhcpHeader> m_offerList;
  uint32_t m_tran;
  TracedCallback<const Ipv4Address &> m_newLease;
  TracedCallback<const Ipv4Address &> m_expiry;
};

} // namespace ns3

#endif
