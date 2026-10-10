
#ifndef IPV6_ADDRESS_H
#define IPV6_ADDRESS_H

#include "ipv4-address.h"
#include "mac8-address.h"

#include "ns3/address.h"
#include "ns3/attribute-helper.h"

#include <cstring>
#include <ostream>
#include <stdint.h>

namespace ns3 {

class Ipv6Prefix;
class Mac16Address;
class Mac48Address;
class Mac64Address;

class Ipv6Address {
public:
  Ipv6Address();

  Ipv6Address(const char *address);

  Ipv6Address(uint8_t address[16]);

  Ipv6Address(const Ipv6Address &addr);

  Ipv6Address(const Ipv6Address *addr);

  ~Ipv6Address();

  void Set(const char *address);

  void Set(uint8_t address[16]);

  void Serialize(uint8_t buf[16]) const;

  static Ipv6Address Deserialize(const uint8_t buf[16]);

  static Ipv6Address MakeSolicitedAddress(Ipv6Address addr);

  static Ipv6Address MakeIpv4MappedAddress(Ipv4Address addr);

  Ipv4Address GetIpv4MappedAddress() const;

  static Ipv6Address MakeAutoconfiguredAddress(Address addr,
                                               Ipv6Address prefix);

  static Ipv6Address MakeAutoconfiguredAddress(Address addr, Ipv6Prefix prefix);

  static Ipv6Address MakeAutoconfiguredAddress(Mac16Address addr,
                                               Ipv6Address prefix);

  static Ipv6Address MakeAutoconfiguredAddress(Mac48Address addr,
                                               Ipv6Address prefix);

  static Ipv6Address MakeAutoconfiguredAddress(Mac64Address addr,
                                               Ipv6Address prefix);

  static Ipv6Address MakeAutoconfiguredAddress(Mac8Address addr,
                                               Ipv6Address prefix);

  static Ipv6Address MakeAutoconfiguredLinkLocalAddress(Address mac);

  static Ipv6Address MakeAutoconfiguredLinkLocalAddress(Mac16Address mac);

  static Ipv6Address MakeAutoconfiguredLinkLocalAddress(Mac48Address mac);

  static Ipv6Address MakeAutoconfiguredLinkLocalAddress(Mac64Address mac);

  static Ipv6Address MakeAutoconfiguredLinkLocalAddress(Mac8Address mac);

  void Print(std::ostream &os) const;

  bool IsLocalhost() const;

  bool IsMulticast() const;

  bool IsLinkLocalMulticast() const;

  bool IsAllNodesMulticast() const;

  bool IsAllRoutersMulticast() const;

  bool IsLinkLocal() const;

  bool IsSolicitedMulticast() const;

  bool IsAny() const;

  bool IsDocumentation() const;

  bool HasPrefix(const Ipv6Prefix &prefix) const;

  Ipv6Address CombinePrefix(const Ipv6Prefix &prefix) const;

  static bool IsMatchingType(const Address &address);

  bool IsIpv4MappedAddress() const;

  operator Address() const;

  static Ipv6Address ConvertFrom(const Address &address);

  Address ConvertTo() const;

  bool IsInitialized() const;

  static Ipv6Address GetZero();

  static Ipv6Address GetAny();

  static Ipv6Address GetAllNodesMulticast();

  static Ipv6Address GetAllRoutersMulticast();

  static Ipv6Address GetAllHostsMulticast();

  static Ipv6Address GetLoopback();

  static Ipv6Address GetOnes();

  void GetBytes(uint8_t buf[16]) const;

private:
  static uint8_t GetType();

  uint8_t m_address[16];
  bool m_initialized;

  friend bool operator==(const Ipv6Address &a, const Ipv6Address &b);

  friend bool operator!=(const Ipv6Address &a, const Ipv6Address &b);

  friend bool operator<(const Ipv6Address &a, const Ipv6Address &b);
};

class Ipv6Prefix {
public:
  Ipv6Prefix();

  Ipv6Prefix(uint8_t prefix[16]);

  Ipv6Prefix(const char *prefix);

  Ipv6Prefix(uint8_t prefix[16], uint8_t prefixLength);

  Ipv6Prefix(const char *prefix, uint8_t prefixLength);

  Ipv6Prefix(uint8_t prefix);

  Ipv6Prefix(const Ipv6Prefix &prefix);

  Ipv6Prefix(const Ipv6Prefix *prefix);

  ~Ipv6Prefix();

  bool IsMatch(Ipv6Address a, Ipv6Address b) const;

  void GetBytes(uint8_t buf[16]) const;

  Ipv6Address ConvertToIpv6Address() const;

  uint8_t GetPrefixLength() const;

  void SetPrefixLength(uint8_t prefixLength);

  uint8_t GetMinimumPrefixLength() const;

  void Print(std::ostream &os) const;

  static Ipv6Prefix GetLoopback();

  static Ipv6Prefix GetOnes();

  static Ipv6Prefix GetZero();

private:
  uint8_t m_prefix[16];

  uint8_t m_prefixLength;

  friend bool operator==(const Ipv6Prefix &a, const Ipv6Prefix &b);

  friend bool operator!=(const Ipv6Prefix &a, const Ipv6Prefix &b);
};

ATTRIBUTE_HELPER_HEADER(Ipv6Address);
ATTRIBUTE_HELPER_HEADER(Ipv6Prefix);

std::ostream &operator<<(std::ostream &os, const Ipv6Address &address);

std::ostream &operator<<(std::ostream &os, const Ipv6Prefix &prefix);

std::istream &operator>>(std::istream &is, Ipv6Address &address);

std::istream &operator>>(std::istream &is, Ipv6Prefix &prefix);

inline bool operator==(const Ipv6Address &a, const Ipv6Address &b) {
  return (!std::memcmp(a.m_address, b.m_address, 16));
}

inline bool operator!=(const Ipv6Address &a, const Ipv6Address &b) {
  return std::memcmp(a.m_address, b.m_address, 16);
}

inline bool operator<(const Ipv6Address &a, const Ipv6Address &b) {
  return (std::memcmp(a.m_address, b.m_address, 16) < 0);
}

inline bool operator==(const Ipv6Prefix &a, const Ipv6Prefix &b) {
  return (!std::memcmp(a.m_prefix, b.m_prefix, 16));
}

inline bool operator!=(const Ipv6Prefix &a, const Ipv6Prefix &b) {
  return std::memcmp(a.m_prefix, b.m_prefix, 16);
}

class Ipv6AddressHash {
public:
  size_t operator()(const Ipv6Address &x) const;
};

} // namespace ns3

#endif
