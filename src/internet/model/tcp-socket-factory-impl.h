#ifndef TCP_SOCKET_FACTORY_IMPL_H
#define TCP_SOCKET_FACTORY_IMPL_H

#include "tcp-socket-factory.h"

#include "ns3/ptr.h"

namespace ns3 {

class TcpL4Protocol;

class TcpSocketFactoryImpl : public TcpSocketFactory {
public:
  TcpSocketFactoryImpl();
  ~TcpSocketFactoryImpl() override;

  void SetTcp(Ptr<TcpL4Protocol> tcp);

  Ptr<Socket> CreateSocket() override;

protected:
  void DoDispose() override;

private:
  Ptr<TcpL4Protocol> m_tcp;
};

} // namespace ns3

#endif
