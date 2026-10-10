
#ifndef OLSR_AGENT_IMPL_H
#define OLSR_AGENT_IMPL_H

#include "olsr-header.h"
#include "olsr-repositories.h"
#include "olsr-state.h"

#include "ns3/event-garbage-collector.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-static-routing.h"
#include "ns3/ipv4.h"
#include "ns3/node.h"
#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/random-variable-stream.h"
#include "ns3/socket.h"
#include "ns3/test.h"
#include "ns3/timer.h"
#include "ns3/traced-callback.h"

#include <map>
#include <vector>

class OlsrMprTestCase;

namespace ns3 {
namespace olsr {

struct RoutingTableEntry {
  Ipv4Address destAddr;
  Ipv4Address nextAddr;
  uint32_t interface;
  uint32_t distance;

  RoutingTableEntry() : destAddr(), nextAddr(), interface(0), distance(0) {}
};

class RoutingProtocol;

class RoutingProtocol : public Ipv4RoutingProtocol {
public:
  friend class ::OlsrMprTestCase;

  static const uint16_t OLSR_PORT_NUMBER;

  static TypeId GetTypeId();

  RoutingProtocol();
  ~RoutingProtocol() override;

  void SetMainInterface(uint32_t interface);

  void Dump();

  std::vector<RoutingTableEntry> GetRoutingTableEntries() const;

  MprSet GetMprSet() const;

  const MprSelectorSet &GetMprSelectors() const;

  const NeighborSet &GetNeighbors() const;

  const TwoHopNeighborSet &GetTwoHopNeighbors() const;

  const TopologySet &GetTopologySet() const;

  const OlsrState &GetOlsrState() const;

  int64_t AssignStreams(int64_t stream);

  typedef void (*PacketTxRxTracedCallback)(const PacketHeader &header,
                                           const MessageList &messages);

  typedef void (*TableChangeTracedCallback)(uint32_t size);

private:
  std::set<uint32_t> m_interfaceExclusions;
  Ptr<Ipv4StaticRouting> m_routingTableAssociation;

public:
  std::set<uint32_t> GetInterfaceExclusions() const {
    return m_interfaceExclusions;
  }

  void SetInterfaceExclusions(std::set<uint32_t> exceptions);

  void AddHostNetworkAssociation(Ipv4Address networkAddr, Ipv4Mask netmask);

  void RemoveHostNetworkAssociation(Ipv4Address networkAddr, Ipv4Mask netmask);

  void SetRoutingTableAssociation(Ptr<Ipv4StaticRouting> routingTable);

  Ptr<const Ipv4StaticRouting> GetRoutingTableAssociation() const;

protected:
  void DoInitialize() override;
  void DoDispose() override;

private:
  std::map<Ipv4Address, RoutingTableEntry> m_table;

  Ptr<Ipv4StaticRouting> m_hnaRoutingTable;

  EventGarbageCollector m_events;

  uint16_t m_packetSequenceNumber;
  uint16_t m_messageSequenceNumber;
  uint16_t m_ansn;

  Time m_helloInterval;
  Time m_tcInterval;
  Time m_midInterval;
  Time m_hnaInterval;
  Willingness m_willingness;

  OlsrState m_state;
  Ptr<Ipv4> m_ipv4;

  void Clear();

  uint32_t GetSize() const { return m_table.size(); }

  void RemoveEntry(const Ipv4Address &dest);
  void AddEntry(const Ipv4Address &dest, const Ipv4Address &next,
                uint32_t interface, uint32_t distance);
  void AddEntry(const Ipv4Address &dest, const Ipv4Address &next,
                const Ipv4Address &interfaceAddress, uint32_t distance);

  bool Lookup(const Ipv4Address &dest, RoutingTableEntry &outEntry) const;

  bool FindSendEntry(const RoutingTableEntry &entry,
                     RoutingTableEntry &outEntry) const;

public:
  Ptr<Ipv4Route> RouteOutput(Ptr<Packet> p, const Ipv4Header &header,
                             Ptr<NetDevice> oif,
                             Socket::SocketErrno &sockerr) override;
  bool RouteInput(Ptr<const Packet> p, const Ipv4Header &header,
                  Ptr<const NetDevice> idev, const UnicastForwardCallback &ucb,
                  const MulticastForwardCallback &mcb,
                  const LocalDeliverCallback &lcb,
                  const ErrorCallback &ecb) override;
  void SetIpv4(Ptr<Ipv4> ipv4) override;

  void PrintRoutingTable(Ptr<OutputStreamWrapper> stream,
                         Time::Unit unit = Time::S) const override;

private:
  void NotifyInterfaceUp(uint32_t interface) override;
  void NotifyInterfaceDown(uint32_t interface) override;
  void NotifyAddAddress(uint32_t interface,
                        Ipv4InterfaceAddress address) override;
  void NotifyRemoveAddress(uint32_t interface,
                           Ipv4InterfaceAddress address) override;

  void SendPacket(Ptr<Packet> packet, const MessageList &containedMessages);

  inline uint16_t GetPacketSequenceNumber();

