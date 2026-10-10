
#include "bulk-send-helper.h"

#include "ns3/inet-socket-address.h"
#include "ns3/names.h"
#include "ns3/packet-socket-address.h"
#include "ns3/string.h"

namespace ns3 {

BulkSendHelper::BulkSendHelper(std::string protocol, Address address) {
  m_factory.SetTypeId("ns3::BulkSendApplication");
  m_factory.Set("Protocol", StringValue(protocol));
  m_factory.Set("Remote", AddressValue(address));
}

void BulkSendHelper::SetAttribute(std::string name,
                                  const AttributeValue &value) {
  m_factory.Set(name, value);
}

ApplicationContainer BulkSendHelper::Install(Ptr<Node> node) const {
  return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer BulkSendHelper::Install(std::string nodeName) const {
  Ptr<Node> node = Names::Find<Node>(nodeName);
  return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer BulkSendHelper::Install(NodeContainer c) const {
  ApplicationContainer apps;
  for (auto i = c.Begin(); i != c.End(); ++i) {
    apps.Add(InstallPriv(*i));
  }

  return apps;
}

Ptr<Application> BulkSendHelper::InstallPriv(Ptr<Node> node) const {
  Ptr<Application> app = m_factory.Create<Application>();
  node->AddApplication(app);

  return app;
}

} // namespace ns3
