
#include "wifi-radio-energy-model.h"

#include "wifi-tx-current-model.h"

#include "ns3/energy-source.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/simulator.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("WifiRadioEnergyModel");

NS_OBJECT_ENSURE_REGISTERED(WifiRadioEnergyModel);

TypeId WifiRadioEnergyModel::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::WifiRadioEnergyModel")
          .SetParent<DeviceEnergyModel>()
          .SetGroupName("Energy")
          .AddConstructor<WifiRadioEnergyModel>()
          .AddAttribute(
              "IdleCurrentA", "The default radio Idle current in Ampere.",
              DoubleValue(0.273),
              MakeDoubleAccessor(&WifiRadioEnergyModel::SetIdleCurrentA,
                                 &WifiRadioEnergyModel::GetIdleCurrentA),
              MakeDoubleChecker<double>())
          .AddAttribute(
              "CcaBusyCurrentA",
              "The default radio CCA Busy State current in Ampere.",
              DoubleValue(0.273),
              MakeDoubleAccessor(&WifiRadioEnergyModel::SetCcaBusyCurrentA,
                                 &WifiRadioEnergyModel::GetCcaBusyCurrentA),
              MakeDoubleChecker<double>())
          .AddAttribute(
              "TxCurrentA", "The radio TX current in Ampere.",
              DoubleValue(0.380),
              MakeDoubleAccessor(&WifiRadioEnergyModel::SetTxCurrentA,
                                 &WifiRadioEnergyModel::GetTxCurrentA),
              MakeDoubleChecker<double>())
          .AddAttribute(
              "RxCurrentA", "The radio RX current in Ampere.",
              DoubleValue(0.313),
              MakeDoubleAccessor(&WifiRadioEnergyModel::SetRxCurrentA,
                                 &WifiRadioEnergyModel::GetRxCurrentA),
              MakeDoubleChecker<double>())
          .AddAttribute(
              "SwitchingCurrentA",
              "The default radio Channel Switch current in Ampere.",
              DoubleValue(0.273),
              MakeDoubleAccessor(&WifiRadioEnergyModel::SetSwitchingCurrentA,
                                 &WifiRadioEnergyModel::GetSwitchingCurrentA),
              MakeDoubleChecker<double>())
          .AddAttribute(
              "SleepCurrentA", "The radio Sleep current in Ampere.",
              DoubleValue(0.033),
              MakeDoubleAccessor(&WifiRadioEnergyModel::SetSleepCurrentA,
                                 &WifiRadioEnergyModel::GetSleepCurrentA),
              MakeDoubleChecker<double>())
          .AddAttribute(
              "TxCurrentModel", "A pointer to the attached TX current model.",
              PointerValue(),
              MakePointerAccessor(&WifiRadioEnergyModel::m_txCurrentModel),
              MakePointerChecker<WifiTxCurrentModel>())
          .AddTraceSource("TotalEnergyConsumption",
                          "Total energy consumption of the radio device.",
                          MakeTraceSourceAccessor(
                              &WifiRadioEnergyModel::m_totalEnergyConsumption),
                          "ns3::TracedValueCallback::Double");
  return tid;
}

WifiRadioEnergyModel::WifiRadioEnergyModel()
    : m_source(nullptr), m_currentState(WifiPhyState::IDLE),
      m_lastUpdateTime(Seconds(0.0)), m_nPendingChangeState(0) {
  NS_LOG_FUNCTION(this);
  m_energyDepletionCallback.Nullify();
  m_listener = new WifiRadioEnergyModelPhyListener;
  m_listener->SetChangeStateCallback(
      MakeCallback(&DeviceEnergyModel::ChangeState, this));
  m_listener->SetUpdateTxCurrentCallback(
      MakeCallback(&WifiRadioEnergyModel::SetTxCurrentFromModel, this));
}

WifiRadioEnergyModel::~WifiRadioEnergyModel() {
  NS_LOG_FUNCTION(this);
  m_txCurrentModel = nullptr;
  delete m_listener;
}

