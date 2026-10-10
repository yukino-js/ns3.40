
#include "msdu-aggregator.h"

#include "qos-txop.h"
#include "wifi-mac-queue.h"
#include "wifi-mac-trailer.h"
#include "wifi-mac.h"
#include "wifi-remote-station-manager.h"
#include "wifi-tx-parameters.h"

#include "ns3/ht-capabilities.h"
#include "ns3/ht-frame-exchange-manager.h"
#include "ns3/log.h"
#include "ns3/packet.h"

#include <algorithm>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("MsduAggregator");

NS_OBJECT_ENSURE_REGISTERED(MsduAggregator);

TypeId MsduAggregator::GetTypeId() {
  static TypeId tid = TypeId("ns3::MsduAggregator")
                          .SetParent<Object>()
                          .SetGroupName("Wifi")
                          .AddConstructor<MsduAggregator>();
  return tid;
}

void MsduAggregator::DoDispose() {
  m_mac = nullptr;
  m_htFem = nullptr;
  Object::DoDispose();
}

void MsduAggregator::SetWifiMac(const Ptr<WifiMac> mac) {
  NS_LOG_FUNCTION(this << mac);
  m_mac = mac;
  m_htFem = DynamicCast<HtFrameExchangeManager>(
      m_mac->GetFrameExchangeManager(m_linkId));
}

void MsduAggregator::SetLinkId(uint8_t linkId) {
  NS_LOG_FUNCTION(this << +linkId);
  m_linkId = linkId;
  if (m_mac) {
    m_htFem = DynamicCast<HtFrameExchangeManager>(
        m_mac->GetFrameExchangeManager(m_linkId));
  }
}

uint16_t MsduAggregator::GetSizeIfAggregated(uint16_t msduSize,
                                             uint16_t amsduSize) {
  NS_LOG_FUNCTION(msduSize << amsduSize);

  return amsduSize + CalculatePadding(amsduSize) + 14 + msduSize;
}

Ptr<WifiMpdu> MsduAggregator::GetNextAmsdu(Ptr<WifiMpdu> peekedItem,
                                           WifiTxParameters &txParams,
                                           Time availableTime) const {
  NS_LOG_FUNCTION(this << *peekedItem << &txParams << availableTime);

  Ptr<WifiMacQueue> queue = m_mac->GetTxopQueue(peekedItem->GetQueueAc());

  uint8_t tid = peekedItem->GetHeader().GetQosTid();
  auto recipient = peekedItem->GetOriginal()->GetHeader().GetAddr1();

  NS_ABORT_MSG_IF(recipient.IsBroadcast(), "Recipient address is broadcast");

  NS_ASSERT(m_htFem);

  if (GetMaxAmsduSize(recipient, tid,
                      txParams.m_txVector.GetModulationClass()) == 0) {
    NS_LOG_DEBUG("A-MSDU aggregation disabled");
    return nullptr;
  }

  Ptr<WifiMpdu> amsdu = queue->GetOriginal(peekedItem);
  uint8_t nMsdu = 1;
  peekedItem =
      queue->PeekByTidAndAddress(tid, recipient, peekedItem->GetOriginal());

  while (peekedItem &&
         m_htFem->TryAggregateMsdu(peekedItem =
                                       m_htFem->CreateAliasIfNeeded(peekedItem),
                                   txParams, availableTime)) {
    Ptr<const WifiMpdu> msdu = peekedItem->GetOriginal();
    peekedItem = queue->PeekByTidAndAddress(tid, recipient, msdu);
    queue->DequeueIfQueued({amsdu});
    amsdu->Aggregate(msdu);
    queue->Replace(msdu, amsdu);

    nMsdu++;
  }

  if (nMsdu == 1) {
    NS_LOG_DEBUG("Aggregation failed (could not aggregate at least two MSDUs)");
    return nullptr;
  }

  return m_htFem->CreateAliasIfNeeded(amsdu);
}

uint8_t MsduAggregator::CalculatePadding(uint16_t amsduSize) {
  return (4 - (amsduSize % 4)) % 4;
}

