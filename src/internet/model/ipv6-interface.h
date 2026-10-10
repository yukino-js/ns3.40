
#ifndef IPV6_INTERFACE_H
#define IPV6_INTERFACE_H

#include "ipv6-interface-address.h"

#include "ns3/object.h"
#include "ns3/ptr.h"

#include <list>

namespace ns3 {

class NetDevice;
class Packet;
class Node;
class NdiscCache;
class Ipv6InterfaceAddress;
class Ipv6Address;
class Ipv6Header;
class TrafficControlLayer;

class Ipv6Interface : public Object {
public:
  static TypeId GetTypeId();

  Ipv6Interface();

  ~Ipv6Interface() override;

  Ipv6Interface(const Ipv6Interface &) = delete;
  Ipv6Interface &operator=(const Ipv6Interface &) = delete;

  void SetNode(Ptr<Node> node);

  void SetDevice(Ptr<NetDevice> device);

  void SetTrafficControl(Ptr<TrafficControlLayer> tc);

  virtual Ptr<NetDevice> GetDevice() const;

  void SetMetric(uint16_t metric);

  uint16_t GetMetric() const;

  bool IsUp() const;

  bool IsDown() const;

  void SetUp();

  void SetDown();

  bool IsForwarding() const;

  void SetForwarding(bool forward);

  void SetCurHopLimit(uint8_t curHopLimit);

  uint8_t GetCurHopLimit() const;

  void SetBaseReachableTime(uint16_t baseReachableTime);

  uint16_t GetBaseReachableTime() const;

  void SetReachableTime(uint16_t reachableTime);

  uint16_t GetReachableTime() const;

  void SetRetransTimer(uint16_t retransTimer);

  uint16_t GetRetransTimer() const;

  void Send(Ptr<Packet> p, const Ipv6Header &hdr, Ipv6Address dest);

  bool AddAddress(Ipv6InterfaceAddress iface);

  Ipv6InterfaceAddress GetLinkLocalAddress() const;

  bool IsSolicitedMulticastAddress(Ipv6Address address) const;

  Ipv6InterfaceAddress GetAddress(uint32_t index) const;

  Ipv6InterfaceAddress GetAddressMatchingDestination(Ipv6Address dst);

  uint32_t GetNAddresses() const;

  Ipv6InterfaceAddress RemoveAddress(uint32_t index);

  Ipv6InterfaceAddress RemoveAddress(Ipv6Address address);

  void SetState(Ipv6Address address, Ipv6InterfaceAddress::State_e state);

  void SetNsDadUid(Ipv6Address address, uint32_t uid);

  Ptr<NdiscCache> GetNdiscCache() const;

  void
  RemoveAddressCallback(Callback<void, Ptr<Ipv6Interface>, Ipv6InterfaceAddress>
                            removeAddressCallback);

  void
  AddAddressCallback(Callback<void, Ptr<Ipv6Interface>, Ipv6InterfaceAddress>
                         addAddressCallback);

protected:
  void DoDispose() override;

private:
  typedef std::list<std::pair<Ipv6InterfaceAddress, Ipv6Address>>
      Ipv6InterfaceAddressList;

  typedef std::list<std::pair<Ipv6InterfaceAddress, Ipv6Address>>::iterator
      Ipv6InterfaceAddressListI;

  typedef std::list<std::pair<Ipv6InterfaceAddress, Ipv6Address>>::
      const_iterator Ipv6InterfaceAddressListCI;

  void DoSetup();

  Ipv6InterfaceAddressList m_addresses;

  Ipv6InterfaceAddress m_linkLocalAddress;

  bool m_ifup;

  bool m_forwarding;

  uint16_t m_metric;

  Ptr<Node> m_node;

  Ptr<NetDevice> m_device;

  Ptr<TrafficControlLayer> m_tc;

  Ptr<NdiscCache> m_ndCache;

  uint8_t m_curHopLimit;

  uint16_t m_baseReachableTime;

  uint16_t m_reachableTime;

  uint16_t m_retransTimer;

  Callback<void, Ptr<Ipv6Interface>, Ipv6InterfaceAddress>
      m_removeAddressCallback;

  Callback<void, Ptr<Ipv6Interface>, Ipv6InterfaceAddress> m_addAddressCallback;
};

} // namespace ns3

#endif
