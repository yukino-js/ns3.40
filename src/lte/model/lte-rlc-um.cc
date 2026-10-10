
#include "lte-rlc-um.h"

#include "lte-rlc-header.h"
#include "lte-rlc-sdu-status-tag.h"
#include "lte-rlc-tag.h"

#include "ns3/log.h"
#include "ns3/simulator.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("LteRlcUm");

NS_OBJECT_ENSURE_REGISTERED(LteRlcUm);

LteRlcUm::LteRlcUm()
    : m_maxTxBufferSize(10 * 1024), m_txBufferSize(0), m_sequenceNumber(0),
      m_vrUr(0), m_vrUx(0), m_vrUh(0), m_windowSize(512),
      m_expectedSeqNumber(0) {
  NS_LOG_FUNCTION(this);
  m_reassemblingState = WAITING_S0_FULL;
}

LteRlcUm::~LteRlcUm() { NS_LOG_FUNCTION(this); }

TypeId LteRlcUm::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::LteRlcUm")
          .SetParent<LteRlc>()
          .SetGroupName("Lte")
          .AddConstructor<LteRlcUm>()
          .AddAttribute("MaxTxBufferSize",
                        "Maximum Size of the Transmission Buffer (in Bytes)",
                        UintegerValue(10 * 1024),
                        MakeUintegerAccessor(&LteRlcUm::m_maxTxBufferSize),
                        MakeUintegerChecker<uint32_t>())
          .AddAttribute("ReorderingTimer",
                        "Value of the t-Reordering timer (See section 7.3 of "
                        "3GPP TS 36.322)",
                        TimeValue(MilliSeconds(100)),
                        MakeTimeAccessor(&LteRlcUm::m_reorderingTimerValue),
                        MakeTimeChecker())
          .AddAttribute("EnablePdcpDiscarding",
                        "Whether to use the PDCP discarding, i.e., perform "
                        "discarding at the moment "
                        "of passing the PDCP SDU to RLC)",
                        BooleanValue(true),
                        MakeBooleanAccessor(&LteRlcUm::m_enablePdcpDiscarding),
                        MakeBooleanChecker())
          .AddAttribute(
              "DiscardTimerMs",
              "Discard timer in milliseconds to be used to discard packets. "
              "If set to 0 then packet delay budget will be used as the "
              "discard "
              "timer value, otherwise it will be used this value.",
              UintegerValue(0),
              MakeUintegerAccessor(&LteRlcUm::m_discardTimerMs),
              MakeUintegerChecker<uint32_t>());
  return tid;
}

void LteRlcUm::DoDispose() {
  NS_LOG_FUNCTION(this);
  m_reorderingTimer.Cancel();
  m_rbsTimer.Cancel();

  LteRlc::DoDispose();
}

void LteRlcUm::DoTransmitPdcpPdu(Ptr<Packet> p) {
  NS_LOG_FUNCTION(this << m_rnti << (uint32_t)m_lcid << p->GetSize());
  if (m_txBufferSize + p->GetSize() <= m_maxTxBufferSize) {
    if (m_enablePdcpDiscarding) {
      uint32_t headOfLineDelayInMs = 0;
      uint32_t discardTimerMs =
          (m_discardTimerMs > 0) ? m_discardTimerMs : m_packetDelayBudgetMs;

      if (!m_txBuffer.empty()) {
        headOfLineDelayInMs =
            (Simulator::Now() - m_txBuffer.begin()->m_waitingSince)
                .GetMilliSeconds();
      }
      NS_LOG_DEBUG("head of line delay in MS:" << headOfLineDelayInMs);
      if (headOfLineDelayInMs > discardTimerMs) {
        NS_LOG_INFO(
            "Tx HOL is higher than this packet can allow. RLC SDU discarded");
        NS_LOG_DEBUG("headOfLineDelayInMs    = " << headOfLineDelayInMs);
        NS_LOG_DEBUG("m_packetDelayBudgetMs    = " << m_packetDelayBudgetMs);
        NS_LOG_DEBUG("packet size     = " << p->GetSize());
        m_txDropTrace(p);
      }
    }

    LteRlcSduStatusTag tag;
    tag.SetStatus(LteRlcSduStatusTag::FULL_SDU);
    p->AddPacketTag(tag);
    NS_LOG_INFO("Adding RLC SDU to Tx Buffer after adding LteRlcSduStatusTag: "
                "FULL_SDU");
    m_txBuffer.emplace_back(p, Simulator::Now());
    m_txBufferSize += p->GetSize();
    NS_LOG_LOGIC("NumOfBuffers = " << m_txBuffer.size());
    NS_LOG_LOGIC("txBufferSize = " << m_txBufferSize);
  } else {
    NS_LOG_INFO("Tx Buffer is full. RLC SDU discarded");
    NS_LOG_LOGIC("MaxTxBufferSize = " << m_maxTxBufferSize);
    NS_LOG_LOGIC("txBufferSize    = " << m_txBufferSize);
    NS_LOG_LOGIC("packet size     = " << p->GetSize());
    m_txDropTrace(p);
  }

  DoReportBufferStatus();
  m_rbsTimer.Cancel();
}

