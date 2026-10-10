
#ifndef IPV4_L3_PROTOCOL_H
#define IPV4_L3_PROTOCOL_H

#include "ipv4-header.h"
#include "ipv4-routing-protocol.h"
#include "ipv4.h"

#include "ns3/ipv4-address.h"
#include "ns3/net-device.h"
#include "ns3/nstime.h"
#include "ns3/ptr.h"
#include "ns3/simulator.h"
#include "ns3/traced-callback.h"

#include <list>
#include <map>
#include <stdint.h>
#include <vector>

class Ipv4L3ProtocolTestCase;

namespace ns3 {

class Packet;
class NetDevice;
class Ipv4Interface;
class Ipv4Address;
class Ipv4Header;
class Ipv4RoutingTableEntry;
class Ipv4Route;
class Node;
class Socket;
class Ipv4RawSocketImpl;
class IpL4Protocol;
class Icmpv4L4Protocol;

class Ipv4L3Protocol : public Ipv4 {
public:
  static TypeId GetTypeId();
  static const uint16_t PROT_NUMBER;

  Ipv4L3Protocol();
  ~Ipv4L3Protocol() override;

  Ipv4L3Protocol(const Ipv4L3Protocol &) = delete;
  Ipv4L3Protocol &operator=(const Ipv4L3Protocol &) = delete;

  enum DropReason {
    DROP_TTL_EXPIRED = 1,
    DROP_NO_ROUTE,
    DROP_BAD_CHECKSUM,
    DROP_INTERFACE_DOWN,
    DROP_ROUTE_ERROR,
    DROP_FRAGMENT_TIMEOUT,
    DROP_DUPLICATE
  };

  void SetNode(Ptr<Node> node);

  void SetRoutingProtocol(Ptr<Ipv4RoutingProtocol> routingProtocol) override;
  Ptr<Ipv4RoutingProtocol> GetRoutingProtocol() const override;

  Ptr<Socket> CreateRawSocket() override;
  void DeleteRawSocket(Ptr<Socket> socket) override;

  void Insert(Ptr<IpL4Protocol> protocol) override;
  void Insert(Ptr<IpL4Protocol> protocol, uint32_t interfaceIndex) override;

  void Remove(Ptr<IpL4Protocol> protocol) override;
  void Remove(Ptr<IpL4Protocol> protocol, uint32_t interfaceIndex) override;

  Ptr<IpL4Protocol> GetProtocol(int protocolNumber) const override;
  Ptr<IpL4Protocol> GetProtocol(int protocolNumber,
                                int32_t interfaceIndex) const override;

  Ipv4Address SourceAddressSelection(uint32_t interface,
                                     Ipv4Address dest) override;

  void SetDefaultTtl(uint8_t ttl);

  void Receive(Ptr<NetDevice> device, Ptr<const Packet> p, uint16_t protocol,
               const Address &from, const Address &to,
               NetDevice::PacketType packetType);

  void Send(Ptr<Packet> packet, Ipv4Address source, Ipv4Address destination,
            uint8_t protocol, Ptr<Ipv4Route> route) override;
  void SendWithHeader(Ptr<Packet> packet, Ipv4Header ipHeader,
                      Ptr<Ipv4Route> route) override;

  uint32_t AddInterface(Ptr<NetDevice> device) override;
  Ptr<Ipv4Interface> GetInterface(uint32_t i) const;
  uint32_t GetNInterfaces() const override;

  int32_t GetInterfaceForAddress(Ipv4Address addr) const override;
  int32_t GetInterfaceForPrefix(Ipv4Address addr, Ipv4Mask mask) const override;
  int32_t GetInterfaceForDevice(Ptr<const NetDevice> device) const override;
  bool IsDestinationAddress(Ipv4Address address, uint32_t iif) const override;

