
#ifndef IPV6_ADDRESS_HELPER_H
#define IPV6_ADDRESS_HELPER_H

#include "ipv6-interface-container.h"

#include "ns3/ipv6-address.h"
#include "ns3/net-device-container.h"

#include <vector>

namespace ns3 {

class Ipv6AddressHelper {
public:
  Ipv6AddressHelper();

  Ipv6AddressHelper(Ipv6Address network, Ipv6Prefix prefix,
                    Ipv6Address base = Ipv6Address("::1"));

  void SetBase(Ipv6Address network, Ipv6Prefix prefix,
               Ipv6Address base = Ipv6Address("::1"));

  void NewNetwork();

  Ipv6Address NewAddress(Address addr);

  Ipv6Address NewAddress();

  Ipv6InterfaceContainer Assign(const NetDeviceContainer &c);

  Ipv6InterfaceContainer Assign(const NetDeviceContainer &c,
                                std::vector<bool> withConfiguration);

  Ipv6InterfaceContainer Assign(const NetDeviceContainer &c,
                                std::vector<bool> withConfiguration,
                                std::vector<bool> onLink);

  Ipv6InterfaceContainer AssignWithoutAddress(const NetDeviceContainer &c);

  Ipv6InterfaceContainer AssignWithoutOnLink(const NetDeviceContainer &c);

private:
  Ipv6Address m_network;
  Ipv6Prefix m_prefix;
  Ipv6Address m_address;
  Ipv6Address m_base;
};

} // namespace ns3

#endif
