
#ifndef LR_WPAN_MAC_H
#define LR_WPAN_MAC_H

#include "lr-wpan-fields.h"
#include "lr-wpan-phy.h"

#include <ns3/event-id.h>
#include <ns3/mac16-address.h>
#include <ns3/mac64-address.h>
#include <ns3/object.h>
#include <ns3/sequence-number.h>
#include <ns3/traced-callback.h>
#include <ns3/traced-value.h>

#include <deque>
#include <memory>

namespace ns3 {

class Packet;
class LrWpanCsmaCa;

enum LrWpanTxOption {
  TX_OPTION_NONE = 0,
  TX_OPTION_ACK = 1,
  TX_OPTION_GTS = 2,
  TX_OPTION_INDIRECT = 4
};

enum LrWpanMacState {
  MAC_IDLE,
  MAC_CSMA,
  MAC_SENDING,
  MAC_ACK_PENDING,
  CHANNEL_ACCESS_FAILURE,
  CHANNEL_IDLE,
  SET_PHY_TX_ON,
  MAC_GTS,
  MAC_INACTIVE,
  MAC_CSMA_DEFERRED
};

enum SuperframeStatus { BEACON, CAP, CFP, INACTIVE };

enum SuperframeType { OUTGOING = 0, INCOMING = 1 };

enum PendingPrimitiveStatus {
  MLME_NONE = 0,
  MLME_START_REQ = 1,
  MLME_SCAN_REQ = 2,
  MLME_ASSOC_REQ = 3,
  MLME_SYNC_REQ = 4
};

namespace TracedValueCallback {

typedef void (*LrWpanMacState)(LrWpanMacState oldValue,
                               LrWpanMacState newValue);

typedef void (*SuperframeStatus)(SuperframeStatus oldValue,
                                 SuperframeStatus newValue);

} // namespace TracedValueCallback

enum LrWpanAddressMode {
  NO_PANID_ADDR = 0,
  ADDR_MODE_RESERVED = 1,
  SHORT_ADDR = 2,
  EXT_ADDR = 3
};

enum LrWpanAssociationStatus {
  ASSOCIATED = 0,
  PAN_AT_CAPACITY = 1,
  PAN_ACCESS_DENIED = 2,
  ASSOCIATED_WITHOUT_ADDRESS = 0xfe,
  DISASSOCIATED = 0xff
};

enum LrWpanMlmeScanType {
  MLMESCAN_ED = 0x00,
  MLMESCAN_ACTIVE = 0x01,
  MLMESCAN_PASSIVE = 0x02,
  MLMESCAN_ORPHAN = 0x03
};

enum LrWpanMcpsDataConfirmStatus {
  IEEE_802_15_4_SUCCESS = 0,
  IEEE_802_15_4_TRANSACTION_OVERFLOW = 1,
  IEEE_802_15_4_TRANSACTION_EXPIRED = 2,
  IEEE_802_15_4_CHANNEL_ACCESS_FAILURE = 3,
  IEEE_802_15_4_INVALID_ADDRESS = 4,
  IEEE_802_15_4_INVALID_GTS = 5,
  IEEE_802_15_4_NO_ACK = 6,
  IEEE_802_15_4_COUNTER_ERROR = 7,
  IEEE_802_15_4_FRAME_TOO_LONG = 8,
  IEEE_802_15_4_UNAVAILABLE_KEY = 9,
  IEEE_802_15_4_UNSUPPORTED_SECURITY = 10,
  IEEE_802_15_4_INVALID_PARAMETER = 11
};

enum LrWpanMlmeStartConfirmStatus {
  MLMESTART_SUCCESS = 0,
  MLMESTART_NO_SHORT_ADDRESS = 1,
  MLMESTART_SUPERFRAME_OVERLAP = 2,
  MLMESTART_TRACKING_OFF = 3,
  MLMESTART_INVALID_PARAMETER = 4,
  MLMESTART_COUNTER_ERROR = 5,
  MLMESTART_FRAME_TOO_LONG = 6,
  MLMESTART_UNAVAILABLE_KEY = 7,
  MLMESTART_UNSUPPORTED_SECURITY = 8,
  MLMESTART_CHANNEL_ACCESS_FAILURE = 9
};

enum LrWpanMlmeScanConfirmStatus {
  MLMESCAN_SUCCESS = 0,
  MLMESCAN_LIMIT_REACHED = 1,
  MLMESCAN_NO_BEACON = 2,
  MLMESCAN_SCAN_IN_PROGRESS = 3,
  MLMESCAN_COUNTER_ERROR = 4,
  MLMESCAN_FRAME_TOO_LONG = 5,
  MLMESCAN_UNAVAILABLE_KEY = 6,
  MLMESCAN_UNSUPPORTED_SECURITY = 7,
  MLMESCAN_INVALID_PARAMETER = 8
};

enum LrWpanMlmeAssociateConfirmStatus {
  MLMEASSOC_SUCCESS = 0,
  MLMEASSOC_FULL_CAPACITY = 1,
  MLMEASSOC_ACCESS_DENIED = 2,
  MLMEASSOC_CHANNEL_ACCESS_FAILURE = 3,
  MLMEASSOC_NO_ACK = 4,
  MLMEASSOC_NO_DATA = 5,
  MLMEASSOC_COUNTER_ERROR = 6,
  MLMEASSOC_FRAME_TOO_LONG = 7,
  MLMEASSOC_UNSUPPORTED_LEGACY = 8,
  MLMEASSOC_INVALID_PARAMETER = 9
};

enum LrWpanSyncLossReason {
  MLMESYNCLOSS_PAN_ID_CONFLICT = 0,
  MLMESYNCLOSS_REALIGMENT = 1,
  MLMESYNCLOSS_BEACON_LOST = 2,
  MLMESYNCLOSS_SUPERFRAME_OVERLAP = 3
};

enum LrWpanMlmeCommStatus {
  MLMECOMMSTATUS_SUCCESS = 0,
  MLMECOMMSTATUS_TRANSACTION_OVERFLOW = 1,
  MLMECOMMSTATUS_TRANSACTION_EXPIRED = 2,
  MLMECOMMSTATUS_CHANNEL_ACCESS_FAILURE = 3,
  MLMECOMMSTATUS_NO_ACK = 4,
  MLMECOMMSTATUS_COUNTER_ERROR = 5,
  MLMECOMMSTATUS_FRAME_TOO_LONG = 6,
  MLMECOMMSTATUS_INVALID_PARAMETER = 7
};

enum LrWpanMlmePollConfirmStatus {
  MLMEPOLL_SUCCESS = 0,
  MLMEPOLL_CHANNEL_ACCESS_FAILURE = 2,
  MLMEPOLL_NO_ACK = 3,
  MLMEPOLL_NO_DATA = 4,
  MLMEPOLL_COUNTER_ERROR = 5,
  MLMEPOLL_FRAME_TOO_LONG = 6,
  MLMEPOLL_UNAVAILABLE_KEY = 7,
  MLMEPOLL_UNSUPPORTED_SECURITY = 8,
  MLMEPOLL_INVALID_PARAMETER = 9
};

enum LrWpanMlmeSetConfirmStatus {
  MLMESET_SUCCESS = 0,
  MLMESET_READ_ONLY = 1,
  MLMESET_UNSUPPORTED_ATTRIBUTE = 2,
  MLMESET_INVALID_INDEX = 3,
  MLMESET_INVALID_PARAMETER = 4
};

enum LrWpanMlmeGetConfirmStatus {
  MLMEGET_SUCCESS = 0,
  MLMEGET_UNSUPPORTED_ATTRIBUTE = 1
};

enum LrWpanMacPibAttributeIdentifier {
  macBeaconPayload = 0,
  macBeaconPayloadLength = 1,
  macShortAddress = 2,
  macExtendedAddress = 3,
  macPanId = 4,
  unsupported = 255
};

struct LrWpanMacPibAttributes : public SimpleRefCount<LrWpanMacPibAttributes> {
  Ptr<Packet> macBeaconPayload;
  uint8_t macBeaconPayloadLength{0};
  Mac16Address macShortAddress;
  Mac64Address macExtendedAddress;
  uint16_t macPanId;
};

struct PanDescriptor {
  LrWpanAddressMode m_coorAddrMode{SHORT_ADDR};
  uint16_t m_coorPanId{0xffff};
  Mac16Address m_coorShortAddr;
  Mac64Address m_coorExtAddr;
  uint8_t m_logCh{11};
  uint8_t m_logChPage{0};
  SuperframeField m_superframeSpec;
  bool m_gtsPermit{false};
  uint8_t m_linkQuality{0};
  Time m_timeStamp;
};

struct McpsDataRequestParams {
  LrWpanAddressMode m_srcAddrMode{SHORT_ADDR};
  LrWpanAddressMode m_dstAddrMode{SHORT_ADDR};
  uint16_t m_dstPanId{0};
  Mac16Address m_dstAddr;
  Mac64Address m_dstExtAddr;
  uint8_t m_msduHandle{0};
  uint8_t m_txOptions{0};
};

struct McpsDataConfirmParams {
  uint8_t m_msduHandle{0};
  LrWpanMcpsDataConfirmStatus m_status{IEEE_802_15_4_INVALID_PARAMETER};
};

struct McpsDataIndicationParams {
  uint8_t m_srcAddrMode{SHORT_ADDR};
  uint16_t m_srcPanId{0};
  Mac16Address m_srcAddr;
  Mac64Address m_srcExtAddr;
  uint8_t m_dstAddrMode{SHORT_ADDR};
  uint16_t m_dstPanId{0};
  Mac16Address m_dstAddr;
  Mac64Address m_dstExtAddr;
  uint8_t m_mpduLinkQuality{0};
  uint8_t m_dsn{0};
};

struct MlmeAssociateIndicationParams {
  Mac64Address m_extDevAddr;
  CapabilityField capabilityInfo;
  uint8_t lqi{0};
};

struct MlmeAssociateResponseParams {
  Mac64Address m_extDevAddr;
  Mac16Address m_assocShortAddr;
  LrWpanAssociationStatus m_status{DISASSOCIATED};
};

struct MlmeStartRequestParams {
  uint16_t m_PanId{0};
  uint8_t m_logCh{11};
  uint32_t m_logChPage{0};
  uint32_t m_startTime{0};
  uint8_t m_bcnOrd{15};
  uint8_t m_sfrmOrd{15};
  bool m_panCoor{false};
  bool m_battLifeExt{false};
  bool m_coorRealgn{false};
};

struct MlmeSyncRequestParams {
  uint8_t m_logCh{11};
  bool m_trackBcn{false};
};

struct MlmePollRequestParams {
  LrWpanAddressMode m_coorAddrMode{SHORT_ADDR};
  uint16_t m_coorPanId{0};
  Mac16Address m_coorShortAddr;
  Mac64Address m_coorExtAddr;
};

struct MlmeScanRequestParams {
  LrWpanMlmeScanType m_scanType{MLMESCAN_PASSIVE};
  uint32_t m_scanChannels{0x7FFF800};
  uint8_t m_scanDuration{14};
  uint32_t m_chPage{0};
};

struct MlmeScanConfirmParams {
  LrWpanMlmeScanConfirmStatus m_status{MLMESCAN_INVALID_PARAMETER};
  LrWpanMlmeScanType m_scanType{MLMESCAN_PASSIVE};
  uint32_t m_chPage{0};
  std::vector<uint8_t> m_unscannedCh;
  uint8_t m_resultListSize{0};
  std::vector<uint8_t> m_energyDetList;
  std::vector<PanDescriptor> m_panDescList;
};

struct MlmeAssociateRequestParams {
  uint8_t m_chNum{11};
  uint32_t m_chPage{0};
  uint8_t m_coordAddrMode{SHORT_ADDR};
  uint16_t m_coordPanId{0};
  Mac16Address m_coordShortAddr;
  Mac64Address m_coordExtAddr;
  CapabilityField m_capabilityInfo;
};

struct MlmeAssociateConfirmParams {
  Mac16Address m_assocShortAddr;
  LrWpanMlmeAssociateConfirmStatus m_status{MLMEASSOC_INVALID_PARAMETER};
};

struct MlmeStartConfirmParams {
  LrWpanMlmeStartConfirmStatus m_status{MLMESTART_INVALID_PARAMETER};
};

struct MlmeBeaconNotifyIndicationParams {
  uint8_t m_bsn{0};
  PanDescriptor m_panDescriptor;
  uint32_t m_sduLength{0};
  Ptr<Packet> m_sdu;
};

struct MlmeSyncLossIndicationParams {
  LrWpanSyncLossReason m_lossReason{MLMESYNCLOSS_PAN_ID_CONFLICT};
  uint16_t m_panId{0};
  uint8_t m_logCh{11};
};

struct MlmeCommStatusIndicationParams {
  uint16_t m_panId{0};
  uint8_t m_srcAddrMode{SHORT_ADDR};
  Mac16Address m_srcShortAddr;
  Mac64Address m_srcExtAddr;
  uint8_t m_dstAddrMode{SHORT_ADDR};
  Mac16Address m_dstShortAddr;
  Mac64Address m_dstExtAddr;
  LrWpanMlmeCommStatus m_status{MLMECOMMSTATUS_INVALID_PARAMETER};
};

struct MlmeOrphanIndicationParams {
  Mac64Address m_orphanAddr;
};

struct MlmeOrphanResponseParams {
  Mac64Address m_orphanAddr;
  Mac16Address m_shortAddr;
  bool m_assocMember{false};
};

struct MlmePollConfirmParams {
  LrWpanMlmePollConfirmStatus m_status{MLMEPOLL_INVALID_PARAMETER};
};

struct MlmeSetConfirmParams {
  LrWpanMlmeSetConfirmStatus m_status{MLMESET_UNSUPPORTED_ATTRIBUTE};
  LrWpanMacPibAttributeIdentifier id;
};

typedef Callback<void, McpsDataConfirmParams> McpsDataConfirmCallback;

typedef Callback<void, McpsDataIndicationParams, Ptr<Packet>>
    McpsDataIndicationCallback;

typedef Callback<void, MlmeStartConfirmParams> MlmeStartConfirmCallback;

typedef Callback<void, MlmeBeaconNotifyIndicationParams>
    MlmeBeaconNotifyIndicationCallback;

typedef Callback<void, MlmeSyncLossIndicationParams>
    MlmeSyncLossIndicationCallback;

typedef Callback<void, MlmePollConfirmParams> MlmePollConfirmCallback;

typedef Callback<void, MlmeScanConfirmParams> MlmeScanConfirmCallback;

typedef Callback<void, MlmeAssociateConfirmParams> MlmeAssociateConfirmCallback;

typedef Callback<void, MlmeAssociateIndicationParams>
    MlmeAssociateIndicationCallback;

typedef Callback<void, MlmeCommStatusIndicationParams>
    MlmeCommStatusIndicationCallback;

typedef Callback<void, MlmeOrphanIndicationParams> MlmeOrphanIndicationCallback;

typedef Callback<void, MlmeSetConfirmParams> MlmeSetConfirmCallback;

typedef Callback<void, LrWpanMlmeGetConfirmStatus,
                 LrWpanMacPibAttributeIdentifier, Ptr<LrWpanMacPibAttributes>>
    MlmeGetConfirmCallback;

class LrWpanMac : public Object {
public:
  static TypeId GetTypeId();

