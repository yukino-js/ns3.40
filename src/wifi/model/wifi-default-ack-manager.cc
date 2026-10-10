
#include "wifi-default-ack-manager.h"

#include "ap-wifi-mac.h"
#include "ctrl-headers.h"
#include "qos-utils.h"
#include "wifi-mac-queue.h"
#include "wifi-mpdu.h"
#include "wifi-protection.h"
#include "wifi-tx-parameters.h"

#include "ns3/he-frame-exchange-manager.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("WifiDefaultAckManager");

NS_OBJECT_ENSURE_REGISTERED(WifiDefaultAckManager);

TypeId WifiDefaultAckManager::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::WifiDefaultAckManager")
          .SetParent<WifiAckManager>()
          .SetGroupName("Wifi")
          .AddConstructor<WifiDefaultAckManager>()
          .AddAttribute(
              "UseExplicitBar",
              "Specify whether to send Block Ack Requests (if true) or use"
              " Implicit Block Ack Request ack policy (if false).",
              BooleanValue(false),
              MakeBooleanAccessor(&WifiDefaultAckManager::m_useExplicitBar),
              MakeBooleanChecker())
          .AddAttribute(
              "BaThreshold",
              "Immediate acknowledgment is requested upon transmission of a "
              "frame "
              "whose sequence number is distant at least BaThreshold "
              "multiplied "
              "by the transmit window size from the starting sequence number "
              "of "
              "the transmit window. Set to zero to request a response for "
              "every "
              "transmitted frame.",
              DoubleValue(0.0),
              MakeDoubleAccessor(&WifiDefaultAckManager::m_baThreshold),
              MakeDoubleChecker<double>(0.0, 1.0))
          .AddAttribute(
              "DlMuAckSequenceType",
              "Type of the acknowledgment sequence for DL MU PPDUs.",
              EnumValue(WifiAcknowledgment::DL_MU_BAR_BA_SEQUENCE),
              MakeEnumAccessor(&WifiDefaultAckManager::m_dlMuAckType),
              MakeEnumChecker(
                  WifiAcknowledgment::DL_MU_BAR_BA_SEQUENCE,
                  "DL_MU_BAR_BA_SEQUENCE", WifiAcknowledgment::DL_MU_TF_MU_BAR,
                  "DL_MU_TF_MU_BAR", WifiAcknowledgment::DL_MU_AGGREGATE_TF,
                  "DL_MU_AGGREGATE_TF"))
          .AddAttribute(
              "MaxBlockAckMcs",
              "The MCS used to send a BlockAck in a TB PPDU is the minimum "
              "between "
              "the MCS used for the PSDU sent in the preceding DL MU PPDU and "
              "the "
              "value of this attribute.",
              UintegerValue(5),
              MakeUintegerAccessor(
                  &WifiDefaultAckManager::m_maxMcsForBlockAckInTbPpdu),
              MakeUintegerChecker<uint8_t>(0, 11));
  return tid;
}

WifiDefaultAckManager::WifiDefaultAckManager() { NS_LOG_FUNCTION(this); }

WifiDefaultAckManager::~WifiDefaultAckManager() { NS_LOG_FUNCTION_NOARGS(); }

