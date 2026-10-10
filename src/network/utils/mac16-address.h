#ifndef MAC16_ADDRESS_H
#define MAC16_ADDRESS_H

#include "ipv4-address.h"
#include "ipv6-address.h"

#include "ns3/attribute-helper.h"
#include "ns3/attribute.h"

#include <ostream>
#include <stdint.h>

namespace ns3 {

class Address;

class Mac16Address {
public:
  Mac16Address();
  Mac16Address(const char *str);

  Mac16Address(uint16_t addr);

  void CopyFrom(const uint8_t buffer[2]);

  void CopyTo(uint8_t buffer[2]) const;

  operator Address() const;

  static Mac16Address ConvertFrom(const Address &address);

  Address ConvertTo() const;

  uint16_t ConvertToInt() const;

  static bool IsMatchingType(const Address &address);

  static Mac16Address Allocate();

  static void ResetAllocationIndex();

  static Mac16Address GetBroadcast();

  static Mac16Address GetMulticast(Ipv6Address address);

  bool IsBroadcast() const;

  bool IsMulticast() const;

private:
  static uint8_t GetType();

  friend bool operator==(const Mac16Address &a, const Mac16Address &b);

  friend bool operator!=(const Mac16Address &a, const Mac16Address &b);

  friend bool operator<(const Mac16Address &a, const Mac16Address &b);

  friend std::ostream &operator<<(std::ostream &os,
                                  const Mac16Address &address);

  friend std::istream &operator>>(std::istream &is, Mac16Address &address);

  static uint64_t m_allocationIndex;
  uint8_t m_address[2];
};

ATTRIBUTE_HELPER_HEADER(Mac16Address);

inline bool operator==(const Mac16Address &a, const Mac16Address &b) {
  return memcmp(a.m_address, b.m_address, 2) == 0;
}

inline bool operator!=(const Mac16Address &a, const Mac16Address &b) {
  return memcmp(a.m_address, b.m_address, 2) != 0;
}

inline bool operator<(const Mac16Address &a, const Mac16Address &b) {
  return memcmp(a.m_address, b.m_address, 2) < 0;
}

std::ostream &operator<<(std::ostream &os, const Mac16Address &address);
std::istream &operator>>(std::istream &is, Mac16Address &address);

} // namespace ns3

#endif
