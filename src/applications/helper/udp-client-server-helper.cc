#include "udp-client-server-helper.h"

#include "ns3/string.h"
#include "ns3/udp-client.h"
#include "ns3/udp-server.h"
#include "ns3/udp-trace-client.h"
#include "ns3/uinteger.h"

namespace ns3 {

UdpServerHelper::UdpServerHelper() {
  m_factory.SetTypeId(UdpServer::GetTypeId());
}

UdpServerHelper::UdpServerHelper(uint16_t port) {
  m_factory.SetTypeId(UdpServer::GetTypeId());
  SetAttribute("Port", UintegerValue(port));
}

void UdpServerHelper::SetAttribute(std::string name,
                                   const AttributeValue &value) {
  m_factory.Set(name, value);
}

ApplicationContainer UdpServerHelper::Install(NodeContainer c) {
  ApplicationContainer apps;
  for (auto i = c.Begin(); i != c.End(); ++i) {
    Ptr<Node> node = *i;

    m_server = m_factory.Create<UdpServer>();
    node->AddApplication(m_server);
    apps.Add(m_server);
  }
  return apps;
}

Ptr<UdpServer> UdpServerHelper::GetServer() { return m_server; }

UdpClientHelper::UdpClientHelper() {
  m_factory.SetTypeId(UdpClient::GetTypeId());
}

UdpClientHelper::UdpClientHelper(Address address, uint16_t port) {
  m_factory.SetTypeId(UdpClient::GetTypeId());
  SetAttribute("RemoteAddress", AddressValue(address));
  SetAttribute("RemotePort", UintegerValue(port));
}

UdpClientHelper::UdpClientHelper(Address address) {
  m_factory.SetTypeId(UdpClient::GetTypeId());
  SetAttribute("RemoteAddress", AddressValue(address));
}

void UdpClientHelper::SetAttribute(std::string name,
                                   const AttributeValue &value) {
  m_factory.Set(name, value);
}

ApplicationContainer UdpClientHelper::Install(NodeContainer c) {
  ApplicationContainer apps;
  for (auto i = c.Begin(); i != c.End(); ++i) {
    Ptr<Node> node = *i;
    Ptr<UdpClient> client = m_factory.Create<UdpClient>();
    node->AddApplication(client);
    apps.Add(client);
  }
  return apps;
}

UdpTraceClientHelper::UdpTraceClientHelper() {
  m_factory.SetTypeId(UdpTraceClient::GetTypeId());
}

UdpTraceClientHelper::UdpTraceClientHelper(Address address, uint16_t port,
                                           std::string filename) {
  m_factory.SetTypeId(UdpTraceClient::GetTypeId());
  SetAttribute("RemoteAddress", AddressValue(address));
  SetAttribute("RemotePort", UintegerValue(port));
  SetAttribute("TraceFilename", StringValue(filename));
}

UdpTraceClientHelper::UdpTraceClientHelper(Address address,
                                           std::string filename) {
  m_factory.SetTypeId(UdpTraceClient::GetTypeId());
  SetAttribute("RemoteAddress", AddressValue(address));
  SetAttribute("TraceFilename", StringValue(filename));
}

void UdpTraceClientHelper::SetAttribute(std::string name,
                                        const AttributeValue &value) {
  m_factory.Set(name, value);
}

ApplicationContainer UdpTraceClientHelper::Install(NodeContainer c) {
  ApplicationContainer apps;
  for (auto i = c.Begin(); i != c.End(); ++i) {
    Ptr<Node> node = *i;
    Ptr<UdpTraceClient> client = m_factory.Create<UdpTraceClient>();
    node->AddApplication(client);
    apps.Add(client);
  }
  return apps;
}

} // namespace ns3
