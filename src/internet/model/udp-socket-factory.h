#ifndef UDP_SOCKET_FACTORY_H
#define UDP_SOCKET_FACTORY_H

#include "ns3/socket-factory.h"

namespace ns3 {

class Socket;

class UdpSocketFactory : public SocketFactory {
public:
  static TypeId GetTypeId();
};

} // namespace ns3

#endif