  bool AddAddress(uint32_t i, Ipv4InterfaceAddress address) override;
  Ipv4InterfaceAddress GetAddress(uint32_t interfaceIndex,
                                  uint32_t addressIndex) const override;
  uint32_t GetNAddresses(uint32_t interface) const override;
  bool RemoveAddress(uint32_t interfaceIndex, uint32_t addressIndex) override;
  bool RemoveAddress(uint32_t interface, Ipv4Address address) override;
  Ipv4Address SelectSourceAddress(
      Ptr<const NetDevice> device, Ipv4Address dst,
      Ipv4InterfaceAddress::InterfaceAddressScope_e scope) override;

  void SetMetric(uint32_t i, uint16_t metric) override;
  uint16_t GetMetric(uint32_t i) const override;
  uint16_t GetMtu(uint32_t i) const override;
  bool IsUp(uint32_t i) const override;
  void SetUp(uint32_t i) override;
  void SetDown(uint32_t i) override;
  bool IsForwarding(uint32_t i) const override;
  void SetForwarding(uint32_t i, bool val) override;

  Ptr<NetDevice> GetNetDevice(uint32_t i) override;

  bool IsUnicast(Ipv4Address ad) const;

  typedef void (*SentTracedCallback)(const Ipv4Header &header,
                                     Ptr<const Packet> packet,
                                     uint32_t interface);

  typedef void (*TxRxTracedCallback)(Ptr<const Packet> packet, Ptr<Ipv4> ipv4,
                                     uint32_t interface);

  typedef void (*DropTracedCallback)(const Ipv4Header &header,
                                     Ptr<const Packet> packet,
                                     DropReason reason, Ptr<Ipv4> ipv4,
                                     uint32_t interface);

protected:
  void DoDispose() override;
  void NotifyNewAggregate() override;

private:
  friend class ::Ipv4L3ProtocolTestCase;

  void SetIpForward(bool forward) override;
  bool GetIpForward() const override;
  void SetWeakEsModel(bool model) override;
  bool GetWeakEsModel() const override;

  void DecreaseIdentification(Ipv4Address source, Ipv4Address destination,
                              uint8_t protocol);

  Ipv4Header BuildHeader(Ipv4Address source, Ipv4Address destination,
                         uint8_t protocol, uint16_t payloadSize, uint8_t ttl,
                         uint8_t tos, bool mayFragment);

  void SendRealOut(Ptr<Ipv4Route> route, Ptr<Packet> packet,
                   const Ipv4Header &ipHeader);

  void IpForward(Ptr<Ipv4Route> rtentry, Ptr<const Packet> p,
                 const Ipv4Header &header);

  void IpMulticastForward(Ptr<Ipv4MulticastRoute> mrtentry, Ptr<const Packet> p,
                          const Ipv4Header &header);

  void LocalDeliver(Ptr<const Packet> p, const Ipv4Header &ip, uint32_t iif);

  void RouteInputError(Ptr<const Packet> p, const Ipv4Header &ipHeader,
                       Socket::SocketErrno sockErrno);

  uint32_t AddIpv4Interface(Ptr<Ipv4Interface> interface);

  void SetupLoopback();

  Ptr<Icmpv4L4Protocol> GetIcmp() const;

  bool IsUnicast(Ipv4Address ad, Ipv4Mask interfaceMask) const;

  typedef std::pair<Ptr<Packet>, Ipv4Header> Ipv4PayloadHeaderPair;

  void DoFragmentation(Ptr<Packet> packet, const Ipv4Header &ipv4Header,
                       uint32_t outIfaceMtu,
                       std::list<Ipv4PayloadHeaderPair> &listFragments);

  bool ProcessFragment(Ptr<Packet> &packet, Ipv4Header &ipHeader, uint32_t iif);

  void CallTxTrace(const Ipv4Header &ipHeader, Ptr<Packet> packet,
                   Ptr<Ipv4> ipv4, uint32_t interface);

  typedef std::vector<Ptr<Ipv4Interface>> Ipv4InterfaceList;
  typedef std::map<Ptr<const NetDevice>, uint32_t>
      Ipv4InterfaceReverseContainer;
  typedef std::list<Ptr<Ipv4RawSocketImpl>> SocketList;

