

#ifndef IPV6_H
#define IPV6_H

#include "ipv6-interface-address.h"

#include "ns3/callback.h"
#include "ns3/ipv6-address.h"
#include "ns3/object.h"
#include "ns3/socket.h"

#include <stdint.h>

namespace ns3 {

class Node;
class NetDevice;
class Packet;
class Ipv6RoutingProtocol;
class IpL4Protocol;
class Ipv6Route;

class Ipv6 : public Object {
public:
  static TypeId GetTypeId();

  Ipv6();

  ~Ipv6() override;

  virtual void SetRoutingProtocol(Ptr<Ipv6RoutingProtocol> routingProtocol) = 0;

  virtual Ptr<Ipv6RoutingProtocol> GetRoutingProtocol() const = 0;

  virtual uint32_t AddInterface(Ptr<NetDevice> device) = 0;

  virtual uint32_t GetNInterfaces() const = 0;

  virtual int32_t GetInterfaceForAddress(Ipv6Address address) const = 0;

  virtual int32_t GetInterfaceForPrefix(Ipv6Address address,
                                        Ipv6Prefix mask) const = 0;

  virtual Ptr<NetDevice> GetNetDevice(uint32_t interface) = 0;

  virtual int32_t GetInterfaceForDevice(Ptr<const NetDevice> device) const = 0;

  virtual bool AddAddress(uint32_t interface, Ipv6InterfaceAddress address,
                          bool addOnLinkRoute = true) = 0;

  virtual uint32_t GetNAddresses(uint32_t interface) const = 0;

  virtual Ipv6InterfaceAddress GetAddress(uint32_t interface,
                                          uint32_t addressIndex) const = 0;

  virtual bool RemoveAddress(uint32_t interface, uint32_t addressIndex) = 0;

  virtual bool RemoveAddress(uint32_t interface, Ipv6Address address) = 0;

  virtual void SetMetric(uint32_t interface, uint16_t metric) = 0;

  virtual uint16_t GetMetric(uint32_t interface) const = 0;

  virtual uint16_t GetMtu(uint32_t interface) const = 0;

  virtual void SetPmtu(Ipv6Address dst, uint32_t pmtu) = 0;

  virtual bool IsUp(uint32_t interface) const = 0;

  virtual void SetUp(uint32_t interface) = 0;

  virtual void SetDown(uint32_t interface) = 0;

  virtual bool IsForwarding(uint32_t interface) const = 0;

  virtual void SetForwarding(uint32_t interface, bool val) = 0;

  virtual Ipv6Address SourceAddressSelection(uint32_t interface,
                                             Ipv6Address dest) = 0;

  virtual void Insert(Ptr<IpL4Protocol> protocol) = 0;

  virtual void Insert(Ptr<IpL4Protocol> protocol, uint32_t interfaceIndex) = 0;

  virtual void Remove(Ptr<IpL4Protocol> protocol) = 0;

  virtual void Remove(Ptr<IpL4Protocol> protocol, uint32_t interfaceIndex) = 0;

  virtual Ptr<IpL4Protocol> GetProtocol(int protocolNumber) const = 0;

  virtual Ptr<IpL4Protocol> GetProtocol(int protocolNumber,
                                        int32_t interfaceIndex) const = 0;

  virtual void Send(Ptr<Packet> packet, Ipv6Address source,
                    Ipv6Address destination, uint8_t protocol,
                    Ptr<Ipv6Route> route) = 0;

  virtual void RegisterExtensions() = 0;

  virtual void RegisterOptions() = 0;

  static const uint32_t IF_ANY = 0xffffffff;

private:
  virtual void SetIpForward(bool forward) = 0;

  virtual bool GetIpForward() const = 0;

  virtual void SetMtuDiscover(bool mtuDiscover) = 0;

  virtual bool GetMtuDiscover() const = 0;
};

} // namespace ns3

#endif
