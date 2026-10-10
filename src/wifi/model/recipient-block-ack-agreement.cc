
#include "recipient-block-ack-agreement.h"

#include "ctrl-headers.h"
#include "mac-rx-middle.h"
#include "wifi-mpdu.h"
#include "wifi-utils.h"

#include "ns3/log.h"
#include "ns3/packet.h"

#include <algorithm>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("RecipientBlockAckAgreement");

bool RecipientBlockAckAgreement::Compare::operator()(const Key &a,
                                                     const Key &b) const {
  return ((a.first - *a.second + SEQNO_SPACE_SIZE) % SEQNO_SPACE_SIZE) <
         ((b.first - *b.second + SEQNO_SPACE_SIZE) % SEQNO_SPACE_SIZE);
}

RecipientBlockAckAgreement::RecipientBlockAckAgreement(
    Mac48Address originator, bool amsduSupported, uint8_t tid,
    uint16_t bufferSize, uint16_t timeout, uint16_t startingSeq,
    bool htSupported)
    : BlockAckAgreement(originator, tid) {
  NS_LOG_FUNCTION(this << originator << amsduSupported << +tid << bufferSize
                       << timeout << startingSeq << htSupported);

  m_amsduSupported = amsduSupported;
  m_bufferSize = bufferSize;
  m_timeout = timeout;
  m_startingSeq = startingSeq;
  m_htSupported = htSupported;

  m_scoreboard.Init(startingSeq, bufferSize);
  m_winStartB = startingSeq;
  m_winSizeB = bufferSize;
}

RecipientBlockAckAgreement::~RecipientBlockAckAgreement() {
  NS_LOG_FUNCTION_NOARGS();
  m_bufferedMpdus.clear();
  m_rxMiddle = nullptr;
}

void RecipientBlockAckAgreement::SetMacRxMiddle(
    const Ptr<MacRxMiddle> rxMiddle) {
  NS_LOG_FUNCTION(this << rxMiddle);
  m_rxMiddle = rxMiddle;
}

void RecipientBlockAckAgreement::PassBufferedMpdusUntilFirstLost() {
  NS_LOG_FUNCTION(this);

  NS_ASSERT(m_bufferedMpdus.empty() ||
            GetDistance(m_bufferedMpdus.rbegin()->first.first, m_winStartB) <
                SEQNO_SPACE_HALF_SIZE);

  auto it = m_bufferedMpdus.begin();

  while (it != m_bufferedMpdus.end() && it->first.first == m_winStartB) {
    NS_LOG_DEBUG("Forwarding up: " << *it->second);
    m_rxMiddle->Receive(it->second, WIFI_LINKID_UNDEFINED);
    it = m_bufferedMpdus.erase(it);
    m_winStartB = (m_winStartB + 1) % SEQNO_SPACE_SIZE;
  }
}

void RecipientBlockAckAgreement::PassBufferedMpdusWithSeqNumberLessThan(
    uint16_t newWinStartB) {
  NS_LOG_FUNCTION(this << newWinStartB);

  NS_ASSERT(m_bufferedMpdus.empty() ||
            GetDistance(m_bufferedMpdus.rbegin()->first.first, m_winStartB) <
                SEQNO_SPACE_HALF_SIZE);

  auto it = m_bufferedMpdus.begin();

  while (it != m_bufferedMpdus.end() &&
         GetDistance(it->first.first, m_winStartB) <
             GetDistance(newWinStartB, m_winStartB)) {
    NS_LOG_DEBUG("Forwarding up: " << *it->second);
    m_rxMiddle->Receive(it->second, WIFI_LINKID_UNDEFINED);
    it = m_bufferedMpdus.erase(it);
  }
  m_winStartB = newWinStartB;
}

void RecipientBlockAckAgreement::NotifyReceivedMpdu(Ptr<const WifiMpdu> mpdu) {
  NS_LOG_FUNCTION(this << *mpdu);

  uint16_t mpduSeqNumber = mpdu->GetHeader().GetSequenceNumber();
  uint16_t distance = GetDistance(mpduSeqNumber, m_scoreboard.GetWinStart());

  if (distance < m_scoreboard.GetWinSize()) {
    m_scoreboard.At(distance) = true;
  } else if (distance < SEQNO_SPACE_HALF_SIZE) {
    m_scoreboard.Advance(distance - m_scoreboard.GetWinSize() + 1);
    m_scoreboard.At(m_scoreboard.GetWinSize() - 1) = true;
  }

  distance = GetDistance(mpduSeqNumber, m_winStartB);

  if (distance < m_winSizeB) {
    m_bufferedMpdus.insert(
        {{mpdu->GetHeader().GetSequenceNumber(), &m_winStartB}, mpdu});

    PassBufferedMpdusUntilFirstLost();
  } else if (distance < SEQNO_SPACE_HALF_SIZE) {
    m_bufferedMpdus.insert(
        {{mpdu->GetHeader().GetSequenceNumber(), &m_winStartB}, mpdu});

    PassBufferedMpdusWithSeqNumberLessThan(
        mpdu->GetHeader().GetSequenceNumber() - m_winSizeB + 1);

    PassBufferedMpdusUntilFirstLost();
  }
}

void RecipientBlockAckAgreement::Flush() {
  NS_LOG_FUNCTION(this);
  PassBufferedMpdusWithSeqNumberLessThan(m_scoreboard.GetWinStart());
  PassBufferedMpdusUntilFirstLost();
}

void RecipientBlockAckAgreement::NotifyReceivedBar(
    uint16_t startingSequenceNumber) {
  NS_LOG_FUNCTION(this << startingSequenceNumber);

  uint16_t distance =
      GetDistance(startingSequenceNumber, m_scoreboard.GetWinStart());

  if (distance > 0 && distance < m_scoreboard.GetWinSize()) {
    m_scoreboard.Advance(distance);
    NS_ASSERT(m_scoreboard.GetWinStart() == startingSequenceNumber);
  } else if (distance > 0 && distance < SEQNO_SPACE_HALF_SIZE) {
    m_scoreboard.Reset(startingSequenceNumber);
  }

  distance = GetDistance(startingSequenceNumber, m_winStartB);

  if (distance > 0 && distance < SEQNO_SPACE_HALF_SIZE) {
    PassBufferedMpdusWithSeqNumberLessThan(startingSequenceNumber);

    PassBufferedMpdusUntilFirstLost();
  }
}

void RecipientBlockAckAgreement::FillBlockAckBitmap(
    CtrlBAckResponseHeader *blockAckHeader, std::size_t index) const {
  NS_LOG_FUNCTION(this << blockAckHeader << index);
  if (blockAckHeader->IsBasic()) {
    NS_FATAL_ERROR("Basic block ack is not supported.");
  } else if (blockAckHeader->IsMultiTid()) {
    NS_FATAL_ERROR("Multi-tid block ack is not supported.");
  } else if (blockAckHeader->IsCompressed() ||
             blockAckHeader->IsExtendedCompressed() ||
             blockAckHeader->IsMultiSta()) {
    uint16_t ssn = m_scoreboard.GetWinStart();
    NS_LOG_DEBUG("SSN=" << ssn);
    blockAckHeader->SetStartingSequence(ssn, index);
    blockAckHeader->ResetBitmap(index);

    for (std::size_t i = 0; i < m_scoreboard.GetWinSize(); i++) {
      if (m_scoreboard.At(i)) {
        blockAckHeader->SetReceivedPacket((ssn + i) % SEQNO_SPACE_SIZE, index);
      }
    }
  }
}

} // namespace ns3
