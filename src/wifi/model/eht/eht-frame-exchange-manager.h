
#ifndef EHT_FRAME_EXCHANGE_MANAGER_H
#define EHT_FRAME_EXCHANGE_MANAGER_H

#include "ns3/he-frame-exchange-manager.h"
#include "ns3/mgt-headers.h"

namespace ns3 {

class EhtFrameExchangeManager : public HeFrameExchangeManager {
public:
  static TypeId GetTypeId();
  EhtFrameExchangeManager();
  ~EhtFrameExchangeManager() override;

  void SetLinkId(uint8_t linkId) override;
  Ptr<WifiMpdu> CreateAliasIfNeeded(Ptr<WifiMpdu> mpdu) const override;
  bool StartTransmission(Ptr<Txop> edca, uint16_t allowedWidth) override;

  void SendEmlOmn(const Mac48Address &dest, const MgtEmlOmn &frame);

  std::optional<double>
  GetMostRecentRssi(const Mac48Address &address) const override;

  bool GetEmlsrSwitchToListening(Ptr<const WifiPsdu> psdu, uint16_t aid,
                                 const Mac48Address &address) const;

  void NotifySwitchingEmlsrLink(Ptr<WifiPhy> phy, uint8_t linkId, Time delay);

protected:
  void DoDispose() override;
  void RxStartIndication(WifiTxVector txVector, Time psduDuration) override;
  void ForwardPsduDown(Ptr<const WifiPsdu> psdu,
                       WifiTxVector &txVector) override;
  void ForwardPsduMapDown(WifiConstPsduMap psduMap,
                          WifiTxVector &txVector) override;
  void SendMuRts(const WifiTxParameters &txParams) override;
  void TransmissionFailed() override;
  void NotifyChannelReleased(Ptr<Txop> txop) override;
  void PostProcessFrame(Ptr<const WifiPsdu> psdu,
                        const WifiTxVector &txVector) override;
  void ReceiveMpdu(Ptr<const WifiMpdu> mpdu, RxSignalInfo rxSignalInfo,
                   const WifiTxVector &txVector, bool inAmpdu) override;

  void EmlsrSwitchToListening(const Mac48Address &address, const Time &delay);

private:
  void UpdateTxopEndOnTxStart(Time txDuration);

  void UpdateTxopEndOnRxStartIndication(Time psduDuration);

  void UpdateTxopEndOnRxEnd();

  void TxopEnd();

  EventId m_ongoingTxopEnd;
};

} // namespace ns3

#endif
