
#include "eht-frame-exchange-manager.h"

#include "eht-phy.h"
#include "emlsr-manager.h"

#include "ns3/abort.h"
#include "ns3/ap-wifi-mac.h"
#include "ns3/log.h"
#include "ns3/sta-wifi-mac.h"
#include "ns3/wifi-mac-queue.h"

#undef NS_LOG_APPEND_CONTEXT
#define NS_LOG_APPEND_CONTEXT                                                  \
  std::clog << "[link=" << +m_linkId << "][mac=" << m_self << "] "

namespace ns3 {

static constexpr uint8_t RX_PHY_START_DELAY_USEC = 48;

NS_LOG_COMPONENT_DEFINE("EhtFrameExchangeManager");

NS_OBJECT_ENSURE_REGISTERED(EhtFrameExchangeManager);

TypeId EhtFrameExchangeManager::GetTypeId() {
  static TypeId tid = TypeId("ns3::EhtFrameExchangeManager")
                          .SetParent<HeFrameExchangeManager>()
                          .AddConstructor<EhtFrameExchangeManager>()
                          .SetGroupName("Wifi");
  return tid;
}

EhtFrameExchangeManager::EhtFrameExchangeManager() { NS_LOG_FUNCTION(this); }

EhtFrameExchangeManager::~EhtFrameExchangeManager() {
  NS_LOG_FUNCTION_NOARGS();
}

void EhtFrameExchangeManager::DoDispose() {
  NS_LOG_FUNCTION(this);
  m_ongoingTxopEnd.Cancel();
  HeFrameExchangeManager::DoDispose();
}

void EhtFrameExchangeManager::RxStartIndication(WifiTxVector txVector,
                                                Time psduDuration) {
  NS_LOG_FUNCTION(this << txVector << psduDuration.As(Time::MS));

  HeFrameExchangeManager::RxStartIndication(txVector, psduDuration);
  UpdateTxopEndOnRxStartIndication(psduDuration);
}

void EhtFrameExchangeManager::SetLinkId(uint8_t linkId) {
  if (auto protectionManager = GetProtectionManager()) {
    protectionManager->SetLinkId(linkId);
  }
  if (auto ackManager = GetAckManager()) {
    ackManager->SetLinkId(linkId);
  }
  m_msduAggregator->SetLinkId(linkId);
  m_mpduAggregator->SetLinkId(linkId);
  HeFrameExchangeManager::SetLinkId(linkId);
}

Ptr<WifiMpdu>
EhtFrameExchangeManager::CreateAliasIfNeeded(Ptr<WifiMpdu> mpdu) const {
  NS_LOG_FUNCTION(this << *mpdu);

  if (!mpdu->GetHeader().IsQosData() || m_mac->GetNLinks() == 1 ||
      mpdu->GetHeader().GetAddr1().IsGroup() ||
      !GetWifiRemoteStationManager()->GetMldAddress(
          mpdu->GetHeader().GetAddr1())) {
    return HeFrameExchangeManager::CreateAliasIfNeeded(mpdu);
  }

  mpdu = mpdu->CreateAlias(m_linkId);
  auto &hdr = mpdu->GetHeader();
  hdr.SetAddr2(GetAddress());
  auto address =
      GetWifiRemoteStationManager()->GetAffiliatedStaAddress(hdr.GetAddr1());
  NS_ASSERT(address);
  hdr.SetAddr1(*address);
  if (hdr.IsQosAmsdu()) {
    if (hdr.IsToDs() && !hdr.IsFromDs()) {
      hdr.SetAddr3(hdr.GetAddr1());
    } else if (!hdr.IsToDs() && hdr.IsFromDs()) {
      hdr.SetAddr3(hdr.GetAddr2());
    }
  }

  return mpdu;
}

bool EhtFrameExchangeManager::StartTransmission(Ptr<Txop> edca,
                                                uint16_t allowedWidth) {
  NS_LOG_FUNCTION(this << edca << allowedWidth);

  auto started = HeFrameExchangeManager::StartTransmission(edca, allowedWidth);

  if (started && m_staMac && m_staMac->IsEmlsrLink(m_linkId)) {
    NS_ASSERT(m_staMac->GetEmlsrManager());
    m_staMac->GetEmlsrManager()->NotifyUlTxopStart(m_linkId);
  }

  return started;
}

void EhtFrameExchangeManager::ForwardPsduDown(Ptr<const WifiPsdu> psdu,
                                              WifiTxVector &txVector) {
  NS_LOG_FUNCTION(this << psdu << txVector);

  if (txVector.GetPreambleType() == WIFI_PREAMBLE_EHT_MU) {
    auto phy = StaticCast<EhtPhy>(m_phy->GetPhyEntity(WIFI_MOD_CLASS_EHT));
    auto sigBMode = phy->GetSigBMode(txVector);
    txVector.SetSigBMode(sigBMode);
  }

  auto txDuration =
      WifiPhy::CalculateTxDuration(psdu, txVector, m_phy->GetPhyBand());

  HeFrameExchangeManager::ForwardPsduDown(psdu, txVector);
  UpdateTxopEndOnTxStart(txDuration);

  if (m_apMac) {
    for (auto clientIt = m_protectedStas.begin();
         clientIt != m_protectedStas.end();) {
      auto aid = GetWifiRemoteStationManager()->GetAssociationId(*clientIt);

      if (GetWifiRemoteStationManager()->GetEmlsrEnabled(*clientIt) &&
          GetEmlsrSwitchToListening(psdu, aid, *clientIt)) {
        EmlsrSwitchToListening(*clientIt, txDuration);
        clientIt = m_protectedStas.erase(clientIt);
      } else {
        clientIt++;
      }
    }
  }
}

void EhtFrameExchangeManager::ForwardPsduMapDown(WifiConstPsduMap psduMap,
                                                 WifiTxVector &txVector) {
  NS_LOG_FUNCTION(this << psduMap << txVector);

  auto txDuration =
      WifiPhy::CalculateTxDuration(psduMap, txVector, m_phy->GetPhyBand());

  HeFrameExchangeManager::ForwardPsduMapDown(psduMap, txVector);
  UpdateTxopEndOnTxStart(txDuration);

  if (m_apMac) {
    for (auto clientIt = m_protectedStas.begin();
         clientIt != m_protectedStas.end();) {
      auto aid = GetWifiRemoteStationManager()->GetAssociationId(*clientIt);

      if (auto psduMapIt = psduMap.find(aid);
          GetWifiRemoteStationManager()->GetEmlsrEnabled(*clientIt) &&
          (psduMapIt == psduMap.cend() ||
           GetEmlsrSwitchToListening(psduMapIt->second, aid, *clientIt))) {
        EmlsrSwitchToListening(*clientIt, txDuration);
        clientIt = m_protectedStas.erase(clientIt);
      } else {
        clientIt++;
      }
    }
  }
}

void EhtFrameExchangeManager::EmlsrSwitchToListening(
    const Mac48Address &address, const Time &delay) {
  NS_LOG_FUNCTION(this << address << delay.As(Time::US));

  auto mldAddress = GetWifiRemoteStationManager()->GetMldAddress(address);
  NS_ASSERT(mldAddress);
  auto emlCapabilities =
      GetWifiRemoteStationManager()->GetStationEmlCapabilities(address);
  NS_ASSERT(emlCapabilities);

  for (uint8_t linkId = 0; linkId < m_mac->GetNLinks(); linkId++) {
    if (m_mac->GetWifiRemoteStationManager(linkId)->GetEmlsrEnabled(
            *mldAddress)) {
      Simulator::Schedule(delay, [=]() {
        if (linkId != m_linkId) {
          m_mac->UnblockUnicastTxOnLinks(
              WifiQueueBlockedReason::USING_OTHER_EMLSR_LINK, *mldAddress,
              {linkId});
        }

        m_mac->BlockUnicastTxOnLinks(
            WifiQueueBlockedReason::WAITING_EMLSR_TRANSITION_DELAY, *mldAddress,
            {linkId});
      });

      Simulator::Schedule(
          delay + CommonInfoBasicMle::DecodeEmlsrTransitionDelay(
                      emlCapabilities->get().emlsrTransitionDelay),
          [=]() {
            m_mac->UnblockUnicastTxOnLinks(
                WifiQueueBlockedReason::WAITING_EMLSR_TRANSITION_DELAY,
                *mldAddress, {linkId});
          });
    }
  }
}

void EhtFrameExchangeManager::NotifySwitchingEmlsrLink(Ptr<WifiPhy> phy,
                                                       uint8_t linkId,
                                                       Time delay) {
  NS_LOG_FUNCTION(this << phy << linkId << delay.As(Time::US));

  NS_ABORT_MSG_IF(!m_staMac, "This method can only be called on a STA");

  if (phy == m_phy) {
    ResetPhy();
  }
  m_staMac->NotifySwitchingEmlsrLink(phy, linkId);
}

void EhtFrameExchangeManager::SendEmlOmn(const Mac48Address &dest,
                                         const MgtEmlOmn &frame) {
  NS_LOG_FUNCTION(this << dest << frame);

  WifiMacHeader hdr;
  hdr.SetType(WIFI_MAC_MGT_ACTION);
  hdr.SetAddr1(dest);
  hdr.SetAddr2(m_self);
  hdr.SetAddr3(m_bssid);
  hdr.SetDsNotTo();
  hdr.SetDsNotFrom();

  const auto sequence = m_txMiddle->GetNextSequenceNumberFor(&hdr);
  hdr.SetSequenceNumber(sequence);

  WifiActionHeader actionHdr;
  WifiActionHeader::ActionValue action;
  action.protectedEhtAction =
      WifiActionHeader::PROTECTED_EHT_EML_OPERATING_MODE_NOTIFICATION;
  actionHdr.SetAction(WifiActionHeader::PROTECTED_EHT, action);

  auto packet = Create<Packet>();
  packet->AddHeader(frame);
  packet->AddHeader(actionHdr);

  m_mac->GetQosTxop(AC_VO)->Queue(Create<WifiMpdu>(packet, hdr));
}

std::optional<double>
EhtFrameExchangeManager::GetMostRecentRssi(const Mac48Address &address) const {
  auto optRssi = HeFrameExchangeManager::GetMostRecentRssi(address);

  if (optRssi) {
    return optRssi;
  }

  auto mldAddress = GetWifiRemoteStationManager()->GetMldAddress(address);

  if (!mldAddress) {
    return std::nullopt;
  }

  for (uint8_t linkId = 0; linkId < m_mac->GetNLinks(); linkId++) {
    std::optional<Mac48Address> linkAddress;
    if (linkId != m_linkId &&
        (linkAddress = m_mac->GetWifiRemoteStationManager(linkId)
                           ->GetAffiliatedStaAddress(*mldAddress)) &&
        (optRssi =
             m_mac->GetWifiRemoteStationManager(linkId)->GetMostRecentRssi(
                 *linkAddress))) {
      return optRssi;
    }
  }

  return std::nullopt;
}

void EhtFrameExchangeManager::SendMuRts(const WifiTxParameters &txParams) {
  NS_LOG_FUNCTION(this << &txParams);

  uint8_t maxPaddingDelay = 0;

  for (const auto &address : m_sentRtsTo) {
    if (!GetWifiRemoteStationManager()->GetEmlsrEnabled(address)) {
      continue;
    }

    auto emlCapabilities =
        GetWifiRemoteStationManager()->GetStationEmlCapabilities(address);
    NS_ASSERT(emlCapabilities);
    maxPaddingDelay =
        std::max(maxPaddingDelay, emlCapabilities->get().emlsrPaddingDelay);

    auto mldAddress = GetWifiRemoteStationManager()->GetMldAddress(address);
    NS_ASSERT(mldAddress);

    for (uint8_t linkId = 0; linkId < m_apMac->GetNLinks(); linkId++) {
      if (linkId != m_linkId &&
          m_mac->GetWifiRemoteStationManager(linkId)->GetEmlsrEnabled(
              *mldAddress)) {
        m_mac->BlockUnicastTxOnLinks(
            WifiQueueBlockedReason::USING_OTHER_EMLSR_LINK, *mldAddress,
            {linkId});
      }
    }
  }

  if (maxPaddingDelay > 0) {
    NS_ASSERT(txParams.m_protection &&
              txParams.m_protection->method == WifiProtection::MU_RTS_CTS);
    auto protection =
        static_cast<WifiMuRtsCtsProtection *>(txParams.m_protection.get());
    NS_ASSERT(protection->muRts.IsMuRts());

    auto rate = protection->muRtsTxVector.GetMode().GetDataRate(
        protection->muRtsTxVector);
    std::size_t nDbps = rate / 1e6 * 4;
    protection->muRts.SetPaddingSize((1 << (maxPaddingDelay + 2)) * nDbps / 8);
  }

  HeFrameExchangeManager::SendMuRts(txParams);
}

bool EhtFrameExchangeManager::GetEmlsrSwitchToListening(
    Ptr<const WifiPsdu> psdu, uint16_t aid, const Mac48Address &address) const {
  NS_LOG_FUNCTION(this << psdu << aid << address);

  if (psdu->GetAddr1() == address) {
    return false;
  }

  for (const auto &mpdu : *PeekPointer(psdu)) {
    if (mpdu->GetHeader().IsTrigger()) {
      CtrlTriggerHeader trigger;
      mpdu->GetPacket()->PeekHeader(trigger);
      if (trigger.FindUserInfoWithAid(aid) != trigger.end()) {
        return false;
      }
    }
  }

  if (psdu->GetHeader(0).IsCts()) {
    if (m_apMac && psdu->GetAddr1() == m_self) {
      return false;
    }
    if (m_staMac && psdu->GetAddr1() == m_bssid) {
      return false;
    }
  }

  if (psdu->GetHeader(0).IsBlockAck()) {
    CtrlBAckResponseHeader blockAck;
    psdu->GetPayload(0)->PeekHeader(blockAck);
    if (blockAck.IsMultiSta() &&
        !blockAck.FindPerAidTidInfoWithAid(aid).empty()) {
      return false;
    }
  }

  return true;
}

void EhtFrameExchangeManager::TransmissionFailed() {
  NS_LOG_FUNCTION(this);

  for (const auto &address : m_txTimer.GetStasExpectedToRespond()) {
    if (GetWifiRemoteStationManager()->GetEmlsrEnabled(address)) {
      NS_LOG_DEBUG("EMLSR client " << address
                                   << " did not respond, continue TXOP");
      TransmissionSucceeded();
      return;
    }
  }

  HeFrameExchangeManager::TransmissionFailed();
}

void EhtFrameExchangeManager::NotifyChannelReleased(Ptr<Txop> txop) {
  NS_LOG_FUNCTION(this << txop);

  if (m_apMac) {
    auto delay = m_phy->GetSifs() + m_phy->GetSlot() +
                 MicroSeconds(RX_PHY_START_DELAY_USEC);
    for (const auto &address : m_protectedStas) {
      if (GetWifiRemoteStationManager()->GetEmlsrEnabled(address)) {
        EmlsrSwitchToListening(address, delay);
      }
    }
  } else if (m_staMac && m_staMac->IsEmlsrLink(m_linkId)) {
    NS_ASSERT(m_staMac->GetEmlsrManager());
    m_staMac->GetEmlsrManager()->NotifyTxopEnd(m_linkId);
  }

  HeFrameExchangeManager::NotifyChannelReleased(txop);
}

void EhtFrameExchangeManager::PostProcessFrame(Ptr<const WifiPsdu> psdu,
                                               const WifiTxVector &txVector) {
  NS_LOG_FUNCTION(this << psdu << txVector);

  HeFrameExchangeManager::PostProcessFrame(psdu, txVector);

  if (m_apMac && m_txopHolder == psdu->GetAddr2() &&
      GetWifiRemoteStationManager()->GetEmlsrEnabled(*m_txopHolder)) {
    if (!m_ongoingTxopEnd.IsRunning()) {
      auto delay = m_phy->GetSifs() + m_phy->GetSlot() +
                   MicroSeconds(RX_PHY_START_DELAY_USEC);
      NS_LOG_DEBUG(
          "Expected TXOP end=" << (Simulator::Now() + delay).As(Time::S));
      m_ongoingTxopEnd =
          Simulator::Schedule(delay, &EhtFrameExchangeManager::TxopEnd, this);

      auto mldAddress =
          GetWifiRemoteStationManager()->GetMldAddress(psdu->GetAddr2());
      NS_ASSERT(mldAddress);

      for (uint8_t linkId = 0; linkId < m_apMac->GetNLinks(); linkId++) {
        if (linkId != m_linkId &&
            m_mac->GetWifiRemoteStationManager(linkId)->GetEmlsrEnabled(
                *mldAddress)) {
          m_mac->BlockUnicastTxOnLinks(
              WifiQueueBlockedReason::USING_OTHER_EMLSR_LINK, *mldAddress,
              {linkId});
        }
      }
    } else {
      UpdateTxopEndOnRxEnd();
    }
  }

  if (m_staMac && m_ongoingTxopEnd.IsRunning()) {
    if (GetEmlsrSwitchToListening(psdu, m_staMac->GetAssociationId(), m_self)) {
      m_ongoingTxopEnd.Cancel();
      m_staMac->GetEmlsrManager()->NotifyTxopEnd(m_linkId);
    } else {
      UpdateTxopEndOnRxEnd();
    }
  }
}

void EhtFrameExchangeManager::ReceiveMpdu(Ptr<const WifiMpdu> mpdu,
                                          RxSignalInfo rxSignalInfo,
                                          const WifiTxVector &txVector,
                                          bool inAmpdu) {
  NS_ASSERT(mpdu->GetHeader().GetAddr1().IsGroup() ||
            mpdu->GetHeader().GetAddr1() == m_self);

  const auto &hdr = mpdu->GetHeader();

  if (hdr.IsTrigger()) {
    if (!m_staMac) {
      return;
    }

    CtrlTriggerHeader trigger;
    mpdu->GetPacket()->PeekHeader(trigger);

    if (hdr.GetAddr1() != m_self &&
        (!hdr.GetAddr1().IsBroadcast() || !m_staMac->IsAssociated() ||
         hdr.GetAddr2() != m_bssid ||
         trigger.FindUserInfoWithAid(m_staMac->GetAssociationId()) ==
             trigger.end())) {
      return;
    }

    if (trigger.IsMuRts() && m_staMac->IsEmlsrLink(m_linkId)) {
      auto apAddress = GetWifiRemoteStationManager()->GetMldAddress(m_bssid);
      NS_ASSERT_MSG(apAddress, "MLD address not found for BSSID " << m_bssid);
      WifiContainerQueueId queueId(WIFI_QOSDATA_QUEUE, WIFI_UNICAST, *apAddress,
                                   0);
      if (auto mask = m_staMac->GetMacQueueScheduler()->GetQueueLinkMask(
              AC_BE, queueId, m_linkId);
          mask && mask->test(static_cast<std::size_t>(
                      WifiQueueBlockedReason::USING_OTHER_EMLSR_LINK))) {
        NS_LOG_DEBUG("Drop ICF because another EMLSR link is being used");
        return;
      }

      NS_ASSERT(m_staMac->GetEmlsrManager());
      Simulator::ScheduleNow(&EmlsrManager::NotifyIcfReceived,
                             m_staMac->GetEmlsrManager(), m_linkId);
      m_ongoingTxopEnd.Cancel();
      NS_LOG_DEBUG("Expected TXOP end="
                   << (Simulator::Now() + m_phy->GetSifs()).As(Time::S));
      m_ongoingTxopEnd =
          Simulator::Schedule(m_phy->GetSifs() + NanoSeconds(1),
                              &EhtFrameExchangeManager::TxopEnd, this);
    }
  }

  HeFrameExchangeManager::ReceiveMpdu(mpdu, rxSignalInfo, txVector, inAmpdu);
}

void EhtFrameExchangeManager::TxopEnd() {
  NS_LOG_FUNCTION(this);

  if (m_staMac && m_staMac->IsEmlsrLink(m_linkId)) {
    m_staMac->GetEmlsrManager()->NotifyTxopEnd(m_linkId);
  } else if (m_apMac && m_txopHolder &&
             GetWifiRemoteStationManager()->GetEmlsrEnabled(*m_txopHolder)) {
    EmlsrSwitchToListening(*m_txopHolder, Seconds(0));
  }
}

void EhtFrameExchangeManager::UpdateTxopEndOnTxStart(Time txDuration) {
  NS_LOG_FUNCTION(this << txDuration.As(Time::MS));

  if (!m_ongoingTxopEnd.IsRunning()) {
    return;
  }

  m_ongoingTxopEnd.Cancel();
  Time delay;

  if (m_txTimer.IsRunning()) {
    delay = m_txTimer.GetDelayLeft();
  } else {
    delay = txDuration + m_phy->GetSifs() + m_phy->GetSlot() +
            MicroSeconds(RX_PHY_START_DELAY_USEC);
  }

  NS_LOG_DEBUG("Expected TXOP end=" << (Simulator::Now() + delay).As(Time::S));
  m_ongoingTxopEnd =
      Simulator::Schedule(delay, &EhtFrameExchangeManager::TxopEnd, this);
}

void EhtFrameExchangeManager::UpdateTxopEndOnRxStartIndication(
    Time psduDuration) {
  NS_LOG_FUNCTION(this << psduDuration.As(Time::MS));

  if (!m_ongoingTxopEnd.IsRunning() || !psduDuration.IsStrictlyPositive()) {
    return;
  }

  m_ongoingTxopEnd.Cancel();

  NS_LOG_DEBUG(
      "Expected TXOP end=" << (Simulator::Now() + psduDuration).As(Time::S));
  m_ongoingTxopEnd = Simulator::Schedule(
      psduDuration + NanoSeconds(1), &EhtFrameExchangeManager::TxopEnd, this);
}

void EhtFrameExchangeManager::UpdateTxopEndOnRxEnd() {
  NS_LOG_FUNCTION(this);

  if (!m_ongoingTxopEnd.IsRunning()) {
    return;
  }

  m_ongoingTxopEnd.Cancel();

  auto delay = m_phy->GetSifs() + m_phy->GetSlot() +
               MicroSeconds(RX_PHY_START_DELAY_USEC);
  NS_LOG_DEBUG("Expected TXOP end=" << (Simulator::Now() + delay).As(Time::S));
  m_ongoingTxopEnd =
      Simulator::Schedule(delay, &EhtFrameExchangeManager::TxopEnd, this);
}

} // namespace ns3
