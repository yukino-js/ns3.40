
#ifndef HE_FRAME_EXCHANGE_MANAGER_H
#define HE_FRAME_EXCHANGE_MANAGER_H

#include "mu-snr-tag.h"

#include "ns3/vht-frame-exchange-manager.h"

#include <map>
#include <unordered_map>

namespace ns3 {

class MultiUserScheduler;
class ApWifiMac;
class StaWifiMac;
class CtrlTriggerHeader;

typedef std::unordered_map<uint16_t, Ptr<WifiPsdu>> WifiPsduMap;
typedef std::unordered_map<uint16_t, Ptr<const WifiPsdu>> WifiConstPsduMap;

bool IsTrigger(const WifiPsduMap &psduMap);

bool IsTrigger(const WifiConstPsduMap &psduMap);

class HeFrameExchangeManager : public VhtFrameExchangeManager {
public:
  static TypeId GetTypeId();
  HeFrameExchangeManager();
  ~HeFrameExchangeManager() override;

  uint16_t GetSupportedBaBufferSize() const override;
  bool StartFrameExchange(Ptr<QosTxop> edca, Time availableTime,
                          bool initialFrame) override;
  void SetWifiMac(const Ptr<WifiMac> mac) override;
  void CalculateAcknowledgmentTime(
      WifiAcknowledgment *acknowledgment) const override;
  void CalculateProtectionTime(WifiProtection *protection) const override;
  void SetTxopHolder(Ptr<const WifiPsdu> psdu,
                     const WifiTxVector &txVector) override;
  bool VirtualCsMediumIdle() const override;

  void SetMultiUserScheduler(const Ptr<MultiUserScheduler> muScheduler);

  static Ptr<WifiPsdu> GetPsduTo(Mac48Address to, const WifiPsduMap &psduMap);

  virtual void SetTargetRssi(CtrlTriggerHeader &trigger) const;

  virtual std::optional<double>
  GetMostRecentRssi(const Mac48Address &address) const;

  bool IsIntraBssPpdu(Ptr<const WifiPsdu> psdu,
                      const WifiTxVector &txVector) const;

  bool UlMuCsMediumIdle(const CtrlTriggerHeader &trigger) const;

protected:
  void DoDispose() override;
  void Reset() override;
  void RxStartIndication(WifiTxVector txVector, Time psduDuration) override;
  void ReceiveMpdu(Ptr<const WifiMpdu> mpdu, RxSignalInfo rxSignalInfo,
                   const WifiTxVector &txVector, bool inAmpdu) override;
  void EndReceiveAmpdu(Ptr<const WifiPsdu> psdu,
                       const RxSignalInfo &rxSignalInfo,
                       const WifiTxVector &txVector,
                       const std::vector<bool> &perMpduStatus) override;
  void PostProcessFrame(Ptr<const WifiPsdu> psdu,
                        const WifiTxVector &txVector) override;
  Time GetTxDuration(uint32_t ppduPayloadSize, Mac48Address receiver,
                     const WifiTxParameters &txParams) const override;
  void NormalAckTimeout(Ptr<WifiMpdu> mpdu,
                        const WifiTxVector &txVector) override;
  void BlockAckTimeout(Ptr<WifiPsdu> psdu,
                       const WifiTxVector &txVector) override;
  void CtsTimeout(Ptr<WifiMpdu> rts, const WifiTxVector &txVector) override;
  void UpdateNav(Ptr<const WifiPsdu> psdu,
                 const WifiTxVector &txVector) override;
  void NavResetTimeout() override;
  void StartProtection(const WifiTxParameters &txParams) override;
  void ProtectionCompleted() override;
  void TransmissionSucceeded() override;

  void ClearTxopHolderIfNeeded() override;

  virtual void IntraBssNavResetTimeout();

  virtual Time GetMuRtsDurationId(uint32_t muRtsSize,
                                  const WifiTxVector &muRtsTxVector,
                                  Time txDuration, Time response) const;

  void RecordSentMuRtsTo(const WifiTxParameters &txParams);

  virtual void SendMuRts(const WifiTxParameters &txParams);

  virtual void CtsAfterMuRtsTimeout(Ptr<WifiMpdu> muRts,
                                    const WifiTxVector &txVector);

  void SendCtsAfterMuRts(const WifiMacHeader &muRtsHdr,
                         const CtrlTriggerHeader &trigger, double muRtsSnr);

  WifiMode GetCtsModeAfterMuRts() const;

  WifiTxVector GetCtsTxVectorAfterMuRts(const CtrlTriggerHeader &trigger,
                                        uint16_t staId) const;

  void SendPsduMapWithProtection(WifiPsduMap psduMap,
                                 WifiTxParameters &txParams);

  virtual void ForwardPsduMapDown(WifiConstPsduMap psduMap,
                                  WifiTxVector &txVector);

  virtual void BlockAcksInTbPpduTimeout(WifiPsduMap *psduMap,
                                        std::size_t nSolicitedStations);

  virtual void TbPpduTimeout(WifiPsduMap *psduMap,
                             std::size_t nSolicitedStations);

  virtual void BlockAckAfterTbPpduTimeout(Ptr<WifiPsdu> psdu,
                                          const WifiTxVector &txVector);

  WifiTxVector GetTrigVector(const CtrlTriggerHeader &trigger) const;

  WifiTxVector GetHeTbTxVector(CtrlTriggerHeader trigger,
                               Mac48Address triggerSender) const;

  Ptr<WifiMpdu>
  PrepareMuBar(const WifiTxVector &responseTxVector,
               std::map<uint16_t, CtrlBAckRequestHeader> recipients) const;

  void SendMultiStaBlockAck(const WifiTxParameters &txParams, Time durationId);

  void SendQosNullFramesInTbPpdu(const CtrlTriggerHeader &trigger,
                                 const WifiMacHeader &hdr);

  Ptr<ApWifiMac> m_apMac;
  Ptr<StaWifiMac> m_staMac;
  WifiTxVector m_trigVector;
  Time m_intraBssNavEnd;
  EventId m_intraBssNavResetEvent;

private:
  void SendPsduMap();

  void ReceiveBasicTrigger(const CtrlTriggerHeader &trigger,
                           const WifiMacHeader &hdr);

  void ReceiveMuBarTrigger(const CtrlTriggerHeader &trigger, uint8_t tid,
                           Time durationId, double snr);

  WifiPsduMap m_psduMap;
  WifiTxParameters m_txParams;
  Ptr<MultiUserScheduler> m_muScheduler;
  Ptr<WifiMpdu> m_triggerFrame;
  EventId m_multiStaBaEvent;
  MuSnrTag m_muSnrTag;
  bool m_triggerFrameInAmpdu;
};

} // namespace ns3

#endif
