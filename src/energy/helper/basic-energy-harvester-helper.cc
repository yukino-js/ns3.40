
#include "basic-energy-harvester-helper.h"

#include "ns3/energy-harvester.h"

namespace ns3 {

BasicEnergyHarvesterHelper::BasicEnergyHarvesterHelper() {
  m_basicEnergyHarvester.SetTypeId("ns3::BasicEnergyHarvester");
}

BasicEnergyHarvesterHelper::~BasicEnergyHarvesterHelper() {}

void BasicEnergyHarvesterHelper::Set(std::string name,
                                     const AttributeValue &v) {
  m_basicEnergyHarvester.Set(name, v);
}

Ptr<EnergyHarvester>
BasicEnergyHarvesterHelper::DoInstall(Ptr<EnergySource> source) const {
  NS_ASSERT(source);
  Ptr<Node> node = source->GetNode();

  Ptr<EnergyHarvester> harvester =
      m_basicEnergyHarvester.Create<EnergyHarvester>();
  NS_ASSERT(harvester);

  source->ConnectEnergyHarvester(harvester);
  harvester->SetNode(node);
  harvester->SetEnergySource(source);
  return harvester;
}

} // namespace ns3
