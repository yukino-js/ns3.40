
#include "three-gpp-http-helper.h"

#include <ns3/names.h>

namespace ns3 {

ThreeGppHttpClientHelper::ThreeGppHttpClientHelper(const Address &address) {
  m_factory.SetTypeId("ns3::ThreeGppHttpClient");
  m_factory.Set("RemoteServerAddress", AddressValue(address));
}

void ThreeGppHttpClientHelper::SetAttribute(const std::string &name,
                                            const AttributeValue &value) {
  m_factory.Set(name, value);
}

ApplicationContainer ThreeGppHttpClientHelper::Install(Ptr<Node> node) const {
  return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer
ThreeGppHttpClientHelper::Install(const std::string &nodeName) const {
  Ptr<Node> node = Names::Find<Node>(nodeName);
  return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer ThreeGppHttpClientHelper::Install(NodeContainer c) const {
  ApplicationContainer apps;
  for (auto i = c.Begin(); i != c.End(); ++i) {
    apps.Add(InstallPriv(*i));
  }

  return apps;
}

Ptr<Application> ThreeGppHttpClientHelper::InstallPriv(Ptr<Node> node) const {
  Ptr<Application> app = m_factory.Create<Application>();
  node->AddApplication(app);

  return app;
}

ThreeGppHttpServerHelper::ThreeGppHttpServerHelper(const Address &address) {
  m_factory.SetTypeId("ns3::ThreeGppHttpServer");
  m_factory.Set("LocalAddress", AddressValue(address));
}

void ThreeGppHttpServerHelper::SetAttribute(const std::string &name,
                                            const AttributeValue &value) {
  m_factory.Set(name, value);
}

ApplicationContainer ThreeGppHttpServerHelper::Install(Ptr<Node> node) const {
  return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer
ThreeGppHttpServerHelper::Install(const std::string &nodeName) const {
  Ptr<Node> node = Names::Find<Node>(nodeName);
  return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer ThreeGppHttpServerHelper::Install(NodeContainer c) const {
  ApplicationContainer apps;
  for (auto i = c.Begin(); i != c.End(); ++i) {
    apps.Add(InstallPriv(*i));
  }

  return apps;
}

Ptr<Application> ThreeGppHttpServerHelper::InstallPriv(Ptr<Node> node) const {
  Ptr<Application> app = m_factory.Create<Application>();
  node->AddApplication(app);

  return app;
}

} // namespace ns3
