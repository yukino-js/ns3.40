
#ifndef MPDU_AGGREGATOR_H
#define MPDU_AGGREGATOR_H

#include "qos-utils.h"
#include "wifi-mode.h"

#include "ns3/nstime.h"
#include "ns3/object.h"

#include <vector>

namespace ns3 {

class AmpduSubframeHeader;
class WifiTxVector;
class QosTxop;
class Packet;
class WifiMac;
class WifiMpdu;
class WifiTxParameters;
class HtFrameExchangeManager;

class MpduAggregator : public Object {
public:
  typedef std::map<AcIndex, Ptr<QosTxop>> EdcaQueues;

  static TypeId GetTypeId();

  MpduAggregator() = default;
  ~MpduAggregator() override = default;

  static void Aggregate(Ptr<const WifiMpdu> mpdu, Ptr<Packet> ampdu,
                        bool isSingle);

  void SetLinkId(uint8_t linkId);

  static uint32_t GetSizeIfAggregated(uint32_t mpduSize, uint32_t ampduSize);

  uint32_t GetMaxAmpduSize(Mac48Address recipient, uint8_t tid,
                           WifiModulationClass modulation) const;

  std::vector<Ptr<WifiMpdu>> GetNextAmpdu(Ptr<WifiMpdu> mpdu,
                                          WifiTxParameters &txParams,
                                          Time availableTime) const;

  void SetWifiMac(const Ptr<WifiMac> mac);

  static uint8_t CalculatePadding(uint32_t ampduSize);

  static AmpduSubframeHeader GetAmpduSubframeHeader(uint16_t mpduSize,
                                                    bool isSingle);

protected:
  void DoDispose() override;

private:
  Ptr<WifiMac> m_mac;
  Ptr<HtFrameExchangeManager> m_htFem;
  uint8_t m_linkId{0};
};

} // namespace ns3

#endif
