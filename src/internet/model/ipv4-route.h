#ifndef IPV4_ROUTE_H
#define IPV4_ROUTE_H

#include "ns3/ipv4-address.h"
#include "ns3/simple-ref-count.h"

#include <list>
#include <map>
#include <ostream>

namespace ns3 {

class NetDevice;

class Ipv4Route : public SimpleRefCount<Ipv4Route> {
public:
  Ipv4Route();

  void SetDestination(Ipv4Address dest);
  Ipv4Address GetDestination() const;

  void SetSource(Ipv4Address src);
  Ipv4Address GetSource() const;

  void SetGateway(Ipv4Address gw);
  Ipv4Address GetGateway() const;

  void SetOutputDevice(Ptr<NetDevice> outputDevice);
  Ptr<NetDevice> GetOutputDevice() const;

#ifdef NOTYET
  void SetInputIfIndex(uint32_t iif);
  uint32_t GetInputIfIndex() const;
#endif

private:
  Ipv4Address m_dest;
  Ipv4Address m_source;
  Ipv4Address m_gateway;
  Ptr<NetDevice> m_outputDevice;
#ifdef NOTYET
  uint32_t m_inputIfIndex;
#endif
};

std::ostream &operator<<(std::ostream &os, const Ipv4Route &route);

class Ipv4MulticastRoute : public SimpleRefCount<Ipv4MulticastRoute> {
public:
  Ipv4MulticastRoute();

  void SetGroup(const Ipv4Address group);
  Ipv4Address GetGroup() const;

  void SetOrigin(const Ipv4Address origin);
  Ipv4Address GetOrigin() const;

  void SetParent(uint32_t iif);
  uint32_t GetParent() const;

  void SetOutputTtl(uint32_t oif, uint32_t ttl);

  std::map<uint32_t, uint32_t> GetOutputTtlMap() const;

  static const uint32_t MAX_INTERFACES = 16;
  static const uint32_t MAX_TTL = 255;

private:
  Ipv4Address m_group;
  Ipv4Address m_origin;
  uint32_t m_parent;
  std::map<uint32_t, uint32_t> m_ttls;
};

} // namespace ns3

#endif
