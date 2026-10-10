
#ifndef OLSR_STATE_H
#define OLSR_STATE_H

#include "olsr-repositories.h"

namespace ns3 {
namespace olsr {

class OlsrState {

protected:
  LinkSet m_linkSet;
  NeighborSet m_neighborSet;
  TwoHopNeighborSet m_twoHopNeighborSet;
  TopologySet m_topologySet;
  MprSet m_mprSet;
  MprSelectorSet m_mprSelectorSet;
  DuplicateSet m_duplicateSet;
  IfaceAssocSet m_ifaceAssocSet;
  AssociationSet m_associationSet;
  Associations m_associations;

public:
  OlsrState() {}

  const MprSelectorSet &GetMprSelectors() const { return m_mprSelectorSet; }

  MprSelectorTuple *FindMprSelectorTuple(const Ipv4Address &mainAddr);

  void EraseMprSelectorTuple(const MprSelectorTuple &tuple);

  void EraseMprSelectorTuples(const Ipv4Address &mainAddr);

  void InsertMprSelectorTuple(const MprSelectorTuple &tuple);

  std::string PrintMprSelectorSet() const;

  const NeighborSet &GetNeighbors() const { return m_neighborSet; }

  NeighborSet &GetNeighbors() { return m_neighborSet; }

  NeighborTuple *FindNeighborTuple(const Ipv4Address &mainAddr);

  const NeighborTuple *FindSymNeighborTuple(const Ipv4Address &mainAddr) const;

  NeighborTuple *FindNeighborTuple(const Ipv4Address &mainAddr,
                                   Willingness willingness);

  void EraseNeighborTuple(const NeighborTuple &neighborTuple);
  void EraseNeighborTuple(const Ipv4Address &mainAddr);

  void InsertNeighborTuple(const NeighborTuple &tuple);

  const TwoHopNeighborSet &GetTwoHopNeighbors() const {
    return m_twoHopNeighborSet;
  }

  TwoHopNeighborSet &GetTwoHopNeighbors() { return m_twoHopNeighborSet; }

  TwoHopNeighborTuple *
  FindTwoHopNeighborTuple(const Ipv4Address &neighbor,
                          const Ipv4Address &twoHopNeighbor);

  void EraseTwoHopNeighborTuple(const TwoHopNeighborTuple &tuple);
  void EraseTwoHopNeighborTuples(const Ipv4Address &neighbor);
  void EraseTwoHopNeighborTuples(const Ipv4Address &neighbor,
                                 const Ipv4Address &twoHopNeighbor);
  void InsertTwoHopNeighborTuple(const TwoHopNeighborTuple &tuple);

  bool FindMprAddress(const Ipv4Address &address);

  void SetMprSet(MprSet mprSet);

  MprSet GetMprSet() const;

  DuplicateTuple *FindDuplicateTuple(const Ipv4Address &address,
                                     uint16_t sequenceNumber);

  void EraseDuplicateTuple(const DuplicateTuple &tuple);
  void InsertDuplicateTuple(const DuplicateTuple &tuple);

  const LinkSet &GetLinks() const { return m_linkSet; }

  LinkTuple *FindLinkTuple(const Ipv4Address &ifaceAddr);
  LinkTuple *FindSymLinkTuple(const Ipv4Address &ifaceAddr, Time time);
  void EraseLinkTuple(const LinkTuple &tuple);
  LinkTuple &InsertLinkTuple(const LinkTuple &tuple);

  const TopologySet &GetTopologySet() const { return m_topologySet; }

  TopologyTuple *FindTopologyTuple(const Ipv4Address &destAddr,
                                   const Ipv4Address &lastAddr);
  TopologyTuple *FindNewerTopologyTuple(const Ipv4Address &lastAddr,
                                        uint16_t ansn);
  void EraseTopologyTuple(const TopologyTuple &tuple);
  void EraseOlderTopologyTuples(const Ipv4Address &lastAddr, uint16_t ansn);
  void InsertTopologyTuple(const TopologyTuple &tuple);

  const IfaceAssocSet &GetIfaceAssocSet() const { return m_ifaceAssocSet; }

  IfaceAssocSet &GetIfaceAssocSetMutable() { return m_ifaceAssocSet; }

  IfaceAssocTuple *FindIfaceAssocTuple(const Ipv4Address &ifaceAddr);
  const IfaceAssocTuple *
  FindIfaceAssocTuple(const Ipv4Address &ifaceAddr) const;
  void EraseIfaceAssocTuple(const IfaceAssocTuple &tuple);
  void InsertIfaceAssocTuple(const IfaceAssocTuple &tuple);

  const AssociationSet &GetAssociationSet() const { return m_associationSet; }

  const Associations &GetAssociations() const { return m_associations; }

  AssociationTuple *FindAssociationTuple(const Ipv4Address &gatewayAddr,
                                         const Ipv4Address &networkAddr,
                                         const Ipv4Mask &netmask);
  void EraseAssociationTuple(const AssociationTuple &tuple);
  void InsertAssociationTuple(const AssociationTuple &tuple);
  void EraseAssociation(const Association &tuple);
  void InsertAssociation(const Association &tuple);

  std::vector<Ipv4Address>
  FindNeighborInterfaces(const Ipv4Address &neighborMainAddr) const;
};

} // namespace olsr
} // namespace ns3

#endif
