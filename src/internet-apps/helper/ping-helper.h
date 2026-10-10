
#ifndef PING_HELPER_H
#define PING_HELPER_H

#include "ns3/application-container.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/ping.h"

#include <stdint.h>

namespace ns3 {

class PingHelper {
public:
  PingHelper();

  PingHelper(Address remote, Address local = Address());

  ApplicationContainer Install(NodeContainer nodes) const;

  ApplicationContainer Install(Ptr<Node> node) const;

  ApplicationContainer Install(std::string nodeName) const;

  void SetAttribute(std::string name, const AttributeValue &value);

private:
  Ptr<Application> InstallPriv(Ptr<Node> node) const;
  ObjectFactory m_factory;
};

} // namespace ns3

#endif
