
#ifndef IPV6_L3_PROTOCOL_H
#define IPV6_L3_PROTOCOL_H

#include "ipv6-header.h"
#include "ipv6-pmtu-cache.h"
#include "ipv6-routing-protocol.h"
#include "ipv6.h"

#include "ns3/ipv6-address.h"
#include "ns3/net-device.h"
#include "ns3/traced-callback.h"

#include <list>

class Ipv6L3ProtocolTestCase;

namespace ns3 {

class Node;
class Ipv6Interface;
class IpL4Protocol;
class Ipv6Route;
class Ipv6MulticastRoute;
class Ipv6RawSocketImpl;
class Icmpv6L4Protocol;
class Ipv6AutoconfiguredPrefix;

class Ipv6L3Protocol : public Ipv6 {
public:
  static TypeId GetTypeId();

  static const uint16_t PROT_NUMBER;

  enum DropReason {
    DROP_TTL_EXPIRED = 1,
    DROP_NO_ROUTE,
    DROP_INTERFACE_DOWN,
    DROP_ROUTE_ERROR,
    DROP_UNKNOWN_PROTOCOL,
    DROP_UNKNOWN_OPTION,
    DROP_MALFORMED_HEADER,
    DROP_FRAGMENT_TIMEOUT,
  };

  Ipv6L3Protocol();

  ~Ipv6L3Protocol() override;

  Ipv6L3Protocol(const Ipv6L3Protocol &) = delete;
  Ipv6L3Protocol &operator=(const Ipv6L3Protocol &) = delete;

  void SetNode(Ptr<Node> node);

  void Insert(Ptr<IpL4Protocol> protocol) override;
  void Insert(Ptr<IpL4Protocol> protocol, uint32_t interfaceIndex) override;

  void Remove(Ptr<IpL4Protocol> protocol) override;
  void Remove(Ptr<IpL4Protocol> protocol, uint32_t interfaceIndex) override;

  Ptr<IpL4Protocol> GetProtocol(int protocolNumber) const override;
  Ptr<IpL4Protocol> GetProtocol(int protocolNumber,
                                int32_t interfaceIndex) const override;

  Ptr<Socket> CreateRawSocket();

  void DeleteRawSocket(Ptr<Socket> socket);

  void SetDefaultTtl(uint8_t ttl);

  void SetDefaultTclass(uint8_t tclass);

  void Receive(Ptr<NetDevice> device, Ptr<const Packet> p, uint16_t protocol,
               const Address &from, const Address &to,
               NetDevice::PacketType packetType);

  void Send(Ptr<Packet> packet, Ipv6Address source, Ipv6Address destination,
            uint8_t protocol, Ptr<Ipv6Route> route) override;

  void SetRoutingProtocol(Ptr<Ipv6RoutingProtocol> routingProtocol) override;

  Ptr<Ipv6RoutingProtocol> GetRoutingProtocol() const override;

  uint32_t AddInterface(Ptr<NetDevice> device) override;

  Ptr<Ipv6Interface> GetInterface(uint32_t i) const;

  uint32_t GetNInterfaces() const override;

  int32_t GetInterfaceForAddress(Ipv6Address addr) const override;

  int32_t GetInterfaceForPrefix(Ipv6Address addr,
                                Ipv6Prefix mask) const override;

  int32_t GetInterfaceForDevice(Ptr<const NetDevice> device) const override;

  bool AddAddress(uint32_t i, Ipv6InterfaceAddress address,
                  bool addOnLinkRoute = true) override;

  Ipv6InterfaceAddress GetAddress(uint32_t interfaceIndex,
                                  uint32_t addressIndex) const override;

  uint32_t GetNAddresses(uint32_t interface) const override;

  bool RemoveAddress(uint32_t interfaceIndex, uint32_t addressIndex) override;

  bool RemoveAddress(uint32_t interfaceIndex, Ipv6Address address) override;

  void SetMetric(uint32_t i, uint16_t metric) override;

