#ifndef IPV4_H
#define IPV4_H

#include "ipv4-interface-address.h"
#include "ipv4-route.h"

#include "ns3/callback.h"
#include "ns3/ipv4-address.h"
#include "ns3/object.h"
#include "ns3/socket.h"

#include <stdint.h>

namespace ns3 {

class Node;
class NetDevice;
class Packet;
class Ipv4RoutingProtocol;
class IpL4Protocol;
class Ipv4Header;

class Ipv4 : public Object {
public:
  static TypeId GetTypeId();
  Ipv4();
  ~Ipv4() override;

  virtual void SetRoutingProtocol(Ptr<Ipv4RoutingProtocol> routingProtocol) = 0;

  virtual Ptr<Ipv4RoutingProtocol> GetRoutingProtocol() const = 0;

  virtual uint32_t AddInterface(Ptr<NetDevice> device) = 0;

  virtual uint32_t GetNInterfaces() const = 0;

  virtual int32_t GetInterfaceForAddress(Ipv4Address address) const = 0;

  virtual void Send(Ptr<Packet> packet, Ipv4Address source,
                    Ipv4Address destination, uint8_t protocol,
                    Ptr<Ipv4Route> route) = 0;

  virtual void SendWithHeader(Ptr<Packet> packet, Ipv4Header ipHeader,
                              Ptr<Ipv4Route> route) = 0;

  virtual void Insert(Ptr<IpL4Protocol> protocol) = 0;

  virtual void Insert(Ptr<IpL4Protocol> protocol, uint32_t interfaceIndex) = 0;

  virtual void Remove(Ptr<IpL4Protocol> protocol) = 0;

  virtual void Remove(Ptr<IpL4Protocol> protocol, uint32_t interfaceIndex) = 0;

  virtual bool IsDestinationAddress(Ipv4Address address,
                                    uint32_t iif) const = 0;

  virtual int32_t GetInterfaceForPrefix(Ipv4Address address,
                                        Ipv4Mask mask) const = 0;

  virtual Ptr<NetDevice> GetNetDevice(uint32_t interface) = 0;

  virtual int32_t GetInterfaceForDevice(Ptr<const NetDevice> device) const = 0;

  virtual bool AddAddress(uint32_t interface, Ipv4InterfaceAddress address) = 0;

  virtual uint32_t GetNAddresses(uint32_t interface) const = 0;

  virtual Ipv4InterfaceAddress GetAddress(uint32_t interface,
                                          uint32_t addressIndex) const = 0;

  virtual bool RemoveAddress(uint32_t interface, uint32_t addressIndex) = 0;

  virtual bool RemoveAddress(uint32_t interface, Ipv4Address address) = 0;

  virtual Ipv4Address
  SelectSourceAddress(Ptr<const NetDevice> device, Ipv4Address dst,
                      Ipv4InterfaceAddress::InterfaceAddressScope_e scope) = 0;

  virtual void SetMetric(uint32_t interface, uint16_t metric) = 0;

  virtual uint16_t GetMetric(uint32_t interface) const = 0;

  virtual uint16_t GetMtu(uint32_t interface) const = 0;

  virtual bool IsUp(uint32_t interface) const = 0;

  virtual void SetUp(uint32_t interface) = 0;

  virtual void SetDown(uint32_t interface) = 0;

  virtual bool IsForwarding(uint32_t interface) const = 0;

  virtual void SetForwarding(uint32_t interface, bool val) = 0;

  virtual Ipv4Address SourceAddressSelection(uint32_t interface,
                                             Ipv4Address dest) = 0;

  virtual Ptr<IpL4Protocol> GetProtocol(int protocolNumber) const = 0;

  virtual Ptr<IpL4Protocol> GetProtocol(int protocolNumber,
                                        int32_t interfaceIndex) const = 0;

  virtual Ptr<Socket> CreateRawSocket() = 0;

  virtual void DeleteRawSocket(Ptr<Socket> socket) = 0;

  static const uint32_t IF_ANY = 0xffffffff;

private:
  virtual void SetIpForward(bool forward) = 0;
  virtual bool GetIpForward() const = 0;

  virtual void SetWeakEsModel(bool model) = 0;
  virtual bool GetWeakEsModel() const = 0;
};

} // namespace ns3

#endif
