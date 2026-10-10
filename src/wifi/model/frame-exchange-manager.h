
#ifndef FRAME_EXCHANGE_MANAGER_H
#define FRAME_EXCHANGE_MANAGER_H

#include "mac-rx-middle.h"
#include "mac-tx-middle.h"
#include "qos-txop.h"
#include "wifi-mac.h"
#include "wifi-phy.h"
#include "wifi-psdu.h"
#include "wifi-tx-parameters.h"
#include "wifi-tx-timer.h"
#include "wifi-tx-vector.h"

#include "channel-access-manager.h"
#include "wifi-ack-manager.h"
#include "wifi-protection-manager.h"

#include "ns3/object.h"

namespace ns3 {

struct RxSignalInfo;
struct WifiProtection;
struct WifiAcknowledgment;

class FrameExchangeManager : public Object {
public:
  static TypeId GetTypeId();
  FrameExchangeManager();
  ~FrameExchangeManager() override;

  typedef Callback<void, WifiMacDropReason, Ptr<const WifiMpdu>> DroppedMpdu;
  typedef Callback<void, Ptr<const WifiMpdu>> AckedMpdu;

  virtual bool StartTransmission(Ptr<Txop> dcf, uint16_t allowedWidth);

  void Receive(Ptr<const WifiPsdu> psdu, RxSignalInfo rxSignalInfo,
               WifiTxVector txVector, std::vector<bool> perMpduStatus);

  virtual void SetLinkId(uint8_t linkId);
  virtual void SetWifiMac(const Ptr<WifiMac> mac);
  virtual void SetMacTxMiddle(const Ptr<MacTxMiddle> txMiddle);
  virtual void SetMacRxMiddle(const Ptr<MacRxMiddle> rxMiddle);
  virtual void
  SetChannelAccessManager(const Ptr<ChannelAccessManager> channelAccessManager);
  virtual void SetWifiPhy(const Ptr<WifiPhy> phy);
  virtual void ResetPhy();
  virtual void
  SetProtectionManager(Ptr<WifiProtectionManager> protectionManager);
  virtual void SetAckManager(Ptr<WifiAckManager> ackManager);
  virtual void SetAddress(Mac48Address address);
  Mac48Address GetAddress() const;
  virtual void SetBssid(Mac48Address bssid);
  Mac48Address GetBssid() const;
  virtual void SetDroppedMpduCallback(DroppedMpdu callback);
  void SetAckedMpduCallback(AckedMpdu callback);
  void SetPromisc();
  bool IsPromisc() const;

  const WifiTxTimer &GetWifiTxTimer() const;

  Ptr<WifiProtectionManager> GetProtectionManager() const;

  virtual void CalculateProtectionTime(WifiProtection *protection) const;

  Ptr<WifiAckManager> GetAckManager() const;

  virtual void
  CalculateAcknowledgmentTime(WifiAcknowledgment *acknowledgment) const;

  virtual bool VirtualCsMediumIdle() const;

  const std::set<Mac48Address> &GetProtectedStas() const;

  virtual void NotifyInternalCollision(Ptr<Txop> txop);

  virtual void NotifySwitchingStartNow(Time duration);

  void NotifySleepNow();

  void NotifyOffNow();

protected:
  void DoDispose() override;

  Ptr<WifiRemoteStationManager> GetWifiRemoteStationManager() const;

  Ptr<WifiMpdu> GetFirstFragmentIfNeeded(Ptr<WifiMpdu> mpdu);

  void SendMpduWithProtection(Ptr<WifiMpdu> mpdu, WifiTxParameters &txParams);

  virtual void StartProtection(const WifiTxParameters &txParams);

  virtual void ProtectionCompleted();

  virtual void UpdateNav(Ptr<const WifiPsdu> psdu,
                         const WifiTxVector &txVector);

  virtual void NavResetTimeout();

  virtual void ReceiveMpdu(Ptr<const WifiMpdu> mpdu, RxSignalInfo rxSignalInfo,
                           const WifiTxVector &txVector, bool inAmpdu);

  virtual void EndReceiveAmpdu(Ptr<const WifiPsdu> psdu,
                               const RxSignalInfo &rxSignalInfo,
                               const WifiTxVector &txVector,
                               const std::vector<bool> &perMpduStatus);

