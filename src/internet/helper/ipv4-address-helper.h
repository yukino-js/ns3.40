
#ifndef IPV4_ADDRESS_HELPER_H
#define IPV4_ADDRESS_HELPER_H

#include "ipv4-interface-container.h"

#include "ns3/ipv4-address.h"
#include "ns3/net-device-container.h"

namespace ns3 {

class Ipv4AddressHelper {
public:
  Ipv4AddressHelper();

  Ipv4AddressHelper(Ipv4Address network, Ipv4Mask mask,
                    Ipv4Address base = "0.0.0.1");

  void SetBase(Ipv4Address network, Ipv4Mask mask,
               Ipv4Address base = "0.0.0.1");

  Ipv4Address NewNetwork();

  Ipv4Address NewAddress();

  Ipv4InterfaceContainer Assign(const NetDeviceContainer &c);

private:
  uint32_t NumAddressBits(uint32_t maskbits) const;

  uint32_t m_network;
  uint32_t m_mask;
  uint32_t m_address;
  uint32_t m_base;
  uint32_t m_shift;
  uint32_t m_max;
};

} // namespace ns3

#endif