void WifiRadioEnergyModel::SetEnergySource(const Ptr<EnergySource> source) {
  NS_LOG_FUNCTION(this << source);
  NS_ASSERT(source);
  m_source = source;
  m_switchToOffEvent.Cancel();
  Time durationToOff = GetMaximumTimeInState(m_currentState);
  m_switchToOffEvent =
      Simulator::Schedule(durationToOff, &WifiRadioEnergyModel::ChangeState,
                          this, WifiPhyState::OFF);
}

double WifiRadioEnergyModel::GetTotalEnergyConsumption() const {
  NS_LOG_FUNCTION(this);

  Time duration = Simulator::Now() - m_lastUpdateTime;
  NS_ASSERT(duration.IsPositive());

  double supplyVoltage = m_source->GetSupplyVoltage();
  double energyToDecrease =
      duration.GetSeconds() * GetStateA(m_currentState) * supplyVoltage;

  m_source->UpdateEnergySource();

  return m_totalEnergyConsumption + energyToDecrease;
}

double WifiRadioEnergyModel::GetIdleCurrentA() const {
  NS_LOG_FUNCTION(this);
  return m_idleCurrentA;
}

void WifiRadioEnergyModel::SetIdleCurrentA(double idleCurrentA) {
  NS_LOG_FUNCTION(this << idleCurrentA);
  m_idleCurrentA = idleCurrentA;
}

double WifiRadioEnergyModel::GetCcaBusyCurrentA() const {
  NS_LOG_FUNCTION(this);
  return m_ccaBusyCurrentA;
}

void WifiRadioEnergyModel::SetCcaBusyCurrentA(double CcaBusyCurrentA) {
  NS_LOG_FUNCTION(this << CcaBusyCurrentA);
  m_ccaBusyCurrentA = CcaBusyCurrentA;
}

double WifiRadioEnergyModel::GetTxCurrentA() const {
  NS_LOG_FUNCTION(this);
  return m_txCurrentA;
}

void WifiRadioEnergyModel::SetTxCurrentA(double txCurrentA) {
  NS_LOG_FUNCTION(this << txCurrentA);
  m_txCurrentA = txCurrentA;
}

double WifiRadioEnergyModel::GetRxCurrentA() const {
  NS_LOG_FUNCTION(this);
  return m_rxCurrentA;
}

void WifiRadioEnergyModel::SetRxCurrentA(double rxCurrentA) {
  NS_LOG_FUNCTION(this << rxCurrentA);
  m_rxCurrentA = rxCurrentA;
}

double WifiRadioEnergyModel::GetSwitchingCurrentA() const {
  NS_LOG_FUNCTION(this);
  return m_switchingCurrentA;
}

void WifiRadioEnergyModel::SetSwitchingCurrentA(double switchingCurrentA) {
  NS_LOG_FUNCTION(this << switchingCurrentA);
  m_switchingCurrentA = switchingCurrentA;
}

double WifiRadioEnergyModel::GetSleepCurrentA() const {
  NS_LOG_FUNCTION(this);
  return m_sleepCurrentA;
}

void WifiRadioEnergyModel::SetSleepCurrentA(double sleepCurrentA) {
  NS_LOG_FUNCTION(this << sleepCurrentA);
  m_sleepCurrentA = sleepCurrentA;
}

WifiPhyState WifiRadioEnergyModel::GetCurrentState() const {
  NS_LOG_FUNCTION(this);
  return m_currentState;
}

void WifiRadioEnergyModel::SetEnergyDepletionCallback(
    WifiRadioEnergyDepletionCallback callback) {
  NS_LOG_FUNCTION(this);
  if (callback.IsNull()) {
    NS_LOG_DEBUG(
        "WifiRadioEnergyModel:Setting NULL energy depletion callback!");
  }
  m_energyDepletionCallback = callback;
}

void WifiRadioEnergyModel::SetEnergyRechargedCallback(
    WifiRadioEnergyRechargedCallback callback) {
  NS_LOG_FUNCTION(this);
  if (callback.IsNull()) {
    NS_LOG_DEBUG(
        "WifiRadioEnergyModel:Setting NULL energy recharged callback!");
  }
  m_energyRechargedCallback = callback;
}