  uint16_t GetMetric(uint32_t i) const override;

  uint16_t GetMtu(uint32_t i) const override;

  void SetPmtu(Ipv6Address dst, uint32_t pmtu) override;

  bool IsUp(uint32_t i) const override;

  void SetUp(uint32_t i) override;

  void SetDown(uint32_t i) override;

  bool IsForwarding(uint32_t i) const override;

  void SetForwarding(uint32_t i, bool val) override;

  Ipv6Address SourceAddressSelection(uint32_t interface,
                                     Ipv6Address dest) override;

  Ptr<NetDevice> GetNetDevice(uint32_t i) override;

  Ptr<Icmpv6L4Protocol> GetIcmpv6() const;

  void
  AddAutoconfiguredAddress(uint32_t interface, Ipv6Address network,
                           Ipv6Prefix mask, uint8_t flags, uint32_t validTime,
                           uint32_t preferredTime,
                           Ipv6Address defaultRouter = Ipv6Address::GetZero());

  void RemoveAutoconfiguredAddress(uint32_t interface, Ipv6Address network,
                                   Ipv6Prefix mask, Ipv6Address defaultRouter);

  void RegisterExtensions() override;
  void RegisterOptions() override;

  virtual void ReportDrop(Ipv6Header ipHeader, Ptr<Packet> p,
                          DropReason dropReason);

  typedef void (*SentTracedCallback)(const Ipv6Header &header,
                                     Ptr<const Packet> packet,
                                     uint32_t interface);

  typedef void (*TxRxTracedCallback)(Ptr<const Packet> packet, Ptr<Ipv6> ipv6,
                                     uint32_t interface);

  typedef void (*DropTracedCallback)(const Ipv6Header &header,
                                     Ptr<const Packet> packet,
                                     DropReason reason, Ptr<Ipv6> ipv6,
                                     uint32_t interface);

  void AddMulticastAddress(Ipv6Address address);

  void AddMulticastAddress(Ipv6Address address, uint32_t interface);

  void RemoveMulticastAddress(Ipv6Address address);

  void RemoveMulticastAddress(Ipv6Address address, uint32_t interface);

  bool IsRegisteredMulticastAddress(Ipv6Address address) const;

  bool IsRegisteredMulticastAddress(Ipv6Address address,
                                    uint32_t interface) const;

  bool ReachabilityHint(uint32_t ipInterfaceIndex, Ipv6Address address);

protected:
  void DoDispose() override;

  void NotifyNewAggregate() override;

private:
  friend class ::Ipv6L3ProtocolTestCase;
  friend class Ipv6ExtensionLooseRouting;

  typedef std::vector<Ptr<Ipv6Interface>> Ipv6InterfaceList;

  typedef std::map<Ptr<const NetDevice>, uint32_t>
      Ipv6InterfaceReverseContainer;

  typedef std::list<Ptr<Ipv6RawSocketImpl>> SocketList;

  typedef std::pair<int, int32_t> L4ListKey_t;

  typedef std::map<L4ListKey_t, Ptr<IpL4Protocol>> L4List_t;

  typedef std::list<Ptr<Ipv6AutoconfiguredPrefix>> Ipv6AutoconfiguredPrefixList;

  typedef std::list<Ptr<Ipv6AutoconfiguredPrefix>>::iterator
      Ipv6AutoconfiguredPrefixListI;

  void CallTxTrace(const Ipv6Header &ipHeader, Ptr<Packet> packet,
                   Ptr<Ipv6> ipv6, uint32_t interface);

  TracedCallback<Ptr<const Packet>, Ptr<Ipv6>, uint32_t> m_txTrace;

  TracedCallback<Ptr<const Packet>, Ptr<Ipv6>, uint32_t> m_rxTrace;

  TracedCallback<const Ipv6Header &, Ptr<const Packet>, DropReason, Ptr<Ipv6>,
                 uint32_t>
      m_dropTrace;