void LteRlcUm::DoNotifyTxOpportunity(
    LteMacSapUser::TxOpportunityParameters txOpParams) {
  NS_LOG_FUNCTION(this << m_rnti << (uint32_t)m_lcid << txOpParams.bytes);
  NS_LOG_INFO("RLC layer is preparing data for the following Tx opportunity of "
              << txOpParams.bytes << " bytes for RNTI=" << m_rnti
              << ", LCID=" << (uint32_t)m_lcid
              << ", CCID=" << (uint32_t)txOpParams.componentCarrierId
              << ", HARQ ID=" << (uint32_t)txOpParams.harqId
              << ", MIMO Layer=" << (uint32_t)txOpParams.layer);

  if (txOpParams.bytes <= 2) {
    NS_LOG_INFO("TX opportunity too small - Only " << txOpParams.bytes
                                                   << " bytes");
    return;
  }

  Ptr<Packet> packet = Create<Packet>();
  LteRlcHeader rlcHeader;

  uint32_t nextSegmentSize = txOpParams.bytes - 2;
  uint32_t nextSegmentId = 1;
  uint32_t dataFieldAddedSize = 0;
  std::vector<Ptr<Packet>> dataField;

  if (m_txBuffer.empty()) {
    NS_LOG_LOGIC("No data pending");
    return;
  }

  Ptr<Packet> firstSegment = m_txBuffer.begin()->m_pdu->Copy();
  Time firstSegmentTime = m_txBuffer.begin()->m_waitingSince;

  NS_LOG_LOGIC("SDUs in TxBuffer  = " << m_txBuffer.size());
  NS_LOG_LOGIC("First SDU buffer  = " << firstSegment);
  NS_LOG_LOGIC("First SDU size    = " << firstSegment->GetSize());
  NS_LOG_LOGIC("Next segment size = " << nextSegmentSize);
  NS_LOG_LOGIC("Remove SDU from TxBuffer");
  m_txBufferSize -= firstSegment->GetSize();
  NS_LOG_LOGIC("txBufferSize      = " << m_txBufferSize);
  m_txBuffer.erase(m_txBuffer.begin());

  while (firstSegment && (firstSegment->GetSize() > 0) &&
         (nextSegmentSize > 0)) {
    NS_LOG_LOGIC("WHILE ( firstSegment && firstSegment->GetSize > 0 && "
                 "nextSegmentSize > 0 )");
    NS_LOG_LOGIC("    firstSegment size = " << firstSegment->GetSize());
    NS_LOG_LOGIC("    nextSegmentSize   = " << nextSegmentSize);
    if ((firstSegment->GetSize() > nextSegmentSize) ||
        (firstSegment->GetSize() > 2047)) {
      uint32_t currSegmentSize =
          std::min(firstSegment->GetSize(), nextSegmentSize);

      NS_LOG_LOGIC("    IF ( firstSegment > nextSegmentSize ||");
      NS_LOG_LOGIC("         firstSegment > 2047 )");

      Ptr<Packet> newSegment = firstSegment->CreateFragment(0, currSegmentSize);
      NS_LOG_LOGIC("    newSegment size   = " << newSegment->GetSize());

      LteRlcSduStatusTag oldTag;
      LteRlcSduStatusTag newTag;
      firstSegment->RemovePacketTag(oldTag);
      newSegment->RemovePacketTag(newTag);
      if (oldTag.GetStatus() == LteRlcSduStatusTag::FULL_SDU) {
        newTag.SetStatus(LteRlcSduStatusTag::FIRST_SEGMENT);
        oldTag.SetStatus(LteRlcSduStatusTag::LAST_SEGMENT);
      } else if (oldTag.GetStatus() == LteRlcSduStatusTag::LAST_SEGMENT) {
        newTag.SetStatus(LteRlcSduStatusTag::MIDDLE_SEGMENT);
      }

      firstSegment->RemoveAtStart(currSegmentSize);
      NS_LOG_LOGIC("    firstSegment size (after RemoveAtStart) = "
                   << firstSegment->GetSize());
      if (firstSegment->GetSize() > 0) {
        firstSegment->AddPacketTag(oldTag);

        m_txBuffer.insert(m_txBuffer.begin(),
                          TxPdu(firstSegment, firstSegmentTime));
        m_txBufferSize += m_txBuffer.begin()->m_pdu->GetSize();

        NS_LOG_LOGIC("    TX buffer: Give back the remaining segment");
        NS_LOG_LOGIC("    TX buffers = " << m_txBuffer.size());
        NS_LOG_LOGIC(
            "    Front buffer size = " << m_txBuffer.begin()->m_pdu->GetSize());
        NS_LOG_LOGIC("    txBufferSize = " << m_txBufferSize);
      } else {
        if (newTag.GetStatus() == LteRlcSduStatusTag::FIRST_SEGMENT) {
          newTag.SetStatus(LteRlcSduStatusTag::FULL_SDU);
        } else if (newTag.GetStatus() == LteRlcSduStatusTag::MIDDLE_SEGMENT) {
          newTag.SetStatus(LteRlcSduStatusTag::LAST_SEGMENT);
        }
      }
      firstSegment = nullptr;

      newSegment->AddPacketTag(newTag);

      dataFieldAddedSize = newSegment->GetSize();
      dataField.push_back(newSegment);
      newSegment = nullptr;

      rlcHeader.PushExtensionBit(LteRlcHeader::DATA_FIELD_FOLLOWS);

      nextSegmentSize -= dataFieldAddedSize;
      nextSegmentId++;

    } else if ((nextSegmentSize - firstSegment->GetSize() <= 2) ||
               m_txBuffer.empty()) {
      NS_LOG_LOGIC("    IF nextSegmentSize - firstSegment->GetSize () <= 2 || "
                   "txBuffer.size == 0");
      dataFieldAddedSize = firstSegment->GetSize();
      dataField.push_back(firstSegment);
      firstSegment = nullptr;

      rlcHeader.PushExtensionBit(LteRlcHeader::DATA_FIELD_FOLLOWS);

      nextSegmentSize -= dataFieldAddedSize;
      nextSegmentId++;

      NS_LOG_LOGIC("        SDUs in TxBuffer  = " << m_txBuffer.size());
      if (!m_txBuffer.empty()) {
        NS_LOG_LOGIC(
            "        First SDU buffer  = " << m_txBuffer.begin()->m_pdu);
        NS_LOG_LOGIC("        First SDU size    = "
                     << m_txBuffer.begin()->m_pdu->GetSize());
      }
      NS_LOG_LOGIC("        Next segment size = " << nextSegmentSize);

    } else {
      NS_LOG_LOGIC(
          "    IF firstSegment < NextSegmentSize && txBuffer.size > 0");
      dataFieldAddedSize = firstSegment->GetSize();
      dataField.push_back(firstSegment);

      rlcHeader.PushExtensionBit(LteRlcHeader::E_LI_FIELDS_FOLLOWS);

      rlcHeader.PushLengthIndicator(firstSegment->GetSize());

      nextSegmentSize -= ((nextSegmentId % 2) ? (2) : (1)) + dataFieldAddedSize;
      nextSegmentId++;

      NS_LOG_LOGIC("        SDUs in TxBuffer  = " << m_txBuffer.size());
      if (!m_txBuffer.empty()) {
        NS_LOG_LOGIC(
            "        First SDU buffer  = " << m_txBuffer.begin()->m_pdu);
        NS_LOG_LOGIC("        First SDU size    = "
                     << m_txBuffer.begin()->m_pdu->GetSize());
      }
      NS_LOG_LOGIC("        Next segment size = " << nextSegmentSize);
      NS_LOG_LOGIC("        Remove SDU from TxBuffer");

      firstSegment = m_txBuffer.begin()->m_pdu->Copy();
      firstSegmentTime = m_txBuffer.begin()->m_waitingSince;
      m_txBufferSize -= firstSegment->GetSize();
      m_txBuffer.erase(m_txBuffer.begin());
      NS_LOG_LOGIC("        txBufferSize = " << m_txBufferSize);
    }
  }

  rlcHeader.SetSequenceNumber(m_sequenceNumber++);

  auto it = dataField.begin();

  uint8_t framingInfo = 0;

  LteRlcSduStatusTag tag;
  NS_ASSERT_MSG((*it)->PeekPacketTag(tag), "LteRlcSduStatusTag is missing");
  (*it)->PeekPacketTag(tag);
  if ((tag.GetStatus() == LteRlcSduStatusTag::FULL_SDU) ||
      (tag.GetStatus() == LteRlcSduStatusTag::FIRST_SEGMENT)) {
    framingInfo |= LteRlcHeader::FIRST_BYTE;
  } else {
    framingInfo |= LteRlcHeader::NO_FIRST_BYTE;
  }

  while (it < dataField.end()) {
    NS_LOG_LOGIC("Adding SDU/segment to packet, length = " << (*it)->GetSize());

    NS_ASSERT_MSG((*it)->PeekPacketTag(tag), "LteRlcSduStatusTag is missing");
    (*it)->RemovePacketTag(tag);
    if (packet->GetSize() > 0) {
      packet->AddAtEnd(*it);
    } else {
      packet = (*it);
    }
    it++;
  }

  it--;
  if ((tag.GetStatus() == LteRlcSduStatusTag::FULL_SDU) ||
      (tag.GetStatus() == LteRlcSduStatusTag::LAST_SEGMENT)) {
    framingInfo |= LteRlcHeader::LAST_BYTE;
  } else {
    framingInfo |= LteRlcHeader::NO_LAST_BYTE;
  }

  rlcHeader.SetFramingInfo(framingInfo);

  NS_LOG_LOGIC("RLC header: " << rlcHeader);
  packet->AddHeader(rlcHeader);

  RlcTag rlcTag(Simulator::Now());
  packet->AddByteTag(rlcTag, 1, rlcHeader.GetSerializedSize());
  m_txPdu(m_rnti, m_lcid, packet->GetSize());

  LteMacSapProvider::TransmitPduParameters params;
  params.pdu = packet;
  params.rnti = m_rnti;
  params.lcid = m_lcid;
  params.layer = txOpParams.layer;
  params.harqProcessId = txOpParams.harqId;
  params.componentCarrierId = txOpParams.componentCarrierId;

  NS_LOG_INFO("Forward RLC PDU to MAC Layer");
  m_macSapProvider->TransmitPdu(params);

  if (!m_txBuffer.empty()) {
    m_rbsTimer.Cancel();
    m_rbsTimer =
        Simulator::Schedule(MilliSeconds(10), &LteRlcUm::ExpireRbsTimer, this);
  }
}

