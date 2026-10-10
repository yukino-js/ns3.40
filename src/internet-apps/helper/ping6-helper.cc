
#include "ping6-helper.h"

#include "ns3/ping6.h"
#include "ns3/uinteger.h"
#include "ns3/warnings.h"

namespace ns3 {

Ping6Helper::Ping6Helper() : m_ifIndex(0) {
  NS_WARNING_PUSH_DEPRECATED;
  m_factory.SetTypeId(Ping6::GetTypeId());
  NS_WARNING_POP;
}

void Ping6Helper::SetLocal(Ipv6Address ip) { m_localIp = ip; }

void Ping6Helper::SetRemote(Ipv6Address ip) { m_remoteIp = ip; }

void Ping6Helper::SetAttribute(std::string name, const AttributeValue &value) {
  m_factory.Set(name, value);
}

ApplicationContainer Ping6Helper::Install(NodeContainer c) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

  ApplicationContainer apps;
  for (auto i = c.Begin(); i != c.End(); ++i) {
    Ptr<Node> node = *i;
    Ptr<Ping6> client = m_factory.Create<Ping6>();
    client->SetLocal(m_localIp);
    client->SetRemote(m_remoteIp);
    client->SetIfIndex(m_ifIndex);
    client->SetRouters(m_routers);
    node->AddApplication(client);
    apps.Add(client);
  }
  return apps;

#pragma GCC diagnostic pop
}

void Ping6Helper::SetIfIndex(uint32_t ifIndex) { m_ifIndex = ifIndex; }

void Ping6Helper::SetRoutersAddress(std::vector<Ipv6Address> routers) {
  m_routers = routers;
}

} // namespace ns3
