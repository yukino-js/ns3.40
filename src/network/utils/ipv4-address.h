
#ifndef IPV4_ADDRESS_H
#define IPV4_ADDRESS_H

#include "ns3/address.h"
#include "ns3/attribute-helper.h"

#include <ostream>
#include <stdint.h>

namespace ns3 {

class Ipv4Mask;

class Ipv4Address {
public:
  Ipv4Address();
  explicit Ipv4Address(uint32_t address);
  Ipv4Address(const char *address);
  uint32_t Get() const;
  void Set(uint32_t address);
  void Set(const char *address);
  void Serialize(uint8_t buf[4]) const;
  static Ipv4Address Deserialize(const uint8_t buf[4]);
  void Print(std::ostream &os) const;

  bool IsInitialized() const;
  bool IsAny() const;
  bool IsLocalhost() const;
  bool IsBroadcast() const;
  bool IsMulticast() const;
  bool IsLocalMulticast() const;
  Ipv4Address CombineMask(const Ipv4Mask &mask) const;
  Ipv4Address GetSubnetDirectedBroadcast(const Ipv4Mask &mask) const;
  bool IsSubnetDirectedBroadcast(const Ipv4Mask &mask) const;
  static bool IsMatchingType(const Address &address);
  operator Address() const;
  static Ipv4Address ConvertFrom(const Address &address);
  Address ConvertTo() const;

  static Ipv4Address GetZero();
  static Ipv4Address GetAny();
  static Ipv4Address GetBroadcast();
  static Ipv4Address GetLoopback();

private:
  static uint8_t GetType();
  uint32_t m_address;
  bool m_initialized;

  friend bool operator==(const Ipv4Address &a, const Ipv4Address &b);

  friend bool operator!=(const Ipv4Address &a, const Ipv4Address &b);

  friend bool operator<(const Ipv4Address &a, const Ipv4Address &b);
};

class Ipv4Mask {
public:
  Ipv4Mask();
  Ipv4Mask(uint32_t mask);
  Ipv4Mask(const char *mask);
  bool IsMatch(Ipv4Address a, Ipv4Address b) const;
  uint32_t Get() const;
  void Set(uint32_t mask);
  uint32_t GetInverse() const;
  void Print(std::ostream &os) const;
  uint16_t GetPrefixLength() const;
  static Ipv4Mask GetLoopback();
  static Ipv4Mask GetZero();
  static Ipv4Mask GetOnes();

  friend bool operator==(const Ipv4Mask &a, const Ipv4Mask &b);

  friend bool operator!=(const Ipv4Mask &a, const Ipv4Mask &b);

private:
  uint32_t m_mask;
};

ATTRIBUTE_HELPER_HEADER(Ipv4Address);
ATTRIBUTE_HELPER_HEADER(Ipv4Mask);

std::ostream &operator<<(std::ostream &os, const Ipv4Address &address);
std::ostream &operator<<(std::ostream &os, const Ipv4Mask &mask);
std::istream &operator>>(std::istream &is, Ipv4Address &address);
std::istream &operator>>(std::istream &is, Ipv4Mask &mask);

inline bool operator==(const Ipv4Address &a, const Ipv4Address &b) {
  return (a.m_address == b.m_address);
}

inline bool operator!=(const Ipv4Address &a, const Ipv4Address &b) {
  return (a.m_address != b.m_address);
}

inline bool operator<(const Ipv4Address &a, const Ipv4Address &b) {
  return (a.m_address < b.m_address);
}

class Ipv4AddressHash {
public:
  size_t operator()(const Ipv4Address &x) const;
};

inline bool operator==(const Ipv4Mask &a, const Ipv4Mask &b) {
  return (a.m_mask == b.m_mask);
}

inline bool operator!=(const Ipv4Mask &a, const Ipv4Mask &b) {
  return (a.m_mask != b.m_mask);
}

} // namespace ns3

#endif
