
#ifndef LTE_MI_ERROR_MODEL_H
#define LTE_MI_ERROR_MODEL_H

#include "lte-harq-phy.h"

#include <ns3/ptr.h>
#include <ns3/spectrum-value.h>

#include <list>
#include <stdint.h>
#include <vector>

namespace ns3 {

const uint16_t PDCCH_PCFICH_CURVE_SIZE = 46;
const uint16_t MI_MAP_QPSK_SIZE = 797;
const uint16_t MI_MAP_16QAM_SIZE = 994;
const uint16_t MI_MAP_64QAM_SIZE = 752;
const uint16_t MI_QPSK_MAX_ID = 9;
const uint16_t MI_16QAM_MAX_ID = 16;
const uint16_t MI_64QAM_MAX_ID = 28;
const uint16_t MI_QPSK_BLER_MAX_ID = 12;
const uint16_t MI_16QAM_BLER_MAX_ID = 22;
const uint16_t MI_64QAM_BLER_MAX_ID = 37;

struct TbStats_t {
  double tbler;
  double mi;
};

class LteMiErrorModel {
public:
  static double Mib(const SpectrumValue &sinr, const std::vector<int> &map,
                    uint8_t mcs);
  static double MappingMiBler(double mib, uint8_t ecrId, uint16_t cbSize);

  static TbStats_t GetTbDecodificationStats(const SpectrumValue &sinr,
                                            const std::vector<int> &map,
                                            uint16_t size, uint8_t mcs,
                                            HarqProcessInfoList_t miHistory);

  static double GetPcfichPdcchError(const SpectrumValue &sinr);
};

} // namespace ns3

#endif