void WifiRadioEnergyModel::SetTxCurrentModel(
    const Ptr<WifiTxCurrentModel> model) {
  m_txCurrentModel = model;
}

void WifiRadioEnergyModel::SetTxCurrentFromModel(double txPowerDbm) {
  if (m_txCurrentModel) {
    m_txCurrentA = m_txCurrentModel->CalcTxCurrent(txPowerDbm);
  }
}

Time WifiRadioEnergyModel::GetMaximumTimeInState(int state) const {
  if (state == WifiPhyState::OFF) {
    NS_FATAL_ERROR("Requested maximum remaining time for OFF state");
  }
  double remainingEnergy = m_source->GetRemainingEnergy();
  double supplyVoltage = m_source->GetSupplyVoltage();
  double current = GetStateA(state);
  return Seconds(remainingEnergy / (current * supplyVoltage));
}

void WifiRadioEnergyModel::ChangeState(int newState) {
  NS_LOG_FUNCTION(this << newState);

  m_nPendingChangeState++;

  if (m_nPendingChangeState > 1 && newState == WifiPhyState::OFF) {
    SetWifiRadioState((WifiPhyState)newState);
    m_nPendingChangeState--;
    return;
  }

  if (newState != WifiPhyState::OFF) {
    m_switchToOffEvent.Cancel();
    Time durationToOff = GetMaximumTimeInState(newState);
    m_switchToOffEvent =
        Simulator::Schedule(durationToOff, &WifiRadioEnergyModel::ChangeState,
                            this, WifiPhyState::OFF);
  }

  Time duration = Simulator::Now() - m_lastUpdateTime;
  NS_ASSERT(duration.IsPositive());

  double supplyVoltage = m_source->GetSupplyVoltage();
  double energyToDecrease =
      duration.GetSeconds() * GetStateA(m_currentState) * supplyVoltage;

  m_totalEnergyConsumption += energyToDecrease;
  NS_ASSERT(m_totalEnergyConsumption <= m_source->GetInitialEnergy());

  m_lastUpdateTime = Simulator::Now();

  m_source->UpdateEnergySource();

  if (m_nPendingChangeState <= 1 && m_currentState != WifiPhyState::OFF) {
    SetWifiRadioState((WifiPhyState)newState);

    NS_LOG_DEBUG("WifiRadioEnergyModel:Total energy consumption is "
                 << m_totalEnergyConsumption << "J");
  }

  m_nPendingChangeState--;
}

void WifiRadioEnergyModel::HandleEnergyDepletion() {
  NS_LOG_FUNCTION(this);
  NS_LOG_DEBUG("WifiRadioEnergyModel:Energy is depleted!");
  if (!m_energyDepletionCallback.IsNull()) {
    m_energyDepletionCallback();
  }
}

void WifiRadioEnergyModel::HandleEnergyRecharged() {
  NS_LOG_FUNCTION(this);
  NS_LOG_DEBUG("WifiRadioEnergyModel:Energy is recharged!");
  if (!m_energyRechargedCallback.IsNull()) {
    m_energyRechargedCallback();
  }
}

void WifiRadioEnergyModel::HandleEnergyChanged() {
  NS_LOG_FUNCTION(this);
  NS_LOG_DEBUG("WifiRadioEnergyModel:Energy is changed!");
  if (m_currentState != WifiPhyState::OFF) {
    m_switchToOffEvent.Cancel();
    Time durationToOff = GetMaximumTimeInState(m_currentState);
    m_switchToOffEvent =
        Simulator::Schedule(durationToOff, &WifiRadioEnergyModel::ChangeState,
                            this, WifiPhyState::OFF);
  }
}

WifiRadioEnergyModelPhyListener *WifiRadioEnergyModel::GetPhyListener() {
  NS_LOG_FUNCTION(this);
  return m_listener;
}

