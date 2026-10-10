
#ifndef LTE_UE_CMAC_SAP_H
#define LTE_UE_CMAC_SAP_H

#include <ns3/packet.h>

namespace ns3 {

class LteMacSapUser;

class LteUeCmacSapProvider {
public:
  virtual ~LteUeCmacSapProvider();

  struct RachConfig {
    uint8_t numberOfRaPreambles;
    uint8_t preambleTransMax;
    uint8_t raResponseWindowSize;
    uint8_t connEstFailCount;
  };

  virtual void ConfigureRach(RachConfig rc) = 0;

  virtual void StartContentionBasedRandomAccessProcedure() = 0;

  virtual void
  StartNonContentionBasedRandomAccessProcedure(uint16_t rnti, uint8_t rapId,
                                               uint8_t prachMask) = 0;

  struct LogicalChannelConfig {
    uint8_t priority;
    uint16_t prioritizedBitRateKbps;
    uint16_t bucketSizeDurationMs;
    uint8_t logicalChannelGroup;
  };

  virtual void AddLc(uint8_t lcId, LogicalChannelConfig lcConfig,
                     LteMacSapUser *msu) = 0;

  virtual void RemoveLc(uint8_t lcId) = 0;

  virtual void Reset() = 0;

  virtual void SetRnti(uint16_t rnti) = 0;

  virtual void NotifyConnectionSuccessful() = 0;

  virtual void SetImsi(uint64_t imsi) = 0;
};

class LteUeCmacSapUser {
public:
  virtual ~LteUeCmacSapUser();

  virtual void SetTemporaryCellRnti(uint16_t rnti) = 0;

  virtual void NotifyRandomAccessSuccessful() = 0;

  virtual void NotifyRandomAccessFailed() = 0;
};

} // namespace ns3

#endif
