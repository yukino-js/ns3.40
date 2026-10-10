
#ifndef IPV6_INTERFACE_CONTAINER_H
#define IPV6_INTERFACE_CONTAINER_H

#include "ns3/ipv6-address.h"
#include "ns3/ipv6.h"

#include <stdint.h>
#include <vector>

namespace ns3 {

class Ipv6InterfaceContainer {
public:
  typedef std::vector<std::pair<Ptr<Ipv6>, uint32_t>>::const_iterator Iterator;

  Ipv6InterfaceContainer();

  uint32_t GetN() const;

  uint32_t GetInterfaceIndex(uint32_t i) const;

  Ipv6Address GetAddress(uint32_t i, uint32_t j) const;

  Ipv6Address GetLinkLocalAddress(uint32_t i);

  Ipv6Address GetLinkLocalAddress(Ipv6Address address);

  void Add(Ptr<Ipv6> ipv6, uint32_t interface);

  Iterator Begin() const;

  Iterator End() const;

  void Add(const Ipv6InterfaceContainer &c);

  void Add(std::string ipv6Name, uint32_t interface);

  std::pair<Ptr<Ipv6>, uint32_t> Get(uint32_t i) const;

  void SetForwarding(uint32_t i, bool state);

  void SetDefaultRouteInAllNodes(uint32_t router);

  void SetDefaultRouteInAllNodes(Ipv6Address routerAddr);

  void SetDefaultRoute(uint32_t i, uint32_t router);

  void SetDefaultRoute(uint32_t i, Ipv6Address routerAddr);

private:
  typedef std::vector<std::pair<Ptr<Ipv6>, uint32_t>> InterfaceVector;

  InterfaceVector m_interfaces;
};

} // namespace ns3

#endif
