
#include "energy-harvester-helper.h"

#include "ns3/config.h"
#include "ns3/names.h"

namespace ns3 {

EnergyHarvesterHelper::~EnergyHarvesterHelper() {}

EnergyHarvesterContainer
EnergyHarvesterHelper::Install(Ptr<EnergySource> source) const {
  return Install(EnergySourceContainer(source));
}

EnergyHarvesterContainer
EnergyHarvesterHelper::Install(EnergySourceContainer sourceContainer) const {
  EnergyHarvesterContainer container;
  for (auto i = sourceContainer.Begin(); i != sourceContainer.End(); ++i) {
    Ptr<EnergyHarvester> harvester = DoInstall(*i);
    container.Add(harvester);
    Ptr<Node> node = (*i)->GetNode();
    Ptr<EnergyHarvesterContainer> EnergyHarvesterContainerOnNode =
        node->GetObject<EnergyHarvesterContainer>();
    if (!EnergyHarvesterContainerOnNode) {
      ObjectFactory fac;
      fac.SetTypeId("ns3::EnergyHarvesterContainer");
      EnergyHarvesterContainerOnNode = fac.Create<EnergyHarvesterContainer>();
      EnergyHarvesterContainerOnNode->Add(harvester);
      node->AggregateObject(EnergyHarvesterContainerOnNode);
    } else {
      EnergyHarvesterContainerOnNode->Add(harvester);
    }
  }
  return container;
}

EnergyHarvesterContainer
EnergyHarvesterHelper::Install(std::string sourceName) const {
  Ptr<EnergySource> source = Names::Find<EnergySource>(sourceName);
  return Install(source);
}

} // namespace ns3