void WifiRadioEnergyModel::DoDispose() {
  NS_LOG_FUNCTION(this);
  m_source = nullptr;
  m_energyDepletionCallback.Nullify();
}

double WifiRadioEnergyModel::GetStateA(int state) const {
  switch (state) {
  case WifiPhyState::IDLE:
    return m_idleCurrentA;
  case WifiPhyState::CCA_BUSY:
    return m_ccaBusyCurrentA;
  case WifiPhyState::TX:
    return m_txCurrentA;
  case WifiPhyState::RX:
    return m_rxCurrentA;
  case WifiPhyState::SWITCHING:
    return m_switchingCurrentA;
  case WifiPhyState::SLEEP:
    return m_sleepCurrentA;
  case WifiPhyState::OFF:
    return 0.0;
  }
  NS_FATAL_ERROR("WifiRadioEnergyModel: undefined radio state " << state);
}

double WifiRadioEnergyModel::DoGetCurrentA() const {
  return GetStateA(m_currentState);
}

void WifiRadioEnergyModel::SetWifiRadioState(const WifiPhyState state) {
  NS_LOG_FUNCTION(this << state);
  m_currentState = state;
  std::string stateName;
  switch (state) {
  case WifiPhyState::IDLE:
    stateName = "IDLE";
    break;
  case WifiPhyState::CCA_BUSY:
    stateName = "CCA_BUSY";
    break;
  case WifiPhyState::TX:
    stateName = "TX";
    break;
  case WifiPhyState::RX:
    stateName = "RX";
    break;
  case WifiPhyState::SWITCHING:
    stateName = "SWITCHING";
    break;
  case WifiPhyState::SLEEP:
    stateName = "SLEEP";
    break;
  case WifiPhyState::OFF:
    stateName = "OFF";
    break;
  }
  NS_LOG_DEBUG("WifiRadioEnergyModel:Switching to state: "
               << stateName << " at time = " << Simulator::Now());
}

WifiRadioEnergyModelPhyListener::WifiRadioEnergyModelPhyListener() {
  NS_LOG_FUNCTION(this);
  m_changeStateCallback.Nullify();
  m_updateTxCurrentCallback.Nullify();
}

WifiRadioEnergyModelPhyListener::~WifiRadioEnergyModelPhyListener() {
  NS_LOG_FUNCTION(this);
}

void WifiRadioEnergyModelPhyListener::SetChangeStateCallback(
    DeviceEnergyModel::ChangeStateCallback callback) {
  NS_LOG_FUNCTION(this << &callback);
  NS_ASSERT(!callback.IsNull());
  m_changeStateCallback = callback;
}

void WifiRadioEnergyModelPhyListener::SetUpdateTxCurrentCallback(
    UpdateTxCurrentCallback callback) {
  NS_LOG_FUNCTION(this << &callback);
  NS_ASSERT(!callback.IsNull());
  m_updateTxCurrentCallback = callback;
}

void WifiRadioEnergyModelPhyListener::NotifyRxStart(Time duration) {
  NS_LOG_FUNCTION(this << duration);
  if (m_changeStateCallback.IsNull()) {
    NS_FATAL_ERROR(
        "WifiRadioEnergyModelPhyListener:Change state callback not set!");
  }
  m_changeStateCallback(WifiPhyState::RX);
  m_switchToIdleEvent.Cancel();
}

void WifiRadioEnergyModelPhyListener::NotifyRxEndOk() {
  NS_LOG_FUNCTION(this);
  if (m_changeStateCallback.IsNull()) {
    NS_FATAL_ERROR(
        "WifiRadioEnergyModelPhyListener:Change state callback not set!");
  }
  m_changeStateCallback(WifiPhyState::IDLE);
}

void WifiRadioEnergyModelPhyListener::NotifyRxEndError() {
  NS_LOG_FUNCTION(this);
  if (m_changeStateCallback.IsNull()) {
    NS_FATAL_ERROR(
        "WifiRadioEnergyModelPhyListener:Change state callback not set!");
  }
  m_changeStateCallback(WifiPhyState::IDLE);
}