uint16_t WifiDefaultAckManager::GetMaxDistFromStartingSeq(
    Ptr<const WifiMpdu> mpdu, const WifiTxParameters &txParams) const {
  NS_LOG_FUNCTION(this << *mpdu << &txParams);

  auto receiver = mpdu->GetHeader().GetAddr1();
  auto origReceiver = mpdu->GetOriginal()->GetHeader().GetAddr1();

  uint8_t tid = mpdu->GetHeader().GetQosTid();
  Ptr<QosTxop> edca = m_mac->GetQosTxop(tid);
  NS_ABORT_MSG_IF(
      !m_mac->GetBaAgreementEstablishedAsOriginator(origReceiver, tid),
      "An established Block Ack agreement is required");

  uint16_t startingSeq = edca->GetBaStartingSequence(origReceiver, tid);
  uint16_t maxDistFromStartingSeq =
      (mpdu->GetHeader().GetSequenceNumber() - startingSeq + SEQNO_SPACE_SIZE) %
      SEQNO_SPACE_SIZE;
  NS_ABORT_MSG_IF(maxDistFromStartingSeq >= SEQNO_SPACE_HALF_SIZE,
                  "The given QoS data frame is too old");

  const WifiTxParameters::PsduInfo *psduInfo = txParams.GetPsduInfo(receiver);

  if (!psduInfo ||
      psduInfo->seqNumbers.find(tid) == psduInfo->seqNumbers.end()) {
    return maxDistFromStartingSeq;
  }

  for (const auto &seqNumber : psduInfo->seqNumbers.at(tid)) {
    if (!QosUtilsIsOldPacket(startingSeq, seqNumber)) {
      uint16_t currDistToStartingSeq =
          (seqNumber - startingSeq + SEQNO_SPACE_SIZE) % SEQNO_SPACE_SIZE;

      if (currDistToStartingSeq > maxDistFromStartingSeq) {
        maxDistFromStartingSeq = currDistToStartingSeq;
      }
    }
  }

  NS_LOG_DEBUG("Returning " << maxDistFromStartingSeq);
  return maxDistFromStartingSeq;
}

bool WifiDefaultAckManager::IsResponseNeeded(
    Ptr<const WifiMpdu> mpdu, const WifiTxParameters &txParams) const {
  NS_LOG_FUNCTION(this << *mpdu << &txParams);

  uint8_t tid = mpdu->GetHeader().GetQosTid();
  Mac48Address receiver = mpdu->GetOriginal()->GetHeader().GetAddr1();
  Ptr<QosTxop> edca = m_mac->GetQosTxop(tid);

  return !(m_baThreshold > 0 &&
           GetMaxDistFromStartingSeq(mpdu, txParams) <
               m_baThreshold * edca->GetBaBufferSize(receiver, tid) &&
           (edca->GetWifiMacQueue()->GetNPackets(
                {WIFI_QOSDATA_QUEUE, WIFI_UNICAST, receiver, tid}) -
                edca->GetBaManager()->GetNBufferedPackets(receiver, tid) >
            1) &&
           !(edca->GetTxopLimit(m_linkId).IsStrictlyPositive() &&
             edca->GetRemainingTxop(m_linkId) == edca->GetTxopLimit(m_linkId) &&
             !(txParams.m_protection &&
               txParams.m_protection->method == WifiProtection::RTS_CTS)));
}

bool WifiDefaultAckManager::ExistInflightOnSameLink(
    Ptr<const WifiMpdu> mpdu) const {
  NS_ASSERT(mpdu->GetHeader().IsQosData());
  auto tid = mpdu->GetHeader().GetQosTid();
  NS_ASSERT(mpdu->IsQueued());
  auto queue = m_mac->GetTxopQueue(mpdu->GetQueueAc());
  auto origReceiver = mpdu->GetOriginal()->GetHeader().GetAddr1();
  auto agreement =
      m_mac->GetBaAgreementEstablishedAsOriginator(origReceiver, tid);
  NS_ASSERT(agreement);
  auto mpduDist =
      agreement->get().GetDistance(mpdu->GetHeader().GetSequenceNumber());

  Ptr<WifiMpdu> item = queue->PeekByTidAndAddress(tid, origReceiver);

  while (item) {
    auto itemDist =
        agreement->get().GetDistance(item->GetHeader().GetSequenceNumber());
    if (itemDist == mpduDist) {
      NS_LOG_DEBUG("No previous MPDU in-flight on the same link");
      return false;
    }
    NS_ABORT_MSG_IF(itemDist > mpduDist,
                    "While searching for given MPDU ("
                        << *mpdu << "), found first another one (" << *item
                        << ") with higher sequence number");
    if (auto linkIds = item->GetInFlightLinkIds();
        linkIds.count(m_linkId) > 0) {
      NS_LOG_DEBUG("Found MPDU inflight on the same link");
      return true;
    }
    item = queue->PeekByTidAndAddress(tid, origReceiver, item);
  }
  NS_ABORT_MSG("Should not get here");
  return false;
}

