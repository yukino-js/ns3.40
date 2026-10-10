
#ifndef DHCP_SERVER_H
#define DHCP_SERVER_H

#include "dhcp-header.h"

#include "ns3/application.h"
#include "ns3/ipv4-address.h"

#include <map>

namespace ns3 {

class InetSocketAddress;
class Socket;
class Packet;

class DhcpServer : public Application {
public:
  static TypeId GetTypeId();

  DhcpServer();
  ~DhcpServer() override;

  void AddStaticDhcpEntry(Address chaddr, Ipv4Address addr);

protected:
  void DoDispose() override;

private:
  static const int PORT = 67;

  void NetHandler(Ptr<Socket> socket);

  void SendOffer(Ptr<NetDevice> iDev, DhcpHeader header,
                 InetSocketAddress from);

  void SendAck(Ptr<NetDevice> iDev, DhcpHeader header, InetSocketAddress from);

  void TimerHandler();

  void StartApplication() override;

  void StopApplication() override;

  Ptr<Socket> m_socket;
  Ipv4Address m_poolAddress;
  Ipv4Address m_minAddress;
  Ipv4Address m_maxAddress;
  Ipv4Mask m_poolMask;
  Ipv4Address m_gateway;

  typedef std::map<Address, std::pair<Ipv4Address, uint32_t>> LeasedAddress;
  typedef std::map<Address, std::pair<Ipv4Address, uint32_t>>::iterator
      LeasedAddressIter;
  typedef std::map<Address, std::pair<Ipv4Address, uint32_t>>::const_iterator
      LeasedAddressCIter;

  typedef std::list<Address> ExpiredAddress;
  typedef std::list<Address>::iterator ExpiredAddressIter;
  typedef std::list<Address>::const_iterator ExpiredAddressCIter;

  typedef std::list<Ipv4Address> AvailableAddress;

  LeasedAddress m_leasedAddresses;
  ExpiredAddress m_expiredAddresses;
  AvailableAddress m_availableAddresses;
  Time m_lease;
  Time m_renew;
  Time m_rebind;
  EventId m_expiredEvent;
};

} // namespace ns3

#endif
