
#ifndef V4TRACEROUTE_HELPER_H
#define V4TRACEROUTE_HELPER_H

#include "ns3/application-container.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/output-stream-wrapper.h"

namespace ns3 {

class V4TraceRouteHelper {
public:
  V4TraceRouteHelper(Ipv4Address remote);

  ApplicationContainer Install(NodeContainer nodes) const;

  ApplicationContainer Install(Ptr<Node> node) const;

  ApplicationContainer Install(std::string nodeName) const;

  void SetAttribute(std::string name, const AttributeValue &value);
  static void PrintTraceRouteAt(Ptr<Node> node,
                                Ptr<OutputStreamWrapper> stream);

private:
  Ptr<Application> InstallPriv(Ptr<Node> node) const;
  ObjectFactory m_factory;
};

} // namespace ns3

#endif
