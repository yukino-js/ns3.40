
#include "qos-frame-exchange-manager.h"

#include "ap-wifi-mac.h"
#include "wifi-mac-queue.h"
#include "wifi-mac-trailer.h"

#include "ns3/abort.h"
#include "ns3/log.h"

#undef NS_LOG_APPEND_CONTEXT
#define NS_LOG_APPEND_CONTEXT                                                  \
  std::clog << "[link=" << +m_linkId << "][mac=" << m_self << "] "

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("QosFrameExchangeManager");

NS_OBJECT_ENSURE_REGISTERED(QosFrameExchangeManager);

TypeId QosFrameExchangeManager::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::QosFrameExchangeManager")
          .SetParent<FrameExchangeManager>()
          .AddConstructor<QosFrameExchangeManager>()
          .SetGroupName("Wifi")
          .AddAttribute(
              "PifsRecovery",
              "Perform a PIFS recovery as a response to transmission failure "
              "within a TXOP",
              BooleanValue(true),
              MakeBooleanAccessor(&QosFrameExchangeManager::m_pifsRecovery),
              MakeBooleanChecker())
          .AddAttribute(
              "SetQueueSize",
              "Whether to set the Queue Size subfield of the QoS Control field "
              "of QoS data frames sent by non-AP stations",
              BooleanValue(false),
              MakeBooleanAccessor(&QosFrameExchangeManager::m_setQosQueueSize),
              MakeBooleanChecker());
  return tid;
}

QosFrameExchangeManager::QosFrameExchangeManager() : m_initialFrame(false) {
  NS_LOG_FUNCTION(this);
}

QosFrameExchangeManager::~QosFrameExchangeManager() {
  NS_LOG_FUNCTION_NOARGS();
}

void QosFrameExchangeManager::DoDispose() {
  NS_LOG_FUNCTION(this);
  m_edca = nullptr;
  m_edcaBackingOff = nullptr;
  m_pifsRecoveryEvent.Cancel();
  FrameExchangeManager::DoDispose();
}

bool QosFrameExchangeManager::SendCfEndIfNeeded() {
  NS_LOG_FUNCTION(this);
  NS_ASSERT(m_edca);
  NS_ASSERT(m_edca->GetTxopLimit(m_linkId).IsStrictlyPositive());

  WifiMacHeader cfEnd;
  cfEnd.SetType(WIFI_MAC_CTL_END);
  cfEnd.SetDsNotFrom();
  cfEnd.SetDsNotTo();
  cfEnd.SetNoRetry();
  cfEnd.SetNoMoreFragments();
  cfEnd.SetDuration(Seconds(0));
  cfEnd.SetAddr1(Mac48Address::GetBroadcast());
  cfEnd.SetAddr2(m_self);

  WifiTxVector cfEndTxVector =
      GetWifiRemoteStationManager()->GetRtsTxVector(cfEnd.GetAddr1());

  auto mpdu = Create<WifiMpdu>(Create<Packet>(), cfEnd);
  auto txDuration = m_phy->CalculateTxDuration(mpdu->GetSize(), cfEndTxVector,
                                               m_phy->GetPhyBand());

  if (m_edca->GetRemainingTxop(m_linkId) > txDuration) {
    NS_LOG_DEBUG("Send CF-End frame");
    ForwardMpduDown(mpdu, cfEndTxVector);
    Simulator::Schedule(txDuration,
                        &QosFrameExchangeManager::NotifyChannelReleased, this,
                        m_edca);
    return true;
  }

  NotifyChannelReleased(m_edca);
  m_edca = nullptr;
  return false;
}

void QosFrameExchangeManager::PifsRecovery() {
  NS_LOG_FUNCTION(this);
  NS_ASSERT(m_edca);
  NS_ASSERT(m_edca->IsTxopStarted(m_linkId));

  if (m_channelAccessManager->GetAccessGrantStart() - m_phy->GetSifs() >
      Simulator::Now() - m_phy->GetPifs()) {
    NotifyChannelReleased(m_edca);
    m_edca = nullptr;
  } else {
    StartTransmission(m_edca, Seconds(0));
  }
}

