
#ifndef IPV6_RAW_SOCKET_FACTORY_H
#define IPV6_RAW_SOCKET_FACTORY_H

#include "ns3/socket-factory.h"

namespace ns3 {

class Socket;

class Ipv6RawSocketFactory : public SocketFactory {
public:
  static TypeId GetTypeId();
};

} // namespace ns3

#endif
