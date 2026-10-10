
#ifndef IPV4_L3_CLICK_PROTOCOL_H
#define IPV4_L3_CLICK_PROTOCOL_H

#include "ns3/ipv4-interface.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4.h"
#include "ns3/log.h"
#include "ns3/net-device.h"
#include "ns3/packet.h"
#include "ns3/traced-callback.h"

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

class Ipv4L3ClickProtocol : public Ipv4 {
public:
  static TypeId GetTypeId();

  static const uint16_t PROT_NUMBER;

  Ipv4L3ClickProtocol();
  ~Ipv4L3ClickProtocol() override;

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

  void Send(Ptr<Packet> packet, Ipv4Address source, Ipv4Address destination,
            uint8_t protocol, Ptr<Ipv4Route> route) override;

  void SendWithHeader(Ptr<Packet> packet, Ipv4Header ipHeader,
                      Ptr<Ipv4Route> route) override;

  void SendDown(Ptr<Packet> packet, int ifid);

  void Receive(Ptr<NetDevice> device, Ptr<const Packet> p, uint16_t protocol,
               const Address &from, const Address &to,
               NetDevice::PacketType packetType);

  void LocalDeliver(Ptr<const Packet> p, const Ipv4Header &ip, uint32_t iif);

  Ptr<Ipv4Interface> GetInterface(uint32_t i) const;

  uint32_t AddIpv4Interface(Ptr<Ipv4Interface> interface);

  void SetNode(Ptr<Node> node);

  Ptr<Icmpv4L4Protocol> GetIcmp() const;

  void SetupLoopback();

  Ptr<Socket> CreateRawSocket() override;

  void DeleteRawSocket(Ptr<Socket> socket) override;

  void SetRoutingProtocol(Ptr<Ipv4RoutingProtocol> routingProtocol) override;
  Ptr<Ipv4RoutingProtocol> GetRoutingProtocol() const override;

  Ptr<NetDevice> GetNetDevice(uint32_t i) override;

  uint32_t AddInterface(Ptr<NetDevice> device) override;
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
  bool RemoveAddress(uint32_t interfaceIndex, Ipv4Address address) override;
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

  void SetPromisc(uint32_t i);

protected:
  void DoDispose() override;
  void NotifyNewAggregate() override;

private:
  Ipv4Header BuildHeader(Ipv4Address source, Ipv4Address destination,
                         uint8_t protocol, uint16_t payloadSize, uint8_t ttl,
                         bool mayFragment);

  void SetIpForward(bool forward) override;
  bool GetIpForward() const override;
  void SetWeakEsModel(bool model) override;
  bool GetWeakEsModel() const override;

  typedef std::vector<Ptr<Ipv4Interface>> Ipv4InterfaceList;

  typedef std::map<Ptr<const NetDevice>, uint32_t>
      Ipv4InterfaceReverseContainer;

  typedef std::list<Ptr<Ipv4RawSocketImpl>> SocketList;

  typedef std::pair<int, int32_t> L4ListKey_t;

  typedef std::map<L4ListKey_t, Ptr<IpL4Protocol>> L4List_t;

  Ptr<Ipv4RoutingProtocol> m_routingProtocol;
  bool m_ipForward;
  bool m_weakEsModel;
  L4List_t m_protocols;
  Ipv4InterfaceList m_interfaces;
  Ipv4InterfaceReverseContainer m_reverseInterfacesContainer;
  uint8_t m_defaultTtl;
  uint16_t m_identification;

  Ptr<Node> m_node;

  TracedCallback<const Ipv4Header &, Ptr<const Packet>, uint32_t>
      m_sendOutgoingTrace;
  TracedCallback<const Ipv4Header &, Ptr<const Packet>, uint32_t>
      m_unicastForwardTrace;
  TracedCallback<const Ipv4Header &, Ptr<const Packet>, uint32_t>
      m_localDeliverTrace;

  SocketList m_sockets;

  std::vector<bool> m_promiscDeviceList;
};

} // namespace ns3

#endif
