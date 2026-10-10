
#ifndef TAP_BRIDGE_HELPER_H
#define TAP_BRIDGE_HELPER_H

#include "ns3/net-device-container.h"
#include "ns3/object-factory.h"
#include "ns3/tap-bridge.h"

#include <string>

namespace ns3 {

class Node;
class AttributeValue;

class TapBridgeHelper {
public:
  TapBridgeHelper();

  TapBridgeHelper(Ipv4Address gateway);

  void SetAttribute(std::string n1, const AttributeValue &v1);

  Ptr<NetDevice> Install(Ptr<Node> node, Ptr<NetDevice> nd);

  Ptr<NetDevice> Install(std::string nodeName, Ptr<NetDevice> nd);

  Ptr<NetDevice> Install(Ptr<Node> node, std::string ndName);

  Ptr<NetDevice> Install(std::string nodeName, std::string ndName);

  Ptr<NetDevice> Install(Ptr<Node> node, Ptr<NetDevice> nd,
                         const AttributeValue &bridgeType);

private:
  ObjectFactory m_deviceFactory;
};

} // namespace ns3

#endif
