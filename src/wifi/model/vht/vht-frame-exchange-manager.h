
#ifndef VHT_FRAME_EXCHANGE_MANAGER_H
#define VHT_FRAME_EXCHANGE_MANAGER_H

#include "ns3/ht-frame-exchange-manager.h"

namespace ns3 {

class VhtFrameExchangeManager : public HtFrameExchangeManager {
public:
  static TypeId GetTypeId();
  VhtFrameExchangeManager();
  ~VhtFrameExchangeManager() override;

protected:
  Ptr<WifiPsdu> GetWifiPsdu(Ptr<WifiMpdu> mpdu,
                            const WifiTxVector &txVector) const override;
  uint32_t GetPsduSize(Ptr<const WifiMpdu> mpdu,
                       const WifiTxVector &txVector) const override;
};

} // namespace ns3

#endif
