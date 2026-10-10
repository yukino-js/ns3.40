
#ifndef THOMPSON_SAMPLING_WIFI_MANAGER_H
#define THOMPSON_SAMPLING_WIFI_MANAGER_H

#include "ns3/random-variable-stream.h"
#include "ns3/traced-value.h"
#include "ns3/wifi-remote-station-manager.h"

namespace ns3 {

class ThompsonSamplingWifiManager : public WifiRemoteStationManager {
public:
  static TypeId GetTypeId();
  ThompsonSamplingWifiManager();
  ~ThompsonSamplingWifiManager() override;

  int64_t AssignStreams(int64_t stream) override;

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

  void InitializeStation(WifiRemoteStation *station) const;

  void UpdateNextMode(WifiRemoteStation *station) const;

  void Decay(WifiRemoteStation *st, size_t i) const;

  uint16_t GetModeGuardInterval(WifiRemoteStation *st, WifiMode mode) const;

  double SampleBetaVariable(uint64_t alpha, uint64_t beta) const;

  Ptr<GammaRandomVariable> m_gammaRandomVariable;

  double m_decay;

  TracedValue<uint64_t> m_currentRate;
};

} // namespace ns3

#endif
