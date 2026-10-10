
#ifndef ACOUSTIC_MODEM_ENERGY_MODEL_H
#define ACOUSTIC_MODEM_ENERGY_MODEL_H

#include "ns3/device-energy-model.h"
#include "ns3/event-id.h"
#include "ns3/nstime.h"
#include "ns3/traced-value.h"

namespace ns3 {

class AcousticModemEnergyModel : public DeviceEnergyModel {
public:
  typedef Callback<void> AcousticModemEnergyDepletionCallback;

  typedef Callback<void> AcousticModemEnergyRechargeCallback;

public:
  static TypeId GetTypeId();
  AcousticModemEnergyModel();
  ~AcousticModemEnergyModel() override;

  virtual void SetNode(Ptr<Node> node);

  virtual Ptr<Node> GetNode() const;

  void SetEnergySource(Ptr<EnergySource> source) override;
  double GetTotalEnergyConsumption() const override;

  double GetTxPowerW() const;

  void SetTxPowerW(double txPowerW);

  double GetRxPowerW() const;

  void SetRxPowerW(double rxPowerW);

  double GetIdlePowerW() const;

  void SetIdlePowerW(double idlePowerW);

  double GetSleepPowerW() const;

  void SetSleepPowerW(double sleepPowerW);

  int GetCurrentState() const;

  void
  SetEnergyDepletionCallback(AcousticModemEnergyDepletionCallback callback);

  void SetEnergyRechargeCallback(AcousticModemEnergyRechargeCallback callback);

  void ChangeState(int newState) override;

  void HandleEnergyDepletion() override;

  void HandleEnergyRecharged() override;

  void HandleEnergyChanged() override;

private:
  void DoDispose() override;

  double DoGetCurrentA() const override;

  bool IsStateTransitionValid(const int destState);

  void SetMicroModemState(const int state);

private:
  Ptr<Node> m_node;
  Ptr<EnergySource> m_source;

  double m_txPowerW;
  double m_rxPowerW;
  double m_idlePowerW;
  double m_sleepPowerW;

  TracedValue<double> m_totalEnergyConsumption;

  int m_currentState;
  Time m_lastUpdateTime;

  AcousticModemEnergyDepletionCallback m_energyDepletionCallback;

  AcousticModemEnergyRechargeCallback m_energyRechargeCallback;
};

} // namespace ns3

#endif
