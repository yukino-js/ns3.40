
#ifndef PHY_TX_STATS_CALCULATOR_H_
#define PHY_TX_STATS_CALCULATOR_H_

#include "lte-stats-calculator.h"

#include "ns3/nstime.h"
#include "ns3/uinteger.h"
#include <ns3/lte-common.h>

#include <fstream>
#include <string>

namespace ns3 {

class PhyTxStatsCalculator : public LteStatsCalculator {
public:
  PhyTxStatsCalculator();

  ~PhyTxStatsCalculator() override;

  static TypeId GetTypeId();

  void SetUlTxOutputFilename(std::string outputFilename);

  std::string GetUlTxOutputFilename();

  void SetDlTxOutputFilename(std::string outputFilename);

  std::string GetDlTxOutputFilename();

  void DlPhyTransmission(PhyTransmissionStatParameters params);

  void UlPhyTransmission(PhyTransmissionStatParameters params);

  static void DlPhyTransmissionCallback(Ptr<PhyTxStatsCalculator> phyTxStats,
                                        std::string path,
                                        PhyTransmissionStatParameters params);

  static void UlPhyTransmissionCallback(Ptr<PhyTxStatsCalculator> phyTxStats,
                                        std::string path,
                                        PhyTransmissionStatParameters params);

private:
  bool m_dlTxFirstWrite;

  bool m_ulTxFirstWrite;

  std::ofstream m_dlTxOutFile;

  std::ofstream m_ulTxOutFile;
};

} // namespace ns3

#endif
