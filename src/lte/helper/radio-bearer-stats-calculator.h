
#ifndef RADIO_BEARER_STATS_CALCULATOR_H_
#define RADIO_BEARER_STATS_CALCULATOR_H_

#include "lte-stats-calculator.h"

#include "ns3/basic-data-calculators.h"
#include "ns3/lte-common.h"
#include "ns3/object.h"
#include "ns3/uinteger.h"

#include <fstream>
#include <map>
#include <string>

namespace ns3 {
typedef std::map<ImsiLcidPair_t, uint32_t> Uint32Map;
typedef std::map<ImsiLcidPair_t, uint64_t> Uint64Map;
typedef std::map<ImsiLcidPair_t, Ptr<MinMaxAvgTotalCalculator<uint32_t>>>
    Uint32StatsMap;
typedef std::map<ImsiLcidPair_t, Ptr<MinMaxAvgTotalCalculator<uint64_t>>>
    Uint64StatsMap;
typedef std::map<ImsiLcidPair_t, double> DoubleMap;
typedef std::map<ImsiLcidPair_t, LteFlowId_t> FlowIdMap;

class RadioBearerStatsCalculator : public LteStatsCalculator {
public:
  RadioBearerStatsCalculator();

  RadioBearerStatsCalculator(std::string protocolType);

  ~RadioBearerStatsCalculator() override;

  static TypeId GetTypeId();
  void DoDispose() override;

  std::string GetUlOutputFilename();

  std::string GetDlOutputFilename();

  void SetUlPdcpOutputFilename(std::string outputFilename);

  std::string GetUlPdcpOutputFilename();

  void SetDlPdcpOutputFilename(std::string outputFilename);

  std::string GetDlPdcpOutputFilename();

  void SetStartTime(Time t);

  Time GetStartTime() const;

  void SetEpoch(Time e);

  Time GetEpoch() const;

  void UlTxPdu(uint16_t cellId, uint64_t imsi, uint16_t rnti, uint8_t lcid,
               uint32_t packetSize);

  void UlRxPdu(uint16_t cellId, uint64_t imsi, uint16_t rnti, uint8_t lcid,
               uint32_t packetSize, uint64_t delay);

  void DlTxPdu(uint16_t cellId, uint64_t imsi, uint16_t rnti, uint8_t lcid,
               uint32_t packetSize);

  void DlRxPdu(uint16_t cellId, uint64_t imsi, uint16_t rnti, uint8_t lcid,
               uint32_t packetSize, uint64_t delay);

  uint32_t GetUlTxPackets(uint64_t imsi, uint8_t lcid);

  uint32_t GetUlRxPackets(uint64_t imsi, uint8_t lcid);

  uint64_t GetUlTxData(uint64_t imsi, uint8_t lcid);

  uint64_t GetUlRxData(uint64_t imsi, uint8_t lcid);

  uint32_t GetUlCellId(uint64_t imsi, uint8_t lcid);

  double GetUlDelay(uint64_t imsi, uint8_t lcid);

  std::vector<double> GetUlDelayStats(uint64_t imsi, uint8_t lcid);

  std::vector<double> GetUlPduSizeStats(uint64_t imsi, uint8_t lcid);

  uint32_t GetDlTxPackets(uint64_t imsi, uint8_t lcid);

  uint32_t GetDlRxPackets(uint64_t imsi, uint8_t lcid);

  uint64_t GetDlTxData(uint64_t imsi, uint8_t lcid);

  uint64_t GetDlRxData(uint64_t imsi, uint8_t lcid);

  uint32_t GetDlCellId(uint64_t imsi, uint8_t lcid);

  double GetDlDelay(uint64_t imsi, uint8_t lcid);

  std::vector<double> GetDlDelayStats(uint64_t imsi, uint8_t lcid);

  std::vector<double> GetDlPduSizeStats(uint64_t imsi, uint8_t lcid);

private:
  void ShowResults();

  void WriteUlResults(std::ofstream &outFile);

  void WriteDlResults(std::ofstream &outFile);

  void ResetResults();

  void RescheduleEndEpoch();

  void EndEpoch();

  EventId m_endEpochEvent;

  FlowIdMap m_flowId;

  Uint32Map m_dlCellId;
  Uint32Map m_dlTxPackets;
  Uint32Map m_dlRxPackets;
  Uint64Map m_dlTxData;
  Uint64Map m_dlRxData;
  Uint64StatsMap m_dlDelay;
  Uint32StatsMap m_dlPduSize;

  Uint32Map m_ulCellId;
  Uint32Map m_ulTxPackets;
  Uint32Map m_ulRxPackets;
  Uint64Map m_ulTxData;
  Uint64Map m_ulRxData;
  Uint64StatsMap m_ulDelay;
  Uint32StatsMap m_ulPduSize;

  Time m_startTime;

  Time m_epochDuration;

  bool m_firstWrite;

  bool m_pendingOutput;

  std::string m_protocolType;

  std::string m_dlPdcpOutputFilename;

  std::string m_ulPdcpOutputFilename;
};

} // namespace ns3

#endif
