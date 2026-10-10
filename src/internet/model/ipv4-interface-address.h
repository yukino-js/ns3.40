
#ifndef IPV4_INTERFACE_ADDRESS_H
#define IPV4_INTERFACE_ADDRESS_H

#include "ns3/ipv4-address.h"

#include <ostream>
#include <stdint.h>

namespace ns3 {

class Ipv4InterfaceAddress {
public:
  enum InterfaceAddressScope_e { HOST, LINK, GLOBAL };

  Ipv4InterfaceAddress();

  Ipv4InterfaceAddress(Ipv4Address local, Ipv4Mask mask);
  Ipv4InterfaceAddress(const Ipv4InterfaceAddress &o);

  void SetLocal(Ipv4Address local);

  void SetAddress(Ipv4Address address);

  Ipv4Address GetLocal() const;

  Ipv4Address GetAddress() const;

  void SetMask(Ipv4Mask mask);
  Ipv4Mask GetMask() const;
  void SetBroadcast(Ipv4Address broadcast);
  Ipv4Address GetBroadcast() const;

  void SetScope(Ipv4InterfaceAddress::InterfaceAddressScope_e scope);

  Ipv4InterfaceAddress::InterfaceAddressScope_e GetScope() const;

  bool IsInSameSubnet(const Ipv4Address b) const;

  bool IsSecondary() const;

  void SetSecondary();
  void SetPrimary();

private:
  Ipv4Address m_local;
  Ipv4Mask m_mask;
  Ipv4Address m_broadcast;

  InterfaceAddressScope_e m_scope;
  bool m_secondary;

  friend bool operator==(const Ipv4InterfaceAddress &a,
                         const Ipv4InterfaceAddress &b);

  friend bool operator!=(const Ipv4InterfaceAddress &a,
                         const Ipv4InterfaceAddress &b);
};

std::ostream &operator<<(std::ostream &os, const Ipv4InterfaceAddress &addr);

inline bool operator==(const Ipv4InterfaceAddress &a,
                       const Ipv4InterfaceAddress &b) {
  return (a.m_local == b.m_local && a.m_mask == b.m_mask &&
          a.m_broadcast == b.m_broadcast && a.m_scope == b.m_scope &&
          a.m_secondary == b.m_secondary);
}

inline bool operator!=(const Ipv4InterfaceAddress &a,
                       const Ipv4InterfaceAddress &b) {
  return (a.m_local != b.m_local || a.m_mask != b.m_mask ||
          a.m_broadcast != b.m_broadcast || a.m_scope != b.m_scope ||
          a.m_secondary != b.m_secondary);
}

} // namespace ns3

#endif
