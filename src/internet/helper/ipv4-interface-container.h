
#ifndef IPV4_INTERFACE_CONTAINER_H
#define IPV4_INTERFACE_CONTAINER_H

#include "ns3/ipv4-address.h"
#include "ns3/ipv4.h"

#include <stdint.h>
#include <vector>

namespace ns3 {

class Ipv4InterfaceContainer {
public:
  typedef std::vector<std::pair<Ptr<Ipv4>, uint32_t>>::const_iterator Iterator;

  Ipv4InterfaceContainer();

  void Add(const Ipv4InterfaceContainer &other);

  Iterator Begin() const;

  Iterator End() const;

  uint32_t GetN() const;

  Ipv4Address GetAddress(uint32_t i, uint32_t j = 0) const;

  void SetMetric(uint32_t i, uint16_t metric);

  void Add(Ptr<Ipv4> ipv4, uint32_t interface);

  void Add(std::pair<Ptr<Ipv4>, uint32_t> ipInterfacePair);

  void Add(std::string ipv4Name, uint32_t interface);

  std::pair<Ptr<Ipv4>, uint32_t> Get(uint32_t i) const;

private:
  typedef std::vector<std::pair<Ptr<Ipv4>, uint32_t>> InterfaceVector;

  InterfaceVector m_interfaces;
};

} // namespace ns3

#endif
