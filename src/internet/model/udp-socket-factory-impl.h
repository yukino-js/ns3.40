#ifndef UDP_SOCKET_FACTORY_IMPL_H
#define UDP_SOCKET_FACTORY_IMPL_H

#include "udp-socket-factory.h"

#include "ns3/ptr.h"

namespace ns3 {

class UdpL4Protocol;

class UdpSocketFactoryImpl : public UdpSocketFactory {
public:
  UdpSocketFactoryImpl();
  ~UdpSocketFactoryImpl() override;

  void SetUdp(Ptr<UdpL4Protocol> udp);

  Ptr<Socket> CreateSocket() override;

protected:
  void DoDispose() override;

private:
  Ptr<UdpL4Protocol> m_udp;
};

} // namespace ns3

#endif
