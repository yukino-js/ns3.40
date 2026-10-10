
#ifndef QOS_TXOP_H
#define QOS_TXOP_H

#include "block-ack-manager.h"
#include "qos-utils.h"
#include "txop.h"

#include "ns3/traced-value.h"

namespace ns3 {

class MgtAddBaResponseHeader;
class MgtDelBaHeader;
class AggregationCapableTransmissionListener;
class WifiTxVector;
class QosFrameExchangeManager;
class WifiTxParameters;

class QosTxop : public Txop {
public:
  static TypeId GetTypeId();

  QosTxop(AcIndex ac = AC_UNDEF);

  ~QosTxop() override;

  bool IsQosTxop() const override;
  bool HasFramesToTransmit(uint8_t linkId) override;
  void NotifyChannelAccessed(uint8_t linkId, Time txopDuration) override;
  void NotifyChannelReleased(uint8_t linkId) override;
  void SetDroppedMpduCallback(DroppedMpdu callback) override;

  AcIndex GetAccessCategory() const;

  bool UseExplicitBarAfterMissedBlockAck() const;

  Ptr<BlockAckManager> GetBaManager();
  uint16_t GetBaBufferSize(Mac48Address address, uint8_t tid) const;
  uint16_t GetBaStartingSequence(Mac48Address address, uint8_t tid) const;
  std::pair<CtrlBAckRequestHeader, WifiMacHeader>
  PrepareBlockAckRequest(Mac48Address recipient, uint8_t tid) const;

  void GotAddBaResponse(const MgtAddBaResponseHeader &respHdr,
                        Mac48Address recipient);
  void GotDelBaFrame(const MgtDelBaHeader *delBaHdr, Mac48Address recipient);
  void NotifyOriginatorAgreementNoReply(const Mac48Address &recipient,
                                        uint8_t tid);
  void AddBaResponseTimeout(Mac48Address recipient, uint8_t tid);
  void ResetBa(Mac48Address recipient, uint8_t tid);

  void SetBlockAckThreshold(uint8_t threshold);
  uint8_t GetBlockAckThreshold() const;

  void SetBlockAckInactivityTimeout(uint16_t timeout);
  uint16_t GetBlockAckInactivityTimeout() const;
  void CompleteMpduTx(Ptr<WifiMpdu> mpdu);
  void SetAddBaResponseTimeout(Time addBaResponseTimeout);
  Time GetAddBaResponseTimeout() const;
  void SetFailedAddBaTimeout(Time failedAddBaTimeout);
  Time GetFailedAddBaTimeout() const;

  uint16_t GetNextSequenceNumberFor(const WifiMacHeader *hdr);
  uint16_t PeekNextSequenceNumberFor(const WifiMacHeader *hdr);
  Ptr<WifiMpdu>
  PeekNextMpdu(uint8_t linkId, uint8_t tid = 8,
               Mac48Address recipient = Mac48Address::GetBroadcast(),
               Ptr<const WifiMpdu> mpdu = nullptr);
  Ptr<WifiMpdu> GetNextMpdu(uint8_t linkId, Ptr<WifiMpdu> peekedItem,
                            WifiTxParameters &txParams, Time availableTime,
                            bool initialFrame);

  void AssignSequenceNumber(Ptr<WifiMpdu> mpdu) const;

  uint8_t GetQosQueueSize(uint8_t tid, Mac48Address receiver) const;

  virtual bool IsTxopStarted(uint8_t linkId) const;
  virtual Time GetRemainingTxop(uint8_t linkId) const;

  void SetMuCwMin(uint16_t cwMin, uint8_t linkId);
  void SetMuCwMax(uint16_t cwMax, uint8_t linkId);
  void SetMuAifsn(uint8_t aifsn, uint8_t linkId);
  void SetMuEdcaTimer(Time timer, uint8_t linkId);
  void StartMuEdcaTimerNow(uint8_t linkId);
  bool MuEdcaTimerRunning(uint8_t linkId) const;
  bool EdcaDisabled(uint8_t linkId) const;
  uint32_t GetMinCw(uint8_t linkId) const override;
  uint32_t GetMaxCw(uint8_t linkId) const override;
  uint8_t GetAifsn(uint8_t linkId) const override;

protected:
  struct QosLinkEntity : public Txop::LinkEntity {
    ~QosLinkEntity() override = default;

    Time startTxop{0};
    Time txopDuration{0};
    uint32_t muCwMin{0};
    uint32_t muCwMax{0};
    uint8_t muAifsn{0};
    Time muEdcaTimer{0};
    Time muEdcaTimerStartTime{0};
  };

  void DoDispose() override;

  QosLinkEntity &GetLink(uint8_t linkId) const;

private:
  friend class AggregationCapableTransmissionListener;

  std::unique_ptr<LinkEntity> CreateLinkEntity() const override;

  bool IsQosOldPacket(Ptr<const WifiMpdu> mpdu);

  AcIndex m_ac;
  Ptr<BlockAckManager> m_baManager;
  uint8_t m_blockAckThreshold;
  uint16_t m_blockAckInactivityTimeout;
  Time m_addBaResponseTimeout;
  Time m_failedAddBaTimeout;
  bool m_useExplicitBarAfterMissedBlockAck;
  uint8_t m_nMaxInflights;

  typedef TracedCallback<Time, Time, uint8_t> TxopTracedCallback;

  TxopTracedCallback m_txopTrace;
};

} // namespace ns3

#endif
