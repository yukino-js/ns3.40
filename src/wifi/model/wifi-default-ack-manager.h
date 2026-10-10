
#ifndef WIFI_DEFAULT_ACK_MANAGER_H
#define WIFI_DEFAULT_ACK_MANAGER_H

#include "wifi-ack-manager.h"

namespace ns3 {

class WifiTxParameters;
class WifiMpdu;

class WifiDefaultAckManager : public WifiAckManager {
public:
  static TypeId GetTypeId();

  WifiDefaultAckManager();
  ~WifiDefaultAckManager() override;

  std::unique_ptr<WifiAcknowledgment>
  TryAddMpdu(Ptr<const WifiMpdu> mpdu,
             const WifiTxParameters &txParams) override;
  std::unique_ptr<WifiAcknowledgment>
  TryAggregateMsdu(Ptr<const WifiMpdu> msdu,
                   const WifiTxParameters &txParams) override;

  uint16_t GetMaxDistFromStartingSeq(Ptr<const WifiMpdu> mpdu,
                                     const WifiTxParameters &txParams) const;

protected:
  bool IsResponseNeeded(Ptr<const WifiMpdu> mpdu,
                        const WifiTxParameters &txParams) const;

  bool ExistInflightOnSameLink(Ptr<const WifiMpdu> mpdu) const;

private:
  virtual std::unique_ptr<WifiAcknowledgment>
  GetAckInfoIfBarBaSequence(Ptr<const WifiMpdu> mpdu,
                            const WifiTxParameters &txParams);
  virtual std::unique_ptr<WifiAcknowledgment>
  GetAckInfoIfTfMuBar(Ptr<const WifiMpdu> mpdu,
                      const WifiTxParameters &txParams);
  virtual std::unique_ptr<WifiAcknowledgment>
  GetAckInfoIfAggregatedMuBar(Ptr<const WifiMpdu> mpdu,
                              const WifiTxParameters &txParams);

  virtual std::unique_ptr<WifiAcknowledgment>
  TryUlMuTransmission(Ptr<const WifiMpdu> mpdu,
                      const WifiTxParameters &txParams);

  bool m_useExplicitBar;
  double m_baThreshold;
  WifiAcknowledgment::Method m_dlMuAckType;
  uint8_t m_maxMcsForBlockAckInTbPpdu;
};

} // namespace ns3

#endif
