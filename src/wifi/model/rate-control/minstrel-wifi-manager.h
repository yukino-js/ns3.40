
#ifndef MINSTREL_WIFI_MANAGER_H
#define MINSTREL_WIFI_MANAGER_H

#include "ns3/traced-value.h"
#include "ns3/wifi-remote-station-manager.h"

#include <fstream>
#include <map>

namespace ns3 {

class UniformRandomVariable;

struct RateInfo {
  Time perfectTxTime;

  uint32_t retryCount;
  uint32_t adjustedRetryCount;
  uint32_t numRateAttempt;
  uint32_t numRateSuccess;
  uint32_t prob;
  uint32_t ewmaProb;
  uint32_t throughput;

  uint32_t prevNumRateAttempt;
  uint32_t prevNumRateSuccess;
  uint64_t successHist;
  uint64_t attemptHist;

  uint8_t numSamplesSkipped;
  int sampleLimit;
};

typedef std::vector<RateInfo> MinstrelRate;
typedef std::vector<std::vector<uint8_t>> SampleRate;

struct MinstrelWifiRemoteStation : public WifiRemoteStation {
  Time m_nextStatsUpdate;

  uint8_t m_col;
  uint8_t m_index;
  uint16_t m_maxTpRate;
  uint16_t m_maxTpRate2;
  uint16_t m_maxProbRate;
  uint8_t m_nModes;
  int m_totalPacketsCount;
  int m_samplePacketsCount;
  int m_numSamplesDeferred;
  bool m_isSampling;
  uint16_t m_sampleRate;
  bool m_sampleDeferred;
  uint32_t m_shortRetry;
  uint32_t m_longRetry;
  uint32_t m_retry;
  uint16_t m_txrate;
  bool m_initialized;
  MinstrelRate m_minstrelTable;
  SampleRate m_sampleTable;
  std::ofstream m_statsFile;
};

class MinstrelWifiManager : public WifiRemoteStationManager {
public:
  static TypeId GetTypeId();
  MinstrelWifiManager();
  ~MinstrelWifiManager() override;

  void SetupPhy(const Ptr<WifiPhy> phy) override;
  void SetupMac(const Ptr<WifiMac> mac) override;
  int64_t AssignStreams(int64_t stream) override;

  void UpdateRate(MinstrelWifiRemoteStation *station);

  void UpdateStats(MinstrelWifiRemoteStation *station);

  uint16_t FindRate(MinstrelWifiRemoteStation *station);

  WifiTxVector GetDataTxVector(MinstrelWifiRemoteStation *station);

  WifiTxVector GetRtsTxVector(MinstrelWifiRemoteStation *station);

  uint32_t CountRetries(MinstrelWifiRemoteStation *station);

  void UpdatePacketCounters(MinstrelWifiRemoteStation *station);

  void UpdateRetry(MinstrelWifiRemoteStation *station);

  void CheckInit(MinstrelWifiRemoteStation *station);

  void InitSampleTable(MinstrelWifiRemoteStation *station);

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

  bool DoNeedRetransmission(WifiRemoteStation *st, Ptr<const Packet> packet,
                            bool normally) override;

  Time GetCalcTxTime(WifiMode mode) const;
  void AddCalcTxTime(WifiMode mode, Time t);

  void RateInit(MinstrelWifiRemoteStation *station);

  uint16_t GetNextSample(MinstrelWifiRemoteStation *station);

  Time CalculateTimeUnicastPacket(Time dataTransmissionTime,
                                  uint32_t shortRetries, uint32_t longRetries);

  void PrintSampleTable(MinstrelWifiRemoteStation *station) const;

  void PrintTable(MinstrelWifiRemoteStation *station);

  typedef std::map<WifiMode, Time> TxTime;

  TxTime m_calcTxTime;
  Time m_updateStats;
  uint8_t m_lookAroundRate;
  uint8_t m_ewmaLevel;
  uint8_t m_sampleCol;
  uint32_t m_pktLen;
  bool m_printStats;
  bool m_printSamples;

  Ptr<UniformRandomVariable> m_uniformRandomVariable;

  TracedValue<uint64_t> m_currentRate;
};

} // namespace ns3

#endif