void LteRlcUm::DoNotifyHarqDeliveryFailure() { NS_LOG_FUNCTION(this); }

void LteRlcUm::DoReceivePdu(LteMacSapUser::ReceivePduParameters rxPduParams) {
  NS_LOG_FUNCTION(this << m_rnti << (uint32_t)m_lcid
                       << rxPduParams.p->GetSize());

  RlcTag rlcTag;
  Time delay;

  bool ret = rxPduParams.p->FindFirstMatchingByteTag(rlcTag);
  NS_ASSERT_MSG(ret, "RlcTag is missing");

  delay = Simulator::Now() - rlcTag.GetSenderTimestamp();
  m_rxPdu(m_rnti, m_lcid, rxPduParams.p->GetSize(), delay.GetNanoSeconds());

  LteRlcHeader rlcHeader;
  rxPduParams.p->PeekHeader(rlcHeader);
  NS_LOG_LOGIC("RLC header: " << rlcHeader);
  SequenceNumber10 seqNumber = rlcHeader.GetSequenceNumber();

  NS_LOG_LOGIC("VR(UR) = " << m_vrUr);
  NS_LOG_LOGIC("VR(UX) = " << m_vrUx);
  NS_LOG_LOGIC("VR(UH) = " << m_vrUh);
  NS_LOG_LOGIC("SN = " << seqNumber);

  m_vrUr.SetModulusBase(m_vrUh - m_windowSize);
  m_vrUh.SetModulusBase(m_vrUh - m_windowSize);
  seqNumber.SetModulusBase(m_vrUh - m_windowSize);

  if (((m_vrUr < seqNumber) && (seqNumber < m_vrUh) &&
       (m_rxBuffer.count(seqNumber.GetValue()) > 0)) ||
      (((m_vrUh - m_windowSize) <= seqNumber) && (seqNumber < m_vrUr))) {
    NS_LOG_LOGIC("PDU discarded");
    rxPduParams.p = nullptr;
    return;
  } else {
    NS_LOG_LOGIC("Place PDU in the reception buffer");
    m_rxBuffer[seqNumber.GetValue()] = rxPduParams.p;
  }

  if (!IsInsideReorderingWindow(seqNumber)) {
    NS_LOG_LOGIC("SN is outside the reordering window");

    m_vrUh = seqNumber + 1;
    NS_LOG_LOGIC("New VR(UH) = " << m_vrUh);

    ReassembleOutsideWindow();

    if (!IsInsideReorderingWindow(m_vrUr)) {
      m_vrUr = m_vrUh - m_windowSize;
      NS_LOG_LOGIC("VR(UR) is outside the reordering window");
      NS_LOG_LOGIC("New VR(UR) = " << m_vrUr);
    }
  }

  if (m_rxBuffer.count(m_vrUr.GetValue()) > 0) {
    NS_LOG_LOGIC("Reception buffer contains SN = " << m_vrUr);

    uint16_t newVrUr;
    SequenceNumber10 oldVrUr = m_vrUr;

    auto it = m_rxBuffer.find(m_vrUr.GetValue());
    newVrUr = (it->first) + 1;
    while (m_rxBuffer.count(newVrUr) > 0) {
      newVrUr++;
    }
    m_vrUr = newVrUr;
    NS_LOG_LOGIC("New VR(UR) = " << m_vrUr);

    ReassembleSnInterval(oldVrUr, m_vrUr);
  }

  m_vrUr.SetModulusBase(m_vrUh - m_windowSize);
  m_vrUx.SetModulusBase(m_vrUh - m_windowSize);
  m_vrUh.SetModulusBase(m_vrUh - m_windowSize);

  if (m_reorderingTimer.IsRunning()) {
    NS_LOG_LOGIC("Reordering timer is running");

    if ((m_vrUx <= m_vrUr) ||
        ((!IsInsideReorderingWindow(m_vrUx)) && (m_vrUx != m_vrUh))) {
      NS_LOG_LOGIC("Stop reordering timer");
      m_reorderingTimer.Cancel();
    }
  }

  if (!m_reorderingTimer.IsRunning()) {
    NS_LOG_LOGIC("Reordering timer is not running");

    if (m_vrUh > m_vrUr) {
      NS_LOG_LOGIC("VR(UH) > VR(UR)");
      NS_LOG_LOGIC("Start reordering timer");
      m_reorderingTimer = Simulator::Schedule(
          m_reorderingTimerValue, &LteRlcUm::ExpireReorderingTimer, this);
      m_vrUx = m_vrUh;
      NS_LOG_LOGIC("New VR(UX) = " << m_vrUx);
    }
  }
}

