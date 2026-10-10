
#ifndef MINSTREL_HT_WIFI_MANAGER_H
#define MINSTREL_HT_WIFI_MANAGER_H

#include "minstrel-wifi-manager.h"

#include "ns3/wifi-mpdu-type.h"
#include "ns3/wifi-remote-station-manager.h"

namespace ns3 {

typedef std::map<WifiMode, Time> TxTime;

enum McsGroupType {
  WIFI_MINSTREL_GROUP_HT = 0,
  WIFI_MINSTREL_GROUP_VHT,
  WIFI_MINSTREL_GROUP_HE
};

inline std::ostream &operator<<(std::ostream &os, McsGroupType type) {
  switch (type) {
  case WIFI_MINSTREL_GROUP_HT:
    return (os << "HT");
  case WIFI_MINSTREL_GROUP_VHT:
    return (os << "VHT");
  case WIFI_MINSTREL_GROUP_HE:
    return (os << "HE");
  default:
    return (os << "INVALID");
  }
}

struct McsGroup {
  uint8_t streams;
  uint16_t gi;
  uint16_t chWidth;
  McsGroupType type;
  bool isSupported;
  TxTime ratesTxTimeTable;
  TxTime ratesFirstMpduTxTimeTable;
};

typedef std::vector<McsGroup> MinstrelMcsGroups;

struct MinstrelHtWifiRemoteStation;

struct MinstrelHtRateInfo {
  Time perfectTxTime;
  bool supported;
  uint8_t mcsIndex;
  uint32_t retryCount;
  uint32_t adjustedRetryCount;
  uint32_t numRateAttempt;
  uint32_t numRateSuccess;
  double prob;
  bool retryUpdated;
  double ewmaProb;
  double ewmsdProb;
  uint32_t prevNumRateAttempt;
  uint32_t prevNumRateSuccess;
  uint32_t numSamplesSkipped;
  uint64_t successHist;
  uint64_t attemptHist;
  double throughput;
};

typedef std::vector<MinstrelHtRateInfo> MinstrelHtRate;

struct GroupInfo {
  uint8_t m_col;
  uint8_t m_index;
  bool m_supported;
  uint16_t m_maxTpRate;
  uint16_t m_maxTpRate2;
  uint16_t m_maxProbRate;
  MinstrelHtRate m_ratesTable;
};

typedef std::vector<GroupInfo> McsGroupData;

static const uint8_t MAX_HT_SUPPORTED_STREAMS = 4;
static const uint8_t MAX_VHT_SUPPORTED_STREAMS = 8;
static const uint8_t MAX_HE_SUPPORTED_STREAMS = 8;
static const uint8_t MAX_HT_STREAM_GROUPS = 4;
static const uint8_t MAX_VHT_STREAM_GROUPS = 8;
static const uint8_t MAX_HE_STREAM_GROUPS = 12;
static const uint8_t MAX_HT_GROUP_RATES = 8;
static const uint8_t MAX_VHT_GROUP_RATES = 10;
static const uint8_t MAX_HE_GROUP_RATES = 12;
static const uint8_t MAX_HT_WIDTH = 40;
static const uint8_t MAX_VHT_WIDTH = 160;
static const uint8_t MAX_HE_WIDTH = 160;

class MinstrelHtWifiManager : public WifiRemoteStationManager {
public:
  static TypeId GetTypeId();
  MinstrelHtWifiManager();
  ~MinstrelHtWifiManager() override;

  void SetupPhy(const Ptr<WifiPhy> phy) override;
  void SetupMac(const Ptr<WifiMac> mac) override;
  int64_t AssignStreams(int64_t stream) override;

  typedef void (*RateChangeTracedCallback)(const uint64_t rate,
                                           const Mac48Address remoteAddress);

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
  void DoReportAmpduTxStatus(WifiRemoteStation *station,
                             uint16_t nSuccessfulMpdus, uint16_t nFailedMpdus,
                             double rxSnr, double dataSnr,
                             uint16_t dataChannelWidth,
                             uint8_t dataNss) override;
  bool DoNeedRetransmission(WifiRemoteStation *st, Ptr<const Packet> packet,
                            bool normally) override;

  bool IsValidMcs(Ptr<WifiPhy> phy, uint8_t streams, uint16_t chWidth,
                  WifiMode mode);

