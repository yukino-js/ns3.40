
#ifndef RIPNG_HELPER_H
#define RIPNG_HELPER_H

#include "ipv6-routing-helper.h"

#include "ns3/node-container.h"
#include "ns3/node.h"
#include "ns3/object-factory.h"

#include <map>

namespace ns3 {

class RipNgHelper : public Ipv6RoutingHelper {
public:
  RipNgHelper();

  RipNgHelper(const RipNgHelper &o);

  ~RipNgHelper() override;

  RipNgHelper &operator=(const RipNgHelper &) = delete;

  RipNgHelper *Copy() const override;

  Ptr<Ipv6RoutingProtocol> Create(Ptr<Node> node) const override;

  void Set(std::string name, const AttributeValue &value);

  int64_t AssignStreams(NodeContainer c, int64_t stream);

  void SetDefaultRouter(Ptr<Node> node, Ipv6Address nextHop,
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
