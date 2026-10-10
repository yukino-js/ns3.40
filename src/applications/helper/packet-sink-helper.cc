
#include "packet-sink-helper.h"

#include "ns3/inet-socket-address.h"
#include "ns3/names.h"
#include "ns3/string.h"

namespace ns3 {

PacketSinkHelper::PacketSinkHelper(std::string protocol, Address address) {
  m_factory.SetTypeId("ns3::PacketSink");
  m_factory.Set("Protocol", StringValue(protocol));
  m_factory.Set("Local", AddressValue(address));
}

void PacketSinkHelper::SetAttribute(std::string name,
                                    const AttributeValue &value) {
  m_factory.Set(name, value);
}

ApplicationContainer PacketSinkHelper::Install(Ptr<Node> node) const {
  return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer PacketSinkHelper::Install(std::string nodeName) const {
  Ptr<Node> node = Names::Find<Node>(nodeName);
  return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer PacketSinkHelper::Install(NodeContainer c) const {
  ApplicationContainer apps;
  for (auto i = c.Begin(); i != c.End(); ++i) {
    apps.Add(InstallPriv(*i));
  }

  return apps;
}

Ptr<Application> PacketSinkHelper::InstallPriv(Ptr<Node> node) const {
  Ptr<Application> app = m_factory.Create<Application>();
  node->AddApplication(app);

  return app;
}

} // namespace ns3
