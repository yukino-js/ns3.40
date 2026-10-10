
#ifndef IPV4_RAW_SOCKET_FACTORY_IMPL_H
#define IPV4_RAW_SOCKET_FACTORY_IMPL_H

#include "ipv4-raw-socket-factory.h"

namespace ns3 {

class Ipv4RawSocketFactoryImpl : public Ipv4RawSocketFactory {
public:
  Ptr<Socket> CreateSocket() override;
};

} // namespace ns3

#endif
