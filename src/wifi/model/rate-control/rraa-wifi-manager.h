
#ifndef RRAA_WIFI_MANAGER_H
#define RRAA_WIFI_MANAGER_H

#include "ns3/nstime.h"
#include "ns3/traced-value.h"
#include "ns3/wifi-remote-station-manager.h"

namespace ns3 {

struct RraaWifiRemoteStation;

struct WifiRraaThresholds {
  double m_ori;
  double m_mtl;
  uint32_t m_ewnd;
};

typedef std::vector<std::pair<WifiRraaThresholds, WifiMode>>
    RraaThresholdsTable;

class RraaWifiManager : public WifiRemoteStationManager {
public:
  static TypeId GetTypeId();

  RraaWifiManager();
  ~RraaWifiManager() override;

  void SetupPhy(const Ptr<WifiPhy> phy) override;
  void SetupMac(const Ptr<WifiMac> mac) override;

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
  bool DoNeedRts(WifiRemoteStation *st, uint32_t size, bool normally) override;

  void CheckInit(RraaWifiRemoteStation *station);
  uint8_t GetMaxRate(RraaWifiRemoteStation *station) const;
  void CheckTimeout(RraaWifiRemoteStation *station);
  void RunBasicAlgorithm(RraaWifiRemoteStation *station);
  void ARts(RraaWifiRemoteStation *station);
  void ResetCountersBasic(RraaWifiRemoteStation *station);
  void InitThresholds(RraaWifiRemoteStation *station);
  WifiRraaThresholds GetThresholds(RraaWifiRemoteStation *station,
                                   WifiMode mode) const;
  WifiRraaThresholds GetThresholds(RraaWifiRemoteStation *station,
                                   uint8_t index) const;
  Time GetCalcTxTime(WifiMode mode) const;
  void AddCalcTxTime(WifiMode mode, Time t);
  typedef std::vector<std::pair<Time, WifiMode>> TxTime;

  TxTime m_calcTxTime;
  Time m_sifs;
  Time m_difs;

  uint32_t m_frameLength;
  uint32_t m_ackLength;

  bool m_basic;
  Time m_timeout;
  double m_alpha;
  double m_beta;
  double m_tau;

  TracedValue<uint64_t> m_currentRate;
};

} // namespace ns3

#endif
