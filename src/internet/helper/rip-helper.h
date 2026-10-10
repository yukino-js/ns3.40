
#ifndef RIP_HELPER_H
#define RIP_HELPER_H

#include "ipv4-routing-helper.h"

#include "ns3/node-container.h"
#include "ns3/node.h"
#include "ns3/object-factory.h"

namespace ns3 {

class RipHelper : public Ipv4RoutingHelper {
public:
  RipHelper();

  RipHelper(const RipHelper &o);

  ~RipHelper() override;

  RipHelper &operator=(const RipHelper &) = delete;

  RipHelper *Copy() const override;

  Ptr<Ipv4RoutingProtocol> Create(Ptr<Node> node) const override;

  void Set(std::string name, const AttributeValue &value);

  int64_t AssignStreams(NodeContainer c, int64_t stream);

  void SetDefaultRouter(Ptr<Node> node, Ipv4Address nextHop,
                        uint32_t interface);

  void ExcludeInterface(Ptr<Node> node, uint32_t interface);

  void SetInterfaceMetric(Ptr<Node> node, uint32_t interface, uint8_t metric);

private:
  ObjectFactory m_factory;

  std::map<Ptr<Node>, std::set<uint32_t>> m_interfaceExclusions;
  std::map<Ptr<Node>, std::map<uint32_t, uint8_t>> m_interfaceMetrics;
};

} // namespace ns3

#endif