  typedef std::pair<int, int32_t> L4ListKey_t;

  typedef std::map<L4ListKey_t, Ptr<IpL4Protocol>> L4List_t;

  bool m_ipForward;
  bool m_weakEsModel;
  L4List_t m_protocols;
  Ipv4InterfaceList m_interfaces;
  Ipv4InterfaceReverseContainer m_reverseInterfacesContainer;
  uint8_t m_defaultTtl;
  std::map<std::pair<uint64_t, uint8_t>, uint16_t> m_identification;
  Ptr<Node> m_node;

  TracedCallback<const Ipv4Header &, Ptr<const Packet>, uint32_t>
      m_sendOutgoingTrace;
  TracedCallback<const Ipv4Header &, Ptr<const Packet>, uint32_t>
      m_unicastForwardTrace;
  TracedCallback<const Ipv4Header &, Ptr<const Packet>, uint32_t>
      m_multicastForwardTrace;
  TracedCallback<const Ipv4Header &, Ptr<const Packet>, uint32_t>
      m_localDeliverTrace;

  TracedCallback<Ptr<const Packet>, Ptr<Ipv4>, uint32_t> m_txTrace;
  TracedCallback<Ptr<const Packet>, Ptr<Ipv4>, uint32_t> m_rxTrace;
  TracedCallback<const Ipv4Header &, Ptr<const Packet>, DropReason, Ptr<Ipv4>,
                 uint32_t>
      m_dropTrace;

  Ptr<Ipv4RoutingProtocol> m_routingProtocol;

  SocketList m_sockets;

  typedef std::pair<uint64_t, uint32_t> FragmentKey_t;

  typedef std::list<std::tuple<Time, FragmentKey_t, Ipv4Header, uint32_t>>
      FragmentsTimeoutsList_t;
  typedef std::list<std::tuple<Time, FragmentKey_t, Ipv4Header,
                               uint32_t>>::iterator FragmentsTimeoutsListI_t;

  void HandleFragmentsTimeout(FragmentKey_t key, Ipv4Header &ipHeader,
                              uint32_t iif);

  FragmentsTimeoutsListI_t SetTimeout(FragmentKey_t key, Ipv4Header ipHeader,
                                      uint32_t iif);

  void HandleTimeout();

  FragmentsTimeoutsList_t m_timeoutEventList;

  EventId m_timeoutEvent;

  class Fragments : public SimpleRefCount<Fragments> {
  public:
    Fragments();

    void AddFragment(Ptr<Packet> fragment, uint16_t fragmentOffset,
                     bool moreFragment);

    bool IsEntire() const;

    Ptr<Packet> GetPacket() const;

    Ptr<Packet> GetPartialPacket() const;

    void SetTimeoutIter(FragmentsTimeoutsListI_t iter);

    FragmentsTimeoutsListI_t GetTimeoutIter();

  private:
    bool m_moreFragment;

    std::list<std::pair<Ptr<Packet>, uint16_t>> m_fragments;

    FragmentsTimeoutsListI_t m_timeoutIter;
  };

  typedef std::map<FragmentKey_t, Ptr<Fragments>> MapFragments_t;

  MapFragments_t m_fragments;
  Time m_fragmentExpirationTimeout;

  typedef std::tuple<uint64_t, uint8_t, Ipv4Address, Ipv4Address> DupTuple_t;
  typedef std::map<DupTuple_t, Time> DupMap_t;

  bool UpdateDuplicate(Ptr<const Packet> p, const Ipv4Header &header);
  void RemoveDuplicates();

  bool m_enableDpd;
  DupMap_t m_dups;
  Time m_expire;
  Time m_purge;
  EventId m_cleanDpd;

  Ipv4RoutingProtocol::UnicastForwardCallback m_ucb;
  Ipv4RoutingProtocol::MulticastForwardCallback m_mcb;
  Ipv4RoutingProtocol::LocalDeliverCallback m_lcb;
  Ipv4RoutingProtocol::ErrorCallback m_ecb;
};

} // namespace ns3

#endif
