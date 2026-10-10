#ifndef RECIPIENT_BLOCK_ACK_AGREEMENT_H
#define RECIPIENT_BLOCK_ACK_AGREEMENT_H

#include "block-ack-agreement.h"
#include "block-ack-window.h"

#include <map>

namespace ns3 {

class WifiMpdu;
class MacRxMiddle;
class CtrlBAckResponseHeader;

class RecipientBlockAckAgreement : public BlockAckAgreement {
public:
  RecipientBlockAckAgreement(Mac48Address originator, bool amsduSupported,
                             uint8_t tid, uint16_t bufferSize, uint16_t timeout,
                             uint16_t startingSeq, bool htSupported);
  ~RecipientBlockAckAgreement() override;

  void SetMacRxMiddle(const Ptr<MacRxMiddle> rxMiddle);

  void NotifyReceivedMpdu(Ptr<const WifiMpdu> mpdu);
  void NotifyReceivedBar(uint16_t startingSequenceNumber);
  void FillBlockAckBitmap(CtrlBAckResponseHeader *blockAckHeader,
                          std::size_t index = 0) const;
  void Flush();

private:
  void PassBufferedMpdusUntilFirstLost();

  void PassBufferedMpdusWithSeqNumberLessThan(uint16_t newWinStartB);

  typedef std::pair<uint16_t, uint16_t *> Key;

  struct Compare {
    bool operator()(const Key &a, const Key &b) const;
  };

  BlockAckWindow m_scoreboard;
  uint16_t m_winStartB;
  std::size_t m_winSizeB;
  std::map<Key, Ptr<const WifiMpdu>, Compare> m_bufferedMpdus;
  Ptr<MacRxMiddle> m_rxMiddle;
};

} // namespace ns3

#endif
