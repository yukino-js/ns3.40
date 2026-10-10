
#ifndef RADIO_BEARER_STATS_CONNECTOR_H
#define RADIO_BEARER_STATS_CONNECTOR_H

#include "ns3/config.h"
#include "ns3/ptr.h"
#include "ns3/simple-ref-count.h"
#include "ns3/traced-callback.h"

#include <map>

namespace ns3 {

class RadioBearerStatsCalculator;

class RadioBearerStatsConnector {
public:
  RadioBearerStatsConnector();

  void EnableRlcStats(Ptr<RadioBearerStatsCalculator> rlcStats);

  void EnablePdcpStats(Ptr<RadioBearerStatsCalculator> pdcpStats);

  void EnsureConnected();

  static void NotifyNewUeContextEnb(RadioBearerStatsConnector *c,
                                    std::string context, uint16_t cellid,
                                    uint16_t rnti);

  static void NotifyRandomAccessSuccessfulUe(RadioBearerStatsConnector *c,
                                             std::string context, uint64_t imsi,
                                             uint16_t cellid, uint16_t rnti);

  static void CreatedSrb1Ue(RadioBearerStatsConnector *c, std::string context,
                            uint64_t imsi, uint16_t cellid, uint16_t rnti);

  static void CreatedDrbEnb(RadioBearerStatsConnector *c, std::string context,
                            uint64_t imsi, uint16_t cellid, uint16_t rnti,
                            uint8_t lcid);

  static void CreatedDrbUe(RadioBearerStatsConnector *c, std::string context,
                           uint64_t imsi, uint16_t cellid, uint16_t rnti,
                           uint8_t lcid);

  void DisconnectTracesEnb(std::string context, uint64_t imsi, uint16_t cellid,
                           uint16_t rnti);

  void DisconnectTracesUe(std::string context, uint64_t imsi, uint16_t cellid,
                          uint16_t rnti);

private:
  void StoreUeManagerPath(std::string ueManagerPath, uint16_t cellId,
                          uint16_t rnti);

  void ConnectTracesSrb0(std::string context, uint64_t imsi, uint16_t cellId,
                         uint16_t rnti);

  void ConnectTracesSrb1(std::string context, uint64_t imsi, uint16_t cellId,
                         uint16_t rnti);

  void ConnectTracesDrbEnb(std::string context, uint64_t imsi, uint16_t cellId,
                           uint16_t rnti, uint8_t lcid);

  void ConnectTracesDrbUe(std::string context, uint64_t imsi, uint16_t cellId,
                          uint16_t rnti, uint8_t lcid);

  Ptr<RadioBearerStatsCalculator> m_rlcStats;
  Ptr<RadioBearerStatsCalculator> m_pdcpStats;

  bool m_connected;

  struct CellIdRnti {
    uint16_t cellId;
    uint16_t rnti;
  };

  friend bool operator<(const CellIdRnti &a, const CellIdRnti &b);

  std::map<CellIdRnti, std::string> m_ueManagerPathByCellIdRnti;
};

} // namespace ns3

#endif