bool LteRlcUm::IsInsideReorderingWindow(SequenceNumber10 seqNumber) {
  NS_LOG_FUNCTION(this << seqNumber);
  NS_LOG_LOGIC("Reordering Window: " << m_vrUh << " - " << m_windowSize
                                     << " <= " << seqNumber << " < " << m_vrUh);

  m_vrUh.SetModulusBase(m_vrUh - m_windowSize);
  seqNumber.SetModulusBase(m_vrUh - m_windowSize);

  if (((m_vrUh - m_windowSize) <= seqNumber) && (seqNumber < m_vrUh)) {
    NS_LOG_LOGIC(seqNumber << " is INSIDE the reordering window");
    return true;
  } else {
    NS_LOG_LOGIC(seqNumber << " is OUTSIDE the reordering window");
    return false;
  }
}

void LteRlcUm::ReassembleAndDeliver(Ptr<Packet> packet) {
  LteRlcHeader rlcHeader;
  packet->RemoveHeader(rlcHeader);
  uint8_t framingInfo = rlcHeader.GetFramingInfo();
  SequenceNumber10 currSeqNumber = rlcHeader.GetSequenceNumber();
  bool expectedSnLost;

  if (currSeqNumber != m_expectedSeqNumber) {
    expectedSnLost = true;
    NS_LOG_LOGIC("There are losses. Expected SN = "
                 << m_expectedSeqNumber << ". Current SN = " << currSeqNumber);
    m_expectedSeqNumber = currSeqNumber + 1;
  } else {
    expectedSnLost = false;
    NS_LOG_LOGIC("No losses. Expected SN = "
                 << m_expectedSeqNumber << ". Current SN = " << currSeqNumber);
    m_expectedSeqNumber++;
  }

  uint8_t extensionBit;
  uint16_t lengthIndicator;
  do {
    extensionBit = rlcHeader.PopExtensionBit();
    NS_LOG_LOGIC("E = " << (uint16_t)extensionBit);

    if (extensionBit == 0) {
      m_sdusBuffer.push_back(packet);
    } else {
      lengthIndicator = rlcHeader.PopLengthIndicator();
      NS_LOG_LOGIC("LI = " << lengthIndicator);

      if (lengthIndicator >= packet->GetSize()) {
        NS_LOG_LOGIC("INTERNAL ERROR: Not enough data in the packet ("
                     << packet->GetSize()
                     << "). Needed LI=" << lengthIndicator);
      }

      Ptr<Packet> data_field = packet->CreateFragment(0, lengthIndicator);
      packet->RemoveAtStart(lengthIndicator);

      m_sdusBuffer.push_back(data_field);
    }
  } while (extensionBit == 1);

  if (m_reassemblingState == WAITING_S0_FULL) {
    NS_LOG_LOGIC("Reassembling State = 'WAITING_S0_FULL'");
  } else if (m_reassemblingState == WAITING_SI_SF) {
    NS_LOG_LOGIC("Reassembling State = 'WAITING_SI_SF'");
  } else {
    NS_LOG_LOGIC("Reassembling State = Unknown state");
  }

  NS_LOG_LOGIC("Framing Info = " << (uint16_t)framingInfo);

  if (!expectedSnLost) {
    switch (m_reassemblingState) {
    case WAITING_S0_FULL:
      switch (framingInfo) {
      case (LteRlcHeader::FIRST_BYTE | LteRlcHeader::LAST_BYTE):
        m_reassemblingState = WAITING_S0_FULL;

        for (auto it = m_sdusBuffer.begin(); it != m_sdusBuffer.end(); it++) {
          m_rlcSapUser->ReceivePdcpPdu(*it);
        }
        m_sdusBuffer.clear();
        break;

      case (LteRlcHeader::FIRST_BYTE | LteRlcHeader::NO_LAST_BYTE):
        m_reassemblingState = WAITING_SI_SF;

        while (m_sdusBuffer.size() > 1) {
          m_rlcSapUser->ReceivePdcpPdu(m_sdusBuffer.front());
          m_sdusBuffer.pop_front();
        }

        m_keepS0 = m_sdusBuffer.front();
        m_sdusBuffer.pop_front();
        break;

      case (LteRlcHeader::NO_FIRST_BYTE | LteRlcHeader::LAST_BYTE):
        m_reassemblingState = WAITING_S0_FULL;

        m_sdusBuffer.pop_front();

        while (!m_sdusBuffer.empty()) {
          m_rlcSapUser->ReceivePdcpPdu(m_sdusBuffer.front());
          m_sdusBuffer.pop_front();
        }
        break;

      case (LteRlcHeader::NO_FIRST_BYTE | LteRlcHeader::NO_LAST_BYTE):
        if (m_sdusBuffer.size() == 1) {
          m_reassemblingState = WAITING_S0_FULL;
        } else {
          m_reassemblingState = WAITING_SI_SF;
        }

        m_sdusBuffer.pop_front();

        if (!m_sdusBuffer.empty()) {
          while (m_sdusBuffer.size() > 1) {
            m_rlcSapUser->ReceivePdcpPdu(m_sdusBuffer.front());
            m_sdusBuffer.pop_front();
          }

          m_keepS0 = m_sdusBuffer.front();
          m_sdusBuffer.pop_front();
        }
        break;

      default:
        NS_LOG_LOGIC("INTERNAL ERROR: Transition not possible. FI = "
                     << (uint32_t)framingInfo);
        break;
      }
      break;

    case WAITING_SI_SF:
      switch (framingInfo) {
      case (LteRlcHeader::NO_FIRST_BYTE | LteRlcHeader::LAST_BYTE):
        m_reassemblingState = WAITING_S0_FULL;

        m_keepS0->AddAtEnd(m_sdusBuffer.front());
        m_sdusBuffer.pop_front();
        m_rlcSapUser->ReceivePdcpPdu(m_keepS0);

        while (!m_sdusBuffer.empty()) {
          m_rlcSapUser->ReceivePdcpPdu(m_sdusBuffer.front());
          m_sdusBuffer.pop_front();
        }
        break;

      case (LteRlcHeader::NO_FIRST_BYTE | LteRlcHeader::NO_LAST_BYTE):
        m_reassemblingState = WAITING_SI_SF;

        if (m_sdusBuffer.size() == 1) {
          m_keepS0->AddAtEnd(m_sdusBuffer.front());
          m_sdusBuffer.pop_front();
        } else {
          m_keepS0->AddAtEnd(m_sdusBuffer.front());
          m_sdusBuffer.pop_front();
          m_rlcSapUser->ReceivePdcpPdu(m_keepS0);

          while (m_sdusBuffer.size() > 1) {
            m_rlcSapUser->ReceivePdcpPdu(m_sdusBuffer.front());
            m_sdusBuffer.pop_front();
          }

          m_keepS0 = m_sdusBuffer.front();
          m_sdusBuffer.pop_front();
        }
        break;

      case (LteRlcHeader::FIRST_BYTE | LteRlcHeader::LAST_BYTE):
      case (LteRlcHeader::FIRST_BYTE | LteRlcHeader::NO_LAST_BYTE):
      default:
        NS_LOG_LOGIC("INTERNAL ERROR: Transition not possible. FI = "
                     << (uint32_t)framingInfo);
        break;
      }
      break;

    default:
      NS_LOG_LOGIC("INTERNAL ERROR: Wrong reassembling state = "
                   << (uint32_t)m_reassemblingState);
      break;
    }
  } else {
    switch (m_reassemblingState) {
    case WAITING_S0_FULL:
      switch (framingInfo) {
      case (LteRlcHeader::FIRST_BYTE | LteRlcHeader::LAST_BYTE):
        m_reassemblingState = WAITING_S0_FULL;

        for (auto it = m_sdusBuffer.begin(); it != m_sdusBuffer.end(); it++) {
          m_rlcSapUser->ReceivePdcpPdu(*it);
        }
        m_sdusBuffer.clear();
        break;

      case (LteRlcHeader::FIRST_BYTE | LteRlcHeader::NO_LAST_BYTE):
        m_reassemblingState = WAITING_SI_SF;

        while (m_sdusBuffer.size() > 1) {
          m_rlcSapUser->ReceivePdcpPdu(m_sdusBuffer.front());
          m_sdusBuffer.pop_front();
        }

        m_keepS0 = m_sdusBuffer.front();
        m_sdusBuffer.pop_front();
        break;

      case (LteRlcHeader::NO_FIRST_BYTE | LteRlcHeader::LAST_BYTE):
        m_reassemblingState = WAITING_S0_FULL;

        m_sdusBuffer.pop_front();

        while (!m_sdusBuffer.empty()) {
          m_rlcSapUser->ReceivePdcpPdu(m_sdusBuffer.front());
          m_sdusBuffer.pop_front();
        }
        break;

      case (LteRlcHeader::NO_FIRST_BYTE | LteRlcHeader::NO_LAST_BYTE):
        if (m_sdusBuffer.size() == 1) {
          m_reassemblingState = WAITING_S0_FULL;
        } else {
          m_reassemblingState = WAITING_SI_SF;
        }

        m_sdusBuffer.pop_front();

        if (!m_sdusBuffer.empty()) {
          while (m_sdusBuffer.size() > 1) {
            m_rlcSapUser->ReceivePdcpPdu(m_sdusBuffer.front());
            m_sdusBuffer.pop_front();
          }

          m_keepS0 = m_sdusBuffer.front();
          m_sdusBuffer.pop_front();
        }
        break;

      default:
        NS_LOG_LOGIC("INTERNAL ERROR: Transition not possible. FI = "
                     << (uint32_t)framingInfo);
        break;
      }
      break;

    case WAITING_SI_SF:
      switch (framingInfo) {
      case (LteRlcHeader::FIRST_BYTE | LteRlcHeader::LAST_BYTE):
        m_reassemblingState = WAITING_S0_FULL;

        m_keepS0 = nullptr;

        while (!m_sdusBuffer.empty()) {
          m_rlcSapUser->ReceivePdcpPdu(m_sdusBuffer.front());
          m_sdusBuffer.pop_front();
        }
        break;

      case (LteRlcHeader::FIRST_BYTE | LteRlcHeader::NO_LAST_BYTE):
        m_reassemblingState = WAITING_SI_SF;

        m_keepS0 = nullptr;

        while (m_sdusBuffer.size() > 1) {
          m_rlcSapUser->ReceivePdcpPdu(m_sdusBuffer.front());
          m_sdusBuffer.pop_front();
        }

        m_keepS0 = m_sdusBuffer.front();
        m_sdusBuffer.pop_front();

        break;

      case (LteRlcHeader::NO_FIRST_BYTE | LteRlcHeader::LAST_BYTE):
        m_reassemblingState = WAITING_S0_FULL;

        m_keepS0 = nullptr;

        m_sdusBuffer.pop_front();

        while (!m_sdusBuffer.empty()) {
          m_rlcSapUser->ReceivePdcpPdu(m_sdusBuffer.front());
          m_sdusBuffer.pop_front();
        }
        break;

      case (LteRlcHeader::NO_FIRST_BYTE | LteRlcHeader::NO_LAST_BYTE):
        if (m_sdusBuffer.size() == 1) {
          m_reassemblingState = WAITING_S0_FULL;
        } else {
          m_reassemblingState = WAITING_SI_SF;
        }

        m_keepS0 = nullptr;

        m_sdusBuffer.pop_front();

        if (!m_sdusBuffer.empty()) {
          while (m_sdusBuffer.size() > 1) {
            m_rlcSapUser->ReceivePdcpPdu(m_sdusBuffer.front());
            m_sdusBuffer.pop_front();
          }

          m_keepS0 = m_sdusBuffer.front();
          m_sdusBuffer.pop_front();
        }
        break;

      default:
        NS_LOG_LOGIC("INTERNAL ERROR: Transition not possible. FI = "
                     << (uint32_t)framingInfo);
        break;
      }
      break;

    default:
      NS_LOG_LOGIC("INTERNAL ERROR: Wrong reassembling state = "
                   << (uint32_t)m_reassemblingState);
      break;
    }
  }
}

