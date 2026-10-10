
#ifndef LTE_ENB_CMAC_SAP_H
#define LTE_ENB_CMAC_SAP_H

#include <ns3/packet.h>

namespace ns3 {

class LteMacSapUser;

class LteEnbCmacSapProvider {
public:
  virtual ~LteEnbCmacSapProvider();
  virtual void ConfigureMac(uint16_t ulBandwidth, uint16_t dlBandwidth) = 0;

  virtual void AddUe(uint16_t rnti) = 0;

  virtual void RemoveUe(uint16_t rnti) = 0;

  struct LcInfo {
    uint16_t rnti;
    uint8_t lcId;
    uint8_t lcGroup;
    uint8_t qci;
    uint8_t resourceType;
    uint64_t mbrUl;
    uint64_t mbrDl;
    uint64_t gbrUl;
    uint64_t gbrDl;
  };

  virtual void AddLc(LcInfo lcinfo, LteMacSapUser *msu) = 0;

  virtual void ReconfigureLc(LcInfo lcinfo) = 0;

  virtual void ReleaseLc(uint16_t rnti, uint8_t lcid) = 0;

  struct UeConfig {
    uint16_t m_rnti;
    uint8_t m_transmissionMode;
  };

  virtual void UeUpdateConfigurationReq(UeConfig params) = 0;

  struct RachConfig {
    uint8_t numberOfRaPreambles;
    uint8_t preambleTransMax;
    uint8_t raResponseWindowSize;
    uint8_t connEstFailCount;
  };

  virtual RachConfig GetRachConfig() = 0;

  struct AllocateNcRaPreambleReturnValue {
    bool valid;
    uint8_t raPreambleId;
    uint8_t raPrachMaskIndex;
  };

  virtual AllocateNcRaPreambleReturnValue
  AllocateNcRaPreamble(uint16_t rnti) = 0;
};

class LteEnbCmacSapUser {
public:
  virtual ~LteEnbCmacSapUser();

  virtual uint16_t AllocateTemporaryCellRnti() = 0;

  virtual void NotifyLcConfigResult(uint16_t rnti, uint8_t lcid,
                                    bool success) = 0;

  struct UeConfig {
    uint16_t m_rnti;
    uint8_t m_transmissionMode;
  };

  virtual void RrcConfigurationUpdateInd(UeConfig params) = 0;

  virtual bool IsRandomAccessCompleted(uint16_t rnti) = 0;
};

} // namespace ns3

#endif
