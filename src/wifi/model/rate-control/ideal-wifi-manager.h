
#ifndef IDEAL_WIFI_MANAGER_H
#define IDEAL_WIFI_MANAGER_H

#include "ns3/traced-value.h"
#include "ns3/wifi-remote-station-manager.h"

namespace ns3 {

struct IdealWifiRemoteStation;

class IdealWifiManager : public WifiRemoteStationManager {
public:
  static TypeId GetTypeId();
  IdealWifiManager();
  ~IdealWifiManager() override;

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
  void DoReportAmpduTxStatus(WifiRemoteStation *station,
                             uint16_t nSuccessfulMpdus, uint16_t nFailedMpdus,
                             double rxSnr, double dataSnr,
                             uint16_t dataChannelWidth,
                             uint8_t dataNss) override;
  void DoReportFinalRtsFailed(WifiRemoteStation *station) override;
  void DoReportFinalDataFailed(WifiRemoteStation *station) override;
  WifiTxVector DoGetDataTxVector(WifiRemoteStation *station,
                                 uint16_t allowedWidth) override;
  WifiTxVector DoGetRtsTxVector(WifiRemoteStation *station) override;

  void Reset(WifiRemoteStation *station) const;

  void BuildSnrThresholds();

  double GetSnrThreshold(WifiTxVector txVector);
  void AddSnrThreshold(WifiTxVector txVector, double snr);

  uint16_t GetChannelWidthForNonHtMode(WifiMode mode) const;

  double GetLastObservedSnr(IdealWifiRemoteStation *station,
                            uint16_t channelWidth, uint8_t nss) const;

  typedef std::vector<std::pair<double, WifiTxVector>> Thresholds;

  double m_ber;
  Thresholds m_thresholds;

  TracedValue<uint64_t> m_currentRate;
};

} // namespace ns3

#endif
