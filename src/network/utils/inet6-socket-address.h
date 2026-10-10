
#ifndef INET6_SOCKET_ADDRESS_H
#define INET6_SOCKET_ADDRESS_H

#include "ipv6-address.h"

#include "ns3/address.h"

#include <stdint.h>

namespace ns3 {

class Inet6SocketAddress {
public:
  Inet6SocketAddress(Ipv6Address ipv6, uint16_t port);

  Inet6SocketAddress(Ipv6Address ipv6);

  Inet6SocketAddress(uint16_t port);

  Inet6SocketAddress(const char *ipv6, uint16_t port);

  Inet6SocketAddress(const char *ipv6);

  uint16_t GetPort() const;

  void SetPort(uint16_t port);

  Ipv6Address GetIpv6() const;

  void SetIpv6(Ipv6Address ipv6);

  static bool IsMatchingType(const Address &addr);

  operator Address() const;

  static Inet6SocketAddress ConvertFrom(const Address &addr);

  Address ConvertTo() const;

private:
  static uint8_t GetType();

  Ipv6Address m_ipv6;

  uint16_t m_port;
};

} // namespace ns3

#endif
