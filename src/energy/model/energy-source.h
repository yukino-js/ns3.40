
#ifndef ENERGY_SOURCE_H
#define ENERGY_SOURCE_H

#include "device-energy-model-container.h"
#include "energy-harvester.h"

#include "ns3/node.h"
#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/type-id.h"

namespace ns3 {

class EnergyHarvester;

class EnergySource : public Object {
public:
  static TypeId GetTypeId();
  EnergySource();
  ~EnergySource() override;

  virtual double GetSupplyVoltage() const = 0;

  virtual double GetInitialEnergy() const = 0;

  virtual double GetRemainingEnergy() = 0;

  virtual double GetEnergyFraction() = 0;

  virtual void UpdateEnergySource() = 0;

  void SetNode(Ptr<Node> node);

  Ptr<Node> GetNode() const;

  void AppendDeviceEnergyModel(Ptr<DeviceEnergyModel> deviceEnergyModelPtr);

  DeviceEnergyModelContainer FindDeviceEnergyModels(TypeId tid);

  DeviceEnergyModelContainer FindDeviceEnergyModels(std::string name);

  void InitializeDeviceModels();

  void DisposeDeviceModels();

  void ConnectEnergyHarvester(Ptr<EnergyHarvester> energyHarvesterPtr);

private:
  void DoDispose() override;

private:
  DeviceEnergyModelContainer m_models;

  Ptr<Node> m_node;

  std::vector<Ptr<EnergyHarvester>> m_harvesters;

protected:
  double CalculateTotalCurrent();

  void NotifyEnergyDrained();

  void NotifyEnergyRecharged();

  void NotifyEnergyChanged();

  void BreakDeviceEnergyModelRefCycle();
};

} // namespace ns3

#endif
