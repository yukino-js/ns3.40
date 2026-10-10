#ifndef SIMPLE_NETDEVICE_HELPER_H
#define SIMPLE_NETDEVICE_HELPER_H

#include "net-device-container.h"
#include "node-container.h"

#include "ns3/attribute.h"
#include "ns3/object-factory.h"
#include "ns3/queue.h"
#include "ns3/simple-channel.h"

#include <string>

namespace ns3 {

class SimpleNetDeviceHelper {
public:
  SimpleNetDeviceHelper();

  virtual ~SimpleNetDeviceHelper() {}

  template <typename... Ts> void SetQueue(std::string type, Ts &&...args);

  template <typename... Ts> void SetChannel(std::string type, Ts &&...args);

  void SetDeviceAttribute(std::string n1, const AttributeValue &v1);

  void SetChannelAttribute(std::string n1, const AttributeValue &v1);

  void SetNetDevicePointToPointMode(bool pointToPointMode);

  void DisableFlowControl();

  NetDeviceContainer Install(Ptr<Node> node) const;

  NetDeviceContainer Install(Ptr<Node> node, Ptr<SimpleChannel> channel) const;

  NetDeviceContainer Install(const NodeContainer &c) const;

  NetDeviceContainer Install(const NodeContainer &c,
                             Ptr<SimpleChannel> channel) const;

private:
  Ptr<NetDevice> InstallPriv(Ptr<Node> node, Ptr<SimpleChannel> channel) const;

  ObjectFactory m_queueFactory;
  ObjectFactory m_deviceFactory;
  ObjectFactory m_channelFactory;
  bool m_pointToPointMode;
  bool m_enableFlowControl;
};

template <typename... Ts>
void SimpleNetDeviceHelper::SetQueue(std::string type, Ts &&...args) {
  QueueBase::AppendItemTypeIfNotPresent(type, "Packet");

  m_queueFactory.SetTypeId(type);
  m_queueFactory.Set(std::forward<Ts>(args)...);
}

template <typename... Ts>
void SimpleNetDeviceHelper::SetChannel(std::string type, Ts &&...args) {
  m_channelFactory.SetTypeId(type);
  m_channelFactory.Set(std::forward<Ts>(args)...);
}

} // namespace ns3

#endif