void WifiRadioEnergyModelPhyListener::NotifyTxStart(Time duration,
                                                    double txPowerDbm) {
  NS_LOG_FUNCTION(this << duration << txPowerDbm);
  if (m_updateTxCurrentCallback.IsNull()) {
    NS_FATAL_ERROR(
        "WifiRadioEnergyModelPhyListener:Update tx current callback not set!");
  }
  m_updateTxCurrentCallback(txPowerDbm);
  if (m_changeStateCallback.IsNull()) {
    NS_FATAL_ERROR(
        "WifiRadioEnergyModelPhyListener:Change state callback not set!");
  }
  m_changeStateCallback(WifiPhyState::TX);
  m_switchToIdleEvent.Cancel();
  m_switchToIdleEvent = Simulator::Schedule(
      duration, &WifiRadioEnergyModelPhyListener::SwitchToIdle, this);
}

void WifiRadioEnergyModelPhyListener::NotifyCcaBusyStart(
    Time duration, WifiChannelListType channelType, const std::vector<Time> &) {
  NS_LOG_FUNCTION(this << duration << channelType);
  if (m_changeStateCallback.IsNull()) {
    NS_FATAL_ERROR(
        "WifiRadioEnergyModelPhyListener:Change state callback not set!");
  }
  m_changeStateCallback(WifiPhyState::CCA_BUSY);
  m_switchToIdleEvent.Cancel();
  m_switchToIdleEvent = Simulator::Schedule(
      duration, &WifiRadioEnergyModelPhyListener::SwitchToIdle, this);
}

void WifiRadioEnergyModelPhyListener::NotifySwitchingStart(Time duration) {
  NS_LOG_FUNCTION(this << duration);
  if (m_changeStateCallback.IsNull()) {
    NS_FATAL_ERROR(
        "WifiRadioEnergyModelPhyListener:Change state callback not set!");
  }
  m_changeStateCallback(WifiPhyState::SWITCHING);
  m_switchToIdleEvent.Cancel();
  m_switchToIdleEvent = Simulator::Schedule(
      duration, &WifiRadioEnergyModelPhyListener::SwitchToIdle, this);
}

void WifiRadioEnergyModelPhyListener::NotifySleep() {
  NS_LOG_FUNCTION(this);
  if (m_changeStateCallback.IsNull()) {
    NS_FATAL_ERROR(
        "WifiRadioEnergyModelPhyListener:Change state callback not set!");
  }
  m_changeStateCallback(WifiPhyState::SLEEP);
  m_switchToIdleEvent.Cancel();
}

void WifiRadioEnergyModelPhyListener::NotifyWakeup() {
  NS_LOG_FUNCTION(this);
  if (m_changeStateCallback.IsNull()) {
    NS_FATAL_ERROR(
        "WifiRadioEnergyModelPhyListener:Change state callback not set!");
  }
  m_changeStateCallback(WifiPhyState::IDLE);
}

void WifiRadioEnergyModelPhyListener::NotifyOff() {
  NS_LOG_FUNCTION(this);
  if (m_changeStateCallback.IsNull()) {
    NS_FATAL_ERROR(
        "WifiRadioEnergyModelPhyListener:Change state callback not set!");
  }
  m_changeStateCallback(WifiPhyState::OFF);
  m_switchToIdleEvent.Cancel();
}

void WifiRadioEnergyModelPhyListener::NotifyOn() {
  NS_LOG_FUNCTION(this);
  if (m_changeStateCallback.IsNull()) {
    NS_FATAL_ERROR(
        "WifiRadioEnergyModelPhyListener:Change state callback not set!");
  }
  m_changeStateCallback(WifiPhyState::IDLE);
}

void WifiRadioEnergyModelPhyListener::SwitchToIdle() {
  NS_LOG_FUNCTION(this);
  if (m_changeStateCallback.IsNull()) {
    NS_FATAL_ERROR(
        "WifiRadioEnergyModelPhyListener:Change state callback not set!");
  }
  m_changeStateCallback(WifiPhyState::IDLE);
}

} // namespace ns3
