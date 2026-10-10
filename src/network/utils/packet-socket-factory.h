#ifndef PACKET_SOCKET_FACTORY_H
#define PACKET_SOCKET_FACTORY_H

#include "ns3/socket-factory.h"

namespace ns3 {

class Socket;

class PacketSocketFactory : public SocketFactory {
public:
  static TypeId GetTypeId();

  PacketSocketFactory();

  Ptr<Socket> CreateSocket() override;
};

} // namespace ns3

#endif