void QosFrameExchangeManager::CancelPifsRecovery() {
  NS_LOG_FUNCTION(this);
  NS_ASSERT(m_pifsRecoveryEvent.IsRunning());
  NS_ASSERT(m_edca);

  NS_LOG_DEBUG("Cancel PIFS recovery being attempted by EDCAF " << m_edca);
  m_pifsRecoveryEvent.Cancel();
  NotifyChannelReleased(m_edca);
}

bool QosFrameExchangeManager::StartTransmission(Ptr<Txop> edca,
                                                uint16_t allowedWidth) {
  NS_LOG_FUNCTION(this << edca << allowedWidth);

  if (m_pifsRecoveryEvent.IsRunning()) {
    CancelPifsRecovery();
  }

  if (!edca->IsQosTxop()) {
    m_edca = nullptr;
    return FrameExchangeManager::StartTransmission(edca, allowedWidth);
  }

  m_allowedWidth = allowedWidth;
  auto qosTxop = StaticCast<QosTxop>(edca);
  return StartTransmission(qosTxop, qosTxop->GetTxopLimit(m_linkId));
}

bool QosFrameExchangeManager::StartTransmission(Ptr<QosTxop> edca,
                                                Time txopDuration) {
  NS_LOG_FUNCTION(this << edca << txopDuration);

  if (m_pifsRecoveryEvent.IsRunning()) {
    CancelPifsRecovery();
  }

  if (m_txTimer.IsRunning()) {
    m_txTimer.Cancel();
  }
  m_dcf = edca;
  m_edca = edca;

  bool backingOff = (m_edcaBackingOff == m_edca);

  if (backingOff) {
    NS_ASSERT(m_edca->GetTxopLimit(m_linkId).IsStrictlyPositive());
    NS_ASSERT(m_edca->IsTxopStarted(m_linkId));
    NS_ASSERT(!m_pifsRecovery);
    NS_ASSERT(!m_initialFrame);

    m_edcaBackingOff = nullptr;
  }

  if (m_edca->GetTxopLimit(m_linkId).IsStrictlyPositive()) {
    if (!m_edca->IsTxopStarted(m_linkId) ||
        (backingOff && m_edca->GetRemainingTxop(m_linkId).IsZero())) {
      m_edca->NotifyChannelAccessed(m_linkId, txopDuration);

      if (StartFrameExchange(m_edca, txopDuration, true)) {
        m_initialFrame = true;
        return true;
      }

      NS_LOG_DEBUG("No frame transmitted");
      NotifyChannelReleased(m_edca);
      m_edca = nullptr;
      return false;
    }

    NS_ASSERT(!m_initialFrame);

    if (!StartFrameExchange(m_edca, m_edca->GetRemainingTxop(m_linkId),
                            false)) {
      NS_LOG_DEBUG("Not enough remaining TXOP time");
      return SendCfEndIfNeeded();
    }

    return true;
  }

  m_initialFrame = true;

  if (StartFrameExchange(m_edca, Time::Min(), true)) {
    m_edca->NotifyChannelAccessed(m_linkId, Seconds(0));
    return true;
  }

  NS_LOG_DEBUG("No frame transmitted");
  NotifyChannelReleased(m_edca);
  m_edca = nullptr;
  return false;
}

