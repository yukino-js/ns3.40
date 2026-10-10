
#ifndef PING6_HELPER_H
#define PING6_HELPER_H

#include "ns3/application-container.h"
#include "ns3/ipv6-address.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"

#include <stdint.h>

namespace ns3 {

class Ping6Helper {
public:
  NS_DEPRECATED_3_38(
      "Use PingHelper instead - the attributes might have been renamed.")
  Ping6Helper();

  void SetLocal(Ipv6Address ip);

  void SetRemote(Ipv6Address ip);

  void SetAttribute(std::string name, const AttributeValue &value);

  ApplicationContainer Install(NodeContainer c);

  NS_DEPRECATED_3_38("Use a source address")
  void SetIfIndex(uint32_t ifIndex);

  void SetRoutersAddress(std::vector<Ipv6Address> routers);

private:
  ObjectFactory m_factory;

  Ipv6Address m_localIp;

  Ipv6Address m_remoteIp;

  uint32_t m_ifIndex;

  std::vector<Ipv6Address> m_routers;
};

} // namespace ns3

#endif
