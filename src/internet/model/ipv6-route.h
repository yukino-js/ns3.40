
#ifndef IPV6_ROUTE_H
#define IPV6_ROUTE_H

#include "ns3/ipv6-address.h"
#include "ns3/simple-ref-count.h"

#include <list>
#include <map>
#include <ostream>

namespace ns3 {

class NetDevice;

class Ipv6Route : public SimpleRefCount<Ipv6Route> {
public:
  Ipv6Route();

  virtual ~Ipv6Route();

  void SetDestination(Ipv6Address dest);

  Ipv6Address GetDestination() const;

  void SetSource(Ipv6Address src);

  Ipv6Address GetSource() const;

  void SetGateway(Ipv6Address gw);

  Ipv6Address GetGateway() const;

  void SetOutputDevice(Ptr<NetDevice> outputDevice);

  Ptr<NetDevice> GetOutputDevice() const;

private:
  Ipv6Address m_dest;

  Ipv6Address m_source;

  Ipv6Address m_gateway;

  Ptr<NetDevice> m_outputDevice;
};

std::ostream &operator<<(std::ostream &os, const Ipv6Route &route);

class Ipv6MulticastRoute : public SimpleRefCount<Ipv6MulticastRoute> {
public:
  static const uint32_t MAX_INTERFACES = 16;

  static const uint32_t MAX_TTL = 255;

  Ipv6MulticastRoute();

  virtual ~Ipv6MulticastRoute();

  void SetGroup(const Ipv6Address group);

  Ipv6Address GetGroup() const;

  void SetOrigin(const Ipv6Address origin);

  Ipv6Address GetOrigin() const;

  void SetParent(uint32_t iif);

  uint32_t GetParent() const;

  void SetOutputTtl(uint32_t oif, uint32_t ttl);

  std::map<uint32_t, uint32_t> GetOutputTtlMap() const;

private:
  Ipv6Address m_group;

  Ipv6Address m_origin;

  uint32_t m_parent;

  std::map<uint32_t, uint32_t> m_ttls;
};

std::ostream &operator<<(std::ostream &os, const Ipv6MulticastRoute &route);

} // namespace ns3

#endif
