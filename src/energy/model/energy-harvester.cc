
#include "energy-harvester.h"

#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("EnergyHarvester");

NS_OBJECT_ENSURE_REGISTERED(EnergyHarvester);

TypeId EnergyHarvester::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::EnergyHarvester").SetParent<Object>().SetGroupName("Energy");
  return tid;
}

EnergyHarvester::EnergyHarvester() { NS_LOG_FUNCTION(this); }

EnergyHarvester::~EnergyHarvester() { NS_LOG_FUNCTION(this); }

void EnergyHarvester::SetNode(Ptr<Node> node) {
  NS_LOG_FUNCTION(this);
  NS_ASSERT(node);
  m_node = node;
}

Ptr<Node> EnergyHarvester::GetNode() const {
  NS_LOG_FUNCTION(this);
  return m_node;
}

void EnergyHarvester::SetEnergySource(Ptr<EnergySource> source) {
  NS_LOG_FUNCTION(this);
  NS_ASSERT(source);
  m_energySource = source;
}

Ptr<EnergySource> EnergyHarvester::GetEnergySource() const {
  NS_LOG_FUNCTION(this);
  return m_energySource;
}

double EnergyHarvester::GetPower() const {
  NS_LOG_FUNCTION(this);
  return DoGetPower();
}

void EnergyHarvester::DoDispose() { NS_LOG_FUNCTION(this); }

double EnergyHarvester::DoGetPower() const {
  NS_LOG_FUNCTION(this);
  return 0.0;
}

} // namespace ns3