  virtual void ReceivedNormalAck(Ptr<WifiMpdu> mpdu,
                                 const WifiTxVector &txVector,
                                 const WifiTxVector &ackTxVector,
                                 const RxSignalInfo &rxInfo, double snr);

  virtual void NotifyReceivedNormalAck(Ptr<WifiMpdu> mpdu);

  virtual void RetransmitMpduAfterMissedAck(Ptr<WifiMpdu> mpdu) const;

  virtual void ReleaseSequenceNumbers(Ptr<const WifiPsdu> psdu) const;

  virtual void NotifyPacketDiscarded(Ptr<const WifiMpdu> mpdu);

  virtual void PreProcessFrame(Ptr<const WifiPsdu> psdu,
                               const WifiTxVector &txVector);

  virtual void PostProcessFrame(Ptr<const WifiPsdu> psdu,
                                const WifiTxVector &txVector);

  virtual Time GetTxDuration(uint32_t ppduPayloadSize, Mac48Address receiver,
                             const WifiTxParameters &txParams) const;

  void UpdateTxDuration(Mac48Address receiver,
                        WifiTxParameters &txParams) const;

  virtual uint32_t GetPsduSize(Ptr<const WifiMpdu> mpdu,
                               const WifiTxVector &txVector) const;

  virtual void NotifyChannelReleased(Ptr<Txop> txop);

  Ptr<Txop> m_dcf;
  WifiTxTimer m_txTimer;
  EventId m_navResetEvent;
  Ptr<WifiMac> m_mac;
  Ptr<MacTxMiddle> m_txMiddle;
  Ptr<MacRxMiddle> m_rxMiddle;
  Ptr<ChannelAccessManager> m_channelAccessManager;
  Ptr<WifiPhy> m_phy;
  Mac48Address m_self;
  Mac48Address m_bssid;
  Time m_navEnd;
  std::set<Mac48Address> m_sentRtsTo;
  std::set<Mac48Address> m_protectedStas;
  uint8_t m_linkId;
  uint16_t m_allowedWidth;
  bool m_promisc;
  DroppedMpdu m_droppedMpduCallback;
  AckedMpdu m_ackedMpduCallback;

  virtual void FinalizeMacHeader(Ptr<const WifiPsdu> psdu);

  virtual void ForwardMpduDown(Ptr<WifiMpdu> mpdu, WifiTxVector &txVector);

  virtual void DequeueMpdu(Ptr<const WifiMpdu> mpdu);

  virtual Time GetFrameDurationId(const WifiMacHeader &header, uint32_t size,
                                  const WifiTxParameters &txParams,
                                  Ptr<Packet> fragmentedPacket) const;

  virtual Time GetRtsDurationId(const WifiTxVector &rtsTxVector,
                                Time txDuration, Time response) const;

  void SendRts(const WifiTxParameters &txParams);

  void SendCtsAfterRts(const WifiMacHeader &rtsHdr, WifiMode rtsTxMode,
                       double rtsSnr);

  void DoSendCtsAfterRts(const WifiMacHeader &rtsHdr, WifiTxVector &ctsTxVector,
                         double rtsSnr);

  virtual Time GetCtsToSelfDurationId(const WifiTxVector &ctsTxVector,
                                      Time txDuration, Time response) const;

  void SendCtsToSelf(const WifiTxParameters &txParams);

  void SendNormalAck(const WifiMacHeader &hdr, const WifiTxVector &dataTxVector,
                     double dataSnr);

  Ptr<WifiMpdu> GetNextFragment();

  virtual void TransmissionSucceeded();

  virtual void TransmissionFailed();

  virtual void NormalAckTimeout(Ptr<WifiMpdu> mpdu,
                                const WifiTxVector &txVector);

  virtual void CtsTimeout(Ptr<WifiMpdu> rts, const WifiTxVector &txVector);
  void DoCtsTimeout(Ptr<WifiPsdu> psdu);

  virtual void Reset();

  virtual void RxStartIndication(WifiTxVector txVector, Time psduDuration);

private:
  void SendMpdu();

  Ptr<WifiMpdu> m_mpdu;
  WifiTxParameters m_txParams;
  Ptr<Packet> m_fragmentedPacket;
  bool m_moreFragments;
  Ptr<WifiProtectionManager> m_protectionManager;
  Ptr<WifiAckManager> m_ackManager;
};

} // namespace ns3

#endif
