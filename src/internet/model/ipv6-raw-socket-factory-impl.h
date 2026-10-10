
#ifndef IPV6_RAW_SOCKET_FACTORY_IMPL_H
#define IPV6_RAW_SOCKET_FACTORY_IMPL_H

#include "ipv6-raw-socket-factory.h"

namespace ns3 {

class Ipv6RawSocketFactoryImpl : public Ipv6RawSocketFactory {
public:
  Ptr<Socket> CreateSocket() override;
};

} // namespace ns3

#endif
