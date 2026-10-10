
#ifndef WIFI_RADIO_ENERGY_MODEL_H
#define WIFI_RADIO_ENERGY_MODEL_H

#include "wifi-phy-listener.h"
#include "wifi-phy-state.h"

#include "ns3/device-energy-model.h"
#include "ns3/event-id.h"
#include "ns3/nstime.h"
#include "ns3/traced-value.h"

namespace ns3 {

class WifiTxCurrentModel;

class WifiRadioEnergyModelPhyListener : public WifiPhyListener {
public:
  typedef Callback<void, double> UpdateTxCurrentCallback;

  WifiRadioEnergyModelPhyListener();
  ~WifiRadioEnergyModelPhyListener() override;

  void SetChangeStateCallback(DeviceEnergyModel::ChangeStateCallback callback);

  void SetUpdateTxCurrentCallback(UpdateTxCurrentCallback callback);

  void NotifyRxStart(Time duration) override;
  void NotifyRxEndOk() override;
  void NotifyRxEndError() override;
  void NotifyTxStart(Time duration, double txPowerDbm) override;
  void NotifyCcaBusyStart(Time duration, WifiChannelListType channelType,
                          const std::vector<Time> &per20MhzDurations) override;
  void NotifySwitchingStart(Time duration) override;
  void NotifySleep() override;
  void NotifyOff() override;
  void NotifyWakeup() override;
  void NotifyOn() override;

private:
  void SwitchToIdle();

  DeviceEnergyModel::ChangeStateCallback m_changeStateCallback;

  UpdateTxCurrentCallback m_updateTxCurrentCallback;

  EventId m_switchToIdleEvent;
};

class WifiRadioEnergyModel : public DeviceEnergyModel {
public:
  typedef Callback<void> WifiRadioEnergyDepletionCallback;

  typedef Callback<void> WifiRadioEnergyRechargedCallback;

  static TypeId GetTypeId();
  WifiRadioEnergyModel();
  ~WifiRadioEnergyModel() override;

  void SetEnergySource(const Ptr<EnergySource> source) override;

  double GetTotalEnergyConsumption() const override;

  double GetIdleCurrentA() const;
  void SetIdleCurrentA(double idleCurrentA);
  double GetCcaBusyCurrentA() const;
  void SetCcaBusyCurrentA(double ccaBusyCurrentA);
  double GetTxCurrentA() const;
  void SetTxCurrentA(double txCurrentA);
  double GetRxCurrentA() const;
  void SetRxCurrentA(double rxCurrentA);
  double GetSwitchingCurrentA() const;
  void SetSwitchingCurrentA(double switchingCurrentA);
  double GetSleepCurrentA() const;
  void SetSleepCurrentA(double sleepCurrentA);

  WifiPhyState GetCurrentState() const;

  void SetEnergyDepletionCallback(WifiRadioEnergyDepletionCallback callback);

  void SetEnergyRechargedCallback(WifiRadioEnergyRechargedCallback callback);

  void SetTxCurrentModel(const Ptr<WifiTxCurrentModel> model);

  void SetTxCurrentFromModel(double txPowerDbm);

  void ChangeState(int newState) override;

  Time GetMaximumTimeInState(int state) const;

  void HandleEnergyDepletion() override;

  void HandleEnergyRecharged() override;

  void HandleEnergyChanged() override;

  WifiRadioEnergyModelPhyListener *GetPhyListener();

private:
  void DoDispose() override;

  double GetStateA(int state) const;

  double DoGetCurrentA() const override;

  void SetWifiRadioState(const WifiPhyState state);

  Ptr<EnergySource> m_source;

  double m_txCurrentA;
  double m_rxCurrentA;
  double m_idleCurrentA;
  double m_ccaBusyCurrentA;
  double m_switchingCurrentA;
  double m_sleepCurrentA;
  Ptr<WifiTxCurrentModel> m_txCurrentModel;

  TracedValue<double> m_totalEnergyConsumption;

  WifiPhyState m_currentState;
  Time m_lastUpdateTime;

  uint8_t m_nPendingChangeState;

  WifiRadioEnergyDepletionCallback m_energyDepletionCallback;

  WifiRadioEnergyRechargedCallback m_energyRechargedCallback;

  WifiRadioEnergyModelPhyListener *m_listener;

  EventId m_switchToOffEvent;
};

} // namespace ns3

#endif
