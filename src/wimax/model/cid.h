
#ifndef CID_H
#define CID_H

#include <ostream>
#include <stdint.h>

namespace ns3 {

class Cid {
public:
  enum Type {
    BROADCAST = 1,
    INITIAL_RANGING,
    BASIC,
    PRIMARY,
    TRANSPORT,
    MULTICAST,
    PADDING
  };

  Cid();
  Cid(uint16_t cid);
  ~Cid();
  uint16_t GetIdentifier() const;
  bool IsMulticast() const;
  bool IsBroadcast() const;
  bool IsPadding() const;
  bool IsInitialRanging() const;
  static Cid Broadcast();
  static Cid Padding();
  static Cid InitialRanging();

private:
  friend class CidFactory;
  friend bool operator==(const Cid &lhs, const Cid &rhs);
  uint16_t m_identifier;
};

bool operator==(const Cid &lhs, const Cid &rhs);
bool operator!=(const Cid &lhs, const Cid &rhs);

std::ostream &operator<<(std::ostream &os, const Cid &cid);

} // namespace ns3

#endif