std::unique_ptr<WifiAcknowledgment>
WifiDefaultAckManager::TryAddMpdu(Ptr<const WifiMpdu> mpdu,
                                  const WifiTxParameters &txParams) {
  NS_LOG_FUNCTION(this << *mpdu << &txParams);

  if (txParams.m_txVector.IsDlMu()) {
    switch (m_dlMuAckType) {
    case WifiAcknowledgment::DL_MU_BAR_BA_SEQUENCE:
      return GetAckInfoIfBarBaSequence(mpdu, txParams);
    case WifiAcknowledgment::DL_MU_TF_MU_BAR:
      return GetAckInfoIfTfMuBar(mpdu, txParams);
    case WifiAcknowledgment::DL_MU_AGGREGATE_TF:
      return GetAckInfoIfAggregatedMuBar(mpdu, txParams);
    default:
      NS_ABORT_MSG("Unknown DL acknowledgment method");
      return nullptr;
    }
  }

  const WifiMacHeader &hdr = mpdu->GetHeader();
  Mac48Address receiver = hdr.GetAddr1();

  if (txParams.m_txVector.IsUlMu()) {
    if (hdr.IsQosData() && !hdr.HasData()) {
      std::unique_ptr<WifiAcknowledgment> acknowledgment;

      if (txParams.m_acknowledgment) {
        NS_ASSERT(txParams.m_acknowledgment->method ==
                  WifiAcknowledgment::NONE);
        acknowledgment = txParams.m_acknowledgment->Copy();
      } else {
        acknowledgment = std::make_unique<WifiNoAck>();
      }
      acknowledgment->SetQosAckPolicy(receiver, hdr.GetQosTid(),
                                      WifiMacHeader::NO_ACK);
      return acknowledgment;
    }

    if (txParams.m_acknowledgment) {
      NS_ASSERT(txParams.m_acknowledgment->method ==
                WifiAcknowledgment::ACK_AFTER_TB_PPDU);
      return nullptr;
    }

    auto acknowledgment = std::make_unique<WifiAckAfterTbPpdu>();
    if (hdr.IsQosData()) {
      acknowledgment->SetQosAckPolicy(receiver, hdr.GetQosTid(),
                                      WifiMacHeader::NORMAL_ACK);
    }
    return acknowledgment;
  }

  if (hdr.IsTrigger()) {
    return TryUlMuTransmission(mpdu, txParams);
  }

  if (txParams.m_acknowledgment &&
      txParams.m_acknowledgment->method == WifiAcknowledgment::BLOCK_ACK) {
    return nullptr;
  }

  if (receiver.IsGroup()) {
    NS_ABORT_MSG_IF(txParams.GetSize(receiver) > 0,
                    "Unicast frames only can be aggregated");
    auto acknowledgment = std::make_unique<WifiNoAck>();
    if (hdr.IsQosData()) {
      acknowledgment->SetQosAckPolicy(receiver, hdr.GetQosTid(),
                                      WifiMacHeader::NO_ACK);
    }
    return acknowledgment;
  }

  if ((!hdr.IsQosData() || !m_mac->GetBaAgreementEstablishedAsOriginator(
                               receiver, hdr.GetQosTid())) &&
      !hdr.IsBlockAckReq()) {
    NS_LOG_DEBUG("Non-QoS data frame or Block Ack agreement not established, "
                 "request Normal Ack");
    auto acknowledgment = std::make_unique<WifiNormalAck>();
    acknowledgment->ackTxVector = GetWifiRemoteStationManager()->GetAckTxVector(
        receiver, txParams.m_txVector);
    if (hdr.IsQosData()) {
      acknowledgment->SetQosAckPolicy(receiver, hdr.GetQosTid(),
                                      WifiMacHeader::NORMAL_ACK);
    }
    return acknowledgment;
  }

  if (!hdr.IsBlockAckReq() && !IsResponseNeeded(mpdu, txParams)) {
    NS_LOG_DEBUG(
        "A response is not needed: no ack for now, use Block Ack policy");
    if (txParams.m_acknowledgment &&
        txParams.m_acknowledgment->method == WifiAcknowledgment::NONE) {
      return nullptr;
    }

    auto acknowledgment = std::make_unique<WifiNoAck>();
    if (hdr.IsQosData()) {
      acknowledgment->SetQosAckPolicy(receiver, hdr.GetQosTid(),
                                      WifiMacHeader::BLOCK_ACK);
    }
    return acknowledgment;
  }

  uint8_t tid = GetTid(mpdu->GetPacket(), hdr);
  if (!hdr.IsBlockAckReq() && txParams.GetSize(receiver) == 0 &&
      !ExistInflightOnSameLink(mpdu)) {
    NS_LOG_DEBUG(
        "Sending a single MPDU, no previous frame to ack: request Normal Ack");
    auto acknowledgment = std::make_unique<WifiNormalAck>();
    acknowledgment->ackTxVector = GetWifiRemoteStationManager()->GetAckTxVector(
        receiver, txParams.m_txVector);
    acknowledgment->SetQosAckPolicy(receiver, tid, WifiMacHeader::NORMAL_ACK);
    return acknowledgment;
  }

  if (!hdr.IsBlockAckReq() &&
      (txParams.GetSize(receiver) == 0 || m_useExplicitBar)) {
    NS_LOG_DEBUG("Request to schedule a Block Ack Request");

    auto acknowledgment = std::make_unique<WifiBarBlockAck>();
    acknowledgment->blockAckReqTxVector =
        GetWifiRemoteStationManager()->GetBlockAckTxVector(receiver,
                                                           txParams.m_txVector);
    acknowledgment->blockAckTxVector = acknowledgment->blockAckReqTxVector;
    acknowledgment->barType = m_mac->GetBarTypeAsOriginator(receiver, tid);
    acknowledgment->baType = m_mac->GetBaTypeAsOriginator(receiver, tid);
    acknowledgment->SetQosAckPolicy(receiver, tid, WifiMacHeader::BLOCK_ACK);
    return acknowledgment;
  }

  NS_LOG_DEBUG("A-MPDU using Implicit Block Ack Request policy or BlockAckReq, "
               "request Block Ack");
  auto acknowledgment = std::make_unique<WifiBlockAck>();
  acknowledgment->blockAckTxVector =
      GetWifiRemoteStationManager()->GetBlockAckTxVector(receiver,
                                                         txParams.m_txVector);
  acknowledgment->baType = m_mac->GetBaTypeAsOriginator(receiver, tid);
  acknowledgment->SetQosAckPolicy(receiver, tid, WifiMacHeader::NORMAL_ACK);
  return acknowledgment;
}

