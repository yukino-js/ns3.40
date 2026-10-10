
#ifndef GENERIC_BATTERY_MODEL_H
#define GENERIC_BATTERY_MODEL_H

#include "energy-source.h"

#include <ns3/event-id.h>
#include <ns3/nstime.h>
#include <ns3/traced-value.h>

namespace ns3 {

enum GenericBatteryType { LION_LIPO = 0, NIMH_NICD = 1, LEADACID = 2 };

enum BatteryModel {
  PANASONIC_HHR650D_NIMH = 0,
  CSB_GP1272_LEADACID = 1,
  PANASONIC_CGR18650DA_LION = 2,
  RSPRO_LGP12100_LEADACID = 3,
  PANASONIC_N700AAC_NICD = 4
};

struct BatteryPresets {
  GenericBatteryType batteryType;
  std::string description;
  double vFull;
  double qMax;
  double vNom;
  double qNom;
  double vExp;
  double qExp;
  double internalResistance;
  double typicalCurrent;
  double cuttoffVoltage;
};

static BatteryPresets g_batteryPreset[] = {
    {NIMH_NICD, "Panasonic HHR650D | NiMH | 1.2V 6.5Ah | Size: D", 1.39, 7.0,
     1.18, 6.25, 1.28, 1.3, 0.0046, 1.3, 1.0},
    {LEADACID, "CSB GP1272 | Lead Acid | 12V 7.2Ah", 12.8, 7.2, 11.5, 4.5, 12.5,
     2, 0.056, 0.36, 8.0},
    {LION_LIPO, "Panasonic CGR18650DA | Li-Ion | 3.6V 2.45Ah | Size: A", 4.17,
     2.33, 3.57, 2.14, 3.714, 1.74, 0.0830, 0.466, 3.0},
    {LEADACID, "Rs PRO LGP12100 | Lead Acid | 12V 100Ah", 12.60, 130, 12.44,
     12.3, 12.52, 12, 0.00069, 5, 11},
    {NIMH_NICD, "PANASONIC N-700AAC | NiCd | 1.2V 700mAh | Size: AA", 1.38,
     0.790, 1.17, 0.60, 1.25, 0.24, 0.016, 0.7, 0.8}};

class GenericBatteryModel : public EnergySource {
public:
  static TypeId GetTypeId();

  GenericBatteryModel();

  ~GenericBatteryModel() override;

  double GetInitialEnergy() const override;

  double GetSupplyVoltage() const override;

  double GetRemainingEnergy() override;

  double GetEnergyFraction() override;

  void UpdateEnergySource() override;

  void SetEnergyUpdateInterval(Time interval);

  void SetDrainedCapacity(double drainedCapacity);

  double GetDrainedCapacity() const;

  double GetStateOfCharge() const;

  Time GetEnergyUpdateInterval() const;

private:
  void DoInitialize() override;
  void DoDispose() override;

  void BatteryDepletedEvent();

  void BatteryChargedEvent();

  void CalculateRemainingEnergy();

  double GetVoltage(double current);

  double GetChargeVoltage(double current);

private:
  TracedValue<double> m_remainingEnergyJ;
  double m_drainedCapacity;
  double m_currentFiltered;
  double m_entn;
  double m_expZone;
  Time m_energyUpdateLapseTime;
  double m_supplyVoltageV;
  double m_lowBatteryTh;
  EventId m_energyUpdateEvent;
  Time m_lastUpdateTime;
  Time m_energyUpdateInterval;
  double m_vFull;
  double m_vNom;
  double m_vExp;
  double m_internalResistance;
  double m_qMax;
  double m_qNom;
  double m_qExp;
  double m_typicalCurrent;
  double m_cutoffVoltage;
  GenericBatteryType m_batteryType;
};

} // namespace ns3

#endif
