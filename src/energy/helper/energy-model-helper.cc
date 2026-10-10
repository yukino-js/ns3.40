
#include "energy-model-helper.h"

#include "ns3/config.h"
#include "ns3/names.h"

namespace ns3 {

EnergySourceHelper::~EnergySourceHelper() {}

EnergySourceContainer EnergySourceHelper::Install(Ptr<Node> node) const {
  return Install(NodeContainer(node));
}

EnergySourceContainer EnergySourceHelper::Install(NodeContainer c) const {
  EnergySourceContainer container;
  for (auto i = c.Begin(); i != c.End(); ++i) {
    Ptr<EnergySource> src = DoInstall(*i);
    container.Add(src);
    Ptr<EnergySourceContainer> EnergySourceContainerOnNode =
        (*i)->GetObject<EnergySourceContainer>();
    if (!EnergySourceContainerOnNode) {
      ObjectFactory fac;
      fac.SetTypeId("ns3::EnergySourceContainer");
      EnergySourceContainerOnNode = fac.Create<EnergySourceContainer>();
      EnergySourceContainerOnNode->Add(src);
      (*i)->AggregateObject(EnergySourceContainerOnNode);
    } else {
      EnergySourceContainerOnNode->Add(src);
    }
  }
  return container;
}

EnergySourceContainer EnergySourceHelper::Install(std::string nodeName) const {
  Ptr<Node> node = Names::Find<Node>(nodeName);
  return Install(node);
}

EnergySourceContainer EnergySourceHelper::InstallAll() const {
  return Install(NodeContainer::GetGlobal());
}

DeviceEnergyModelHelper::~DeviceEnergyModelHelper() {}

DeviceEnergyModelContainer
DeviceEnergyModelHelper::Install(Ptr<NetDevice> device,
                                 Ptr<EnergySource> source) const {
  NS_ASSERT(device);
  NS_ASSERT(source);
  NS_ASSERT(device->GetNode() == source->GetNode());
  DeviceEnergyModelContainer container(DoInstall(device, source));
  return container;
}

DeviceEnergyModelContainer
DeviceEnergyModelHelper::Install(NetDeviceContainer deviceContainer,
                                 EnergySourceContainer sourceContainer) const {
  NS_ASSERT(deviceContainer.GetN() <= sourceContainer.GetN());
  DeviceEnergyModelContainer container;
  auto dev = deviceContainer.Begin();
  auto src = sourceContainer.Begin();
  while (dev != deviceContainer.End()) {
    NS_ASSERT((*dev)->GetNode() == (*src)->GetNode());
    Ptr<DeviceEnergyModel> model = DoInstall(*dev, *src);
    container.Add(model);
    dev++;
    src++;
  }
  return container;
}

} // namespace ns3