  Time CalculateMpduTxDuration(Ptr<WifiPhy> phy, uint8_t streams, uint16_t gi,
                               uint16_t chWidth, WifiMode mode,
                               MpduType mpduType);

  Time GetMpduTxTime(uint8_t groupId, WifiMode mode) const;

  void AddMpduTxTime(uint8_t groupId, WifiMode mode, Time t);

  Time GetFirstMpduTxTime(uint8_t groupId, WifiMode mode) const;

  void AddFirstMpduTxTime(uint8_t groupId, WifiMode mode, Time t);

  void UpdateRetry(MinstrelHtWifiRemoteStation *station);

  void UpdatePacketCounters(MinstrelHtWifiRemoteStation *station,
                            uint16_t nSuccessfulMpdus, uint16_t nFailedMpdus);

  uint16_t GetNextSample(MinstrelHtWifiRemoteStation *station);

  void SetNextSample(MinstrelHtWifiRemoteStation *station);

  uint16_t FindRate(MinstrelHtWifiRemoteStation *station);

  void UpdateStats(MinstrelHtWifiRemoteStation *station);

  void RateInit(MinstrelHtWifiRemoteStation *station);

  double CalculateThroughput(MinstrelHtWifiRemoteStation *station,
                             uint8_t groupId, uint8_t rateId, double ewmaProb);

  void SetBestStationThRates(MinstrelHtWifiRemoteStation *station,
                             uint16_t index);

  void SetBestProbabilityRate(MinstrelHtWifiRemoteStation *station,
                              uint16_t index);

  void CalculateRetransmits(MinstrelHtWifiRemoteStation *station,
                            uint16_t index);

  void CalculateRetransmits(MinstrelHtWifiRemoteStation *station,
                            uint8_t groupId, uint8_t rateId);

  Time CalculateTimeUnicastPacket(Time dataTransmissionTime,
                                  uint32_t shortRetries, uint32_t longRetries);

  double CalculateEwmsd(double oldEwmsd, double currentProb, double ewmaProb,
                        double weight);

  void InitSampleTable(MinstrelHtWifiRemoteStation *station);

  void PrintSampleTable(MinstrelHtWifiRemoteStation *station);

  void PrintTable(MinstrelHtWifiRemoteStation *station);

  void StatsDump(MinstrelHtWifiRemoteStation *station, uint8_t groupId,
                 std::ofstream &of);

  void CheckInit(MinstrelHtWifiRemoteStation *station);

  uint32_t CountRetries(MinstrelHtWifiRemoteStation *station);

  void UpdateRate(MinstrelHtWifiRemoteStation *station);

  uint8_t GetRateId(uint16_t index);

  uint8_t GetGroupId(uint16_t index);

  uint16_t GetIndex(uint8_t groupId, uint8_t rateId);

  uint8_t GetHtGroupId(uint8_t txstreams, uint16_t gi, uint16_t chWidth);

  uint8_t GetVhtGroupId(uint8_t txstreams, uint16_t gi, uint16_t chWidth);

  uint8_t GetHeGroupId(uint8_t txstreams, uint16_t gi, uint16_t chWidth);

  uint16_t GetLowestIndex(MinstrelHtWifiRemoteStation *station);

  uint16_t GetLowestIndex(MinstrelHtWifiRemoteStation *station,
                          uint8_t groupId);

  WifiModeList GetHeDeviceMcsList() const;

  WifiModeList GetVhtDeviceMcsList() const;

  WifiModeList GetHtDeviceMcsList() const;

  uint16_t UpdateRateAfterAllowedWidth(uint16_t txRate, uint16_t allowedWidth);

  Time m_updateStats;
  Time m_legacyUpdateStats;
  uint8_t m_lookAroundRate;
  uint8_t m_ewmaLevel;
  uint8_t m_nSampleCol;
  uint32_t m_frameLength;
  uint8_t m_numGroups;
  uint8_t m_numRates;
  bool m_useLatestAmendmentOnly;
  bool m_printStats;

  MinstrelMcsGroups m_minstrelGroups;

  Ptr<MinstrelWifiManager> m_legacyManager;

  Ptr<UniformRandomVariable> m_uniformRandomVariable;

  TracedValue<uint64_t> m_currentRate;
};

} // namespace ns3

#endif
