
#ifndef AMRR_WIFI_MANAGER_H
#define AMRR_WIFI_MANAGER_H

#include "ns3/traced-value.h"
#include "ns3/wifi-remote-station-manager.h"

namespace ns3 {

struct AmrrWifiRemoteStation;

class AmrrWifiManager : public WifiRemoteStationManager {
public:
  static TypeId GetTypeId();

  AmrrWifiManager();
  ~AmrrWifiManager() override;

private:
  void DoInitialize() override;
  WifiRemoteStation *DoCreateStation() const override;
  void DoReportRxOk(WifiRemoteStation *station, double rxSnr,
                    WifiMode txMode) override;
  void DoReportRtsFailed(WifiRemoteStation *station) override;
  void DoReportDataFailed(WifiRemoteStation *station) override;
  void DoReportRtsOk(WifiRemoteStation *station, double ctsSnr,
                     WifiMode ctsMode, double rtsSnr) override;
  void DoReportDataOk(WifiRemoteStation *station, double ackSnr,
                      WifiMode ackMode, double dataSnr,
                      uint16_t dataChannelWidth, uint8_t dataNss) override;
  void DoReportFinalRtsFailed(WifiRemoteStation *station) override;
  void DoReportFinalDataFailed(WifiRemoteStation *station) override;
  WifiTxVector DoGetDataTxVector(WifiRemoteStation *station,
                                 uint16_t allowedWidth) override;
  WifiTxVector DoGetRtsTxVector(WifiRemoteStation *station) override;

  void UpdateMode(AmrrWifiRemoteStation *station);
  void ResetCnt(AmrrWifiRemoteStation *station);
  void IncreaseRate(AmrrWifiRemoteStation *station);
  void DecreaseRate(AmrrWifiRemoteStation *station);
  bool IsMinRate(AmrrWifiRemoteStation *station) const;
  bool IsMaxRate(AmrrWifiRemoteStation *station) const;
  bool IsSuccess(AmrrWifiRemoteStation *station) const;
  bool IsFailure(AmrrWifiRemoteStation *station) const;
  bool IsEnough(AmrrWifiRemoteStation *station) const;

  Time m_updatePeriod;
  double m_failureRatio;
  double m_successRatio;
  uint32_t m_maxSuccessThreshold;
  uint32_t m_minSuccessThreshold;

  TracedValue<uint64_t> m_currentRate;
};

} // namespace ns3

#endif
