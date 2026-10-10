
#ifndef IPV6_END_POINT_DEMUX_H
#define IPV6_END_POINT_DEMUX_H

#include "ipv6-interface.h"

#include "ns3/ipv6-address.h"

#include <list>
#include <stdint.h>

namespace ns3 {

class Ipv6EndPoint;

class Ipv6EndPointDemux {
public:
  typedef std::list<Ipv6EndPoint *> EndPoints;

  typedef std::list<Ipv6EndPoint *>::iterator EndPointsI;

  Ipv6EndPointDemux();
  ~Ipv6EndPointDemux();

  bool LookupPortLocal(uint16_t port);

  bool LookupLocal(Ptr<NetDevice> boundNetDevice, Ipv6Address addr,
                   uint16_t port);

  EndPoints Lookup(Ipv6Address dst, uint16_t dport, Ipv6Address src,
                   uint16_t sport, Ptr<Ipv6Interface> incomingInterface);

  Ipv6EndPoint *SimpleLookup(Ipv6Address dst, uint16_t dport, Ipv6Address src,
                             uint16_t sport);

  Ipv6EndPoint *Allocate();

  Ipv6EndPoint *Allocate(Ipv6Address address);

  Ipv6EndPoint *Allocate(Ptr<NetDevice> boundNetDevice, uint16_t port);

  Ipv6EndPoint *Allocate(Ptr<NetDevice> boundNetDevice, Ipv6Address address,
                         uint16_t port);

  Ipv6EndPoint *Allocate(Ptr<NetDevice> boundNetDevice,
                         Ipv6Address localAddress, uint16_t localPort,
                         Ipv6Address peerAddress, uint16_t peerPort);

  void DeAllocate(Ipv6EndPoint *endPoint);

  EndPoints GetEndPoints() const;

private:
  uint16_t AllocateEphemeralPort();

  uint16_t m_ephemeral;

  uint16_t m_portFirst;

  uint16_t m_portLast;

  EndPoints m_endPoints;
};

} // namespace ns3

#endif