std::unique_ptr<WifiAcknowledgment>
WifiDefaultAckManager::TryAggregateMsdu(Ptr<const WifiMpdu> msdu,
                                        const WifiTxParameters &txParams) {
  NS_LOG_FUNCTION(this << *msdu << &txParams);

  return nullptr;
}

std::unique_ptr<WifiAcknowledgment>
WifiDefaultAckManager::GetAckInfoIfBarBaSequence(
    Ptr<const WifiMpdu> mpdu, const WifiTxParameters &txParams) {
  NS_LOG_FUNCTION(this << *mpdu << &txParams);
  NS_ASSERT(txParams.m_txVector.IsDlMu());
  NS_ASSERT(m_dlMuAckType == WifiAcknowledgment::DL_MU_BAR_BA_SEQUENCE);

  const WifiMacHeader &hdr = mpdu->GetHeader();
  Mac48Address receiver = hdr.GetAddr1();

  const WifiTxParameters::PsduInfo *psduInfo = txParams.GetPsduInfo(receiver);

  NS_ABORT_MSG_IF(
      !hdr.IsQosData(),
      "QoS data frames only can be aggregated when transmitting a "
      "DL MU PPDU acknowledged via a sequence of BAR and BA frames");
  uint8_t tid = hdr.GetQosTid();
  Ptr<QosTxop> edca = m_mac->GetQosTxop(QosUtilsMapTidToAc(tid));

  NS_ASSERT(!txParams.m_acknowledgment ||
            txParams.m_acknowledgment->method ==
                WifiAcknowledgment::DL_MU_BAR_BA_SEQUENCE);

  WifiDlMuBarBaSequence *acknowledgment = nullptr;
  if (txParams.m_acknowledgment) {
    acknowledgment =
        static_cast<WifiDlMuBarBaSequence *>(txParams.m_acknowledgment.get());
  }

  if (psduInfo) {
    NS_ASSERT(acknowledgment);

    if ((acknowledgment->stationsSendBlockAckReqTo.find(receiver) !=
         acknowledgment->stationsSendBlockAckReqTo.end()) ||
        (acknowledgment->stationsReplyingWithBlockAck.find(receiver) !=
         acknowledgment->stationsReplyingWithBlockAck.end())) {
      return nullptr;
    }

    NS_ASSERT(acknowledgment->stationsReplyingWithNormalAck.size() == 1 &&
              acknowledgment->stationsReplyingWithNormalAck.begin()->first ==
                  receiver);

    acknowledgment = new WifiDlMuBarBaSequence(*acknowledgment);
    acknowledgment->stationsReplyingWithNormalAck.clear();

    acknowledgment->stationsReplyingWithBlockAck.emplace(
        receiver, WifiDlMuBarBaSequence::BlockAckInfo{
                      GetWifiRemoteStationManager()->GetBlockAckTxVector(
                          receiver, txParams.m_txVector),
                      m_mac->GetBaTypeAsOriginator(receiver, tid)});
    return std::unique_ptr<WifiDlMuBarBaSequence>(acknowledgment);
  }

  auto htFem = DynamicCast<HtFrameExchangeManager>(
      m_mac->GetFrameExchangeManager(m_linkId));
  NS_ASSERT(htFem);
  if (auto bar = htFem->GetBar(QosUtilsMapTidToAc(tid), tid, receiver);
      bar || (acknowledgment &&
              (!acknowledgment->stationsReplyingWithNormalAck.empty() ||
               !acknowledgment->stationsReplyingWithBlockAck.empty()))) {
    if (acknowledgment) {
      acknowledgment = new WifiDlMuBarBaSequence(*acknowledgment);
    } else {
      acknowledgment = new WifiDlMuBarBaSequence;
    }

    NS_LOG_DEBUG("Adding STA "
                 << receiver
                 << " to the list of stations receiving a BlockAckReq");
    acknowledgment->stationsSendBlockAckReqTo.emplace(
        receiver, WifiDlMuBarBaSequence::BlockAckReqInfo{
                      GetWifiRemoteStationManager()->GetBlockAckTxVector(
                          receiver, txParams.m_txVector),
                      m_mac->GetBarTypeAsOriginator(receiver, tid),
                      GetWifiRemoteStationManager()->GetBlockAckTxVector(
                          receiver, txParams.m_txVector),
                      m_mac->GetBaTypeAsOriginator(receiver, tid)});

    acknowledgment->SetQosAckPolicy(receiver, tid, WifiMacHeader::BLOCK_ACK);
    return std::unique_ptr<WifiDlMuBarBaSequence>(acknowledgment);
  }

  if (acknowledgment) {
    acknowledgment = new WifiDlMuBarBaSequence(*acknowledgment);
  } else {
    acknowledgment = new WifiDlMuBarBaSequence;
  }

  NS_LOG_DEBUG(
      "Adding STA "
      << receiver
      << " as the station that will immediately reply with a Normal Ack");
  acknowledgment->stationsReplyingWithNormalAck.emplace(
      receiver, WifiDlMuBarBaSequence::AckInfo{
                    GetWifiRemoteStationManager()->GetAckTxVector(
                        receiver, txParams.m_txVector)});

  acknowledgment->SetQosAckPolicy(receiver, tid, WifiMacHeader::NORMAL_ACK);
  return std::unique_ptr<WifiDlMuBarBaSequence>(acknowledgment);
}

