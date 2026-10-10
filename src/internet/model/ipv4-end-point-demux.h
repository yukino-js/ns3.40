
#ifndef IPV4_END_POINT_DEMUX_H
#define IPV4_END_POINT_DEMUX_H

#include "ipv4-interface.h"

#include "ns3/ipv4-address.h"

#include <list>
#include <stdint.h>

namespace ns3 {

class Ipv4EndPoint;

class Ipv4EndPointDemux {
public:
  typedef std::list<Ipv4EndPoint *> EndPoints;

  typedef std::list<Ipv4EndPoint *>::iterator EndPointsI;

  Ipv4EndPointDemux();
  ~Ipv4EndPointDemux();

  EndPoints GetAllEndPoints();

  bool LookupPortLocal(uint16_t port);

  bool LookupLocal(Ptr<NetDevice> boundNetDevice, Ipv4Address addr,
                   uint16_t port);

  EndPoints Lookup(Ipv4Address daddr, uint16_t dport, Ipv4Address saddr,
                   uint16_t sport, Ptr<Ipv4Interface> incomingInterface);

  Ipv4EndPoint *SimpleLookup(Ipv4Address daddr, uint16_t dport,
                             Ipv4Address saddr, uint16_t sport);

  Ipv4EndPoint *Allocate();

  Ipv4EndPoint *Allocate(Ipv4Address address);

  Ipv4EndPoint *Allocate(Ptr<NetDevice> boundNetDevice, uint16_t port);

  Ipv4EndPoint *Allocate(Ptr<NetDevice> boundNetDevice, Ipv4Address address,
                         uint16_t port);

  Ipv4EndPoint *Allocate(Ptr<NetDevice> boundNetDevice,
                         Ipv4Address localAddress, uint16_t localPort,
                         Ipv4Address peerAddress, uint16_t peerPort);

  void DeAllocate(Ipv4EndPoint *endPoint);

private:
  uint16_t AllocateEphemeralPort();

  uint16_t m_ephemeral;

  uint16_t m_portLast;

  uint16_t m_portFirst;

  EndPoints m_endPoints;
};

} // namespace ns3

#endif
