
#ifndef V4PING_HELPER_H
#define V4PING_HELPER_H

#include "ns3/application-container.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"

namespace ns3 {

class V4PingHelper {
public:
  NS_DEPRECATED_3_38(
      "Use PingHelper instead - the attributes might have been renamed.")
  V4PingHelper(Ipv4Address remote);

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
