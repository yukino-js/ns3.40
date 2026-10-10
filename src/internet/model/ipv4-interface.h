#ifndef IPV4_INTERFACE_H
#define IPV4_INTERFACE_H

#include "ns3/object.h"
#include "ns3/ptr.h"

#include <list>

namespace ns3 {

class NetDevice;
class Packet;
class Node;
class ArpCache;
class Ipv4InterfaceAddress;
class Ipv4Address;
class Ipv4Header;
class TrafficControlLayer;

class Ipv4Interface : public Object {
public:
  static TypeId GetTypeId();

  Ipv4Interface();
  ~Ipv4Interface() override;

  Ipv4Interface(const Ipv4Interface &) = delete;
  Ipv4Interface &operator=(const Ipv4Interface &) = delete;

  void SetNode(Ptr<Node> node);
  void SetDevice(Ptr<NetDevice> device);
  void SetTrafficControl(Ptr<TrafficControlLayer> tc);
  void SetArpCache(Ptr<ArpCache> arpCache);

  Ptr<NetDevice> GetDevice() const;

  Ptr<ArpCache> GetArpCache() const;

  void SetMetric(uint16_t metric);

  uint16_t GetMetric() const;

  bool IsUp() const;

  bool IsDown() const;

  void SetUp();

  void SetDown();

  bool IsForwarding() const;

  void SetForwarding(bool val);

  void Send(Ptr<Packet> p, const Ipv4Header &hdr, Ipv4Address dest);

  bool AddAddress(Ipv4InterfaceAddress address);

  Ipv4InterfaceAddress GetAddress(uint32_t index) const;

  uint32_t GetNAddresses() const;

  Ipv4InterfaceAddress RemoveAddress(uint32_t index);

  Ipv4InterfaceAddress RemoveAddress(Ipv4Address address);

  void
  RemoveAddressCallback(Callback<void, Ptr<Ipv4Interface>, Ipv4InterfaceAddress>
                            removeAddressCallback);

  void
  AddAddressCallback(Callback<void, Ptr<Ipv4Interface>, Ipv4InterfaceAddress>
                         addAddressCallback);

protected:
  void DoDispose() override;

private:
  void DoSetup();

  typedef std::list<Ipv4InterfaceAddress> Ipv4InterfaceAddressList;

  typedef std::list<Ipv4InterfaceAddress>::const_iterator
      Ipv4InterfaceAddressListCI;

  typedef std::list<Ipv4InterfaceAddress>::iterator Ipv4InterfaceAddressListI;

  bool m_ifup;
  bool m_forwarding;
  uint16_t m_metric;
  Ipv4InterfaceAddressList m_ifaddrs;
  Ptr<Node> m_node;
  Ptr<NetDevice> m_device;
  Ptr<TrafficControlLayer> m_tc;
  Ptr<ArpCache> m_cache;
  Callback<void, Ptr<Ipv4Interface>, Ipv4InterfaceAddress>
      m_removeAddressCallback;
  Callback<void, Ptr<Ipv4Interface>, Ipv4InterfaceAddress> m_addAddressCallback;
};

} // namespace ns3

#endif
