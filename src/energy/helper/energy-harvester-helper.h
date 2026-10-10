
#ifndef ENERGY_HARVESTER_HELPER_H
#define ENERGY_HARVESTER_HELPER_H

#include "energy-harvester-container.h"
#include "energy-source-container.h"

#include "ns3/attribute.h"
#include "ns3/energy-harvester.h"
#include "ns3/energy-source.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/ptr.h"

namespace ns3 {

class EnergyHarvesterHelper {
public:
  virtual ~EnergyHarvesterHelper();

  virtual void Set(std::string name, const AttributeValue &v) = 0;

  EnergyHarvesterContainer Install(Ptr<EnergySource> source) const;

  EnergyHarvesterContainer Install(EnergySourceContainer sourceContainer) const;

  EnergyHarvesterContainer Install(std::string sourceName) const;

private:
  virtual Ptr<EnergyHarvester> DoInstall(Ptr<EnergySource> source) const = 0;
};

} // namespace ns3

#endif
