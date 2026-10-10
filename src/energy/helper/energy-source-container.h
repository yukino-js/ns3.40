
#ifndef ENERGY_SOURCE_CONTAINER_H
#define ENERGY_SOURCE_CONTAINER_H

#include "ns3/energy-source.h"
#include "ns3/object.h"

#include <stdint.h>
#include <vector>

namespace ns3 {

class EnergySourceContainer : public Object {
public:
  typedef std::vector<Ptr<EnergySource>>::const_iterator Iterator;

public:
  static TypeId GetTypeId();
  EnergySourceContainer();
  ~EnergySourceContainer() override;

  EnergySourceContainer(Ptr<EnergySource> source);

  EnergySourceContainer(std::string sourceName);

  EnergySourceContainer(const EnergySourceContainer &a,
                        const EnergySourceContainer &b);

  Iterator Begin() const;

  Iterator End() const;

  uint32_t GetN() const;

  Ptr<EnergySource> Get(uint32_t i) const;

  void Add(EnergySourceContainer container);

  void Add(Ptr<EnergySource> source);

  void Add(std::string sourceName);

private:
  void DoDispose() override;

  void DoInitialize() override;

private:
  std::vector<Ptr<EnergySource>> m_sources;
};

} // namespace ns3

#endif
