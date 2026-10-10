#ifndef UDP_CLIENT_SERVER_HELPER_H
#define UDP_CLIENT_SERVER_HELPER_H

#include "ns3/application-container.h"
#include "ns3/ipv4-address.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/udp-client.h"
#include "ns3/udp-server.h"

#include <stdint.h>

namespace ns3 {
class UdpServerHelper {
public:
  UdpServerHelper();

  UdpServerHelper(uint16_t port);

  void SetAttribute(std::string name, const AttributeValue &value);

  ApplicationContainer Install(NodeContainer c);

  Ptr<UdpServer> GetServer();

private:
  ObjectFactory m_factory;
  Ptr<UdpServer> m_server;
};

class UdpClientHelper {
public:
  UdpClientHelper();

  UdpClientHelper(Address ip, uint16_t port);

  UdpClientHelper(Address addr);

  void SetAttribute(std::string name, const AttributeValue &value);

  ApplicationContainer Install(NodeContainer c);

private:
  ObjectFactory m_factory;
};

class UdpTraceClientHelper {
public:
  UdpTraceClientHelper();

  UdpTraceClientHelper(Address ip, uint16_t port, std::string filename);
  UdpTraceClientHelper(Address addr, std::string filename);

  void SetAttribute(std::string name, const AttributeValue &value);

  ApplicationContainer Install(NodeContainer c);

private:
  ObjectFactory m_factory;
};

} // namespace ns3

#endif
