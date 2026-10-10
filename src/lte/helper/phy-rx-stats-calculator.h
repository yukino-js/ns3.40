
#ifndef PHY_RX_STATS_CALCULATOR_H_
#define PHY_RX_STATS_CALCULATOR_H_

#include "lte-stats-calculator.h"

#include "ns3/nstime.h"
#include "ns3/uinteger.h"
#include <ns3/lte-common.h>

#include <fstream>
#include <string>

namespace ns3 {

class PhyRxStatsCalculator : public LteStatsCalculator {
public:
  PhyRxStatsCalculator();

  ~PhyRxStatsCalculator() override;

  static TypeId GetTypeId();

  void SetUlRxOutputFilename(std::string outputFilename);

  std::string GetUlRxOutputFilename();

  void SetDlRxOutputFilename(std::string outputFilename);

  std::string GetDlRxOutputFilename();

  void DlPhyReception(PhyReceptionStatParameters params);

  void UlPhyReception(PhyReceptionStatParameters params);

  static void DlPhyReceptionCallback(Ptr<PhyRxStatsCalculator> phyRxStats,
                                     std::string path,
                                     PhyReceptionStatParameters params);

  static void UlPhyReceptionCallback(Ptr<PhyRxStatsCalculator> phyRxStats,
                                     std::string path,
                                     PhyReceptionStatParameters params);

private:
  bool m_dlRxFirstWrite;

  bool m_ulRxFirstWrite;

  std::ofstream m_dlRxOutFile;

  std::ofstream m_ulRxOutFile;
};

} // namespace ns3

#endif
