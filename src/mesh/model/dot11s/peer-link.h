
#ifndef PEER_LINK_H
#define PEER_LINK_H

#include "ie-dot11s-beacon-timing.h"
#include "ie-dot11s-configuration.h"
#include "ie-dot11s-peer-management.h"

#include "ns3/callback.h"
#include "ns3/event-id.h"
#include "ns3/mac48-address.h"
#include "ns3/nstime.h"
#include "ns3/object.h"

namespace ns3 {
namespace dot11s {

class PeerManagementProtocolMac;

class PeerLink : public Object {
public:
  friend class PeerManagementProtocol;
  static TypeId GetTypeId();
  PeerLink();
  ~PeerLink() override;

  PeerLink(const PeerLink &) = delete;
  PeerLink &operator=(const PeerLink &) = delete;

  void DoDispose() override;

  enum PeerState {
    IDLE,
    OPN_SNT,
    CNF_RCVD,
    OPN_RCVD,
    ESTAB,
    HOLDING,
  };

  static const char *const PeerStateNames[6];
  void SetBeaconInformation(Time lastBeacon, Time BeaconInterval);
  void SetLinkStatusCallback(Callback<void, uint32_t, Mac48Address, bool> cb);
  void SetPeerAddress(Mac48Address macaddr);
  void SetPeerMeshPointAddress(Mac48Address macaddr);
  void SetInterface(uint32_t interface);
  void SetLocalLinkId(uint16_t id);
  void SetLocalAid(uint16_t aid);
  uint16_t GetPeerAid() const;
  void SetBeaconTimingElement(IeBeaconTiming beaconTiming);
  Mac48Address GetPeerAddress() const;
  uint16_t GetLocalAid() const;
  Time GetLastBeacon() const;
  Time GetBeaconInterval() const;
  IeBeaconTiming GetBeaconTimingElement() const;

  void MLMECancelPeerLink(PmpReasonCode reason);
  void MLMEActivePeerLinkOpen();
  void MLMEPeeringRequestReject();
  typedef Callback<void, uint32_t, Mac48Address, Mac48Address,
                   PeerLink::PeerState, PeerLink::PeerState>
      SignalStatusCallback;
  void MLMESetSignalStatusCallback(SignalStatusCallback cb);
  void TransmissionSuccess();
  void TransmissionFailure();

  void Report(std::ostream &os) const;

private:
  enum PeerEvent {
    CNCL,
    ACTOPN,
    CLS_ACPT,
    OPN_ACPT,
    OPN_RJCT,
    REQ_RJCT,
    CNF_ACPT,
    CNF_RJCT,
    TOR1,
    TOR2,
    TOC,
    TOH
  };

  void StateMachine(PeerEvent event, PmpReasonCode = REASON11S_RESERVED);
  void Close(uint16_t localLinkID, uint16_t peerLinkID, PmpReasonCode reason);
  void OpenAccept(uint16_t localLinkId, IeConfiguration conf,
                  Mac48Address peerMp);
  void OpenReject(uint16_t localLinkId, IeConfiguration conf,
                  Mac48Address peerMp, PmpReasonCode reason);
  void ConfirmAccept(uint16_t localLinkId, uint16_t peerLinkId,
                     uint16_t peerAid, IeConfiguration conf,
                     Mac48Address peerMp);
  void ConfirmReject(uint16_t localLinkId, uint16_t peerLinkId,
                     IeConfiguration conf, Mac48Address peerMp,
                     PmpReasonCode reason);

  bool LinkIsEstab() const;
  bool LinkIsIdle() const;
  void SetMacPlugin(Ptr<PeerManagementProtocolMac> plugin);
  void ClearRetryTimer();
  void ClearConfirmTimer();
  void ClearHoldingTimer();
  void SetHoldingTimer();
  void SetRetryTimer();
  void SetConfirmTimer();

  void SendPeerLinkClose(PmpReasonCode reasoncode);
  void SendPeerLinkOpen();
  void SendPeerLinkConfirm();

  void HoldingTimeout();
  void RetryTimeout();
  void ConfirmTimeout();

  void BeaconLoss();

private:
  uint32_t m_interface;
  Ptr<PeerManagementProtocolMac> m_macPlugin;
  Mac48Address m_peerAddress;
  Mac48Address m_peerMeshPointAddress;
  uint16_t m_localLinkId;
  uint16_t m_peerLinkId;
  uint16_t m_assocId;
  uint16_t m_peerAssocId;

  Time m_lastBeacon;
  Time m_beaconInterval;
  uint16_t m_packetFail;

  PeerState m_state;
  IeConfiguration m_configuration;
  IeBeaconTiming m_beaconTiming;

  uint16_t m_dot11MeshMaxRetries;
  Time m_dot11MeshRetryTimeout;
  Time m_dot11MeshHoldingTimeout;
  Time m_dot11MeshConfirmTimeout;

  EventId m_retryTimer;
  EventId m_holdingTimer;
  EventId m_confirmTimer;
  uint16_t m_retryCounter;
  EventId m_beaconLossTimer;
  uint16_t m_maxBeaconLoss;
  uint16_t m_maxPacketFail;

  SignalStatusCallback m_linkStatusCallback;
};

} // namespace dot11s
} // namespace ns3

#endif
