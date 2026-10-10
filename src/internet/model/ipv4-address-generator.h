
#ifndef IPV4_ADDRESS_GENERATOR_H
#define IPV4_ADDRESS_GENERATOR_H

#include "ns3/ipv4-address.h"

namespace ns3 {

class Ipv4AddressGenerator {
public:
  static void Init(const Ipv4Address net, const Ipv4Mask mask,
                   const Ipv4Address addr = "0.0.0.1");

  static Ipv4Address NextNetwork(const Ipv4Mask mask);

  static Ipv4Address GetNetwork(const Ipv4Mask mask);

  static void InitAddress(const Ipv4Address addr, const Ipv4Mask mask);

  static Ipv4Address NextAddress(const Ipv4Mask mask);

  static Ipv4Address GetAddress(const Ipv4Mask mask);

  static void Reset();

  static bool AddAllocated(const Ipv4Address addr);

  static bool IsAddressAllocated(const Ipv4Address addr);

  static bool IsNetworkAllocated(const Ipv4Address addr, const Ipv4Mask mask);

  static void TestMode();
};

} // namespace ns3

#endif
