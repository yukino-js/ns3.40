
#ifndef WIFI_PHY_STATE_HELPER_H
#define WIFI_PHY_STATE_HELPER_H

#include "wifi-phy-common.h"
#include "wifi-phy-state.h"
#include "wifi-ppdu.h"

#include "ns3/callback.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/traced-callback.h"

#include <vector>

namespace ns3 {

class WifiPhyListener;
class WifiTxVector;
class WifiMode;
class Packet;
class WifiPsdu;
struct RxSignalInfo;

typedef Callback<void, Ptr<const WifiPsdu>, RxSignalInfo, WifiTxVector,
                 std::vector<bool>>
    RxOkCallback;
typedef Callback<void, Ptr<const WifiPsdu>> RxErrorCallback;

class WifiPhyStateHelper : public Object {
public:
  static TypeId GetTypeId();

  WifiPhyStateHelper();

  void SetReceiveOkCallback(RxOkCallback callback);
  void SetReceiveErrorCallback(RxErrorCallback callback);
  void RegisterListener(WifiPhyListener *listener);
  void UnregisterListener(WifiPhyListener *listener);
  WifiPhyState GetState() const;
  bool IsStateCcaBusy() const;
  bool IsStateIdle() const;
  bool IsStateRx() const;
  bool IsStateTx() const;
  bool IsStateSwitching() const;
  bool IsStateSleep() const;
  bool IsStateOff() const;
  Time GetDelayUntilIdle() const;
  Time GetLastRxStartTime() const;
  Time GetLastRxEndTime() const;

  void SwitchToTx(Time txDuration, WifiConstPsduMap psdus, double txPowerDbm,
                  const WifiTxVector &txVector);
  void SwitchToRx(Time rxDuration);
  void SwitchToChannelSwitching(Time switchingDuration);
  void NotifyRxMpdu(Ptr<const WifiPsdu> psdu, RxSignalInfo rxSignalInfo,
                    const WifiTxVector &txVector);
  void NotifyRxPsduSucceeded(Ptr<const WifiPsdu> psdu,
                             RxSignalInfo rxSignalInfo,
                             const WifiTxVector &txVector, uint16_t staId,
                             const std::vector<bool> &statusPerMpdu);
  void NotifyRxPsduFailed(Ptr<const WifiPsdu> psdu, double snr);
  void SwitchFromRxEndOk();
  void SwitchFromRxEndError();
  void SwitchFromRxAbort(uint16_t operatingWidth);
  void SwitchMaybeToCcaBusy(Time duration, WifiChannelListType channelType,
                            const std::vector<Time> &per20MhzDurations);
  void SwitchToSleep();
  void SwitchFromSleep();
  void SwitchToOff();
  void SwitchFromOff();

  typedef void (*StateTracedCallback)(Time start, Time duration,
                                      WifiPhyState state);

  typedef void (*RxOkTracedCallback)(Ptr<const Packet> packet, double snr,
                                     WifiMode mode, WifiPreamble preamble);

  typedef void (*RxEndErrorTracedCallback)(Ptr<const Packet> packet,
                                           double snr);

  typedef void (*TxTracedCallback)(Ptr<const Packet> packet, WifiMode mode,
                                   WifiPreamble preamble, uint8_t power);

private:
  typedef std::vector<WifiPhyListener *> Listeners;

  void LogPreviousIdleAndCcaBusyStates();

  void NotifyTxStart(Time duration, double txPowerDbm);
  void NotifyRxStart(Time duration);
  void NotifyRxEndOk();
  void NotifyRxEndError();
  void NotifyCcaBusyStart(Time duration, WifiChannelListType channelType,
                          const std::vector<Time> &per20MhzDurations);
  void NotifySwitchingStart(Time duration);
  void NotifySleep();
  void NotifyOff();
  void NotifyWakeup();
  void DoSwitchFromRx();
  void NotifyOn();

  TracedCallback<Time, Time, WifiPhyState> m_stateLogger;

  bool m_sleeping;
  bool m_isOff;
  Time m_endTx;
  Time m_endRx;
  Time m_endCcaBusy;
  Time m_endSwitching;
  Time m_startTx;
  Time m_startRx;
  Time m_startCcaBusy;
  Time m_startSwitching;
  Time m_startSleep;
  Time m_previousStateChangeTime;

  Listeners m_listeners;
  TracedCallback<Ptr<const Packet>, double, WifiMode, WifiPreamble> m_rxOkTrace;
  TracedCallback<Ptr<const Packet>, double> m_rxErrorTrace;
  TracedCallback<Ptr<const Packet>, WifiMode, WifiPreamble, uint8_t> m_txTrace;
  RxOkCallback m_rxOkCallback;
  RxErrorCallback m_rxErrorCallback;
};

} // namespace ns3

#endif
