
#ifndef MAC_STATS_CALCULATOR_H_
#define MAC_STATS_CALCULATOR_H_

#include "lte-stats-calculator.h"

#include "ns3/lte-enb-mac.h"
#include "ns3/nstime.h"
#include "ns3/uinteger.h"

#include <fstream>
#include <string>

namespace ns3 {

class MacStatsCalculator : public LteStatsCalculator {
public:
  MacStatsCalculator();

  ~MacStatsCalculator() override;

  static TypeId GetTypeId();

  void SetUlOutputFilename(std::string outputFilename);

  std::string GetUlOutputFilename();

  void SetDlOutputFilename(std::string outputFilename);

  std::string GetDlOutputFilename();

  void DlScheduling(uint16_t cellId, uint64_t imsi,
                    DlSchedulingCallbackInfo dlSchedulingCallbackInfo);

  void UlScheduling(uint16_t cellId, uint64_t imsi, uint32_t frameNo,
                    uint32_t subframeNo, uint16_t rnti, uint8_t mcsTb,
                    uint16_t sizeTb, uint8_t componentCarrierId);

  static void
  DlSchedulingCallback(Ptr<MacStatsCalculator> macStats, std::string path,
                       DlSchedulingCallbackInfo dlSchedulingCallbackInfo);

  static void UlSchedulingCallback(Ptr<MacStatsCalculator> macStats,
                                   std::string path, uint32_t frameNo,
                                   uint32_t subframeNo, uint16_t rnti,
                                   uint8_t mcs, uint16_t size,
                                   uint8_t componentCarrierId);

private:
  bool m_dlFirstWrite;

  bool m_ulFirstWrite;

  std::ofstream m_dlOutFile;

  std::ofstream m_ulOutFile;
};

} // namespace ns3

#endif
