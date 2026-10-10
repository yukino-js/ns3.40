
#ifndef IPV4_END_POINT_H
#define IPV4_END_POINT_H

#include "ipv4-header.h"
#include "ipv4-interface.h"

#include "ns3/callback.h"
#include "ns3/ipv4-address.h"
#include "ns3/net-device.h"

#include <stdint.h>

namespace ns3 {

class Header;
class Packet;

class Ipv4EndPoint {
public:
  Ipv4EndPoint(Ipv4Address address, uint16_t port);
  ~Ipv4EndPoint();

  Ipv4Address GetLocalAddress() const;

  void SetLocalAddress(Ipv4Address address);

  uint16_t GetLocalPort() const;

  Ipv4Address GetPeerAddress() const;

  uint16_t GetPeerPort() const;

  void SetPeer(Ipv4Address address, uint16_t port);

  void BindToNetDevice(Ptr<NetDevice> netdevice);

  Ptr<NetDevice> GetBoundNetDevice() const;

  void SetRxCallback(
      Callback<void, Ptr<Packet>, Ipv4Header, uint16_t, Ptr<Ipv4Interface>>
          callback);
  void SetIcmpCallback(
      Callback<void, Ipv4Address, uint8_t, uint8_t, uint8_t, uint32_t>
          callback);
  void SetDestroyCallback(Callback<void> callback);

  void ForwardUp(Ptr<Packet> p, const Ipv4Header &header, uint16_t sport,
                 Ptr<Ipv4Interface> incomingInterface);

  void ForwardIcmp(Ipv4Address icmpSource, uint8_t icmpTtl, uint8_t icmpType,
                   uint8_t icmpCode, uint32_t icmpInfo);

  void SetRxEnabled(bool enabled);

  bool IsRxEnabled() const;

private:
  Ipv4Address m_localAddr;

  uint16_t m_localPort;

  Ipv4Address m_peerAddr;

  uint16_t m_peerPort;

  Ptr<NetDevice> m_boundnetdevice;

  Callback<void, Ptr<Packet>, Ipv4Header, uint16_t, Ptr<Ipv4Interface>>
      m_rxCallback;

  Callback<void, Ipv4Address, uint8_t, uint8_t, uint8_t, uint32_t>
      m_icmpCallback;

  Callback<void> m_destroyCallback;

  bool m_rxEnabled;
};

} // namespace ns3

#endif