  LrWpanMac();
  ~LrWpanMac() override;

  bool GetRxOnWhenIdle() const;

  void SetRxOnWhenIdle(bool rxOnWhenIdle);

  void SetShortAddress(Mac16Address address);

  Mac16Address GetShortAddress() const;

  void SetExtendedAddress(Mac64Address address);

  Mac64Address GetExtendedAddress() const;

  void SetPanId(uint16_t panId);

  uint16_t GetPanId() const;

  Mac16Address GetCoordShortAddress() const;

  Mac64Address GetCoordExtAddress() const;

  void McpsDataRequest(McpsDataRequestParams params, Ptr<Packet> p);

  void MlmeStartRequest(MlmeStartRequestParams params);

  void MlmeScanRequest(MlmeScanRequestParams params);

  void MlmeAssociateRequest(MlmeAssociateRequestParams params);

  void MlmeAssociateResponse(MlmeAssociateResponseParams params);

  void MlmeOrphanResponse(MlmeOrphanResponseParams params);

  void MlmeSyncRequest(MlmeSyncRequestParams params);

  void MlmePollRequest(MlmePollRequestParams params);

  void MlmeSetRequest(LrWpanMacPibAttributeIdentifier id,
                      Ptr<LrWpanMacPibAttributes> attribute);

