#include "udp-socket-factory-impl.h"

#include "udp-l4-protocol.h"

#include "ns3/assert.h"
#include "ns3/socket.h"

namespace ns3 {

UdpSocketFactoryImpl::UdpSocketFactoryImpl() : m_udp(nullptr) {}

UdpSocketFactoryImpl::~UdpSocketFactoryImpl() { NS_ASSERT(!m_udp); }

void UdpSocketFactoryImpl::SetUdp(Ptr<UdpL4Protocol> udp) { m_udp = udp; }

Ptr<Socket> UdpSocketFactoryImpl::CreateSocket() {
  return m_udp->CreateSocket();
}

void UdpSocketFactoryImpl::DoDispose() {
  m_udp = nullptr;
  UdpSocketFactory::DoDispose();
}

} // namespace ns3
