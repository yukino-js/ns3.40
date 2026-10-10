
#ifndef DSDV_HELPER_H
#define DSDV_HELPER_H

#include "ns3/ipv4-routing-helper.h"
#include "ns3/node-container.h"
#include "ns3/node.h"
#include "ns3/object-factory.h"

namespace ns3 {
class DsdvHelper : public Ipv4RoutingHelper {
public:
  DsdvHelper();
  ~DsdvHelper() override;
  DsdvHelper *Copy() const override;

  Ptr<Ipv4RoutingProtocol> Create(Ptr<Node> node) const override;
  void Set(std::string name, const AttributeValue &value);

private:
  ObjectFactory m_agentFactory;
};

} // namespace ns3

#endif