bool QosFrameExchangeManager::StartFrameExchange(Ptr<QosTxop> edca,
                                                 Time availableTime,
                                                 bool initialFrame) {
  NS_LOG_FUNCTION(this << edca << availableTime << initialFrame);

  Ptr<WifiMpdu> mpdu = edca->PeekNextMpdu(m_linkId);

  if (!mpdu) {
    NS_LOG_DEBUG("Queue empty");
    return false;
  }

  mpdu = CreateAliasIfNeeded(mpdu);
  WifiTxParameters txParams;
  txParams.m_txVector = GetWifiRemoteStationManager()->GetDataTxVector(
      mpdu->GetHeader(), m_allowedWidth);

  Ptr<WifiMpdu> item =
      edca->GetNextMpdu(m_linkId, mpdu, txParams, availableTime, initialFrame);

  if (!item) {
    NS_LOG_DEBUG("Not enough time to transmit a frame");
    return false;
  }

  NS_ASSERT_MSG(!item->GetHeader().IsQosData() ||
                    !item->GetHeader().IsQosAmsdu(),
                "We should not get an A-MSDU here");

  item = GetFirstFragmentIfNeeded(item);

  if (item->IsFragment() && item->GetSize() != mpdu->GetSize()) {
    WifiTxParameters fragmentTxParams;
    fragmentTxParams.m_txVector = txParams.m_txVector;
    txParams.m_protection =
        GetProtectionManager()->TryAddMpdu(item, fragmentTxParams);
    NS_ASSERT(txParams.m_protection);
  }

  SendMpduWithProtection(item, txParams);

  return true;
}

Ptr<WifiMpdu>
QosFrameExchangeManager::CreateAliasIfNeeded(Ptr<WifiMpdu> mpdu) const {
  return mpdu;
}

bool QosFrameExchangeManager::TryAddMpdu(Ptr<const WifiMpdu> mpdu,
                                         WifiTxParameters &txParams,
                                         Time availableTime) const {
  NS_ASSERT(mpdu);
  NS_LOG_FUNCTION(this << *mpdu << &txParams << availableTime);

  Time protectionTime = Time::Min();
  if (txParams.m_protection) {
    protectionTime = txParams.m_protection->protectionTime;
  }

  std::unique_ptr<WifiProtection> protection;
  protection = GetProtectionManager()->TryAddMpdu(mpdu, txParams);
  bool protectionSwapped = false;

  if (protection) {
    CalculateProtectionTime(protection.get());
    protectionTime = protection->protectionTime;
    txParams.m_protection.swap(protection);
    protectionSwapped = true;
  }
  NS_ASSERT(protectionTime != Time::Min());
  NS_LOG_DEBUG("protection time=" << protectionTime);

  Time acknowledgmentTime = Time::Min();
  if (txParams.m_acknowledgment) {
    acknowledgmentTime = txParams.m_acknowledgment->acknowledgmentTime;
  }

  std::unique_ptr<WifiAcknowledgment> acknowledgment;
  acknowledgment = GetAckManager()->TryAddMpdu(mpdu, txParams);
  bool acknowledgmentSwapped = false;

  if (acknowledgment) {
    CalculateAcknowledgmentTime(acknowledgment.get());
    acknowledgmentTime = acknowledgment->acknowledgmentTime;
    txParams.m_acknowledgment.swap(acknowledgment);
    acknowledgmentSwapped = true;
  }
  NS_ASSERT(acknowledgmentTime != Time::Min());
  NS_LOG_DEBUG("acknowledgment time=" << acknowledgmentTime);

  Time ppduDurationLimit = Time::Min();
  if (availableTime != Time::Min()) {
    ppduDurationLimit = availableTime - protectionTime - acknowledgmentTime;
  }

  if (!IsWithinLimitsIfAddMpdu(mpdu, txParams, ppduDurationLimit)) {
    if (protectionSwapped) {
      txParams.m_protection.swap(protection);
    }
    if (acknowledgmentSwapped) {
      txParams.m_acknowledgment.swap(acknowledgment);
    }
    return false;
  }

  txParams.AddMpdu(mpdu);
  UpdateTxDuration(mpdu->GetHeader().GetAddr1(), txParams);

  return true;
}

bool QosFrameExchangeManager::IsWithinLimitsIfAddMpdu(
    Ptr<const WifiMpdu> mpdu, const WifiTxParameters &txParams,
    Time ppduDurationLimit) const {
  NS_ASSERT(mpdu);
  NS_LOG_FUNCTION(this << *mpdu << &txParams << ppduDurationLimit);

  return IsWithinSizeAndTimeLimits(mpdu->GetSize(),
                                   mpdu->GetHeader().GetAddr1(), txParams,
                                   ppduDurationLimit);
}

