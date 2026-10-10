
#include "basic-energy-source-helper.h"

#include "ns3/energy-source.h"

namespace ns3 {

BasicEnergySourceHelper::BasicEnergySourceHelper() {
  m_basicEnergySource.SetTypeId("ns3::BasicEnergySource");
}

BasicEnergySourceHelper::~BasicEnergySourceHelper() {}

void BasicEnergySourceHelper::Set(std::string name, const AttributeValue &v) {
  m_basicEnergySource.Set(name, v);
}

Ptr<EnergySource> BasicEnergySourceHelper::DoInstall(Ptr<Node> node) const {
  NS_ASSERT(node);
  Ptr<EnergySource> source = m_basicEnergySource.Create<EnergySource>();
  NS_ASSERT(source);
  source->SetNode(node);
  return source;
}

} // namespace ns3