  void MlmeGetRequest(LrWpanMacPibAttributeIdentifier id);

  void SetCsmaCa(Ptr<LrWpanCsmaCa> csmaCa);

  void SetPhy(Ptr<LrWpanPhy> phy);

  Ptr<LrWpanPhy> GetPhy();

  void SetMcpsDataIndicationCallback(McpsDataIndicationCallback c);

  void SetMlmeAssociateIndicationCallback(MlmeAssociateIndicationCallback c);

  void SetMlmeCommStatusIndicationCallback(MlmeCommStatusIndicationCallback c);

  void SetMlmeOrphanIndicationCallback(MlmeOrphanIndicationCallback c);

  void SetMcpsDataConfirmCallback(McpsDataConfirmCallback c);

  void SetMlmeStartConfirmCallback(MlmeStartConfirmCallback c);

  void SetMlmeScanConfirmCallback(MlmeScanConfirmCallback c);

  void SetMlmeAssociateConfirmCallback(MlmeAssociateConfirmCallback c);

  void
  SetMlmeBeaconNotifyIndicationCallback(MlmeBeaconNotifyIndicationCallback c);

  void SetMlmeSyncLossIndicationCallback(MlmeSyncLossIndicationCallback c);

  void SetMlmePollConfirmCallback(MlmePollConfirmCallback c);

