
#ifndef CONSTANT_RATE_WIFI_MANAGER_H
#define CONSTANT_RATE_WIFI_MANAGER_H

#include "ns3/wifi-remote-station-manager.h"

namespace ns3 {

class ConstantRateWifiManager : public WifiRemoteStationManager {
public:
  static TypeId GetTypeId();
  ConstantRateWifiManager();
  ~ConstantRateWifiManager() override;

private:
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

  WifiMode m_dataMode;
  WifiMode m_ctlMode;
};

} // namespace ns3

#endif