std::unique_ptr<WifiAcknowledgment>
WifiDefaultAckManager::GetAckInfoIfTfMuBar(Ptr<const WifiMpdu> mpdu,
                                           const WifiTxParameters &txParams) {
  NS_LOG_FUNCTION(this << *mpdu << &txParams);
  NS_ASSERT(txParams.m_txVector.IsDlMu());
  NS_ASSERT(m_dlMuAckType == WifiAcknowledgment::DL_MU_TF_MU_BAR);

  const WifiMacHeader &hdr = mpdu->GetHeader();
  Mac48Address receiver = hdr.GetAddr1();

  const WifiTxParameters::PsduInfo *psduInfo = txParams.GetPsduInfo(receiver);

  NS_ASSERT(!txParams.m_acknowledgment ||
            txParams.m_acknowledgment->method ==
                WifiAcknowledgment::DL_MU_TF_MU_BAR);

  WifiDlMuTfMuBar *acknowledgment = nullptr;
  if (txParams.m_acknowledgment) {
    acknowledgment =
        static_cast<WifiDlMuTfMuBar *>(txParams.m_acknowledgment.get());
  }

  if (!psduInfo) {
    Ptr<ApWifiMac> apMac = DynamicCast<ApWifiMac>(m_mac);
    NS_ABORT_MSG_IF(!apMac, "HE APs only can send DL MU PPDUs");
    uint16_t staId = apMac->GetAssociationId(receiver, m_linkId);

    NS_ABORT_MSG_IF(
        !hdr.IsQosData(),
        "QoS data frames only can be aggregated when transmitting a "
        "DL MU PPDU acknowledged via a MU-BAR sent as SU frame");
    uint8_t tid = hdr.GetQosTid();

    if (acknowledgment) {
      acknowledgment = new WifiDlMuTfMuBar(*acknowledgment);
    } else {
      acknowledgment = new WifiDlMuTfMuBar;
    }

    WifiTxVector blockAckTxVector;
    auto preamble = IsEht(txParams.m_txVector.GetPreambleType())
                        ? WIFI_PREAMBLE_EHT_TB
                        : WIFI_PREAMBLE_HE_TB;
    blockAckTxVector.SetPreambleType(preamble);
    blockAckTxVector.SetChannelWidth(txParams.m_txVector.GetChannelWidth());
    blockAckTxVector.SetGuardInterval(
        std::max<uint16_t>(txParams.m_txVector.GetGuardInterval(), 1600));
    const auto &userInfo = txParams.m_txVector.GetHeMuUserInfo(staId);
    blockAckTxVector.SetHeMuUserInfo(
        staId,
        {userInfo.ru, std::min(userInfo.mcs, m_maxMcsForBlockAckInTbPpdu),
         userInfo.nss});

    NS_LOG_DEBUG(
        "Adding STA "
        << receiver
        << " to the list of stations that will be solicited by the MU-BAR");
    Ptr<QosTxop> edca = m_mac->GetQosTxop(QosUtilsMapTidToAc(tid));
    acknowledgment->stationsReplyingWithBlockAck.emplace(
        receiver,
        WifiDlMuTfMuBar::BlockAckInfo{
            edca->GetBaManager()->GetBlockAckReqHeader(
                mpdu->GetOriginal()->GetHeader().GetAddr1(), tid),
            blockAckTxVector, m_mac->GetBaTypeAsOriginator(receiver, tid)});

    acknowledgment->barTypes.push_back(
        m_mac->GetBarTypeAsOriginator(receiver, tid));
    acknowledgment->muBarTxVector =
        GetWifiRemoteStationManager()->GetRtsTxVector(receiver);
    acknowledgment->SetQosAckPolicy(receiver, tid, WifiMacHeader::BLOCK_ACK);
    return std::unique_ptr<WifiDlMuTfMuBar>(acknowledgment);
  }

  NS_ASSERT(acknowledgment);
  NS_ABORT_MSG_IF(
      !hdr.IsQosData(),
      "QoS data frames only can be aggregated when transmitting a DL MU PPDU");

  return nullptr;
}

