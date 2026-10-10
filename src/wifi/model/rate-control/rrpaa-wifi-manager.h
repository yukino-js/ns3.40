
#ifndef RRPAA_WIFI_MANAGER_H
#define RRPAA_WIFI_MANAGER_H

#include "ns3/nstime.h"
#include "ns3/random-variable-stream.h"
#include "ns3/wifi-remote-station-manager.h"

namespace ns3 {

struct RrpaaWifiRemoteStation;

struct WifiRrpaaThresholds {
  double m_ori;
  double m_mtl;
  uint32_t m_ewnd;
};

typedef std::vector<std::pair<WifiRrpaaThresholds, WifiMode>>
    RrpaaThresholdsTable;

typedef std::vector<std::vector<double>> RrpaaProbabilitiesTable;

class RrpaaWifiManager : public WifiRemoteStationManager {
public:
  static TypeId GetTypeId();
  RrpaaWifiManager();
  ~RrpaaWifiManager() override;

  void SetupPhy(const Ptr<WifiPhy> phy) override;
  void SetupMac(const Ptr<WifiMac> mac) override;

  int64_t AssignStreams(int64_t stream) override;

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

  void CheckInit(RrpaaWifiRemoteStation *station);

  void CheckTimeout(RrpaaWifiRemoteStation *station);
  void RunBasicAlgorithm(RrpaaWifiRemoteStation *station);
  void RunAdaptiveRtsAlgorithm(RrpaaWifiRemoteStation *station);
  void ResetCountersBasic(RrpaaWifiRemoteStation *station);

  void InitThresholds(RrpaaWifiRemoteStation *station);

  WifiRrpaaThresholds GetThresholds(RrpaaWifiRemoteStation *station,
                                    WifiMode mode) const;

  WifiRrpaaThresholds GetThresholds(RrpaaWifiRemoteStation *station,
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
  double m_gamma;
  double m_delta;

  uint8_t m_minPowerLevel;
  uint8_t m_maxPowerLevel;
  uint8_t m_nPowerLevels;

  TracedCallback<double, double, Mac48Address> m_powerChange;
  TracedCallback<DataRate, DataRate, Mac48Address> m_rateChange;

  Ptr<UniformRandomVariable> m_uniformRandomVariable;
};

} // namespace ns3

#endif
