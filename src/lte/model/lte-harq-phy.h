
#ifndef LTE_HARQ_PHY_MODULE_H
#define LTE_HARQ_PHY_MODULE_H

#include <ns3/assert.h>
#include <ns3/log.h>
#include <ns3/simple-ref-count.h>

#include <map>
#include <math.h>
#include <vector>

namespace ns3 {

struct HarqProcessInfoElement_t {
  double m_mi;
  uint8_t m_rv;
  uint16_t m_infoBits;
  uint16_t m_codeBits;
};

typedef std::vector<HarqProcessInfoElement_t> HarqProcessInfoList_t;

class LteHarqPhy : public SimpleRefCount<LteHarqPhy> {
public:
  LteHarqPhy();
  ~LteHarqPhy();

  void SubframeIndication(uint32_t frameNo, uint32_t subframeNo);

  double GetAccumulatedMiDl(uint8_t harqProcId, uint8_t layer);

  HarqProcessInfoList_t GetHarqProcessInfoDl(uint8_t harqProcId, uint8_t layer);

  double GetAccumulatedMiUl(uint16_t rnti);

  HarqProcessInfoList_t GetHarqProcessInfoUl(uint16_t rnti, uint8_t harqProcId);

  void UpdateDlHarqProcessStatus(uint8_t id, uint8_t layer, double mi,
                                 uint16_t infoBytes, uint16_t codeBytes);

  void ResetDlHarqProcessStatus(uint8_t id);

  void UpdateUlHarqProcessStatus(uint16_t rnti, double mi, uint16_t infoBytes,
                                 uint16_t codeBytes);

  void ResetUlHarqProcessStatus(uint16_t rnti, uint8_t id);

  void ClearDlHarqBuffer(uint16_t rnti);

private:
  std::vector<std::vector<HarqProcessInfoList_t>> m_miDlHarqProcessesInfoMap;
  std::map<uint16_t, std::vector<HarqProcessInfoList_t>>
      m_miUlHarqProcessesInfoMap;
};

} // namespace ns3

#endif