  void SetMlmeSetConfirmCallback(MlmeSetConfirmCallback c);

  void SetMlmeGetConfirmCallback(MlmeGetConfirmCallback c);

  void PdDataIndication(uint32_t psduLength, Ptr<Packet> p, uint8_t lqi);

  void PdDataConfirm(LrWpanPhyEnumeration status);

  void PlmeCcaConfirm(LrWpanPhyEnumeration status);

  void PlmeEdConfirm(LrWpanPhyEnumeration status, uint8_t energyLevel);

  void PlmeGetAttributeConfirm(LrWpanPhyEnumeration status,
                               LrWpanPibAttributeIdentifier id,
                               Ptr<LrWpanPhyPibAttributes> attribute);

  void PlmeSetTRXStateConfirm(LrWpanPhyEnumeration status);

  void PlmeSetAttributeConfirm(LrWpanPhyEnumeration status,
                               LrWpanPibAttributeIdentifier id);

  void SetLrWpanMacState(LrWpanMacState macState);

  LrWpanAssociationStatus GetAssociationStatus() const;

  void SetAssociationStatus(LrWpanAssociationStatus status);

  void SetTxQMaxSize(uint32_t queueSize);

  void SetIndTxQMaxSize(uint32_t queueSize);

  Time m_macBeaconTxTime;

