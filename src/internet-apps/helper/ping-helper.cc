
#include "ping-helper.h"

#include "ns3/names.h"
#include "ns3/ping.h"

namespace ns3 {

PingHelper::PingHelper() { m_factory.SetTypeId("ns3::Ping"); }

PingHelper::PingHelper(Address remote, Address local) {
  m_factory.SetTypeId("ns3::Ping");
  m_factory.Set("Destination", AddressValue(remote));
  m_factory.Set("InterfaceAddress", AddressValue(local));
}

void PingHelper::SetAttribute(std::string name, const AttributeValue &value) {
  m_factory.Set(name, value);
}

ApplicationContainer PingHelper::Install(Ptr<Node> node) const {
  return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer PingHelper::Install(std::string nodeName) const {
  Ptr<Node> node = Names::Find<Node>(nodeName);
  return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer PingHelper::Install(NodeContainer c) const {
  ApplicationContainer apps;
  for (auto i = c.Begin(); i != c.End(); ++i) {
    apps.Add(InstallPriv(*i));
  }

  return apps;
}

Ptr<Application> PingHelper::InstallPriv(Ptr<Node> node) const {
  Ptr<Ping> app = m_factory.Create<Ping>();
  node->AddApplication(app);

  return app;
}

} // namespace ns3
