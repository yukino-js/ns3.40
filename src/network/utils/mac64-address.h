#ifndef MAC64_ADDRESS_H
#define MAC64_ADDRESS_H

#include "ipv4-address.h"
#include "ipv6-address.h"

#include "ns3/attribute-helper.h"
#include "ns3/attribute.h"

#include <ostream>
#include <stdint.h>

namespace ns3 {

class Address;

class Mac64Address {
public:
  Mac64Address();
  Mac64Address(const char *str);

  Mac64Address(uint64_t addr);

  void CopyFrom(const uint8_t buffer[8]);
  void CopyTo(uint8_t buffer[8]) const;
  operator Address() const;
  static Mac64Address ConvertFrom(const Address &address);
  Address ConvertTo() const;

  uint64_t ConvertToInt() const;

  static bool IsMatchingType(const Address &address);
  static Mac64Address Allocate();

  static void ResetAllocationIndex();

private:
  static uint8_t GetType();

  friend bool operator==(const Mac64Address &a, const Mac64Address &b);

  friend bool operator!=(const Mac64Address &a, const Mac64Address &b);

  friend bool operator<(const Mac64Address &a, const Mac64Address &b);

  friend std::ostream &operator<<(std::ostream &os,
                                  const Mac64Address &address);

  friend std::istream &operator>>(std::istream &is, Mac64Address &address);

  static uint64_t m_allocationIndex;
  uint8_t m_address[8];
};

ATTRIBUTE_HELPER_HEADER(Mac64Address);

inline bool operator==(const Mac64Address &a, const Mac64Address &b) {
  return memcmp(a.m_address, b.m_address, 8) == 0;
}

inline bool operator!=(const Mac64Address &a, const Mac64Address &b) {
  return memcmp(a.m_address, b.m_address, 8) != 0;
}

inline bool operator<(const Mac64Address &a, const Mac64Address &b) {
  return memcmp(a.m_address, b.m_address, 8) < 0;
}

std::ostream &operator<<(std::ostream &os, const Mac64Address &address);
std::istream &operator>>(std::istream &is, Mac64Address &address);

} // namespace ns3

#endif
