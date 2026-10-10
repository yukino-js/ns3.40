
#ifndef BASIC_ENERGY_HARVESTER_HELPER_H
#define BASIC_ENERGY_HARVESTER_HELPER_H

#include "energy-harvester-helper.h"

#include "ns3/energy-source.h"
#include "ns3/node.h"

namespace ns3 {

class BasicEnergyHarvesterHelper : public EnergyHarvesterHelper {
public:
  BasicEnergyHarvesterHelper();
  ~BasicEnergyHarvesterHelper() override;

  void Set(std::string name, const AttributeValue &v) override;

private:
  Ptr<EnergyHarvester> DoInstall(Ptr<EnergySource> source) const override;

private:
  ObjectFactory m_basicEnergyHarvester;
};

} // namespace ns3

#endif
