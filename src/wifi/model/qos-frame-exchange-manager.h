
#ifndef QOS_FRAME_EXCHANGE_MANAGER_H
#define QOS_FRAME_EXCHANGE_MANAGER_H

#include "frame-exchange-manager.h"

#include <optional>

namespace ns3 {

class QosFrameExchangeManager : public FrameExchangeManager {
public:
  static TypeId GetTypeId();
  QosFrameExchangeManager();
  ~QosFrameExchangeManager() override;

  bool StartTransmission(Ptr<Txop> edca, uint16_t allowedWidth) override;

  bool TryAddMpdu(Ptr<const WifiMpdu> mpdu, WifiTxParameters &txParams,
                  Time availableTime) const;

  virtual bool IsWithinLimitsIfAddMpdu(Ptr<const WifiMpdu> mpdu,
                                       const WifiTxParameters &txParams,
                                       Time ppduDurationLimit) const;

  virtual bool IsWithinSizeAndTimeLimits(uint32_t ppduPayloadSize,
                                         Mac48Address receiver,
                                         const WifiTxParameters &txParams,
                                         Time ppduDurationLimit) const;

  virtual Ptr<WifiMpdu> CreateAliasIfNeeded(Ptr<WifiMpdu> mpdu) const;

protected:
  void DoDispose() override;

  void ReceiveMpdu(Ptr<const WifiMpdu> mpdu, RxSignalInfo rxSignalInfo,
                   const WifiTxVector &txVector, bool inAmpdu) override;
  void PreProcessFrame(Ptr<const WifiPsdu> psdu,
                       const WifiTxVector &txVector) override;
  void PostProcessFrame(Ptr<const WifiPsdu> psdu,
                        const WifiTxVector &txVector) override;
  void NavResetTimeout() override;
  void UpdateNav(Ptr<const WifiPsdu> psdu,
                 const WifiTxVector &txVector) override;
  Time GetFrameDurationId(const WifiMacHeader &header, uint32_t size,
                          const WifiTxParameters &txParams,
                          Ptr<Packet> fragmentedPacket) const override;
  Time GetRtsDurationId(const WifiTxVector &rtsTxVector, Time txDuration,
                        Time response) const override;
  Time GetCtsToSelfDurationId(const WifiTxVector &ctsTxVector, Time txDuration,
                              Time response) const override;
  void TransmissionSucceeded() override;
  void TransmissionFailed() override;
  void ForwardMpduDown(Ptr<WifiMpdu> mpdu, WifiTxVector &txVector) override;

  virtual bool StartTransmission(Ptr<QosTxop> edca, Time txopDuration);

  virtual bool StartFrameExchange(Ptr<QosTxop> edca, Time availableTime,
                                  bool initialFrame);

  void PifsRecovery();

  virtual bool SendCfEndIfNeeded();

  virtual void SetTxopHolder(Ptr<const WifiPsdu> psdu,
                             const WifiTxVector &txVector);

  virtual void ClearTxopHolderIfNeeded();

  Ptr<QosTxop> m_edca;
  std::optional<Mac48Address> m_txopHolder;
  bool m_setQosQueueSize;

private:
  void CancelPifsRecovery();

  bool m_initialFrame;
  bool m_pifsRecovery;
  EventId m_pifsRecoveryEvent;
  Ptr<Txop> m_edcaBackingOff;
};

} // namespace ns3

#endif
