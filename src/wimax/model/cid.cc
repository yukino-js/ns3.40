
#include "cid.h"

#define CID_UNINITIALIZED 60000

namespace ns3 {

Cid::Cid() { m_identifier = CID_UNINITIALIZED; }

Cid::Cid(uint16_t identifier) { m_identifier = identifier; }

Cid::~Cid() {}

uint16_t Cid::GetIdentifier() const { return m_identifier; }

bool Cid::IsMulticast() const {
  return m_identifier >= 0xff00 && m_identifier <= 0xfffd;
}

bool Cid::IsBroadcast() const { return *this == Broadcast(); }

bool Cid::IsPadding() const { return *this == Padding(); }

bool Cid::IsInitialRanging() const { return *this == InitialRanging(); }

Cid Cid::Broadcast() { return 0xffff; }

Cid Cid::Padding() { return 0xfffe; }

Cid Cid::InitialRanging() { return 0; }

bool operator==(const Cid &lhs, const Cid &rhs) {
  return lhs.m_identifier == rhs.m_identifier;
}

bool operator!=(const Cid &lhs, const Cid &rhs) { return !(lhs == rhs); }

std::ostream &operator<<(std::ostream &os, const Cid &cid) {
  os << cid.GetIdentifier();
  return os;
}

} // namespace ns3
