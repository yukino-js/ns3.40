#ifndef IPV4_RAW_SOCKET_FACTORY_H
#define IPV4_RAW_SOCKET_FACTORY_H

#include "ns3/socket-factory.h"

namespace ns3 {

class Socket;

class Ipv4RawSocketFactory : public SocketFactory {
public:
  static TypeId GetTypeId();
};

} // namespace ns3

#endif
