
#include "mpdu-aggregator.h"

#include "ampdu-subframe-header.h"
#include "ctrl-headers.h"
#include "msdu-aggregator.h"
#include "qos-txop.h"
#include "wifi-mac-trailer.h"
#include "wifi-mac.h"
#include "wifi-mpdu.h"
#include "wifi-net-device.h"
#include "wifi-phy.h"
#include "wifi-remote-station-manager.h"
#include "wifi-tx-parameters.h"
#include "wifi-tx-vector.h"

#include "ns3/he-capabilities.h"
#include "ns3/ht-capabilities.h"
#include "ns3/ht-frame-exchange-manager.h"
#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/vht-capabilities.h"

NS_LOG_COMPONENT_DEFINE("MpduAggregator");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(MpduAggregator);

TypeId MpduAggregator::GetTypeId() {
  static TypeId tid = TypeId("ns3::MpduAggregator")
                          .SetParent<Object>()
                          .SetGroupName("Wifi")
                          .AddConstructor<MpduAggregator>();
  return tid;
}

void MpduAggregator::DoDispose() {
  m_mac = nullptr;
  m_htFem = nullptr;
  Object::DoDispose();
}

void MpduAggregator::SetWifiMac(const Ptr<WifiMac> mac) {
  NS_LOG_FUNCTION(this << mac);
  m_mac = mac;
  m_htFem = DynamicCast<HtFrameExchangeManager>(
      m_mac->GetFrameExchangeManager(m_linkId));
}

void MpduAggregator::SetLinkId(uint8_t linkId) {
  NS_LOG_FUNCTION(this << +linkId);
  m_linkId = linkId;
  if (m_mac) {
    m_htFem = DynamicCast<HtFrameExchangeManager>(
        m_mac->GetFrameExchangeManager(m_linkId));
  }
}

void MpduAggregator::Aggregate(Ptr<const WifiMpdu> mpdu, Ptr<Packet> ampdu,
                               bool isSingle) {
  NS_LOG_FUNCTION(mpdu << ampdu << isSingle);
  NS_ASSERT(ampdu);
  NS_ASSERT(!isSingle || ampdu->GetSize() == 0);

  if (ampdu->GetSize() > 0) {
    uint8_t padding = CalculatePadding(ampdu->GetSize());

    if (padding) {
      Ptr<Packet> pad = Create<Packet>(padding);
      ampdu->AddAtEnd(pad);
    }
  }

  Ptr<Packet> tmp = mpdu->GetPacket()->Copy();
  tmp->AddHeader(mpdu->GetHeader());
  AddWifiMacTrailer(tmp);

  AmpduSubframeHeader hdr =
      GetAmpduSubframeHeader(static_cast<uint16_t>(tmp->GetSize()), isSingle);

  tmp->AddHeader(hdr);
  ampdu->AddAtEnd(tmp);
}

uint32_t MpduAggregator::GetSizeIfAggregated(uint32_t mpduSize,
                                             uint32_t ampduSize) {
  NS_LOG_FUNCTION(mpduSize << ampduSize);

  return ampduSize + CalculatePadding(ampduSize) + 4 + mpduSize;
}