bool QosFrameExchangeManager::IsWithinSizeAndTimeLimits(
    uint32_t ppduPayloadSize, Mac48Address receiver,
    const WifiTxParameters &txParams, Time ppduDurationLimit) const {
  NS_LOG_FUNCTION(this << ppduPayloadSize << receiver << &txParams
                       << ppduDurationLimit);

  if (ppduDurationLimit != Time::Min() && ppduDurationLimit.IsNegative()) {
    NS_LOG_DEBUG("ppduDurationLimit is null or negative, time limit is "
                 "trivially exceeded");
    return false;
  }

  if (ppduPayloadSize >
      WifiPhy::GetMaxPsduSize(txParams.m_txVector.GetModulationClass())) {
    NS_LOG_DEBUG("the frame exceeds the max PSDU size");
    return false;
  }

  Time maxPpduDuration = GetPpduMaxTime(txParams.m_txVector.GetPreambleType());

  Time txTime = GetTxDuration(ppduPayloadSize, receiver, txParams);
  NS_LOG_DEBUG("PPDU duration: " << txTime.As(Time::MS));

  if ((ppduDurationLimit.IsStrictlyPositive() && txTime > ppduDurationLimit) ||
      (maxPpduDuration.IsStrictlyPositive() && txTime > maxPpduDuration)) {
    NS_LOG_DEBUG("the frame does not meet the constraint on max PPDU duration "
                 "or PPDU duration limit");
    return false;
  }

  return true;
}

Time QosFrameExchangeManager::GetFrameDurationId(
    const WifiMacHeader &header, uint32_t size,
    const WifiTxParameters &txParams, Ptr<Packet> fragmentedPacket) const {
  NS_LOG_FUNCTION(this << header << size << &txParams << fragmentedPacket);

  if (!m_edca) {
    return FrameExchangeManager::GetFrameDurationId(header, size, txParams,
                                                    fragmentedPacket);
  }

  if (m_edca->GetTxopLimit(m_linkId).IsZero()) {
    return FrameExchangeManager::GetFrameDurationId(header, size, txParams,
                                                    fragmentedPacket);
  }

  NS_ASSERT(txParams.m_acknowledgment &&
            txParams.m_acknowledgment->acknowledgmentTime != Time::Min());

  return std::max(m_edca->GetRemainingTxop(m_linkId) -
                      m_phy->CalculateTxDuration(size, txParams.m_txVector,
                                                 m_phy->GetPhyBand()),
                  txParams.m_acknowledgment->acknowledgmentTime);
}

Time QosFrameExchangeManager::GetRtsDurationId(const WifiTxVector &rtsTxVector,
                                               Time txDuration,
                                               Time response) const {
  NS_LOG_FUNCTION(this << rtsTxVector << txDuration << response);

  if (!m_edca) {
    return FrameExchangeManager::GetRtsDurationId(rtsTxVector, txDuration,
                                                  response);
  }

  if (m_edca->GetTxopLimit(m_linkId).IsZero()) {
    return FrameExchangeManager::GetRtsDurationId(rtsTxVector, txDuration,
                                                  response);
  }

  return std::max(m_edca->GetRemainingTxop(m_linkId) -
                      m_phy->CalculateTxDuration(GetRtsSize(), rtsTxVector,
                                                 m_phy->GetPhyBand()),
                  Seconds(0));
}

Time QosFrameExchangeManager::GetCtsToSelfDurationId(
    const WifiTxVector &ctsTxVector, Time txDuration, Time response) const {
  NS_LOG_FUNCTION(this << ctsTxVector << txDuration << response);

  if (!m_edca) {
    return FrameExchangeManager::GetCtsToSelfDurationId(ctsTxVector, txDuration,
                                                        response);
  }

  if (m_edca->GetTxopLimit(m_linkId).IsZero()) {
    return FrameExchangeManager::GetCtsToSelfDurationId(ctsTxVector, txDuration,
                                                        response);
  }

  return std::max(m_edca->GetRemainingTxop(m_linkId) -
                      m_phy->CalculateTxDuration(GetCtsSize(), ctsTxVector,
                                                 m_phy->GetPhyBand()),
                  Seconds(0));
}