  Time m_macBeaconRxTime;

  uint64_t m_macResponseWaitTime;

  uint64_t m_assocRespCmdWaitTime;

  Mac16Address m_macCoordShortAddress;

  Mac64Address m_macCoordExtendedAddress;

  uint64_t m_macSyncSymbolOffset;

  uint8_t m_macBeaconOrder;

  uint8_t m_macSuperframeOrder;

  uint16_t m_macTransactionPersistenceTime;

  uint64_t m_rxBeaconSymbols;

  uint8_t m_fnlCapSlot;

  uint8_t m_incomingBeaconOrder;

  uint8_t m_incomingSuperframeOrder;

  uint8_t m_incomingFnlCapSlot;

  bool m_macPromiscuousMode;

  uint16_t m_macPanId;

  uint16_t m_macPanIdScan;

  SequenceNumber8 m_macDsn;

  SequenceNumber8 m_macBsn;

  Ptr<Packet> m_macBeaconPayload;

  uint32_t m_macBeaconPayloadLength;

  uint8_t m_macMaxFrameRetries;

  bool m_macRxOnWhenIdle;

  uint32_t m_macLIFSPeriod;

  uint32_t m_macSIFSPeriod;

  bool m_macAssociationPermit;

  bool m_macAutoRequest;

  uint8_t m_maxEnergyLevel;