std::unique_ptr<WifiAcknowledgment>
WifiDefaultAckManager::GetAckInfoIfAggregatedMuBar(
    Ptr<const WifiMpdu> mpdu, const WifiTxParameters &txParams) {
  NS_LOG_FUNCTION(this << *mpdu << &txParams);
  NS_ASSERT(txParams.m_txVector.IsDlMu());
  NS_ASSERT(m_dlMuAckType == WifiAcknowledgment::DL_MU_AGGREGATE_TF);

  const WifiMacHeader &hdr = mpdu->GetHeader();
  Mac48Address receiver = hdr.GetAddr1();

  const WifiTxParameters::PsduInfo *psduInfo = txParams.GetPsduInfo(receiver);

  NS_ASSERT(!txParams.m_acknowledgment ||
            txParams.m_acknowledgment->method ==
                WifiAcknowledgment::DL_MU_AGGREGATE_TF);

  WifiDlMuAggregateTf *acknowledgment = nullptr;
  if (txParams.m_acknowledgment) {
    acknowledgment =
        static_cast<WifiDlMuAggregateTf *>(txParams.m_acknowledgment.get());
  }

  if (!psduInfo) {
    Ptr<ApWifiMac> apMac = DynamicCast<ApWifiMac>(m_mac);
    NS_ABORT_MSG_IF(!apMac, "HE APs only can send DL MU PPDUs");
    uint16_t staId = apMac->GetAssociationId(receiver, m_linkId);

    NS_ABORT_MSG_IF(
        !hdr.IsQosData(),
        "QoS data frames only can be aggregated when transmitting a "
        "DL MU PPDU acknowledged via a sequence of BAR and BA frames");
    uint8_t tid = hdr.GetQosTid();

    if (acknowledgment) {
      acknowledgment = new WifiDlMuAggregateTf(*acknowledgment);
    } else {
      acknowledgment = new WifiDlMuAggregateTf;
    }

    WifiTxVector blockAckTxVector;
    auto preamble = IsEht(txParams.m_txVector.GetPreambleType())
                        ? WIFI_PREAMBLE_EHT_TB
                        : WIFI_PREAMBLE_HE_TB;
    blockAckTxVector.SetPreambleType(preamble);
    blockAckTxVector.SetChannelWidth(txParams.m_txVector.GetChannelWidth());
    blockAckTxVector.SetGuardInterval(
        std::max<uint16_t>(txParams.m_txVector.GetGuardInterval(), 1600));
    const auto &userInfo = txParams.m_txVector.GetHeMuUserInfo(staId);
    blockAckTxVector.SetHeMuUserInfo(
        staId,
        {userInfo.ru, std::min(userInfo.mcs, m_maxMcsForBlockAckInTbPpdu),
         userInfo.nss});

    NS_LOG_DEBUG(
        "Adding STA "
        << receiver
        << " to the list of stations that will reply with a Block Ack");
    Ptr<QosTxop> edca = m_mac->GetQosTxop(QosUtilsMapTidToAc(tid));
    acknowledgment->stationsReplyingWithBlockAck.emplace(
        receiver,
        WifiDlMuAggregateTf::BlockAckInfo{
            GetMuBarSize({m_mac->GetBarTypeAsOriginator(receiver, tid)}),
            edca->GetBaManager()->GetBlockAckReqHeader(
                mpdu->GetOriginal()->GetHeader().GetAddr1(), tid),
            blockAckTxVector, m_mac->GetBaTypeAsOriginator(receiver, tid)});

    acknowledgment->SetQosAckPolicy(receiver, tid,
                                    WifiMacHeader::NO_EXPLICIT_ACK);
    return std::unique_ptr<WifiDlMuAggregateTf>(acknowledgment);
  }

  NS_ASSERT(acknowledgment);
  NS_ABORT_MSG_IF(!hdr.IsQosData(),
                  "QoS data and MU-BAR Trigger frames only can be aggregated "
                  "when transmitting a DL MU PPDU");

  return nullptr;
}

