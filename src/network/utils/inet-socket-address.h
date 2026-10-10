
#ifndef INET_SOCKET_ADDRESS_H
#define INET_SOCKET_ADDRESS_H

#include "ipv4-address.h"

#include "ns3/address.h"

#include <stdint.h>

namespace ns3 {

class InetSocketAddress {
public:
  InetSocketAddress(Ipv4Address ipv4, uint16_t port);
  InetSocketAddress(Ipv4Address ipv4);
  InetSocketAddress(uint16_t port);
  InetSocketAddress(const char *ipv4, uint16_t port);
  InetSocketAddress(const char *ipv4);
  uint16_t GetPort() const;
  Ipv4Address GetIpv4() const;
  uint8_t GetTos() const;

  void SetPort(uint16_t port);
  void SetIpv4(Ipv4Address address);
  void SetTos(uint8_t tos);

  static bool IsMatchingType(const Address &address);

  operator Address() const;

  static InetSocketAddress ConvertFrom(const Address &address);

  Address ConvertTo() const;

private:
  static uint8_t GetType();
  Ipv4Address m_ipv4;
  uint16_t m_port;
  uint8_t m_tos;
};

} // namespace ns3

#endif
