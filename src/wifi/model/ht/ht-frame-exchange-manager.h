
#ifndef HT_FRAME_EXCHANGE_MANAGER_H
#define HT_FRAME_EXCHANGE_MANAGER_H

#include "ns3/mpdu-aggregator.h"
#include "ns3/msdu-aggregator.h"
#include "ns3/qos-frame-exchange-manager.h"
#include "ns3/wifi-psdu.h"

class AmpduAggregationTest;
class TwoLevelAggregationTest;
class HeAggregationTest;

namespace ns3 {

class MgtAddBaResponseHeader;
class RecipientBlockAckAgreement;

class HtFrameExchangeManager : public QosFrameExchangeManager {
public:
  friend class ::AmpduAggregationTest;
  friend class ::TwoLevelAggregationTest;
  friend class ::HeAggregationTest;

  static TypeId GetTypeId();
  HtFrameExchangeManager();
  ~HtFrameExchangeManager() override;

  bool StartFrameExchange(Ptr<QosTxop> edca, Time availableTime,
                          bool initialFrame) override;
  void SetWifiMac(const Ptr<WifiMac> mac) override;
  void CalculateAcknowledgmentTime(
      WifiAcknowledgment *acknowledgment) const override;

  Ptr<MsduAggregator> GetMsduAggregator() const;
  Ptr<MpduAggregator> GetMpduAggregator() const;

  bool IsWithinLimitsIfAddMpdu(Ptr<const WifiMpdu> mpdu,
                               const WifiTxParameters &txParams,
                               Time ppduDurationLimit) const override;

  virtual bool IsWithinAmpduSizeLimit(uint32_t ampduSize, Mac48Address receiver,
                                      uint8_t tid,
                                      WifiModulationClass modulation) const;

  virtual bool TryAggregateMsdu(Ptr<const WifiMpdu> msdu,
                                WifiTxParameters &txParams,
                                Time availableTime) const;

  virtual bool IsWithinLimitsIfAggregateMsdu(Ptr<const WifiMpdu> msdu,
                                             const WifiTxParameters &txParams,
                                             Time ppduDurationLimit) const;

  void SendAddBaResponse(const MgtAddBaRequestHeader *reqHdr,
                         Mac48Address originator);
  virtual uint16_t GetSupportedBaBufferSize() const;

  void SendDelbaFrame(Mac48Address addr, uint8_t tid, bool byOriginator);

  Ptr<WifiMpdu> GetBar(AcIndex ac, std::optional<uint8_t> optTid = std::nullopt,
                       std::optional<Mac48Address> optAddress = std::nullopt);

protected:
  void DoDispose() override;

  void ReceiveMpdu(Ptr<const WifiMpdu> mpdu, RxSignalInfo rxSignalInfo,
                   const WifiTxVector &txVector, bool inAmpdu) override;
  void EndReceiveAmpdu(Ptr<const WifiPsdu> psdu,
                       const RxSignalInfo &rxSignalInfo,
                       const WifiTxVector &txVector,
                       const std::vector<bool> &perMpduStatus) override;
  void NotifyReceivedNormalAck(Ptr<WifiMpdu> mpdu) override;
  void NotifyPacketDiscarded(Ptr<const WifiMpdu> mpdu) override;
  void RetransmitMpduAfterMissedAck(Ptr<WifiMpdu> mpdu) const override;
  void ReleaseSequenceNumbers(Ptr<const WifiPsdu> psdu) const override;
  void ForwardMpduDown(Ptr<WifiMpdu> mpdu, WifiTxVector &txVector) override;
  void FinalizeMacHeader(Ptr<const WifiPsdu> psdu) override;
  void CtsTimeout(Ptr<WifiMpdu> rts, const WifiTxVector &txVector) override;
  void TransmissionSucceeded() override;
  void ProtectionCompleted() override;

  virtual Ptr<WifiPsdu> GetWifiPsdu(Ptr<WifiMpdu> mpdu,
                                    const WifiTxVector &txVector) const;

  Ptr<BlockAckManager> GetBaManager(uint8_t tid) const;

  virtual Time GetPsduDurationId(Time txDuration,
                                 const WifiTxParameters &txParams) const;

  void SendPsduWithProtection(Ptr<WifiPsdu> psdu, WifiTxParameters &txParams);

  virtual void NotifyTxToEdca(Ptr<const WifiPsdu> psdu) const;

  virtual void ForwardPsduDown(Ptr<const WifiPsdu> psdu,
                               WifiTxVector &txVector);

  void DequeuePsdu(Ptr<const WifiPsdu> psdu);

  virtual bool SendMpduFromBaManager(Ptr<WifiMpdu> mpdu, Time availableTime,
                                     bool initialFrame);

  virtual bool SendDataFrame(Ptr<WifiMpdu> peekedItem, Time availableTime,
                             bool initialFrame);

  virtual bool NeedSetupBlockAck(Mac48Address recipient, uint8_t tid);

  bool SendAddBaRequest(Mac48Address recipient, uint8_t tid,
                        uint16_t startingSeq, uint16_t timeout,
                        bool immediateBAck, Time availableTime);

  void SendBlockAck(const RecipientBlockAckAgreement &agreement,
                    Time durationId, WifiTxVector &blockAckTxVector,
                    double rxSnr);

  virtual void BlockAckTimeout(Ptr<WifiPsdu> psdu,
                               const WifiTxVector &txVector);

  virtual void MissedBlockAck(Ptr<WifiPsdu> psdu, const WifiTxVector &txVector,
                              bool &resetCw);

  typedef std::pair<Mac48Address, uint8_t> AgreementKey;

  Ptr<MsduAggregator> m_msduAggregator;
  Ptr<MpduAggregator> m_mpduAggregator;

  std::map<AgreementKey, Ptr<WifiMpdu>> m_pendingAddBaResp;

private:
  void SendPsdu();

  Ptr<WifiPsdu> m_psdu;
  WifiTxParameters m_txParams;
};

} // namespace ns3

#endif