std::unique_ptr<WifiAcknowledgment>
WifiDefaultAckManager::TryUlMuTransmission(Ptr<const WifiMpdu> mpdu,
                                           const WifiTxParameters &txParams) {
  NS_LOG_FUNCTION(this << *mpdu << &txParams);
  NS_ASSERT(mpdu->GetHeader().IsTrigger());

  Ptr<ApWifiMac> apMac = DynamicCast<ApWifiMac>(m_mac);
  NS_ABORT_MSG_IF(!apMac, "HE APs only can send Trigger Frames");

  auto heFem = DynamicCast<HeFrameExchangeManager>(
      m_mac->GetFrameExchangeManager(m_linkId));
  NS_ABORT_MSG_IF(!heFem, "HE APs only can send Trigger Frames");

  CtrlTriggerHeader trigger;
  mpdu->GetPacket()->PeekHeader(trigger);

  if (trigger.IsBasic()) {
    auto acknowledgment = std::make_unique<WifiUlMuMultiStaBa>();

    for (const auto &userInfo : trigger) {
      uint16_t aid12 = userInfo.GetAid12();

      if (aid12 == NO_USER_STA_ID) {
        NS_LOG_INFO("Unallocated RU");
        continue;
      }
      NS_ABORT_MSG_IF(aid12 == 0 || aid12 > 2007,
                      "Allocation of RA-RUs is not supported");

      NS_ASSERT(apMac->GetStaList(m_linkId).find(aid12) !=
                apMac->GetStaList(m_linkId).end());
      Mac48Address staAddress = apMac->GetStaList(m_linkId).find(aid12)->second;

      uint8_t tid = 0;
      while (tid < 8 &&
             !m_mac->GetBaAgreementEstablishedAsRecipient(staAddress, tid)) {
        tid++;
      }
      NS_ASSERT_MSG(tid < 8,
                    "No Block Ack agreement established with originator "
                        << staAddress);

      std::size_t index = acknowledgment->baType.m_bitmapLen.size();
      acknowledgment->stationsReceivingMultiStaBa.emplace(
          std::make_pair(staAddress, tid), index);

      acknowledgment->baType.m_bitmapLen.push_back(
          m_mac->GetBaTypeAsRecipient(staAddress, tid).m_bitmapLen.at(0));
    }

    uint16_t staId = trigger.begin()->GetAid12();
    acknowledgment->tbPpduTxVector = trigger.GetHeTbTxVector(staId);
    acknowledgment->multiStaBaTxVector =
        GetWifiRemoteStationManager()->GetBlockAckTxVector(
            apMac->GetStaList(m_linkId).find(staId)->second,
            acknowledgment->tbPpduTxVector);
    return acknowledgment;
  } else if (trigger.IsBsrp()) {
    return std::make_unique<WifiNoAck>();
  }

  return nullptr;
}

} // namespace ns3