void LteRlcUm::ReassembleOutsideWindow() {
  NS_LOG_LOGIC("Reassemble Outside Window");

  auto it = m_rxBuffer.begin();

  while ((it != m_rxBuffer.end()) &&
         !IsInsideReorderingWindow(SequenceNumber10(it->first))) {
    NS_LOG_LOGIC("SN = " << it->first);

    ReassembleAndDeliver(it->second);

    auto it_tmp = it;
    ++it;
    m_rxBuffer.erase(it_tmp);
  }

  if (it != m_rxBuffer.end()) {
    NS_LOG_LOGIC("(SN = " << it->first << ") is inside the reordering window");
  }
}

void LteRlcUm::ReassembleSnInterval(SequenceNumber10 lowSeqNumber,
                                    SequenceNumber10 highSeqNumber) {
  NS_LOG_LOGIC("Reassemble SN between " << lowSeqNumber << " and "
                                        << highSeqNumber);

  SequenceNumber10 reassembleSn = lowSeqNumber;
  NS_LOG_LOGIC("reassembleSN = " << reassembleSn);
  NS_LOG_LOGIC("highSeqNumber = " << highSeqNumber);
  while (reassembleSn < highSeqNumber) {
    NS_LOG_LOGIC("reassembleSn < highSeqNumber");
    auto it = m_rxBuffer.find(reassembleSn.GetValue());
    NS_LOG_LOGIC("it->first  = " << it->first);
    NS_LOG_LOGIC("it->second = " << it->second);
    if (it != m_rxBuffer.end()) {
      NS_LOG_LOGIC("SN = " << it->first);

      ReassembleAndDeliver(it->second);

      m_rxBuffer.erase(it);
    }

    reassembleSn++;
  }
}

