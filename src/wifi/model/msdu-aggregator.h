
#ifndef MSDU_AGGREGATOR_H
#define MSDU_AGGREGATOR_H

#include "qos-utils.h"
#include "wifi-mode.h"
#include "wifi-mpdu.h"

#include "ns3/nstime.h"
#include "ns3/object.h"

#include <map>

namespace ns3 {

class Packet;
class QosTxop;
class WifiTxVector;
class WifiMac;
class HtFrameExchangeManager;
class WifiTxParameters;

class MsduAggregator : public Object {
public:
  typedef std::map<AcIndex, Ptr<QosTxop>> EdcaQueues;

  static TypeId GetTypeId();

  MsduAggregator() = default;
  ~MsduAggregator() override = default;

  void SetLinkId(uint8_t linkId);

  static uint16_t GetSizeIfAggregated(uint16_t msduSize, uint16_t amsduSize);

  Ptr<WifiMpdu> GetNextAmsdu(Ptr<WifiMpdu> peekedItem,
                             WifiTxParameters &txParams,
                             Time availableTime) const;

  uint16_t GetMaxAmsduSize(Mac48Address recipient, uint8_t tid,
                           WifiModulationClass modulation) const;

  static WifiMpdu::DeaggregatedMsdus Deaggregate(Ptr<Packet> aggregatedPacket);

  void SetWifiMac(const Ptr<WifiMac> mac);

  static uint8_t CalculatePadding(uint16_t amsduSize);

protected:
  void DoDispose() override;

private:
  Ptr<WifiMac> m_mac;
  Ptr<HtFrameExchangeManager> m_htFem;
  uint8_t m_linkId{0};
};

} // namespace ns3

#endif