void QosFrameExchangeManager::ForwardMpduDown(Ptr<WifiMpdu> mpdu,
                                              WifiTxVector &txVector) {
  NS_LOG_FUNCTION(this << *mpdu << txVector);

  WifiMacHeader &hdr = mpdu->GetHeader();

  if (hdr.IsQosData() && m_mac->GetTypeOfStation() == STA &&
      (m_setQosQueueSize || hdr.IsQosEosp())) {
    uint8_t tid = hdr.GetQosTid();
    hdr.SetQosEosp();
    hdr.SetQosQueueSize(
        m_mac->GetQosTxop(tid)->GetQosQueueSize(tid, hdr.GetAddr1()));
  }
  FrameExchangeManager::ForwardMpduDown(mpdu, txVector);
}

void QosFrameExchangeManager::TransmissionSucceeded() {
  NS_LOG_DEBUG(this);

  if (!m_edca) {
    FrameExchangeManager::TransmissionSucceeded();
    return;
  }

  if (m_edca->GetTxopLimit(m_linkId).IsStrictlyPositive() &&
      m_edca->GetRemainingTxop(m_linkId) > m_phy->GetSifs()) {
    NS_LOG_DEBUG("Schedule another transmission in a SIFS");
    bool (QosFrameExchangeManager::*fp)(Ptr<QosTxop>, Time) =
        &QosFrameExchangeManager::StartTransmission;

    Simulator::Schedule(m_phy->GetSifs(), fp, this, m_edca, Seconds(0));
  } else {
    NotifyChannelReleased(m_edca);
    m_edca = nullptr;
  }
  m_initialFrame = false;
}

void QosFrameExchangeManager::TransmissionFailed() {
  NS_LOG_FUNCTION(this);

  if (!m_edca) {
    FrameExchangeManager::TransmissionFailed();
    return;
  }

  if (m_initialFrame) {
    NS_LOG_DEBUG("TX of the initial frame of a TXOP failed: terminate TXOP");
    NotifyChannelReleased(m_edca);
    m_edca = nullptr;
  } else {
    NS_ASSERT_MSG(m_edca->GetTxopLimit(m_linkId).IsStrictlyPositive(),
                  "Cannot transmit more than one frame if TXOP Limit is zero");

    if (m_pifsRecovery) {
      NS_LOG_DEBUG(
          "TX of a non-initial frame of a TXOP failed: perform PIFS recovery");
      NS_ASSERT(!m_pifsRecoveryEvent.IsRunning());
      m_pifsRecoveryEvent = Simulator::Schedule(
          m_phy->GetPifs(), &QosFrameExchangeManager::PifsRecovery, this);
    } else {
      NS_LOG_DEBUG(
          "TX of a non-initial frame of a TXOP failed: invoke backoff");
      m_edca->Txop::NotifyChannelReleased(m_linkId);
      m_edcaBackingOff = m_edca;
      m_edca = nullptr;
    }
  }
  m_initialFrame = false;
}

void QosFrameExchangeManager::PreProcessFrame(Ptr<const WifiPsdu> psdu,
                                              const WifiTxVector &txVector) {
  NS_LOG_FUNCTION(this << psdu << txVector);

  if (m_mac->GetTypeOfStation() == AP && psdu->GetAddr1() == m_self) {
    for (const auto &mpdu : *PeekPointer(psdu)) {
      const WifiMacHeader &hdr = mpdu->GetHeader();

      if (hdr.IsQosData() && hdr.IsQosEosp()) {
        NS_LOG_DEBUG("Station " << hdr.GetAddr2()
                                << " reported a buffer status of "
                                << +hdr.GetQosQueueSize()
                                << " for tid=" << +hdr.GetQosTid());
        StaticCast<ApWifiMac>(m_mac)->SetBufferStatus(
            hdr.GetQosTid(), mpdu->GetOriginal()->GetHeader().GetAddr2(),
            hdr.GetQosQueueSize());
      }
    }
  }

  ClearTxopHolderIfNeeded();

  FrameExchangeManager::PreProcessFrame(psdu, txVector);
}

