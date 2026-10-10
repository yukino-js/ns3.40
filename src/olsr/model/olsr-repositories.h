
#ifndef OLSR_REPOSITORIES_H
#define OLSR_REPOSITORIES_H

#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"

#include <iostream>
#include <set>
#include <vector>

namespace ns3 {
namespace olsr {

enum Willingness : uint8_t {
  NEVER = 0,
  LOW = 1,
  DEFAULT = 3,
  HIGH = 6,
  ALWAYS = 7,
};

inline std::ostream &operator<<(std::ostream &os, Willingness willingness) {
  switch (willingness) {
  case Willingness::NEVER:
    return (os << "NEVER");
  case Willingness::LOW:
    return (os << "LOW");
  case Willingness::DEFAULT:
    return (os << "DEFAULT");
  case Willingness::HIGH:
    return (os << "HIGH");
  case Willingness::ALWAYS:
    return (os << "ALWAYS");
  default:
    return (os << static_cast<uint32_t>(willingness));
  }
  return os;
}

struct IfaceAssocTuple {
  Ipv4Address ifaceAddr;
  Ipv4Address mainAddr;
  Time time;
};

inline bool operator==(const IfaceAssocTuple &a, const IfaceAssocTuple &b) {
  return (a.ifaceAddr == b.ifaceAddr && a.mainAddr == b.mainAddr);
}

inline std::ostream &operator<<(std::ostream &os,
                                const IfaceAssocTuple &tuple) {
  os << "IfaceAssocTuple(ifaceAddr=" << tuple.ifaceAddr
     << ", mainAddr=" << tuple.mainAddr << ", time=" << tuple.time << ")";
  return os;
}

struct LinkTuple {
  Ipv4Address localIfaceAddr;
  Ipv4Address neighborIfaceAddr;
  Time symTime;
  Time asymTime;
  Time time;
};

inline bool operator==(const LinkTuple &a, const LinkTuple &b) {
  return (a.localIfaceAddr == b.localIfaceAddr &&
          a.neighborIfaceAddr == b.neighborIfaceAddr);
}

inline std::ostream &operator<<(std::ostream &os, const LinkTuple &tuple) {
  os << "LinkTuple(localIfaceAddr=" << tuple.localIfaceAddr
     << ", neighborIfaceAddr=" << tuple.neighborIfaceAddr
     << ", symTime=" << tuple.symTime << ", asymTime=" << tuple.asymTime
     << ", expTime=" << tuple.time << ")";
  return os;
}

struct NeighborTuple {
  Ipv4Address neighborMainAddr;

  enum Status {
    STATUS_NOT_SYM = 0,
    STATUS_SYM = 1,
  };

  Status status;

  Willingness willingness;
};

inline bool operator==(const NeighborTuple &a, const NeighborTuple &b) {
  return (a.neighborMainAddr == b.neighborMainAddr && a.status == b.status &&
          a.willingness == b.willingness);
}

inline std::ostream &operator<<(std::ostream &os, const NeighborTuple &tuple) {
  os << "NeighborTuple(neighborMainAddr=" << tuple.neighborMainAddr
     << ", status="
     << (tuple.status == NeighborTuple::STATUS_SYM ? "SYM" : "NOT_SYM")
     << ", willingness=" << tuple.willingness << ")";
  return os;
}

struct TwoHopNeighborTuple {
  Ipv4Address neighborMainAddr;
  Ipv4Address twoHopNeighborAddr;
  Time expirationTime;
};

inline std::ostream &operator<<(std::ostream &os,
                                const TwoHopNeighborTuple &tuple) {
  os << "TwoHopNeighborTuple(neighborMainAddr=" << tuple.neighborMainAddr
     << ", twoHopNeighborAddr=" << tuple.twoHopNeighborAddr
     << ", expirationTime=" << tuple.expirationTime << ")";
  return os;
}

inline bool operator==(const TwoHopNeighborTuple &a,
                       const TwoHopNeighborTuple &b) {
  return (a.neighborMainAddr == b.neighborMainAddr &&
          a.twoHopNeighborAddr == b.twoHopNeighborAddr);
}

struct MprSelectorTuple {
  Ipv4Address mainAddr;
  Time expirationTime;
};

inline bool operator==(const MprSelectorTuple &a, const MprSelectorTuple &b) {
  return (a.mainAddr == b.mainAddr);
}

struct DuplicateTuple {
  Ipv4Address address;
  uint16_t sequenceNumber;
  bool retransmitted;
  std::vector<Ipv4Address> ifaceList;
  Time expirationTime;
};

inline bool operator==(const DuplicateTuple &a, const DuplicateTuple &b) {
  return (a.address == b.address && a.sequenceNumber == b.sequenceNumber);
}

struct TopologyTuple {
  Ipv4Address destAddr;
  Ipv4Address lastAddr;
  uint16_t sequenceNumber;
  Time expirationTime;
};

inline bool operator==(const TopologyTuple &a, const TopologyTuple &b) {
  return (a.destAddr == b.destAddr && a.lastAddr == b.lastAddr &&
          a.sequenceNumber == b.sequenceNumber);
}

inline std::ostream &operator<<(std::ostream &os, const TopologyTuple &tuple) {
  os << "TopologyTuple(destAddr=" << tuple.destAddr
     << ", lastAddr=" << tuple.lastAddr
     << ", sequenceNumber=" << (int)tuple.sequenceNumber
     << ", expirationTime=" << tuple.expirationTime << ")";
  return os;
}

struct Association {
  Ipv4Address networkAddr;
  Ipv4Mask netmask;
};

inline bool operator==(const Association &a, const Association &b) {
  return (a.networkAddr == b.networkAddr && a.netmask == b.netmask);
}

inline std::ostream &operator<<(std::ostream &os, const Association &tuple) {
  os << "Association(networkAddr=" << tuple.networkAddr
     << ", netmask=" << tuple.netmask << ")";
  return os;
}

struct AssociationTuple {
  Ipv4Address gatewayAddr;
  Ipv4Address networkAddr;
  Ipv4Mask netmask;
  Time expirationTime;
};

inline bool operator==(const AssociationTuple &a, const AssociationTuple &b) {
  return (a.gatewayAddr == b.gatewayAddr && a.networkAddr == b.networkAddr &&
          a.netmask == b.netmask);
}

inline std::ostream &operator<<(std::ostream &os,
                                const AssociationTuple &tuple) {
  os << "AssociationTuple(gatewayAddr=" << tuple.gatewayAddr
     << ", networkAddr=" << tuple.networkAddr << ", netmask=" << tuple.netmask
     << ", expirationTime=" << tuple.expirationTime << ")";
  return os;
}

typedef std::set<Ipv4Address> MprSet;
typedef std::vector<MprSelectorTuple> MprSelectorSet;
typedef std::vector<LinkTuple> LinkSet;
typedef std::vector<NeighborTuple> NeighborSet;
typedef std::vector<TwoHopNeighborTuple> TwoHopNeighborSet;
typedef std::vector<TopologyTuple> TopologySet;
typedef std::vector<DuplicateTuple> DuplicateSet;
typedef std::vector<IfaceAssocTuple> IfaceAssocSet;
typedef std::vector<AssociationTuple> AssociationSet;
typedef std::vector<Association> Associations;

} // namespace olsr
} // namespace ns3

#endif