uint16_t MsduAggregator::GetMaxAmsduSize(Mac48Address recipient, uint8_t tid,
                                         WifiModulationClass modulation) const {
  NS_LOG_FUNCTION(this << recipient << +tid << modulation);

  AcIndex ac = QosUtilsMapTidToAc(tid);

  uint16_t maxAmsduSize = m_mac->GetMaxAmsduSize(ac);

  if (maxAmsduSize == 0) {
    NS_LOG_DEBUG("A-MSDU Aggregation is disabled on this station for AC "
                 << ac);
    return 0;
  }

  Ptr<WifiRemoteStationManager> stationManager =
      m_mac->GetWifiRemoteStationManager(m_linkId);
  NS_ASSERT(stationManager);

  auto ehtCapabilities = stationManager->GetStationEhtCapabilities(recipient);
  auto vhtCapabilities = stationManager->GetStationVhtCapabilities(recipient);
  auto htCapabilities = stationManager->GetStationHtCapabilities(recipient);

  uint16_t maxMpduSize = 0;
  if (ehtCapabilities &&
      m_mac->GetWifiPhy(m_linkId)->GetPhyBand() == WIFI_PHY_BAND_2_4GHZ) {
    maxMpduSize = ehtCapabilities->GetMaxMpduLength();
  } else if (vhtCapabilities && m_mac->GetWifiPhy(m_linkId)->GetPhyBand() !=
                                    WIFI_PHY_BAND_2_4GHZ) {
    maxMpduSize = vhtCapabilities->GetMaxMpduLength();
  }

  if (!htCapabilities) {
    NS_LOG_DEBUG("A-MSDU Aggregation disabled because the recipient did not"
                 " send an HT Capabilities element");
    return 0;
  }

  if (modulation >= WIFI_MOD_CLASS_EHT) {
    NS_ABORT_MSG_IF(maxMpduSize == 0, "Max MPDU size not advertised");
    maxAmsduSize =
        std::min(maxAmsduSize, static_cast<uint16_t>(maxMpduSize - 56));
  } else if (modulation == WIFI_MOD_CLASS_HE) {
    if (m_mac->GetWifiPhy(m_linkId)->GetStandard() < WIFI_STANDARD_80211be &&
        m_mac->GetWifiPhy(m_linkId)->GetPhyBand() == WIFI_PHY_BAND_2_4GHZ) {
      maxAmsduSize =
          std::min(maxAmsduSize, htCapabilities->GetMaxAmsduLength());
    } else {
      NS_ABORT_MSG_IF(maxMpduSize == 0, "Max MPDU size not advertised");
      maxAmsduSize =
          std::min(maxAmsduSize, static_cast<uint16_t>(maxMpduSize - 56));
    }
  } else if (modulation == WIFI_MOD_CLASS_VHT) {
    NS_ABORT_MSG_IF(maxMpduSize == 0, "Max MPDU size not advertised");
    maxAmsduSize =
        std::min(maxAmsduSize, static_cast<uint16_t>(maxMpduSize - 56));
  } else if (modulation >= WIFI_MOD_CLASS_HT) {
    maxAmsduSize = std::min(maxAmsduSize, htCapabilities->GetMaxAmsduLength());
  } else {
    maxAmsduSize = std::min(maxAmsduSize, static_cast<uint16_t>(3839));
  }

  return maxAmsduSize;
}

WifiMpdu::DeaggregatedMsdus
MsduAggregator::Deaggregate(Ptr<Packet> aggregatedPacket) {
  NS_LOG_FUNCTION_NOARGS();
  WifiMpdu::DeaggregatedMsdus set;

  AmsduSubframeHeader hdr;
  Ptr<Packet> extractedMsdu = Create<Packet>();
  uint32_t maxSize = aggregatedPacket->GetSize();
  uint16_t extractedLength;
  uint8_t padding;
  uint32_t deserialized = 0;

  while (deserialized < maxSize) {
    deserialized += aggregatedPacket->RemoveHeader(hdr);
    extractedLength = hdr.GetLength();
    extractedMsdu = aggregatedPacket->CreateFragment(
        0, static_cast<uint32_t>(extractedLength));
    aggregatedPacket->RemoveAtStart(extractedLength);
    deserialized += extractedLength;

    padding = (4 - ((extractedLength + 14) % 4)) % 4;

    if (padding > 0 && deserialized < maxSize) {
      aggregatedPacket->RemoveAtStart(padding);
      deserialized += padding;
    }

    std::pair<Ptr<const Packet>, AmsduSubframeHeader> packetHdr(extractedMsdu,
                                                                hdr);
    set.push_back(packetHdr);
  }
  NS_LOG_INFO("Deaggreated A-MSDU: extracted " << set.size() << " MSDUs");
  return set;
}

} // namespace ns3
