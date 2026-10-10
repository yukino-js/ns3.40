
#ifndef CARA_WIFI_MANAGER_H
#define CARA_WIFI_MANAGER_H

#include "ns3/traced-value.h"
#include "ns3/wifi-remote-station-manager.h"

namespace ns3 {

class CaraWifiManager : public WifiRemoteStationManager {
public:
  static TypeId GetTypeId();
  CaraWifiManager();
  ~CaraWifiManager() override;

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
  bool DoNeedRts(WifiRemoteStation *station, uint32_t size,
                 bool normally) override;

  uint32_t m_timerTimeout;
  uint32_t m_successThreshold;
  uint32_t m_failureThreshold;
  uint32_t m_probeThreshold;

  TracedValue<uint64_t> m_currentRate;
};

} // namespace ns3

#endif