void QosFrameExchangeManager::PostProcessFrame(Ptr<const WifiPsdu> psdu,
                                               const WifiTxVector &txVector) {
  NS_LOG_FUNCTION(this << psdu << txVector);

  SetTxopHolder(psdu, txVector);
  FrameExchangeManager::PostProcessFrame(psdu, txVector);
}

void QosFrameExchangeManager::SetTxopHolder(Ptr<const WifiPsdu> psdu,
                                            const WifiTxVector &txVector) {
  NS_LOG_FUNCTION(this << psdu << txVector);

  const WifiMacHeader &hdr = psdu->GetHeader(0);

  if ((hdr.IsQosData() || hdr.IsMgt() || hdr.IsRts()) &&
      (hdr.GetAddr1() == m_bssid || hdr.GetAddr2() == m_bssid)) {
    m_txopHolder = psdu->GetAddr2();
  } else if (hdr.IsCts() && hdr.GetAddr1() == m_bssid) {
    m_txopHolder = psdu->GetAddr1();
  }
}

void QosFrameExchangeManager::ClearTxopHolderIfNeeded() {
  NS_LOG_FUNCTION(this);
  if (m_navEnd <= Simulator::Now()) {
    m_txopHolder.reset();
  }
}

void QosFrameExchangeManager::UpdateNav(Ptr<const WifiPsdu> psdu,
                                        const WifiTxVector &txVector) {
  NS_LOG_FUNCTION(this << psdu << txVector);
  if (psdu->GetHeader(0).IsCfEnd()) {
    NS_LOG_DEBUG("Received CF-End, resetting NAV");
    NavResetTimeout();
    return;
  }

  FrameExchangeManager::UpdateNav(psdu, txVector);
}

void QosFrameExchangeManager::NavResetTimeout() {
  NS_LOG_FUNCTION(this);
  FrameExchangeManager::NavResetTimeout();
  ClearTxopHolderIfNeeded();
}

void QosFrameExchangeManager::ReceiveMpdu(Ptr<const WifiMpdu> mpdu,
                                          RxSignalInfo rxSignalInfo,
                                          const WifiTxVector &txVector,
                                          bool inAmpdu) {
  NS_ASSERT(mpdu->GetHeader().GetAddr1().IsGroup() ||
            mpdu->GetHeader().GetAddr1() == m_self);

  double rxSnr = rxSignalInfo.snr;
  const WifiMacHeader &hdr = mpdu->GetHeader();

  if (hdr.IsRts()) {
    NS_ABORT_MSG_IF(inAmpdu, "Received RTS as part of an A-MPDU");

    if (hdr.GetAddr2() == m_txopHolder || VirtualCsMediumIdle()) {
      NS_LOG_DEBUG("Received RTS from=" << hdr.GetAddr2() << ", schedule CTS");
      Simulator::Schedule(m_phy->GetSifs(),
                          &QosFrameExchangeManager::SendCtsAfterRts, this, hdr,
                          txVector.GetMode(), rxSnr);
    } else {
      NS_LOG_DEBUG("Received RTS from=" << hdr.GetAddr2()
                                        << ", cannot schedule CTS");
    }
    return;
  }

  if (hdr.IsQosData()) {
    if (hdr.GetAddr1() == m_self &&
        hdr.GetQosAckPolicy() == WifiMacHeader::NORMAL_ACK) {
      NS_LOG_DEBUG("Received " << hdr.GetTypeString() << " from="
                               << hdr.GetAddr2() << ", schedule ACK");
      Simulator::Schedule(m_phy->GetSifs(),
                          &QosFrameExchangeManager::SendNormalAck, this, hdr,
                          txVector, rxSnr);
    }

    m_rxMiddle->Receive(mpdu, m_linkId);

    return;
  }

  return FrameExchangeManager::ReceiveMpdu(mpdu, rxSignalInfo, txVector,
                                           inAmpdu);
}

} // namespace ns3
