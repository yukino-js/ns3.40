
#ifndef PHY_STATS_CALCULATOR_H_
#define PHY_STATS_CALCULATOR_H_

#include "lte-stats-calculator.h"

#include "ns3/nstime.h"
#include "ns3/spectrum-value.h"
#include "ns3/uinteger.h"

#include <fstream>
#include <string>

namespace ns3 {

class PhyStatsCalculator : public LteStatsCalculator {
public:
  PhyStatsCalculator();

  ~PhyStatsCalculator() override;

  static TypeId GetTypeId();

  void SetCurrentCellRsrpSinrFilename(std::string filename);

  std::string GetCurrentCellRsrpSinrFilename();

  void SetUeSinrFilename(std::string filename);

  std::string GetUeSinrFilename();

  void SetInterferenceFilename(std::string filename);

  std::string GetInterferenceFilename();

  void ReportCurrentCellRsrpSinr(uint16_t cellId, uint64_t imsi, uint16_t rnti,
                                 double rsrp, double sinr,
                                 uint8_t componentCarrierId);

  void ReportUeSinr(uint16_t cellId, uint64_t imsi, uint16_t rnti,
                    double sinrLinear, uint8_t componentCarrierId);

  void ReportInterference(uint16_t cellId, Ptr<SpectrumValue> interference);

  static void ReportCurrentCellRsrpSinrCallback(
      Ptr<PhyStatsCalculator> phyStats, std::string path, uint16_t cellId,
      uint16_t rnti, double rsrp, double sinr, uint8_t componentCarrierId);

  static void ReportUeSinr(Ptr<PhyStatsCalculator> phyStats, std::string path,
                           uint16_t cellId, uint16_t rnti, double sinrLinear,
                           uint8_t componentCarrierId);

  static void ReportInterference(Ptr<PhyStatsCalculator> phyStats,
                                 std::string path, uint16_t cellId,
                                 Ptr<SpectrumValue> interference);

private:
  bool m_RsrpSinrFirstWrite;

  bool m_UeSinrFirstWrite;

  bool m_InterferenceFirstWrite;

  std::string m_RsrpSinrFilename;

  std::string m_ueSinrFilename;

  std::string m_interferenceFilename;

  std::ofstream m_rsrpOutFile;

  std::ofstream m_ueSinrOutFile;

  std::ofstream m_interferenceOutFile;
};

} // namespace ns3

#endif
