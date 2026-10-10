
#ifndef RV_BATTERY_MODEL_H
#define RV_BATTERY_MODEL_H

#include "energy-source.h"

#include "ns3/event-id.h"
#include "ns3/nstime.h"
#include "ns3/traced-value.h"

namespace ns3 {

class RvBatteryModel : public EnergySource {
public:
  static TypeId GetTypeId();
  RvBatteryModel();
  ~RvBatteryModel() override;

  double GetInitialEnergy() const override;

  double GetSupplyVoltage() const override;

  double GetRemainingEnergy() override;

  double GetEnergyFraction() override;

  void UpdateEnergySource() override;

  void SetSamplingInterval(Time interval);

  Time GetSamplingInterval() const;

  void SetOpenCircuitVoltage(double voltage);

  double GetOpenCircuitVoltage() const;

  void SetCutoffVoltage(double voltage);

  double GetCutoffVoltage() const;

  void SetAlpha(double alpha);

  double GetAlpha() const;

  void SetBeta(double beta);

  double GetBeta() const;

  double GetBatteryLevel();

  Time GetLifetime() const;

  void SetNumOfTerms(int num);

  int GetNumOfTerms() const;

private:
  void DoInitialize() override;

  void DoDispose() override;

  void HandleEnergyDrainedEvent();

  double Discharge(double load, Time t);

  double RvModelAFunction(Time t, Time sk, Time sk_1, double beta);

private:
  double m_openCircuitVoltage;
  double m_cutoffVoltage;
  double m_alpha;
  double m_beta;

  double m_previousLoad;
  std::vector<double> m_load;
  std::vector<Time> m_timeStamps;
  Time m_lastSampleTime;

  int m_numOfTerms;

  TracedValue<double> m_batteryLevel;

  double m_lowBatteryTh;

  Time m_samplingInterval;
  EventId m_currentSampleEvent;

  TracedValue<Time> m_lifetime;
};

} // namespace ns3

#endif
