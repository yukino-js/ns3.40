#ifndef POINT_TO_POINT_HELPER_H
#define POINT_TO_POINT_HELPER_H

#include "ns3/net-device-container.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/queue.h"
#include "ns3/trace-helper.h"

#include <string>

namespace ns3 {

class NetDevice;
class Node;

class PointToPointHelper : public PcapHelperForDevice,
                           public AsciiTraceHelperForDevice {
public:
  PointToPointHelper();

  ~PointToPointHelper() override {}

  template <typename... Ts> void SetQueue(std::string type, Ts &&...args);

  void SetDeviceAttribute(std::string name, const AttributeValue &value);

  void SetChannelAttribute(std::string name, const AttributeValue &value);

  void DisableFlowControl();

  NetDeviceContainer Install(NodeContainer c);

  NetDeviceContainer Install(Ptr<Node> a, Ptr<Node> b);

  NetDeviceContainer Install(Ptr<Node> a, std::string bName);

  NetDeviceContainer Install(std::string aName, Ptr<Node> b);

  NetDeviceContainer Install(std::string aNode, std::string bNode);

private:
  void EnablePcapInternal(std::string prefix, Ptr<NetDevice> nd,
                          bool promiscuous, bool explicitFilename) override;

  void EnableAsciiInternal(Ptr<OutputStreamWrapper> stream, std::string prefix,
                           Ptr<NetDevice> nd, bool explicitFilename) override;

  ObjectFactory m_queueFactory;
  ObjectFactory m_channelFactory;
  ObjectFactory m_deviceFactory;
  bool m_enableFlowControl;
};

template <typename... Ts>
void PointToPointHelper::SetQueue(std::string type, Ts &&...args) {
  QueueBase::AppendItemTypeIfNotPresent(type, "Packet");

  m_queueFactory.SetTypeId(type);
  m_queueFactory.Set(std::forward<Ts>(args)...);
}

} // namespace ns3

#endif
