
#ifndef IPV6_ADDRESS_GENERATOR_H
#define IPV6_ADDRESS_GENERATOR_H

#include "ns3/ipv6-address.h"

namespace ns3 {

class Ipv6AddressGenerator {
public:
  static void Init(const Ipv6Address net, const Ipv6Prefix prefix,
                   const Ipv6Address interfaceId = "::1");

  static Ipv6Address NextNetwork(const Ipv6Prefix prefix);

  static Ipv6Address GetNetwork(const Ipv6Prefix prefix);

  static void InitAddress(const Ipv6Address interfaceId,
                          const Ipv6Prefix prefix);

  static Ipv6Address NextAddress(const Ipv6Prefix prefix);

  static Ipv6Address GetAddress(const Ipv6Prefix prefix);

  static void Reset();

  static bool AddAllocated(const Ipv6Address addr);

  static bool IsAddressAllocated(const Ipv6Address addr);

  static bool IsNetworkAllocated(const Ipv6Address addr,
                                 const Ipv6Prefix prefix);

  static void TestMode();
};

}; // namespace ns3

#endif
