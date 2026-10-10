
#ifndef ENERGY_HARVESTER_H
#define ENERGY_HARVESTER_H

#include <iostream>

#include "ns3/energy-source-container.h"
#include "ns3/node.h"
#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/type-id.h"

namespace ns3 {

class EnergySource;

class EnergyHarvester : public Object {
public:
  static TypeId GetTypeId();

  EnergyHarvester();

  ~EnergyHarvester() override;

  void SetNode(Ptr<Node> node);

  Ptr<Node> GetNode() const;

  void SetEnergySource(Ptr<EnergySource> source);

  Ptr<EnergySource> GetEnergySource() const;

  double GetPower() const;

private:
  void DoDispose() override;

  virtual double DoGetPower() const;

private:
  Ptr<Node> m_node;

  Ptr<EnergySource> m_energySource;

protected:
};

} // namespace ns3

#endif
