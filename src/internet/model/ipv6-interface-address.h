
#ifndef IPV6_INTERFACE_ADDRESS_H
#define IPV6_INTERFACE_ADDRESS_H

#include "ns3/ipv6-address.h"

#include <stdint.h>

namespace ns3 {

class Ipv6InterfaceAddress {
public:
  enum State_e {
    TENTATIVE,
    DEPRECATED,
    PREFERRED,
    PERMANENT,
    HOMEADDRESS,
    TENTATIVE_OPTIMISTIC,
    INVALID,
  };

  enum Scope_e {
    HOST,
    LINKLOCAL,
    GLOBAL,
  };

  Ipv6InterfaceAddress();

  Ipv6InterfaceAddress(Ipv6Address address);

  Ipv6InterfaceAddress(Ipv6Address address, Ipv6Prefix prefix);

  Ipv6InterfaceAddress(Ipv6Address address, Ipv6Prefix prefix, bool onLink);

  Ipv6InterfaceAddress(const Ipv6InterfaceAddress &o);

  ~Ipv6InterfaceAddress();

  void SetAddress(Ipv6Address address);

  Ipv6Address GetAddress() const;

  Ipv6Prefix GetPrefix() const;

  void SetState(Ipv6InterfaceAddress::State_e state);

  Ipv6InterfaceAddress::State_e GetState() const;

  void SetScope(Ipv6InterfaceAddress::Scope_e scope);

  Ipv6InterfaceAddress::Scope_e GetScope() const;

  bool IsInSameSubnet(Ipv6Address b) const;

  void SetNsDadUid(uint32_t uid);

  uint32_t GetNsDadUid() const;

  void SetOnLink(bool onLink);

  bool GetOnLink() const;

#if 0
  void StartDadTimer (Ptr<Ipv6Interface> interface);

  void StopDadTimer ();
#endif

private:
  Ipv6Address m_address;

  Ipv6Prefix m_prefix;

  State_e m_state;

  Scope_e m_scope;

  bool m_onLink;

  friend bool operator==(const Ipv6InterfaceAddress &a,
                         const Ipv6InterfaceAddress &b);

  friend bool operator!=(const Ipv6InterfaceAddress &a,
                         const Ipv6InterfaceAddress &b);

  uint32_t m_nsDadUid;
};

std::ostream &operator<<(std::ostream &os, const Ipv6InterfaceAddress &addr);

inline bool operator==(const Ipv6InterfaceAddress &a,
                       const Ipv6InterfaceAddress &b) {
  return (a.m_address == b.m_address && a.m_prefix == b.m_prefix &&
          a.m_state == b.m_state && a.m_scope == b.m_scope);
}

inline bool operator!=(const Ipv6InterfaceAddress &a,
                       const Ipv6InterfaceAddress &b) {
  return (a.m_address != b.m_address || a.m_prefix != b.m_prefix ||
          a.m_state != b.m_state || a.m_scope != b.m_scope);
}

} // namespace ns3

#endif
