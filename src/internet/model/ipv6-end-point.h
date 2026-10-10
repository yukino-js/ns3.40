
#ifndef IPV6_END_POINT_H
#define IPV6_END_POINT_H

#include "ipv6-header.h"
#include "ipv6-interface.h"

#include "ns3/callback.h"
#include "ns3/ipv6-address.h"
#include "ns3/net-device.h"

#include <stdint.h>

namespace ns3 {

class Header;
class Packet;

class Ipv6EndPoint {
public:
  Ipv6EndPoint(Ipv6Address addr, uint16_t port);

  ~Ipv6EndPoint();

  Ipv6Address GetLocalAddress() const;

  void SetLocalAddress(Ipv6Address addr);

  uint16_t GetLocalPort() const;

  void SetLocalPort(uint16_t port);

  Ipv6Address GetPeerAddress() const;

  uint16_t GetPeerPort() const;

  void SetPeer(Ipv6Address addr, uint16_t port);

  void BindToNetDevice(Ptr<NetDevice> netdevice);

  Ptr<NetDevice> GetBoundNetDevice() const;

  void SetRxCallback(
      Callback<void, Ptr<Packet>, Ipv6Header, uint16_t, Ptr<Ipv6Interface>>
          callback);

  void SetIcmpCallback(
      Callback<void, Ipv6Address, uint8_t, uint8_t, uint8_t, uint32_t>
          callback);

  void SetDestroyCallback(Callback<void> callback);

  void ForwardUp(Ptr<Packet> p, Ipv6Header header, uint16_t port,
                 Ptr<Ipv6Interface> incomingInterface);

  void ForwardIcmp(Ipv6Address src, uint8_t ttl, uint8_t type, uint8_t code,
                   uint32_t info);

  void SetRxEnabled(bool enabled);

  bool IsRxEnabled() const;

private:
  Ipv6Address m_localAddr;

  uint16_t m_localPort;

  Ipv6Address m_peerAddr;

  uint16_t m_peerPort;

  Ptr<NetDevice> m_boundnetdevice;

  Callback<void, Ptr<Packet>, Ipv6Header, uint16_t, Ptr<Ipv6Interface>>
      m_rxCallback;

  Callback<void, Ipv6Address, uint8_t, uint8_t, uint8_t, uint32_t>
      m_icmpCallback;

  Callback<void> m_destroyCallback;

  bool m_rxEnabled;
};

} // namespace ns3

#endif
