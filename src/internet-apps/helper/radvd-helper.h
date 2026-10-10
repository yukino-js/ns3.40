
#ifndef RADVD_HELPER_H
#define RADVD_HELPER_H

#include "ns3/application-container.h"
#include "ns3/ipv6-address.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/radvd-interface.h"

#include <list>
#include <map>
#include <stdint.h>

namespace ns3 {

class RadvdHelper {
public:
  RadvdHelper();

  void AddAnnouncedPrefix(uint32_t interface, Ipv6Address prefix,
                          uint32_t prefixLength);

  void EnableDefaultRouterForInterface(uint32_t interface);

  void DisableDefaultRouterForInterface(uint32_t interface);

  Ptr<RadvdInterface> GetRadvdInterface(uint32_t interface);

  void ClearPrefixes();

  void SetAttribute(std::string name, const AttributeValue &value);

  ApplicationContainer Install(Ptr<Node> node);

private:
  ObjectFactory m_factory;

  typedef std::map<uint32_t, Ptr<RadvdInterface>> RadvdInterfaceMap;
  typedef std::map<uint32_t, Ptr<RadvdInterface>>::iterator RadvdInterfaceMapI;

  RadvdInterfaceMap m_radvdInterfaces;
};

} // namespace ns3

#endif
