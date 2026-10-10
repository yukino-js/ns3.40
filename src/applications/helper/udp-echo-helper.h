#ifndef UDP_ECHO_HELPER_H
#define UDP_ECHO_HELPER_H

#include "ns3/application-container.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"

#include <stdint.h>

namespace ns3 {

class UdpEchoServerHelper {
public:
  UdpEchoServerHelper(uint16_t port);

  void SetAttribute(std::string name, const AttributeValue &value);

  ApplicationContainer Install(Ptr<Node> node) const;

  ApplicationContainer Install(std::string nodeName) const;

  ApplicationContainer Install(NodeContainer c) const;

private:
  Ptr<Application> InstallPriv(Ptr<Node> node) const;

  ObjectFactory m_factory;
};

class UdpEchoClientHelper {
public:
  UdpEchoClientHelper(Address ip, uint16_t port);
  UdpEchoClientHelper(Address addr);

  void SetAttribute(std::string name, const AttributeValue &value);

  void SetFill(Ptr<Application> app, std::string fill);

  void SetFill(Ptr<Application> app, uint8_t fill, uint32_t dataLength);

  void SetFill(Ptr<Application> app, uint8_t *fill, uint32_t fillLength,
               uint32_t dataLength);

  ApplicationContainer Install(Ptr<Node> node) const;

  ApplicationContainer Install(std::string nodeName) const;

  ApplicationContainer Install(NodeContainer c) const;

private:
  Ptr<Application> InstallPriv(Ptr<Node> node) const;
  ObjectFactory m_factory;
};

} // namespace ns3

#endif