  TracedCallback<const Ipv6Header &, Ptr<const Packet>, uint32_t>
      m_sendOutgoingTrace;
  TracedCallback<const Ipv6Header &, Ptr<const Packet>, uint32_t>
      m_unicastForwardTrace;
  TracedCallback<const Ipv6Header &, Ptr<const Packet>, uint32_t>
      m_localDeliverTrace;

  Ipv6Header BuildHeader(Ipv6Address src, Ipv6Address dst, uint8_t protocol,
                         uint16_t payloadSize, uint8_t hopLimit,
                         uint8_t tclass);

  void SendRealOut(Ptr<Ipv6Route> route, Ptr<Packet> packet,
                   const Ipv6Header &ipHeader);

  void IpForward(Ptr<const NetDevice> idev, Ptr<Ipv6Route> rtentry,
                 Ptr<const Packet> p, const Ipv6Header &header);

  void IpMulticastForward(Ptr<const NetDevice> idev,
                          Ptr<Ipv6MulticastRoute> mrtentry, Ptr<const Packet> p,
                          const Ipv6Header &header);

  void LocalDeliver(Ptr<const Packet> p, const Ipv6Header &ip, uint32_t iif);

  void RouteInputError(Ptr<const Packet> p, const Ipv6Header &ipHeader,
                       Socket::SocketErrno sockErrno);

  uint32_t AddIpv6Interface(Ptr<Ipv6Interface> interface);

  void SetupLoopback();

  void SetIpForward(bool forward) override;

  bool GetIpForward() const override;

  void SetMtuDiscover(bool mtuDiscover) override;

  bool GetMtuDiscover() const override;

  virtual void SetSendIcmpv6Redirect(bool sendIcmpv6Redirect);

  virtual bool GetSendIcmpv6Redirect() const;

  Ptr<Node> m_node;

  bool m_ipForward;

  bool m_mtuDiscover;

  Ptr<Ipv6PmtuCache> m_pmtuCache;

  L4List_t m_protocols;

  Ipv6InterfaceList m_interfaces;

  Ipv6InterfaceReverseContainer m_reverseInterfacesContainer;

  uint32_t m_nInterfaces;

  uint8_t m_defaultTtl;

  uint8_t m_defaultTclass;

  bool m_strongEndSystemModel;

  Ptr<Ipv6RoutingProtocol> m_routingProtocol;

  SocketList m_sockets;

  Ipv6AutoconfiguredPrefixList m_prefixes;

  bool m_sendIcmpv6Redirect;

  typedef std::pair<Ipv6Address, uint64_t> Ipv6RegisteredMulticastAddressKey_t;

  typedef std::map<Ipv6RegisteredMulticastAddressKey_t, uint32_t>
      Ipv6RegisteredMulticastAddress_t;

  typedef std::map<Ipv6RegisteredMulticastAddressKey_t, uint32_t>::iterator
      Ipv6RegisteredMulticastAddressIter_t;

  typedef std::map<Ipv6RegisteredMulticastAddressKey_t,
                   uint32_t>::const_iterator
      Ipv6RegisteredMulticastAddressCIter_t;

  typedef std::map<Ipv6Address, uint32_t>
      Ipv6RegisteredMulticastAddressNoInterface_t;

  typedef std::map<Ipv6Address, uint32_t>::iterator
      Ipv6RegisteredMulticastAddressNoInterfaceIter_t;

  typedef std::map<Ipv6Address, uint32_t>::const_iterator
      Ipv6RegisteredMulticastAddressNoInterfaceCIter_t;

  Ipv6RegisteredMulticastAddress_t m_multicastAddresses;

  Ipv6RegisteredMulticastAddressNoInterface_t m_multicastAddressesNoInterface;

  Ipv6RoutingProtocol::UnicastForwardCallback m_ucb;
  Ipv6RoutingProtocol::MulticastForwardCallback m_mcb;
  Ipv6RoutingProtocol::LocalDeliverCallback m_lcb;
  Ipv6RoutingProtocol::ErrorCallback m_ecb;
};

} // namespace ns3

#endif
