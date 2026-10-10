#ifndef MAC48_ADDRESS_H
#define MAC48_ADDRESS_H

#include "ipv4-address.h"
#include "ipv6-address.h"

#include "ns3/attribute-helper.h"
#include "ns3/attribute.h"

#include <ostream>
#include <stdint.h>

namespace ns3 {

class Address;

class Mac48Address {
public:
  Mac48Address();
  Mac48Address(const char *str);

  void CopyFrom(const uint8_t buffer[6]);
  void CopyTo(uint8_t buffer[6]) const;

  operator Address() const;
  static Mac48Address ConvertFrom(const Address &address);
  Address ConvertTo() const;

  static bool IsMatchingType(const Address &address);
  static Mac48Address Allocate();

  static void ResetAllocationIndex();

  bool IsBroadcast() const;

  bool IsGroup() const;

  static Mac48Address GetBroadcast();

  static Mac48Address GetMulticast(Ipv4Address address);

  static Mac48Address GetMulticast(Ipv6Address address);

  static Mac48Address GetMulticastPrefix();

  static Mac48Address GetMulticast6Prefix();

  typedef void (*TracedCallback)(Mac48Address value);

private:
  static uint8_t GetType();

  friend bool operator==(const Mac48Address &a, const Mac48Address &b);

  friend bool operator!=(const Mac48Address &a, const Mac48Address &b);

  friend bool operator<(const Mac48Address &a, const Mac48Address &b);

  friend std::ostream &operator<<(std::ostream &os,
                                  const Mac48Address &address);

  friend std::istream &operator>>(std::istream &is, Mac48Address &address);

  static uint64_t m_allocationIndex;
  uint8_t m_address[6];
};

ATTRIBUTE_HELPER_HEADER(Mac48Address);

inline bool operator==(const Mac48Address &a, const Mac48Address &b) {
  return memcmp(a.m_address, b.m_address, 6) == 0;
}

inline bool operator!=(const Mac48Address &a, const Mac48Address &b) {
  return memcmp(a.m_address, b.m_address, 6) != 0;
}

inline bool operator<(const Mac48Address &a, const Mac48Address &b) {
  return memcmp(a.m_address, b.m_address, 6) < 0;
}

std::ostream &operator<<(std::ostream &os, const Mac48Address &address);
std::istream &operator>>(std::istream &is, Mac48Address &address);

} // namespace ns3

#endif