  uint32_t m_ifs;

  bool m_panCoor;

  bool m_coor;

  uint32_t m_beaconInterval;

  uint32_t m_superframeDuration;

  uint32_t m_incomingBeaconInterval;

  uint32_t m_incomingSuperframeDuration;

  uint8_t m_deviceCapability;

  bool m_beaconTrackingOn;

  uint8_t m_numLostBeacons;

  uint64_t GetMacAckWaitDuration() const;

  uint8_t GetMacMaxFrameRetries() const;

  void PrintTransmitQueueSize();

  void SetMacMaxFrameRetries(uint8_t retries);

  bool isCoordDest();

  void SetAssociatedCoor(Mac16Address mac);

  void SetAssociatedCoor(Mac64Address mac);

  uint32_t GetIfsSize();

  uint64_t GetTxPacketSymbols();

  bool isTxAckReq();

  void PrintPendingTxQueue(std::ostream &os) const;

  void PrintTxQueue(std::ostream &os) const;

  typedef void (*SentTracedCallback)(Ptr<const Packet> packet, uint8_t retries,
                                     uint8_t backoffs);

  typedef void (*StateTracedCallback)(LrWpanMacState oldState,
                                      LrWpanMacState newState);

protected:
  void DoInitialize() override;
  void DoDispose() override;

private:
  struct TxQueueElement : public SimpleRefCount<TxQueueElement> {
    uint8_t txQMsduHandle;
    Ptr<Packet> txQPkt;
  };

  struct IndTxQueueElement : public SimpleRefCount<IndTxQueueElement> {
    uint8_t seqNum;
    Mac16Address dstShortAddress;
    Mac64Address dstExtAddress;
    Ptr<Packet> txQPkt;
    Time expireTime;
  };

  void SendOneBeacon();

  void SendAssocRequestCommand();

  void SendDataRequestCommand();

  void SendAssocResponseCommand(Ptr<Packet> rxDataReqPkt);

  void LostAssocRespCommand();

  void SendBeaconRequestCommand();

  void SendOrphanNotificationCommand();

  void EndStartRequest();

  void EndChannelScan();

  void EndChannelEnergyScan();

  void EndAssociateRequest();

  void StartCFP(SuperframeType superframeType);

  void StartCAP(SuperframeType superframeType);

  void StartInactivePeriod(SuperframeType superframeType);

  void AwaitBeacon();

  void BeaconSearchTimeout();

  void SendAck(uint8_t seqno);

  void EnqueueTxQElement(Ptr<TxQueueElement> txQElement);

  void RemoveFirstTxQElement();

  void ChangeMacState(LrWpanMacState newState);

  void AckWaitTimeout();

  void IfsWaitTimeout(Time ifsTime);

  bool PrepareRetransmission();

  void EnqueueInd(Ptr<Packet> p);

  bool DequeueInd(Mac64Address dst, Ptr<IndTxQueueElement> entry);

  void PurgeInd();

  void RemovePendTxQElement(Ptr<Packet> p);

  void CheckQueue();

  SuperframeField GetSuperframeField();

  GtsFields GetGtsFields();

  PendingAddrFields GetPendingAddrFields();

  TracedCallback<Time> m_macIfsEndTrace;

  TracedCallback<Ptr<const Packet>, uint8_t, uint8_t> m_sentPktTrace;

  TracedCallback<Ptr<const Packet>> m_macTxEnqueueTrace;

