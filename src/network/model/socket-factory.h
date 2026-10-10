#ifndef SOCKET_FACTORY_H
#define SOCKET_FACTORY_H

#include "ns3/object.h"
#include "ns3/ptr.h"

namespace ns3 {

class Socket;

class SocketFactory : public Object {
public:
  static TypeId GetTypeId();

  SocketFactory();

  virtual Ptr<Socket> CreateSocket() = 0;
};

} // namespace ns3

#endif
