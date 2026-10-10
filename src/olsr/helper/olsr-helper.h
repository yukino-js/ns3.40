#ifndef OLSR_HELPER_H
#define OLSR_HELPER_H

#include "ns3/ipv4-routing-helper.h"
#include "ns3/node-container.h"
#include "ns3/node.h"
#include "ns3/object-factory.h"

#include <map>
#include <set>

namespace ns3 {

class OlsrHelper : public Ipv4RoutingHelper {
public:
  OlsrHelper();

  OlsrHelper(const OlsrHelper &o);

  OlsrHelper &operator=(const OlsrHelper &) = delete;

  OlsrHelper *Copy() const override;

  void ExcludeInterface(Ptr<Node> node, uint32_t interface);

  Ptr<Ipv4RoutingProtocol> Create(Ptr<Node> node) const override;

  void Set(std::string name, const AttributeValue &value);

  int64_t AssignStreams(NodeContainer c, int64_t stream);

private:
  ObjectFactory m_agentFactory;

  std::map<Ptr<Node>, std::set<uint32_t>> m_interfaceExclusions;
};

} // namespace ns3

#endif