uint32_t MpduAggregator::GetMaxAmpduSize(Mac48Address recipient, uint8_t tid,
                                         WifiModulationClass modulation) const {
  NS_LOG_FUNCTION(this << recipient << +tid << modulation);

  AcIndex ac = QosUtilsMapTidToAc(tid);

  uint32_t maxAmpduSize = m_mac->GetMaxAmpduSize(ac);

  if (maxAmpduSize == 0) {
    NS_LOG_DEBUG("A-MPDU Aggregation is disabled on this station for AC "
                 << ac);
    return 0;
  }

  Ptr<WifiRemoteStationManager> stationManager =
      m_mac->GetWifiRemoteStationManager(m_linkId);
  NS_ASSERT(stationManager);

  Ptr<const HeCapabilities> heCapabilities =
      stationManager->GetStationHeCapabilities(recipient);
  Ptr<const VhtCapabilities> vhtCapabilities =
      stationManager->GetStationVhtCapabilities(recipient);
  Ptr<const HtCapabilities> htCapabilities =
      stationManager->GetStationHtCapabilities(recipient);

  if (modulation >= WIFI_MOD_CLASS_HE) {
    NS_ABORT_MSG_IF(!heCapabilities, "HE Capabilities element not received");

    maxAmpduSize = std::min(maxAmpduSize, heCapabilities->GetMaxAmpduLength());
  } else if (modulation == WIFI_MOD_CLASS_VHT) {
    NS_ABORT_MSG_IF(!vhtCapabilities, "VHT Capabilities element not received");

    maxAmpduSize = std::min(maxAmpduSize, vhtCapabilities->GetMaxAmpduLength());
  } else if (modulation == WIFI_MOD_CLASS_HT) {
    NS_ABORT_MSG_IF(!htCapabilities, "HT Capabilities element not received");

    maxAmpduSize = std::min(maxAmpduSize, htCapabilities->GetMaxAmpduLength());
  } else {
    NS_LOG_DEBUG("A-MPDU aggregation is not available for non-HT PHYs");

    maxAmpduSize = 0;
  }

  return maxAmpduSize;
}

uint8_t MpduAggregator::CalculatePadding(uint32_t ampduSize) {
  return (4 - (ampduSize % 4)) % 4;
}

AmpduSubframeHeader MpduAggregator::GetAmpduSubframeHeader(uint16_t mpduSize,
                                                           bool isSingle) {
  AmpduSubframeHeader hdr;
  hdr.SetLength(mpduSize);
  if (isSingle) {
    hdr.SetEof(true);
  }
  return hdr;
}

std::vector<Ptr<WifiMpdu>>
MpduAggregator::GetNextAmpdu(Ptr<WifiMpdu> mpdu, WifiTxParameters &txParams,
                             Time availableTime) const {
  NS_LOG_FUNCTION(this << *mpdu << &txParams << availableTime);

  std::vector<Ptr<WifiMpdu>> mpduList;

  Mac48Address recipient = mpdu->GetHeader().GetAddr1();
  NS_ASSERT(mpdu->GetHeader().IsQosData() && !recipient.IsBroadcast());
  uint8_t tid = mpdu->GetHeader().GetQosTid();
  auto origRecipient = mpdu->GetOriginal()->GetHeader().GetAddr1();

  Ptr<QosTxop> qosTxop = m_mac->GetQosTxop(tid);
  NS_ASSERT(qosTxop);

  if (m_mac->GetBaAgreementEstablishedAsOriginator(recipient, tid) &&
      GetMaxAmpduSize(recipient, tid,
                      txParams.m_txVector.GetModulationClass()) > 0) {
    Ptr<WifiMpdu> nextMpdu = mpdu;

    while (nextMpdu) {
      NS_LOG_DEBUG("Adding packet with sequence number "
                   << nextMpdu->GetHeader().GetSequenceNumber()
                   << " to A-MPDU, packet size = " << nextMpdu->GetSize()
                   << ", A-MPDU size = " << txParams.GetSize(recipient));

      mpduList.push_back(nextMpdu);

      auto peekedMpdu = qosTxop->PeekNextMpdu(m_linkId, tid, origRecipient,
                                              nextMpdu->GetOriginal());
      nextMpdu = nullptr;

      if (peekedMpdu) {
        NS_ASSERT(IsInWindow(peekedMpdu->GetHeader().GetSequenceNumber(),
                             qosTxop->GetBaStartingSequence(origRecipient, tid),
                             qosTxop->GetBaBufferSize(origRecipient, tid)));

        peekedMpdu = m_htFem->CreateAliasIfNeeded(peekedMpdu);
        NS_LOG_DEBUG("Trying to aggregate another MPDU");
        nextMpdu = qosTxop->GetNextMpdu(m_linkId, peekedMpdu, txParams,
                                        availableTime, false);
      }
    }

    if (mpduList.size() == 1) {
      mpduList.clear();
    }
  }

  return mpduList;
}

} // namespace ns3
