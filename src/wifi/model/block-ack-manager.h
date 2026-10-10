
#ifndef BLOCK_ACK_MANAGER_H
#define BLOCK_ACK_MANAGER_H

#include "block-ack-type.h"
#include "originator-block-ack-agreement.h"
#include "recipient-block-ack-agreement.h"
#include "wifi-mac-header.h"
#include "wifi-mpdu.h"
#include "wifi-tx-vector.h"

#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/traced-callback.h"

#include <map>
#include <optional>

namespace ns3 {

class MgtAddBaResponseHeader;
class MgtAddBaRequestHeader;
class CtrlBAckResponseHeader;
class CtrlBAckRequestHeader;
class WifiMacQueue;
class MacRxMiddle;

class BlockAckManager : public Object {
private:
  enum MpduStatus : uint8_t { STAY_INFLIGHT = 0, TO_RETRANSMIT, ACKNOWLEDGED };

public:
  static TypeId GetTypeId();

  BlockAckManager();
  ~BlockAckManager() override;

  BlockAckManager(const BlockAckManager &) = delete;
  BlockAckManager &operator=(const BlockAckManager &) = delete;

  using OriginatorAgreementOptConstRef =
      std::optional<std::reference_wrapper<const OriginatorBlockAckAgreement>>;
  using RecipientAgreementOptConstRef =
      std::optional<std::reference_wrapper<const RecipientBlockAckAgreement>>;

  OriginatorAgreementOptConstRef
  GetAgreementAsOriginator(const Mac48Address &recipient, uint8_t tid) const;
  RecipientAgreementOptConstRef
  GetAgreementAsRecipient(const Mac48Address &originator, uint8_t tid) const;

  void CreateOriginatorAgreement(const MgtAddBaRequestHeader &reqHdr,
                                 const Mac48Address &recipient,
                                 bool htSupported = true);
  void DestroyOriginatorAgreement(const Mac48Address &recipient, uint8_t tid);
  void UpdateOriginatorAgreement(const MgtAddBaResponseHeader &respHdr,
                                 const Mac48Address &recipient,
                                 uint16_t startingSeq);

  void CreateRecipientAgreement(const MgtAddBaResponseHeader &respHdr,
                                const Mac48Address &originator,
                                uint16_t startingSeq, bool htSupported,
                                Ptr<MacRxMiddle> rxMiddle);
  void DestroyRecipientAgreement(const Mac48Address &originator, uint8_t tid);

  void StorePacket(Ptr<WifiMpdu> mpdu);
  void NotifyGotAck(uint8_t linkId, Ptr<const WifiMpdu> mpdu);
  void NotifyMissedAck(uint8_t linkId, Ptr<WifiMpdu> mpdu);
  std::pair<uint16_t, uint16_t>
  NotifyGotBlockAck(uint8_t linkId, const CtrlBAckResponseHeader &blockAck,
                    const Mac48Address &recipient,
                    const std::set<uint8_t> &tids, size_t index = 0);
  void NotifyMissedBlockAck(uint8_t linkId, const Mac48Address &recipient,
                            uint8_t tid);
  void NotifyGotBlockAckRequest(const Mac48Address &originator, uint8_t tid,
                                uint16_t startingSeq);
  void NotifyGotMpdu(Ptr<const WifiMpdu> mpdu);
  uint32_t GetNBufferedPackets(const Mac48Address &recipient,
                               uint8_t tid) const;
  void NotifyOriginatorAgreementEstablished(const Mac48Address &recipient,
                                            uint8_t tid, uint16_t startingSeq);
  void NotifyOriginatorAgreementRejected(const Mac48Address &recipient,
                                         uint8_t tid);
  void NotifyOriginatorAgreementNoReply(const Mac48Address &recipient,
                                        uint8_t tid);
  void NotifyOriginatorAgreementReset(const Mac48Address &recipient,
                                      uint8_t tid);
  void SetBlockAckThreshold(uint8_t nPackets);

  void SetQueue(const Ptr<WifiMacQueue> queue);

  void SetBlockAckInactivityCallback(
      Callback<void, Mac48Address, uint8_t, bool> callback);
  void
  SetBlockDestinationCallback(Callback<void, Mac48Address, uint8_t> callback);
  void
  SetUnblockDestinationCallback(Callback<void, Mac48Address, uint8_t> callback);

  bool NeedBarRetransmission(uint8_t tid, const Mac48Address &recipient);
  uint16_t GetRecipientBufferSize(const Mac48Address &recipient,
                                  uint8_t tid) const;
  uint16_t GetOriginatorStartingSequence(const Mac48Address &recipient,
                                         uint8_t tid) const;

  typedef Callback<void, Ptr<const WifiMpdu>> TxOk;
  typedef Callback<void, Ptr<const WifiMpdu>> TxFailed;
  typedef Callback<void, Ptr<const WifiMpdu>> DroppedOldMpdu;

  void SetTxOkCallback(TxOk callback);
  void SetTxFailedCallback(TxFailed callback);
  void SetDroppedOldMpduCallback(DroppedOldMpdu callback);

  typedef void (*AgreementStateTracedCallback)(
      Time now, const Mac48Address &recipient, uint8_t tid,
      OriginatorBlockAckAgreement::State state);

  void NotifyDiscardedMpdu(Ptr<const WifiMpdu> mpdu);

  CtrlBAckRequestHeader GetBlockAckReqHeader(const Mac48Address &recipient,
                                             uint8_t tid) const;

  void ScheduleBar(const CtrlBAckRequestHeader &reqHdr,
                   const WifiMacHeader &hdr);

  using AgreementKey = std::pair<Mac48Address, uint8_t>;

  const std::list<AgreementKey> &GetSendBarIfDataQueuedList() const;
  void AddToSendBarIfDataQueuedList(const Mac48Address &recipient, uint8_t tid);
  void RemoveFromSendBarIfDataQueuedList(const Mac48Address &recipient,
                                         uint8_t tid);

protected:
  void DoDispose() override;

private:
  void InactivityTimeout(const Mac48Address &recipient, uint8_t tid);

  typedef std::list<Ptr<WifiMpdu>> PacketQueue;
  typedef std::list<Ptr<WifiMpdu>>::iterator PacketQueueI;

  using OriginatorAgreements =
      std::map<AgreementKey,
               std::pair<OriginatorBlockAckAgreement, PacketQueue>>;
  using OriginatorAgreementsI = OriginatorAgreements::iterator;

  using RecipientAgreements =
      std::map<AgreementKey, RecipientBlockAckAgreement>;

  PacketQueueI HandleInFlightMpdu(uint8_t linkId, PacketQueueI mpduIt,
                                  MpduStatus status,
                                  const OriginatorAgreementsI &it,
                                  const Time &now);

  OriginatorAgreements m_originatorAgreements;
  RecipientAgreements m_recipientAgreements;

  std::list<AgreementKey> m_sendBarIfDataQueued;

  uint8_t m_blockAckThreshold;
  Ptr<WifiMacQueue> m_queue;
  Callback<void, Mac48Address, uint8_t, bool> m_blockAckInactivityTimeout;
  Callback<void, Mac48Address, uint8_t> m_blockPackets;
  Callback<void, Mac48Address, uint8_t> m_unblockPackets;
  TxOk m_txOkCallback;
  TxFailed m_txFailedCallback;
  DroppedOldMpdu m_droppedOldMpduCallback;

  TracedCallback<Time, Mac48Address, uint8_t,
                 OriginatorBlockAckAgreement::State>
      m_originatorAgreementState;
};

} // namespace ns3

#endif