void LteRlcUm::DoReportBufferStatus() {
  Time holDelay(0);
  uint32_t queueSize = 0;

  if (!m_txBuffer.empty()) {
    holDelay = Simulator::Now() - m_txBuffer.front().m_waitingSince;

    queueSize = m_txBufferSize + 2 * m_txBuffer.size();
  }

  LteMacSapProvider::ReportBufferStatusParameters r;
  r.rnti = m_rnti;
  r.lcid = m_lcid;
  r.txQueueSize = queueSize;
  r.txQueueHolDelay = holDelay.GetMilliSeconds();
  r.retxQueueSize = 0;
  r.retxQueueHolDelay = 0;
  r.statusPduSize = 0;

  NS_LOG_LOGIC("Send ReportBufferStatus = " << r.txQueueSize << ", "
                                            << r.txQueueHolDelay);
  m_macSapProvider->ReportBufferStatus(r);
}

void LteRlcUm::ExpireReorderingTimer() {
  NS_LOG_FUNCTION(this << m_rnti << (uint32_t)m_lcid);
  NS_LOG_LOGIC("Reordering timer has expired");

  SequenceNumber10 newVrUr = m_vrUx;

  while (m_rxBuffer.find(newVrUr.GetValue()) != m_rxBuffer.end()) {
    newVrUr++;
  }
  SequenceNumber10 oldVrUr = m_vrUr;
  m_vrUr = newVrUr;
  NS_LOG_LOGIC("New VR(UR) = " << m_vrUr);

  ReassembleSnInterval(oldVrUr, m_vrUr);

  if (m_vrUh > m_vrUr) {
    NS_LOG_LOGIC("Start reordering timer");
    m_reorderingTimer = Simulator::Schedule(
        m_reorderingTimerValue, &LteRlcUm::ExpireReorderingTimer, this);
    m_vrUx = m_vrUh;
    NS_LOG_LOGIC("New VR(UX) = " << m_vrUx);
  }
}

void LteRlcUm::ExpireRbsTimer() {
  NS_LOG_LOGIC("RBS Timer expires");

  if (!m_txBuffer.empty()) {
    DoReportBufferStatus();
    m_rbsTimer =
        Simulator::Schedule(MilliSeconds(10), &LteRlcUm::ExpireRbsTimer, this);
  }
}

} // namespace ns3
