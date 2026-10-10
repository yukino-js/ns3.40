#ifndef OPENFLOW_SWITCH_HELPER_H
#define OPENFLOW_SWITCH_HELPER_H

#include "ns3/net-device-container.h"
#include "ns3/object-factory.h"
#include "ns3/openflow-interface.h"

#include <string>

namespace ns3 {

class Node;
class AttributeValue;
class Controller;

class OpenFlowSwitchHelper {
public:
  OpenFlowSwitchHelper();

  void SetDeviceAttribute(std::string n1, const AttributeValue &v1);

  NetDeviceContainer Install(Ptr<Node> node, NetDeviceContainer c,
                             Ptr<ns3::ofi::Controller> controller);

  NetDeviceContainer Install(Ptr<Node> node, NetDeviceContainer c);

  NetDeviceContainer Install(std::string nodeName, NetDeviceContainer c);

private:
  ObjectFactory m_deviceFactory;
};

} // namespace ns3

#endif
