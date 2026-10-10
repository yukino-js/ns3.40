#ifndef TCP_SOCKET_FACTORY_H
#define TCP_SOCKET_FACTORY_H

#include "ns3/socket-factory.h"

namespace ns3 {

class Socket;

class TcpSocketFactory : public SocketFactory {
public:
  static TypeId GetTypeId();
};

} // namespace ns3

#endif
