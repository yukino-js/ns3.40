
#ifndef PARF_WIFI_MANAGER_H
#define PARF_WIFI_MANAGER_H

#include "ns3/wifi-remote-station-manager.h"

namespace ns3 {

struct ParfWifiRemoteStation;

class ParfWifiManager : public WifiRemoteStationManager {
public:
  static TypeId GetTypeId();
  ParfWifiManager();
  ~ParfWifiManager() override;

  void SetupPhy(const Ptr<WifiPhy> phy) override;

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

  void CheckInit(ParfWifiRemoteStation *station);

  uint32_t m_attemptThreshold;
  uint32_t m_successThreshold;

  uint8_t m_minPower;

  uint8_t m_maxPower;

  TracedCallback<double, double, Mac48Address> m_powerChange;
  TracedCallback<DataRate, DataRate, Mac48Address> m_rateChange;
};

} // namespace ns3

#endif
