
#ifndef IPV4_STATIC_ROUTING_HELPER_H
#define IPV4_STATIC_ROUTING_HELPER_H

#include "ipv4-routing-helper.h"

#include "ns3/ipv4-address.h"
#include "ns3/ipv4-static-routing.h"
#include "ns3/ipv4.h"
#include "ns3/net-device-container.h"
#include "ns3/net-device.h"
#include "ns3/node-container.h"
#include "ns3/node.h"
#include "ns3/ptr.h"

namespace ns3 {

class Ipv4StaticRoutingHelper : public Ipv4RoutingHelper {
public:
  Ipv4StaticRoutingHelper();

  Ipv4StaticRoutingHelper(const Ipv4StaticRoutingHelper &o);

  Ipv4StaticRoutingHelper &operator=(const Ipv4StaticRoutingHelper &) = delete;

  Ipv4StaticRoutingHelper *Copy() const override;

  Ptr<Ipv4RoutingProtocol> Create(Ptr<Node> node) const override;

  Ptr<Ipv4StaticRouting> GetStaticRouting(Ptr<Ipv4> ipv4) const;

  void AddMulticastRoute(Ptr<Node> n, Ipv4Address source, Ipv4Address group,
                         Ptr<NetDevice> input, NetDeviceContainer output);

  void AddMulticastRoute(std::string n, Ipv4Address source, Ipv4Address group,
                         Ptr<NetDevice> input, NetDeviceContainer output);

  void AddMulticastRoute(Ptr<Node> n, Ipv4Address source, Ipv4Address group,
                         std::string inputName, NetDeviceContainer output);

  void AddMulticastRoute(std::string nName, Ipv4Address source,
                         Ipv4Address group, std::string inputName,
                         NetDeviceContainer output);

  void SetDefaultMulticastRoute(Ptr<Node> n, Ptr<NetDevice> nd);

  void SetDefaultMulticastRoute(Ptr<Node> n, std::string ndName);

  void SetDefaultMulticastRoute(std::string nName, Ptr<NetDevice> nd);

  void SetDefaultMulticastRoute(std::string nName, std::string ndName);
};

} // namespace ns3

#endif
