#include "dsdv-helper.h"

#include "ns3/dsdv-routing-protocol.h"
#include "ns3/ipv4-list-routing.h"
#include "ns3/names.h"
#include "ns3/node-list.h"

namespace ns3 {
DsdvHelper::~DsdvHelper() {}

DsdvHelper::DsdvHelper() : Ipv4RoutingHelper() {
  m_agentFactory.SetTypeId("ns3::dsdv::RoutingProtocol");
}

DsdvHelper *DsdvHelper::Copy() const { return new DsdvHelper(*this); }

Ptr<Ipv4RoutingProtocol> DsdvHelper::Create(Ptr<Node> node) const {
  Ptr<dsdv::RoutingProtocol> agent =
      m_agentFactory.Create<dsdv::RoutingProtocol>();
  node->AggregateObject(agent);
  return agent;
}

void DsdvHelper::Set(std::string name, const AttributeValue &value) {
  m_agentFactory.Set(name, value);
}

} // namespace ns3
