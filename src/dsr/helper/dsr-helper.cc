
#include "dsr-helper.h"

#include "ns3/callback.h"
#include "ns3/dsr-options.h"
#include "ns3/dsr-routing.h"
#include "ns3/ipv4-route.h"
#include "ns3/log.h"
#include "ns3/names.h"
#include "ns3/node-container.h"
#include "ns3/node-list.h"
#include "ns3/node.h"
#include "ns3/ptr.h"
#include "ns3/tcp-l4-protocol.h"
#include "ns3/udp-l4-protocol.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("DsrHelper");

DsrHelper::DsrHelper() {
  NS_LOG_FUNCTION(this);
  m_agentFactory.SetTypeId("ns3::dsr::DsrRouting");
}

DsrHelper::DsrHelper(const DsrHelper &o) : m_agentFactory(o.m_agentFactory) {
  NS_LOG_FUNCTION(this);
}

DsrHelper::~DsrHelper() { NS_LOG_FUNCTION(this); }

DsrHelper *DsrHelper::Copy() const {
  NS_LOG_FUNCTION(this);
  return new DsrHelper(*this);
}

Ptr<ns3::dsr::DsrRouting> DsrHelper::Create(Ptr<Node> node) const {
  NS_LOG_FUNCTION(this);
  Ptr<ns3::dsr::DsrRouting> agent =
      m_agentFactory.Create<ns3::dsr::DsrRouting>();
  Ptr<UdpL4Protocol> udp = node->GetObject<UdpL4Protocol>();
  agent->SetDownTarget(udp->GetDownTarget());
  udp->SetDownTarget(MakeCallback(&dsr::DsrRouting::Send, agent));
  Ptr<TcpL4Protocol> tcp = node->GetObject<TcpL4Protocol>();
  tcp->SetDownTarget(MakeCallback(&dsr::DsrRouting::Send, agent));
  Ptr<Icmpv4L4Protocol> icmp = node->GetObject<Icmpv4L4Protocol>();
  icmp->SetDownTarget(MakeCallback(&dsr::DsrRouting::Send, agent));
  node->AggregateObject(agent);
  return agent;
}

void DsrHelper::Set(std::string name, const AttributeValue &value) {
  m_agentFactory.Set(name, value);
}

} // namespace ns3