  TracedCallback<Ptr<const Packet>> m_macTxDequeueTrace;

  TracedCallback<Ptr<const Packet>> m_macIndTxEnqueueTrace;

  TracedCallback<Ptr<const Packet>> m_macIndTxDequeueTrace;

  TracedCallback<Ptr<const Packet>> m_macTxTrace;

  TracedCallback<Ptr<const Packet>> m_macTxOkTrace;

  TracedCallback<Ptr<const Packet>> m_macTxDropTrace;

  TracedCallback<Ptr<const Packet>> m_macIndTxDropTrace;

  TracedCallback<Ptr<const Packet>> m_macPromiscRxTrace;

  TracedCallback<Ptr<const Packet>> m_macRxTrace;

  TracedCallback<Ptr<const Packet>> m_macRxDropTrace;

  TracedCallback<Ptr<const Packet>> m_snifferTrace;

  TracedCallback<Ptr<const Packet>> m_promiscSnifferTrace;

  TracedCallback<LrWpanMacState, LrWpanMacState> m_macStateLogger;

  Ptr<LrWpanPhy> m_phy;

  Ptr<LrWpanCsmaCa> m_csmaCa;

  MlmeSetConfirmCallback m_mlmeSetConfirmCallback;

  MlmeGetConfirmCallback m_mlmeGetConfirmCallback;

  MlmeBeaconNotifyIndicationCallback m_mlmeBeaconNotifyIndicationCallback;

  MlmeSyncLossIndicationCallback m_mlmeSyncLossIndicationCallback;

  MlmeScanConfirmCallback m_mlmeScanConfirmCallback;

  MlmeAssociateConfirmCallback m_mlmeAssociateConfirmCallback;

  MlmePollConfirmCallback m_mlmePollConfirmCallback;

  MlmeStartConfirmCallback m_mlmeStartConfirmCallback;

  McpsDataIndicationCallback m_mcpsDataIndicationCallback;

  MlmeAssociateIndicationCallback m_mlmeAssociateIndicationCallback;

  MlmeCommStatusIndicationCallback m_mlmeCommStatusIndicationCallback;

  MlmeOrphanIndicationCallback m_mlmeOrphanIndicationCallback;

  McpsDataConfirmCallback m_mcpsDataConfirmCallback;

  TracedValue<LrWpanMacState> m_lrWpanMacState;

  TracedValue<SuperframeStatus> m_incSuperframeStatus;

  TracedValue<SuperframeStatus> m_outSuperframeStatus;

  LrWpanAssociationStatus m_associationStatus;

  Ptr<Packet> m_txPkt;

  Ptr<Packet> m_rxPkt;

  Mac16Address m_shortAddress;

  Mac64Address m_selfExt;

  std::deque<Ptr<TxQueueElement>> m_txQueue;

  std::deque<Ptr<IndTxQueueElement>> m_indTxQueue;

  uint32_t m_maxTxQueueSize;

  uint32_t m_maxIndTxQueueSize;

  std::vector<PanDescriptor> m_panDescriptorList;

  std::vector<uint8_t> m_energyDetectList;

  std::vector<uint8_t> m_unscannedChannels;

  MlmeScanRequestParams m_scanParams;

  MlmeStartRequestParams m_startParams;

  MlmeAssociateRequestParams m_associateParams;

  uint16_t m_channelScanIndex;

  PendingPrimitiveStatus m_pendPrimitive;

  uint8_t m_retransmission;

  uint8_t m_numCsmacaRetry;

  uint8_t m_lastRxFrameLqi;

  EventId m_ackWaitTimeout;

  EventId m_respWaitTimeout;

  EventId m_assocResCmdWaitTimeout;

  EventId m_setMacState;

  EventId m_ifsEvent;

  EventId m_beaconEvent;

  EventId m_capEvent;

  EventId m_cfpEvent;

  EventId m_incCapEvent;

  EventId m_incCfpEvent;

  EventId m_trackingEvent;

  EventId m_scanEvent;

  EventId m_scanOrphanEvent;

  EventId m_scanEnergyEvent;
};
} // namespace ns3

#endif
