
#ifndef ENERGY_HARVESTER_CONTAINER_H
#define ENERGY_HARVESTER_CONTAINER_H

#include "ns3/energy-harvester.h"
#include "ns3/object.h"

#include <stdint.h>
#include <vector>

namespace ns3 {

class EnergyHarvester;

class EnergyHarvesterContainer : public Object {
public:
  typedef std::vector<Ptr<EnergyHarvester>>::const_iterator Iterator;

public:
  static TypeId GetTypeId();
  EnergyHarvesterContainer();
  ~EnergyHarvesterContainer() override;

  EnergyHarvesterContainer(Ptr<EnergyHarvester> harvester);

  EnergyHarvesterContainer(std::string harvesterName);

  EnergyHarvesterContainer(const EnergyHarvesterContainer &a,
                           const EnergyHarvesterContainer &b);

  Iterator Begin() const;

  Iterator End() const;

  uint32_t GetN() const;

  Ptr<EnergyHarvester> Get(uint32_t i) const;

  void Add(EnergyHarvesterContainer container);

  void Add(Ptr<EnergyHarvester> harvester);

  void Add(std::string harvesterName);

  void Clear();

private:
  void DoDispose() override;

  void DoInitialize() override;

private:
  std::vector<Ptr<EnergyHarvester>> m_harvesters;
};

} // namespace ns3

#endif
