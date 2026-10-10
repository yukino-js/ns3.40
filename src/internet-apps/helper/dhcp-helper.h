
#ifndef DHCP_HELPER_H
#define DHCP_HELPER_H

#include "ns3/application-container.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4-interface-container.h"
#include "ns3/net-device-container.h"
#include "ns3/object-factory.h"

#include <stdint.h>

namespace ns3 {

class DhcpHelper {
public:
  DhcpHelper();

  void SetClientAttribute(std::string name, const AttributeValue &value);

  void SetServerAttribute(std::string name, const AttributeValue &value);

  ApplicationContainer InstallDhcpClient(Ptr<NetDevice> netDevice) const;

  ApplicationContainer InstallDhcpClient(NetDeviceContainer netDevices) const;

  ApplicationContainer InstallDhcpServer(Ptr<NetDevice> netDevice,
                                         Ipv4Address serverAddr,
                                         Ipv4Address poolAddr,
                                         Ipv4Mask poolMask, Ipv4Address minAddr,
                                         Ipv4Address maxAddr,
                                         Ipv4Address gateway = Ipv4Address());
  Ipv4InterfaceContainer InstallFixedAddress(Ptr<NetDevice> netDevice,
                                             Ipv4Address addr, Ipv4Mask mask);

private:
  Ptr<Application> InstallDhcpClientPriv(Ptr<NetDevice> netDevice) const;
  ObjectFactory m_clientFactory;
  ObjectFactory m_serverFactory;
  std::list<Ipv4Address> m_fixedAddresses;
  std::list<std::pair<Ipv4Address, Ipv4Address>> m_addressPools;
};

} // namespace ns3

#endif
