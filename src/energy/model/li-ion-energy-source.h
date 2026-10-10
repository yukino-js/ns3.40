
#ifndef LI_ION_ENERGY_SOURCE_H
#define LI_ION_ENERGY_SOURCE_H

#include "energy-source.h"

#include "ns3/event-id.h"
#include "ns3/nstime.h"
#include "ns3/traced-value.h"

namespace ns3 {

class LiIonEnergySource : public EnergySource {
public:
  static TypeId GetTypeId();
  LiIonEnergySource();
  ~LiIonEnergySource() override;

  double GetInitialEnergy() const override;

  void SetInitialEnergy(double initialEnergyJ);

  double GetSupplyVoltage() const override;

  void SetInitialSupplyVoltage(double supplyVoltageV);

  double GetRemainingEnergy() override;

  double GetEnergyFraction() override;

  NS_DEPRECATED_3_40("Use GenericBatteryModel instead")
  virtual void DecreaseRemainingEnergy(double energyJ);

  NS_DEPRECATED_3_40("Use GenericBatteryModel instead")
  virtual void IncreaseRemainingEnergy(double energyJ);

  void UpdateEnergySource() override;

  void SetEnergyUpdateInterval(Time interval);

  Time GetEnergyUpdateInterval() const;

private:
  void DoInitialize() override;
  void DoDispose() override;

  void HandleEnergyDrainedEvent();

  void CalculateRemainingEnergy();

  double GetVoltage(double current) const;

private:
  double m_initialEnergyJ;
  TracedValue<double> m_remainingEnergyJ;
  double m_drainedCapacity;
  double m_supplyVoltageV;
  double m_lowBatteryTh;
  EventId m_energyUpdateEvent;
  Time m_lastUpdateTime;
  Time m_energyUpdateInterval;
  double m_eFull;
  double m_eNom;
  double m_eExp;
  double m_internalResistance;
  double m_qRated;
  double m_qNom;
  double m_qExp;
  double m_typCurrent;
  double m_minVoltTh;
};

} // namespace ns3

#endif
