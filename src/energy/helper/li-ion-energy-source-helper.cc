
#include "li-ion-energy-source-helper.h"

#include "ns3/energy-source.h"

namespace ns3 {

LiIonEnergySourceHelper::LiIonEnergySourceHelper() {
  m_liIonEnergySource.SetTypeId("ns3::LiIonEnergySource");
}

LiIonEnergySourceHelper::~LiIonEnergySourceHelper() {}

void LiIonEnergySourceHelper::Set(std::string name, const AttributeValue &v) {
  m_liIonEnergySource.Set(name, v);
}

Ptr<EnergySource> LiIonEnergySourceHelper::DoInstall(Ptr<Node> node) const {
  NS_ASSERT(node);
  Ptr<EnergySource> source = m_liIonEnergySource.Create<EnergySource>();
  NS_ASSERT(source);
  source->SetNode(node);
  return source;
}

} // namespace ns3