  inline uint16_t GetMessageSequenceNumber();

  void RecvOlsr(Ptr<Socket> socket);

  void MprComputation();

  void RoutingTableComputation();

public:
  Ipv4Address GetMainAddress(Ipv4Address iface_addr) const;

private:
  bool UsesNonOlsrOutgoingInterface(const Ipv4RoutingTableEntry &route);

  Timer m_helloTimer;
  void HelloTimerExpire();

  Timer m_tcTimer;
  void TcTimerExpire();

  Timer m_midTimer;
  void MidTimerExpire();

  Timer m_hnaTimer;
  void HnaTimerExpire();

  void DupTupleTimerExpire(Ipv4Address address, uint16_t sequenceNumber);

  bool m_linkTupleTimerFirstTime;
  void LinkTupleTimerExpire(Ipv4Address neighborIfaceAddr);

  void Nb2hopTupleTimerExpire(Ipv4Address neighborMainAddr,
                              Ipv4Address twoHopNeighborAddr);

  void MprSelTupleTimerExpire(Ipv4Address mainAddr);

  void TopologyTupleTimerExpire(Ipv4Address destAddr, Ipv4Address lastAddr);

  void IfaceAssocTupleTimerExpire(Ipv4Address ifaceAddr);

  void AssociationTupleTimerExpire(Ipv4Address gatewayAddr,
                                   Ipv4Address networkAddr, Ipv4Mask netmask);

  void IncrementAnsn();

  olsr::MessageList m_queuedMessages;
  Timer m_queuedMessagesTimer;

  void ForwardDefault(olsr::MessageHeader olsrMessage,
                      DuplicateTuple *duplicated, const Ipv4Address &localIface,
                      const Ipv4Address &senderAddress);

  void QueueMessage(const olsr::MessageHeader &message, Time delay);

  void SendQueuedMessages();

  void SendHello();

  void SendTc();

  void SendMid();

  void SendHna();

  void NeighborLoss(const LinkTuple &tuple);

  void AddDuplicateTuple(const DuplicateTuple &tuple);

  void RemoveDuplicateTuple(const DuplicateTuple &tuple);

  void LinkTupleAdded(const LinkTuple &tuple, Willingness willingness);

  void RemoveLinkTuple(const LinkTuple &tuple);

  void LinkTupleUpdated(const LinkTuple &tuple, Willingness willingness);

  void AddNeighborTuple(const NeighborTuple &tuple);

  void RemoveNeighborTuple(const NeighborTuple &tuple);

  void AddTwoHopNeighborTuple(const TwoHopNeighborTuple &tuple);

  void RemoveTwoHopNeighborTuple(const TwoHopNeighborTuple &tuple);

  void AddMprSelectorTuple(const MprSelectorTuple &tuple);

  void RemoveMprSelectorTuple(const MprSelectorTuple &tuple);

  void AddTopologyTuple(const TopologyTuple &tuple);

  void RemoveTopologyTuple(const TopologyTuple &tuple);

  void AddIfaceAssocTuple(const IfaceAssocTuple &tuple);

  void RemoveIfaceAssocTuple(const IfaceAssocTuple &tuple);

  void AddAssociationTuple(const AssociationTuple &tuple);

  void RemoveAssociationTuple(const AssociationTuple &tuple);

  void ProcessHello(const olsr::MessageHeader &msg,
                    const Ipv4Address &receiverIface,
                    const Ipv4Address &senderIface);

  void ProcessTc(const olsr::MessageHeader &msg,
                 const Ipv4Address &senderIface);

  void ProcessMid(const olsr::MessageHeader &msg,
                  const Ipv4Address &senderIface);

  void ProcessHna(const olsr::MessageHeader &msg,
                  const Ipv4Address &senderIface);

  void LinkSensing(const olsr::MessageHeader &msg,
                   const olsr::MessageHeader::Hello &hello,
                   const Ipv4Address &receiverIface,
                   const Ipv4Address &senderIface);

  void PopulateNeighborSet(const olsr::MessageHeader &msg,
                           const olsr::MessageHeader::Hello &hello);

  void PopulateTwoHopNeighborSet(const olsr::MessageHeader &msg,
                                 const olsr::MessageHeader::Hello &hello);

  void PopulateMprSelectorSet(const olsr::MessageHeader &msg,
                              const olsr::MessageHeader::Hello &hello);

  int Degree(const NeighborTuple &tuple);

  bool IsMyOwnAddress(const Ipv4Address &a) const;

  Ipv4Address m_mainAddress;

  std::map<Ptr<Socket>, Ipv4InterfaceAddress> m_sendSockets;
  Ptr<Socket> m_recvSocket;

  TracedCallback<const PacketHeader &, const MessageList &> m_rxPacketTrace;

  TracedCallback<const PacketHeader &, const MessageList &> m_txPacketTrace;

  TracedCallback<uint32_t> m_routingTableChanged;

  Ptr<UniformRandomVariable> m_uniformRandomVariable;
};

} // namespace olsr
} // namespace ns3

#endif
