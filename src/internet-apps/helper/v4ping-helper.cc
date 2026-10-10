
#include "v4ping-helper.h"

#include "ns3/names.h"
#include "ns3/v4ping.h"
#include "ns3/warnings.h"

namespace ns3 {

V4PingHelper::V4PingHelper(Ipv4Address remote) {
  NS_WARNING_PUSH_DEPRECATED;
  m_factory.SetTypeId("ns3::V4Ping");
  m_factory.Set("Remote", Ipv4AddressValue(remote));
  NS_WARNING_POP;
}

void V4PingHelper::SetAttribute(std::string name, const AttributeValue &value) {
  m_factory.Set(name, value);
}

ApplicationContainer V4PingHelper::Install(Ptr<Node> node) const {
  return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer V4PingHelper::Install(std::string nodeName) const {
  Ptr<Node> node = Names::Find<Node>(nodeName);
  return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer V4PingHelper::Install(NodeContainer c) const {
  ApplicationContainer apps;
  for (auto i = c.Begin(); i != c.End(); ++i) {
    apps.Add(InstallPriv(*i));
  }

  return apps;
}

Ptr<Application> V4PingHelper::InstallPriv(Ptr<Node> node) const {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

  Ptr<V4Ping> app = m_factory.Create<V4Ping>();
  node->AddApplication(app);

  return app;

#pragma GCC diagnostic pop
}

} // namespace ns3
