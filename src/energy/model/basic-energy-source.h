
#ifndef BASIC_ENERGY_SOURCE_H
#define BASIC_ENERGY_SOURCE_H

#include "energy-source.h"

#include "ns3/event-id.h"
#include "ns3/nstime.h"
#include "ns3/traced-value.h"

namespace ns3 {

class BasicEnergySource : public EnergySource {
public:
  static TypeId GetTypeId();
  BasicEnergySource();
  ~BasicEnergySource() override;

  double GetInitialEnergy() const override;

  double GetSupplyVoltage() const override;

  double GetRemainingEnergy() override;

  double GetEnergyFraction() override;

  void UpdateEnergySource() override;

  void SetInitialEnergy(double initialEnergyJ);

  void SetSupplyVoltage(double supplyVoltageV);

  void SetEnergyUpdateInterval(Time interval);

  Time GetEnergyUpdateInterval() const;

private:
  void DoInitialize() override;

  void DoDispose() override;

  void HandleEnergyDrainedEvent();

  void HandleEnergyRechargedEvent();

  void CalculateRemainingEnergy();

private:
  double m_initialEnergyJ;
  double m_supplyVoltageV;
  double m_lowBatteryTh;
  double m_highBatteryTh;
  bool m_depleted;
  TracedValue<double> m_remainingEnergyJ;
  EventId m_energyUpdateEvent;
  Time m_lastUpdateTime;
  Time m_energyUpdateInterval;
};

} // namespace ns3

#endif
