#include "lr-wpan-mac.h"

#include "lr-wpan-constants.h"
#include "lr-wpan-csmaca.h"
#include "lr-wpan-mac-header.h"
#include "lr-wpan-mac-pl-headers.h"
#include "lr-wpan-mac-trailer.h"

#include <ns3/double.h>
#include <ns3/log.h>
#include <ns3/node.h>
#include <ns3/packet.h>
#include <ns3/random-variable-stream.h>
#include <ns3/simulator.h>
#include <ns3/uinteger.h>

#undef NS_LOG_APPEND_CONTEXT
#define NS_LOG_APPEND_CONTEXT                                                  \
  std::clog << "[address " << m_shortAddress << " | " << m_selfExt << "] ";

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("LrWpanMac");
NS_OBJECT_ENSURE_REGISTERED(LrWpanMac);

TypeId LrWpanMac::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::LrWpanMac")
          .SetParent<Object>()
          .SetGroupName("LrWpan")
          .AddConstructor<LrWpanMac>()
          .AddAttribute("PanId", "16-bit identifier of the associated PAN",
                        UintegerValue(),
                        MakeUintegerAccessor(&LrWpanMac::m_macPanId),
                        MakeUintegerChecker<uint16_t>())
          .AddTraceSource(
              "MacTxEnqueue",
              "Trace source indicating a packet has been "
              "enqueued in the transaction queue",
              MakeTraceSourceAccessor(&LrWpanMac::m_macTxEnqueueTrace),
              "ns3::Packet::TracedCallback")
          .AddTraceSource(
              "MacTxDequeue",
              "Trace source indicating a packet has was "
              "dequeued from the transaction queue",
              MakeTraceSourceAccessor(&LrWpanMac::m_macTxDequeueTrace),
              "ns3::Packet::TracedCallback")
          .AddTraceSource(
              "MacIndTxEnqueue",
              "Trace source indicating a packet has been "
              "enqueued in the indirect transaction queue",
              MakeTraceSourceAccessor(&LrWpanMac::m_macIndTxEnqueueTrace),
              "ns3::Packet::TracedCallback")
          .AddTraceSource(
              "MacIndTxDequeue",
              "Trace source indicating a packet has was "
              "dequeued from the indirect transaction queue",
              MakeTraceSourceAccessor(&LrWpanMac::m_macIndTxDequeueTrace),
              "ns3::Packet::TracedCallback")
          .AddTraceSource("MacTx",
                          "Trace source indicating a packet has "
                          "arrived for transmission by this device",
                          MakeTraceSourceAccessor(&LrWpanMac::m_macTxTrace),
                          "ns3::Packet::TracedCallback")
          .AddTraceSource("MacTxOk",
                          "Trace source indicating a packet has been "
                          "successfully sent",
                          MakeTraceSourceAccessor(&LrWpanMac::m_macTxOkTrace),
                          "ns3::Packet::TracedCallback")
          .AddTraceSource("MacTxDrop",
                          "Trace source indicating a packet has been "
                          "dropped during transmission",
                          MakeTraceSourceAccessor(&LrWpanMac::m_macTxDropTrace),
                          "ns3::Packet::TracedCallback")
          .AddTraceSource(
              "MacIndTxDrop",
              "Trace source indicating a packet has been "
              "dropped from the indirect transaction queue"
              "(The pending transaction list)",
              MakeTraceSourceAccessor(&LrWpanMac::m_macIndTxDropTrace),
              "ns3::Packet::TracedCallback")
          .AddTraceSource(
              "MacPromiscRx",
              "A packet has been received by this device, "
              "has been passed up from the physical layer "
              "and is being forwarded up the local protocol stack.  "
              "This is a promiscuous trace,",
              MakeTraceSourceAccessor(&LrWpanMac::m_macPromiscRxTrace),
              "ns3::Packet::TracedCallback")
          .AddTraceSource(
              "MacRx",
              "A packet has been received by this device, "
              "has been passed up from the physical layer "
              "and is being forwarded up the local protocol stack.  "
              "This is a non-promiscuous trace,",
              MakeTraceSourceAccessor(&LrWpanMac::m_macRxTrace),
              "ns3::Packet::TracedCallback")
          .AddTraceSource("MacRxDrop",
                          "Trace source indicating a packet was received, "
                          "but dropped before being forwarded up the stack",
                          MakeTraceSourceAccessor(&LrWpanMac::m_macRxDropTrace),
                          "ns3::Packet::TracedCallback")
          .AddTraceSource("Sniffer",
                          "Trace source simulating a non-promiscuous "
                          "packet sniffer attached to the device",
                          MakeTraceSourceAccessor(&LrWpanMac::m_snifferTrace),
                          "ns3::Packet::TracedCallback")
          .AddTraceSource(
              "PromiscSniffer",
              "Trace source simulating a promiscuous "
              "packet sniffer attached to the device",
              MakeTraceSourceAccessor(&LrWpanMac::m_promiscSnifferTrace),
              "ns3::Packet::TracedCallback")
          .AddTraceSource("MacStateValue", "The state of LrWpan Mac",
                          MakeTraceSourceAccessor(&LrWpanMac::m_lrWpanMacState),
                          "ns3::TracedValueCallback::LrWpanMacState")
          .AddTraceSource(
              "MacIncSuperframeStatus",
              "The period status of the incoming superframe",
              MakeTraceSourceAccessor(&LrWpanMac::m_incSuperframeStatus),
              "ns3::TracedValueCallback::SuperframeState")
          .AddTraceSource(
              "MacOutSuperframeStatus",
              "The period status of the outgoing superframe",
              MakeTraceSourceAccessor(&LrWpanMac::m_outSuperframeStatus),
              "ns3::TracedValueCallback::SuperframeState")
          .AddTraceSource("MacState", "The state of LrWpan Mac",
                          MakeTraceSourceAccessor(&LrWpanMac::m_macStateLogger),
                          "ns3::LrWpanMac::StateTracedCallback")
          .AddTraceSource("MacSentPkt",
                          "Trace source reporting some information about "
                          "the sent packet",
                          MakeTraceSourceAccessor(&LrWpanMac::m_sentPktTrace),
                          "ns3::LrWpanMac::SentTracedCallback")
          .AddTraceSource("IfsEnd",
                          "Trace source reporting the end of an "
                          "Interframe space (IFS)",
                          MakeTraceSourceAccessor(&LrWpanMac::m_macIfsEndTrace),
                          "ns3::Packet::TracedCallback");
  return tid;
}

LrWpanMac::LrWpanMac() {
  m_lrWpanMacState = MAC_IDLE;

  ChangeMacState(MAC_IDLE);

  m_incSuperframeStatus = INACTIVE;
  m_outSuperframeStatus = INACTIVE;

  m_macRxOnWhenIdle = true;
  m_macPanId = 0xffff;
  m_macCoordShortAddress = Mac16Address("ff:ff");
  m_macCoordExtendedAddress = Mac64Address("ff:ff:ff:ff:ff:ff:ff:ed");
  m_deviceCapability = DeviceType::FFD;
  m_associationStatus = ASSOCIATED;
  m_selfExt = Mac64Address::Allocate();
  m_macPromiscuousMode = false;
  m_macMaxFrameRetries = 3;
  m_retransmission = 0;
  m_numCsmacaRetry = 0;
  m_txPkt = nullptr;
  m_rxPkt = nullptr;
  m_lastRxFrameLqi = 0;
  m_ifs = 0;

  m_macLIFSPeriod = 40;
  m_macSIFSPeriod = 12;

  m_panCoor = false;
  m_coor = false;
  m_macBeaconOrder = 15;
  m_macSuperframeOrder = 15;
  m_macTransactionPersistenceTime = 500;
  m_macAssociationPermit = true;
  m_macAutoRequest = true;

  m_incomingBeaconOrder = 15;
  m_incomingSuperframeOrder = 15;
  m_beaconTrackingOn = false;
  m_numLostBeacons = 0;

  m_pendPrimitive = MLME_NONE;
  m_channelScanIndex = 0;
  m_maxEnergyLevel = 0;

  m_macResponseWaitTime = lrwpan::aBaseSuperframeDuration * 32;
  m_assocRespCmdWaitTime = 960;

  m_maxTxQueueSize = m_txQueue.max_size();
  m_maxIndTxQueueSize = m_indTxQueue.max_size();

  Ptr<UniformRandomVariable> uniformVar = CreateObject<UniformRandomVariable>();
  uniformVar->SetAttribute("Min", DoubleValue(0.0));
  uniformVar->SetAttribute("Max", DoubleValue(255.0));
  m_macDsn = SequenceNumber8(uniformVar->GetValue());
  m_macBsn = SequenceNumber8(uniformVar->GetValue());
  m_macBeaconPayload = nullptr;
  m_macBeaconPayloadLength = 0;
  m_shortAddress = Mac16Address("FF:FF");
}

LrWpanMac::~LrWpanMac() {}

void LrWpanMac::DoInitialize() {
  if (m_macRxOnWhenIdle) {
    m_phy->PlmeSetTRXStateRequest(IEEE_802_15_4_PHY_RX_ON);
  } else {
    m_phy->PlmeSetTRXStateRequest(IEEE_802_15_4_PHY_TRX_OFF);
  }

  Object::DoInitialize();
}

void LrWpanMac::DoDispose() {
  if (m_csmaCa) {
    m_csmaCa->Dispose();
    m_csmaCa = nullptr;
  }
  m_txPkt = nullptr;

  for (uint32_t i = 0; i < m_txQueue.size(); i++) {
    m_txQueue[i]->txQPkt = nullptr;
    m_txQueue[i]->txQMsduHandle = 0;
  }
  m_txQueue.clear();

  for (uint32_t i = 0; i < m_indTxQueue.size(); i++) {
    m_indTxQueue[i]->txQPkt = nullptr;
    m_indTxQueue[i]->seqNum = 0;
    m_indTxQueue[i]->dstExtAddress = nullptr;
    m_indTxQueue[i]->dstShortAddress = nullptr;
  }
  m_indTxQueue.clear();

  m_phy = nullptr;
  m_mcpsDataConfirmCallback = MakeNullCallback<void, McpsDataConfirmParams>();
  m_mcpsDataIndicationCallback =
      MakeNullCallback<void, McpsDataIndicationParams, Ptr<Packet>>();
  m_mlmeStartConfirmCallback = MakeNullCallback<void, MlmeStartConfirmParams>();
  m_mlmeBeaconNotifyIndicationCallback =
      MakeNullCallback<void, MlmeBeaconNotifyIndicationParams>();
  m_mlmeSyncLossIndicationCallback =
      MakeNullCallback<void, MlmeSyncLossIndicationParams>();
  m_mlmePollConfirmCallback = MakeNullCallback<void, MlmePollConfirmParams>();
  m_mlmeScanConfirmCallback = MakeNullCallback<void, MlmeScanConfirmParams>();
  m_mlmeAssociateConfirmCallback =
      MakeNullCallback<void, MlmeAssociateConfirmParams>();
  m_mlmeAssociateIndicationCallback =
      MakeNullCallback<void, MlmeAssociateIndicationParams>();
  m_mlmeCommStatusIndicationCallback =
      MakeNullCallback<void, MlmeCommStatusIndicationParams>();
  m_mlmeOrphanIndicationCallback =
      MakeNullCallback<void, MlmeOrphanIndicationParams>();

  m_panDescriptorList.clear();
  m_energyDetectList.clear();
  m_unscannedChannels.clear();

  m_scanEvent.Cancel();
  m_scanEnergyEvent.Cancel();
  m_scanOrphanEvent.Cancel();
  m_beaconEvent.Cancel();

  Object::DoDispose();
}

bool LrWpanMac::GetRxOnWhenIdle() const { return m_macRxOnWhenIdle; }

void LrWpanMac::SetRxOnWhenIdle(bool rxOnWhenIdle) {
  NS_LOG_FUNCTION(this << rxOnWhenIdle);
  m_macRxOnWhenIdle = rxOnWhenIdle;

  if (m_lrWpanMacState == MAC_IDLE) {
    if (m_macRxOnWhenIdle) {
      m_phy->PlmeSetTRXStateRequest(IEEE_802_15_4_PHY_RX_ON);
    } else {
      m_phy->PlmeSetTRXStateRequest(IEEE_802_15_4_PHY_TRX_OFF);
    }
  }
}

void LrWpanMac::SetShortAddress(Mac16Address address) {
  NS_LOG_FUNCTION(this << address);
  m_shortAddress = address;
}

void LrWpanMac::SetExtendedAddress(Mac64Address address) {
  NS_LOG_FUNCTION(this << address);
  m_selfExt = address;
}

Mac16Address LrWpanMac::GetShortAddress() const {
  NS_LOG_FUNCTION(this);
  return m_shortAddress;
}

Mac64Address LrWpanMac::GetExtendedAddress() const {
  NS_LOG_FUNCTION(this);
  return m_selfExt;
}

void LrWpanMac::McpsDataRequest(McpsDataRequestParams params, Ptr<Packet> p) {
  NS_LOG_FUNCTION(this << p);

  McpsDataConfirmParams confirmParams;
  confirmParams.m_msduHandle = params.m_msduHandle;

  LrWpanMacHeader macHdr(LrWpanMacHeader::LRWPAN_MAC_DATA, m_macDsn.GetValue());
  m_macDsn++;

  if (p->GetSize() > lrwpan::aMaxPhyPacketSize - lrwpan::aMinMPDUOverhead) {
    NS_LOG_ERROR(this << " packet too big: " << p->GetSize());
    confirmParams.m_status = IEEE_802_15_4_FRAME_TOO_LONG;
    if (!m_mcpsDataConfirmCallback.IsNull()) {
      m_mcpsDataConfirmCallback(confirmParams);
    }
    return;
  }

  if ((params.m_srcAddrMode == NO_PANID_ADDR) &&
      (params.m_dstAddrMode == NO_PANID_ADDR)) {
    NS_LOG_ERROR(this << " Can not send packet with no Address field");
    confirmParams.m_status = IEEE_802_15_4_INVALID_ADDRESS;
    if (!m_mcpsDataConfirmCallback.IsNull()) {
      m_mcpsDataConfirmCallback(confirmParams);
    }
    return;
  }
  switch (params.m_srcAddrMode) {
  case NO_PANID_ADDR:
    macHdr.SetSrcAddrMode(params.m_srcAddrMode);
    macHdr.SetNoPanIdComp();
    break;
  case ADDR_MODE_RESERVED:
    NS_ABORT_MSG(
        "Can not set source address type to ADDR_MODE_RESERVED. Aborting.");
    break;
  case SHORT_ADDR:
    macHdr.SetSrcAddrMode(params.m_srcAddrMode);
    macHdr.SetSrcAddrFields(GetPanId(), GetShortAddress());
    break;
  case EXT_ADDR:
    macHdr.SetSrcAddrMode(params.m_srcAddrMode);
    macHdr.SetSrcAddrFields(GetPanId(), GetExtendedAddress());
    break;
  default:
    NS_LOG_ERROR(
        this << " Can not send packet with incorrect Source Address mode = "
             << params.m_srcAddrMode);
    confirmParams.m_status = IEEE_802_15_4_INVALID_ADDRESS;
    if (!m_mcpsDataConfirmCallback.IsNull()) {
      m_mcpsDataConfirmCallback(confirmParams);
    }
    return;
  }
  switch (params.m_dstAddrMode) {
  case NO_PANID_ADDR:
    macHdr.SetDstAddrMode(params.m_dstAddrMode);
    macHdr.SetNoPanIdComp();
    break;
  case ADDR_MODE_RESERVED:
    NS_ABORT_MSG("Can not set destination address type to ADDR_MODE_RESERVED. "
                 "Aborting.");
    break;
  case SHORT_ADDR:
    macHdr.SetDstAddrMode(params.m_dstAddrMode);
    macHdr.SetDstAddrFields(params.m_dstPanId, params.m_dstAddr);
    break;
  case EXT_ADDR:
    macHdr.SetDstAddrMode(params.m_dstAddrMode);
    macHdr.SetDstAddrFields(params.m_dstPanId, params.m_dstExtAddr);
    break;
  default:
    NS_LOG_ERROR(
        this
        << " Can not send packet with incorrect Destination Address mode = "
        << params.m_dstAddrMode);
    confirmParams.m_status = IEEE_802_15_4_INVALID_ADDRESS;
    if (!m_mcpsDataConfirmCallback.IsNull()) {
      m_mcpsDataConfirmCallback(confirmParams);
    }
    return;
  }

  if ((params.m_dstAddrMode != NO_PANID_ADDR &&
       params.m_srcAddrMode != NO_PANID_ADDR) &&
      (macHdr.GetDstPanId() == macHdr.GetSrcPanId())) {
    macHdr.SetPanIdComp();
  }

  macHdr.SetSecDisable();
  int b0 = params.m_txOptions & TX_OPTION_ACK;
  int b1 = params.m_txOptions & TX_OPTION_GTS;
  int b2 = params.m_txOptions & TX_OPTION_INDIRECT;

  if (b0 == TX_OPTION_ACK) {
    if (macHdr.GetDstAddrMode() == SHORT_ADDR) {
      Mac16Address shortAddr = macHdr.GetShortDstAddr();
      if (shortAddr.IsBroadcast() || shortAddr.IsMulticast()) {
        NS_LOG_LOGIC(
            "LrWpanMac::McpsDataRequest: requested an ACK on broadcast or "
            "multicast destination ("
            << shortAddr << ") - forcefully removing it.");
        macHdr.SetNoAckReq();
        params.m_txOptions &= ~uint8_t(TX_OPTION_ACK);
      } else {
        macHdr.SetAckReq();
      }
    } else {
      macHdr.SetAckReq();
    }
  } else {
    macHdr.SetNoAckReq();
  }

  if (b1 == TX_OPTION_GTS) {
  } else if (b2 == TX_OPTION_INDIRECT) {

    NS_ASSERT(m_coor);
    p->AddHeader(macHdr);

    LrWpanMacTrailer macTrailer;
    if (Node::ChecksumEnabled()) {
      macTrailer.EnableFcs(true);
      macTrailer.SetFcs(p);
    }
    p->AddTrailer(macTrailer);

    NS_LOG_ERROR(this << " Indirect transmissions not currently supported");
  } else {

    p->AddHeader(macHdr);

    LrWpanMacTrailer macTrailer;
    if (Node::ChecksumEnabled()) {
      macTrailer.EnableFcs(true);
      macTrailer.SetFcs(p);
    }
    p->AddTrailer(macTrailer);

    Ptr<TxQueueElement> txQElement = Create<TxQueueElement>();
    txQElement->txQMsduHandle = params.m_msduHandle;
    txQElement->txQPkt = p;
    EnqueueTxQElement(txQElement);
    CheckQueue();
  }
}

void LrWpanMac::MlmeStartRequest(MlmeStartRequestParams params) {
  NS_LOG_FUNCTION(this);
  NS_ASSERT(m_deviceCapability == DeviceType::FFD);

  MlmeStartConfirmParams confirmParams;

  if (GetShortAddress() == Mac16Address("ff:ff")) {
    NS_LOG_ERROR(this << " Invalid MAC short address");
    confirmParams.m_status = MLMESTART_NO_SHORT_ADDRESS;
    if (!m_mlmeStartConfirmCallback.IsNull()) {
      m_mlmeStartConfirmCallback(confirmParams);
    }
    return;
  }

  if ((params.m_bcnOrd > 15) || (params.m_sfrmOrd > params.m_bcnOrd)) {
    confirmParams.m_status = MLMESTART_INVALID_PARAMETER;
    if (!m_mlmeStartConfirmCallback.IsNull()) {
      m_mlmeStartConfirmCallback(confirmParams);
    }
    NS_LOG_ERROR(this << "Incorrect superframe order or beacon order.");
    return;
  }

  m_pendPrimitive = MLME_START_REQ;
  m_startParams = params;

  Ptr<LrWpanPhyPibAttributes> pibAttr = Create<LrWpanPhyPibAttributes>();
  pibAttr->phyCurrentPage = m_startParams.m_logChPage;
  m_phy->PlmeSetAttributeRequest(LrWpanPibAttributeIdentifier::phyCurrentPage,
                                 pibAttr);
}

void LrWpanMac::MlmeScanRequest(MlmeScanRequestParams params) {
  NS_LOG_FUNCTION(this);

  MlmeScanConfirmParams confirmParams;
  confirmParams.m_scanType = params.m_scanType;
  confirmParams.m_chPage = params.m_chPage;

  if ((m_scanEvent.IsRunning() || m_scanEnergyEvent.IsRunning()) ||
      m_scanOrphanEvent.IsRunning()) {
    if (!m_mlmeScanConfirmCallback.IsNull()) {
      confirmParams.m_status = MLMESCAN_SCAN_IN_PROGRESS;
      m_mlmeScanConfirmCallback(confirmParams);
    }
    NS_LOG_ERROR(this << " A channel scan is already in progress");
    return;
  }

  if (params.m_scanDuration > 14 || params.m_scanType > MLMESCAN_ORPHAN) {
    if (!m_mlmeScanConfirmCallback.IsNull()) {
      confirmParams.m_status = MLMESCAN_INVALID_PARAMETER;
      m_mlmeScanConfirmCallback(confirmParams);
    }
    NS_LOG_ERROR(this << "Invalid scan duration or unsupported scan type");
    return;
  }
  m_macPanIdScan = m_macPanId;
  m_macPanId = 0xFFFF;

  m_panDescriptorList.clear();
  m_energyDetectList.clear();
  m_unscannedChannels.clear();

  m_csmaCa->Cancel();
  m_capEvent.Cancel();
  m_cfpEvent.Cancel();
  m_incCapEvent.Cancel();
  m_incCfpEvent.Cancel();
  m_trackingEvent.Cancel();
  m_csmaCa->SetUnSlottedCsmaCa();

  m_channelScanIndex = 0;

  m_scanParams = params;
  m_pendPrimitive = MLME_SCAN_REQ;

  Ptr<LrWpanPhyPibAttributes> pibAttr = Create<LrWpanPhyPibAttributes>();
  pibAttr->phyCurrentPage = params.m_chPage;
  m_phy->PlmeSetAttributeRequest(LrWpanPibAttributeIdentifier::phyCurrentPage,
                                 pibAttr);
}

void LrWpanMac::MlmeAssociateRequest(MlmeAssociateRequestParams params) {
  NS_LOG_FUNCTION(this);

  m_pendPrimitive = MLME_ASSOC_REQ;
  m_associateParams = params;
  bool invalidRequest = false;

  if (params.m_coordPanId == 0xffff) {
    invalidRequest = true;
  }

  if (!invalidRequest && params.m_coordAddrMode == SHORT_ADDR) {
    if (params.m_coordShortAddr == Mac16Address("ff:ff") ||
        params.m_coordShortAddr == Mac16Address("ff:fe")) {
      invalidRequest = true;
    }
  } else if (!invalidRequest && params.m_coordAddrMode == EXT_ADDR) {
    if (params.m_coordExtAddr == Mac64Address("ff:ff:ff:ff:ff:ff:ff:ff") ||
        params.m_coordExtAddr == Mac64Address("ff:ff:ff:ff:ff:ff:ff:ed")) {
      invalidRequest = true;
    }
  }

  if (invalidRequest) {
    m_pendPrimitive = MLME_NONE;
    m_associateParams = MlmeAssociateRequestParams();
    NS_LOG_ERROR(this << " Invalid PAN id in Association request");
    if (!m_mlmeAssociateConfirmCallback.IsNull()) {
      MlmeAssociateConfirmParams confirmParams;
      confirmParams.m_assocShortAddr = Mac16Address("FF:FF");
      confirmParams.m_status = MLMEASSOC_INVALID_PARAMETER;
      m_mlmeAssociateConfirmCallback(confirmParams);
    }
  } else {
    Ptr<LrWpanPhyPibAttributes> pibAttr = Create<LrWpanPhyPibAttributes>();
    pibAttr->phyCurrentPage = params.m_chPage;
    m_phy->PlmeSetAttributeRequest(LrWpanPibAttributeIdentifier::phyCurrentPage,
                                   pibAttr);
  }
}

void LrWpanMac::EndAssociateRequest() {
  m_pendPrimitive = MLME_NONE;
  m_macPanId = m_associateParams.m_coordPanId;
  if (m_associateParams.m_coordAddrMode == SHORT_ADDR) {
    m_macCoordShortAddress = m_associateParams.m_coordShortAddr;
  } else {
    m_macCoordExtendedAddress = m_associateParams.m_coordExtAddr;
    m_macCoordShortAddress = Mac16Address("ff:fe");
  }

  SendAssocRequestCommand();
}

void LrWpanMac::MlmeAssociateResponse(MlmeAssociateResponseParams params) {

  NS_LOG_FUNCTION(this);

  LrWpanMacHeader macHdr(LrWpanMacHeader::LRWPAN_MAC_COMMAND,
                         m_macDsn.GetValue());
  m_macDsn++;
  LrWpanMacTrailer macTrailer;
  Ptr<Packet> commandPacket = Create<Packet>();

  macHdr.SetDstAddrMode(LrWpanMacHeader::EXTADDR);
  macHdr.SetSrcAddrMode(LrWpanMacHeader::EXTADDR);
  macHdr.SetPanIdComp();
  macHdr.SetDstAddrFields(m_macPanId, params.m_extDevAddr);
  macHdr.SetSrcAddrFields(0xffff, GetExtendedAddress());

  CommandPayloadHeader macPayload(CommandPayloadHeader::ASSOCIATION_RESP);
  macPayload.SetShortAddr(params.m_assocShortAddr);
  switch (params.m_status) {
  case LrWpanAssociationStatus::ASSOCIATED:
    macPayload.SetAssociationStatus(CommandPayloadHeader::SUCCESSFUL);
    break;
  case LrWpanAssociationStatus::PAN_AT_CAPACITY:
    macPayload.SetAssociationStatus(CommandPayloadHeader::FULL_CAPACITY);
    break;
  case LrWpanAssociationStatus::PAN_ACCESS_DENIED:
    macPayload.SetAssociationStatus(CommandPayloadHeader::ACCESS_DENIED);
    break;
  case LrWpanAssociationStatus::ASSOCIATED_WITHOUT_ADDRESS:
    NS_LOG_ERROR("Error, Associated without address");
    break;
  case LrWpanAssociationStatus::DISASSOCIATED:
    NS_LOG_ERROR("Error, device not associated");
    break;
  }

  macHdr.SetSecDisable();
  macHdr.SetAckReq();

  commandPacket->AddHeader(macPayload);
  commandPacket->AddHeader(macHdr);

  if (Node::ChecksumEnabled()) {
    macTrailer.EnableFcs(true);
    macTrailer.SetFcs(commandPacket);
  }

  commandPacket->AddTrailer(macTrailer);

  EnqueueInd(commandPacket);
}

void LrWpanMac::MlmeOrphanResponse(MlmeOrphanResponseParams params) {
  NS_LOG_FUNCTION(this);
  LrWpanMacHeader macHdr(LrWpanMacHeader::LRWPAN_MAC_COMMAND,
                         m_macDsn.GetValue());
  m_macDsn++;
  LrWpanMacTrailer macTrailer;
  Ptr<Packet> commandPacket = Create<Packet>();
  macHdr.SetPanIdComp();
  macHdr.SetDstAddrMode(LrWpanMacHeader::EXTADDR);
  macHdr.SetDstAddrFields(0xffff, params.m_orphanAddr);

  macHdr.SetSrcAddrMode(LrWpanMacHeader::EXTADDR);
  macHdr.SetSrcAddrFields(m_macPanId, GetExtendedAddress());
  macHdr.SetSrcAddrFields(m_macPanId, Mac16Address("FF:FF"));

  macHdr.SetFrameVer(0x01);
  macHdr.SetSecDisable();
  macHdr.SetAckReq();

  CommandPayloadHeader macPayload(CommandPayloadHeader::COOR_REALIGN);
  macPayload.SetPanId(m_macPanId);
  macPayload.SetCoordShortAddr(GetShortAddress());
  macPayload.SetChannel(m_phy->GetCurrentChannelNum());
  macPayload.SetPage(m_phy->GetCurrentPage());

  if (params.m_assocMember) {

    macPayload.SetShortAddr(params.m_shortAddr);
  } else {
    macPayload.SetShortAddr(Mac16Address("FF:FF"));
  }

  commandPacket->AddHeader(macPayload);
  commandPacket->AddHeader(macHdr);

  if (Node::ChecksumEnabled()) {
    macTrailer.EnableFcs(true);
    macTrailer.SetFcs(commandPacket);
  }

  commandPacket->AddTrailer(macTrailer);

  Ptr<TxQueueElement> txQElement = Create<TxQueueElement>();
  txQElement->txQPkt = commandPacket;
  EnqueueTxQElement(txQElement);
  CheckQueue();
}

void LrWpanMac::MlmeSyncRequest(MlmeSyncRequestParams params) {
  NS_LOG_FUNCTION(this);
  NS_ASSERT(params.m_logCh <= 26 && m_macPanId != 0xffff);

  auto symbolRate = (uint64_t)m_phy->GetDataOrSymbolRate(false);
  Ptr<LrWpanPhyPibAttributes> pibAttr = Create<LrWpanPhyPibAttributes>();
  pibAttr->phyCurrentChannel = params.m_logCh;
  m_phy->PlmeSetAttributeRequest(
      LrWpanPibAttributeIdentifier::phyCurrentChannel, pibAttr);

  m_phy->PlmeSetTRXStateRequest(IEEE_802_15_4_PHY_RX_ON);

  uint64_t searchSymbols;
  Time searchBeaconTime;

  if (m_trackingEvent.IsRunning()) {
    m_trackingEvent.Cancel();
  }

  if (params.m_trackBcn) {
    m_numLostBeacons = 0;
    searchSymbols = ((uint64_t)1 << m_incomingBeaconOrder) +
                    1 * lrwpan::aBaseSuperframeDuration;
    searchBeaconTime = Seconds((double)searchSymbols / symbolRate);
    m_beaconTrackingOn = true;
    m_trackingEvent = Simulator::Schedule(
        searchBeaconTime, &LrWpanMac::BeaconSearchTimeout, this);
  } else {
    m_beaconTrackingOn = false;
  }
}

void LrWpanMac::MlmePollRequest(MlmePollRequestParams params) {
  NS_LOG_FUNCTION(this);

  LrWpanMacHeader macHdr(LrWpanMacHeader::LRWPAN_MAC_COMMAND,
                         m_macBsn.GetValue());
  m_macBsn++;

  CommandPayloadHeader macPayload(CommandPayloadHeader::DATA_REQ);

  Ptr<Packet> beaconPacket = Create<Packet>();
  NS_FATAL_ERROR(this << " Poll request currently not supported");
}

void LrWpanMac::MlmeSetRequest(LrWpanMacPibAttributeIdentifier id,
                               Ptr<LrWpanMacPibAttributes> attribute) {
  MlmeSetConfirmParams confirmParams;
  confirmParams.m_status = MLMESET_SUCCESS;

  switch (id) {
  case macBeaconPayload:
    if (attribute->macBeaconPayload->GetSize() >
        lrwpan::aMaxBeaconPayloadLength) {
      confirmParams.m_status = MLMESET_INVALID_PARAMETER;
    } else {
      m_macBeaconPayload = attribute->macBeaconPayload;
      m_macBeaconPayloadLength = attribute->macBeaconPayload->GetSize();
    }
    break;
  case macBeaconPayloadLength:
    confirmParams.m_status = MLMESET_INVALID_PARAMETER;
    break;
  case macShortAddress:
    m_shortAddress = attribute->macShortAddress;
    break;
  case macExtendedAddress:
    confirmParams.m_status = MLMESET_READ_ONLY;
    break;
  case macPanId:
    m_macPanId = macPanId;
    break;
  default:
    confirmParams.m_status = MLMESET_UNSUPPORTED_ATTRIBUTE;
    break;
  }

  if (!m_mlmeSetConfirmCallback.IsNull()) {
    confirmParams.id = id;
    m_mlmeSetConfirmCallback(confirmParams);
  }
}

void LrWpanMac::MlmeGetRequest(LrWpanMacPibAttributeIdentifier id) {
  LrWpanMlmeGetConfirmStatus status = MLMEGET_SUCCESS;
  Ptr<LrWpanMacPibAttributes> attributes = Create<LrWpanMacPibAttributes>();

  switch (id) {
  case macBeaconPayload:
    attributes->macBeaconPayload = m_macBeaconPayload;
    break;
  case macBeaconPayloadLength:
    attributes->macBeaconPayloadLength = m_macBeaconPayloadLength;
    break;
  case macShortAddress:
    attributes->macShortAddress = m_shortAddress;
    break;
  case macExtendedAddress:
    attributes->macExtendedAddress = m_selfExt;
    break;
  case macPanId:
    attributes->macPanId = m_macPanId;
    break;
  default:
    status = MLMEGET_UNSUPPORTED_ATTRIBUTE;
    break;
  }

  if (!m_mlmeGetConfirmCallback.IsNull()) {
    m_mlmeGetConfirmCallback(status, id, attributes);
  }
}

void LrWpanMac::SendOneBeacon() {
  NS_LOG_FUNCTION(this);
  NS_ASSERT(m_lrWpanMacState == MAC_IDLE);

  LrWpanMacHeader macHdr(LrWpanMacHeader::LRWPAN_MAC_BEACON,
                         m_macBsn.GetValue());
  m_macBsn++;
  BeaconPayloadHeader macPayload;
  Ptr<Packet> beaconPacket;
  LrWpanMacTrailer macTrailer;

  if (m_macBeaconPayload == nullptr) {
    beaconPacket = Create<Packet>();
  } else {
    beaconPacket = m_macBeaconPayload;
  }

  macHdr.SetDstAddrMode(LrWpanMacHeader::SHORTADDR);
  macHdr.SetDstAddrFields(GetPanId(), Mac16Address("ff:ff"));

  if (GetShortAddress() == Mac16Address("ff:fe")) {
    macHdr.SetSrcAddrMode(LrWpanMacHeader::EXTADDR);
    macHdr.SetSrcAddrFields(GetPanId(), GetExtendedAddress());
  } else {
    macHdr.SetSrcAddrMode(LrWpanMacHeader::SHORTADDR);
    macHdr.SetSrcAddrFields(GetPanId(), GetShortAddress());
  }

  macHdr.SetSecDisable();
  macHdr.SetNoAckReq();

  macPayload.SetSuperframeSpecField(GetSuperframeField());
  macPayload.SetGtsFields(GetGtsFields());
  macPayload.SetPndAddrFields(GetPendingAddrFields());

  beaconPacket->AddHeader(macPayload);
  beaconPacket->AddHeader(macHdr);

  if (Node::ChecksumEnabled()) {
    macTrailer.EnableFcs(true);
    macTrailer.SetFcs(beaconPacket);
  }

  beaconPacket->AddTrailer(macTrailer);

  m_txPkt = beaconPacket;

  if (m_csmaCa->IsSlottedCsmaCa()) {
    m_outSuperframeStatus = BEACON;
    NS_LOG_DEBUG("Outgoing superframe Active Portion (Beacon + CAP + CFP): "
                 << m_superframeDuration << " symbols");
  }

  ChangeMacState(MAC_SENDING);
  m_phy->PlmeSetTRXStateRequest(IEEE_802_15_4_PHY_TX_ON);
}

void LrWpanMac::SendBeaconRequestCommand() {
  NS_LOG_FUNCTION(this);

  LrWpanMacHeader macHdr(LrWpanMacHeader::LRWPAN_MAC_COMMAND,
                         m_macDsn.GetValue());
  m_macDsn++;
  LrWpanMacTrailer macTrailer;
  Ptr<Packet> commandPacket = Create<Packet>();

  macHdr.SetNoPanIdComp();
  macHdr.SetDstAddrMode(LrWpanMacHeader::SHORTADDR);
  macHdr.SetSrcAddrMode(LrWpanMacHeader::NOADDR);

  macHdr.SetDstAddrFields(0xFFFF, Mac16Address("FF:FF"));

  macHdr.SetSecDisable();
  macHdr.SetNoAckReq();

  CommandPayloadHeader macPayload;
  macPayload.SetCommandFrameType(CommandPayloadHeader::BEACON_REQ);

  commandPacket->AddHeader(macPayload);
  commandPacket->AddHeader(macHdr);

  if (Node::ChecksumEnabled()) {
    macTrailer.EnableFcs(true);
    macTrailer.SetFcs(commandPacket);
  }

  commandPacket->AddTrailer(macTrailer);

  Ptr<TxQueueElement> txQElement = Create<TxQueueElement>();
  txQElement->txQPkt = commandPacket;
  EnqueueTxQElement(txQElement);
  CheckQueue();
}

void LrWpanMac::SendOrphanNotificationCommand() {
  LrWpanMacHeader macHdr(LrWpanMacHeader::LRWPAN_MAC_COMMAND,
                         m_macDsn.GetValue());
  m_macDsn++;
  LrWpanMacTrailer macTrailer;
  Ptr<Packet> commandPacket = Create<Packet>();

  macHdr.SetPanIdComp();

  macHdr.SetSrcAddrMode(LrWpanMacHeader::EXTADDR);
  macHdr.SetSrcAddrFields(0xFFFF, GetExtendedAddress());

  macHdr.SetDstAddrMode(LrWpanMacHeader::SHORTADDR);
  macHdr.SetDstAddrFields(0xFFFF, Mac16Address("FF:FF"));

  macHdr.SetSecDisable();
  macHdr.SetNoAckReq();

  CommandPayloadHeader macPayload;
  macPayload.SetCommandFrameType(CommandPayloadHeader::ORPHAN_NOTIF);

  commandPacket->AddHeader(macPayload);
  commandPacket->AddHeader(macHdr);

  if (Node::ChecksumEnabled()) {
    macTrailer.EnableFcs(true);
    macTrailer.SetFcs(commandPacket);
  }

  commandPacket->AddTrailer(macTrailer);

  Ptr<TxQueueElement> txQElement = Create<TxQueueElement>();
  txQElement->txQPkt = commandPacket;
  EnqueueTxQElement(txQElement);
  CheckQueue();
}

void LrWpanMac::SendAssocRequestCommand() {
  NS_LOG_FUNCTION(this);

  LrWpanMacHeader macHdr(LrWpanMacHeader::LRWPAN_MAC_COMMAND,
                         m_macDsn.GetValue());
  m_macDsn++;
  LrWpanMacTrailer macTrailer;
  Ptr<Packet> commandPacket = Create<Packet>();

  macHdr.SetSrcAddrMode(LrWpanMacHeader::EXTADDR);
  macHdr.SetSrcAddrFields(0xffff, GetExtendedAddress());

  if (m_associateParams.m_coordAddrMode == SHORT_ADDR) {
    macHdr.SetDstAddrMode(LrWpanMacHeader::SHORTADDR);
    macHdr.SetDstAddrFields(m_associateParams.m_coordPanId,
                            m_associateParams.m_coordShortAddr);
  } else {
    macHdr.SetDstAddrMode(LrWpanMacHeader::EXTADDR);
    macHdr.SetDstAddrFields(m_associateParams.m_coordPanId,
                            m_associateParams.m_coordExtAddr);
  }

  macHdr.SetSecDisable();
  macHdr.SetAckReq();

  CommandPayloadHeader macPayload(CommandPayloadHeader::ASSOCIATION_REQ);
  macPayload.SetCapabilityField(m_associateParams.m_capabilityInfo);

  commandPacket->AddHeader(macPayload);
  commandPacket->AddHeader(macHdr);

  if (Node::ChecksumEnabled()) {
    macTrailer.EnableFcs(true);
    macTrailer.SetFcs(commandPacket);
  }

  commandPacket->AddTrailer(macTrailer);

  Ptr<TxQueueElement> txQElement = Create<TxQueueElement>();
  txQElement->txQPkt = commandPacket;
  EnqueueTxQElement(txQElement);
  CheckQueue();
}

void LrWpanMac::SendDataRequestCommand() {

  NS_LOG_FUNCTION(this);

  LrWpanMacHeader macHdr(LrWpanMacHeader::LRWPAN_MAC_COMMAND,
                         m_macDsn.GetValue());
  m_macDsn++;
  LrWpanMacTrailer macTrailer;
  Ptr<Packet> commandPacket = Create<Packet>();

  macHdr.SetSrcAddrMode(LrWpanMacHeader::EXTADDR);
  macHdr.SetSrcAddrFields(0xffff, m_selfExt);

  if (m_macCoordShortAddress == Mac16Address("ff:fe")) {
    macHdr.SetDstAddrMode(LrWpanMacHeader::EXTADDR);
    macHdr.SetDstAddrFields(m_macPanId, m_macCoordExtendedAddress);
  } else {
    macHdr.SetDstAddrMode(LrWpanMacHeader::SHORTADDR);
    macHdr.SetDstAddrFields(m_macPanId, m_macCoordShortAddress);
  }

  macHdr.SetSecDisable();
  macHdr.SetAckReq();

  CommandPayloadHeader macPayload(CommandPayloadHeader::DATA_REQ);

  commandPacket->AddHeader(macPayload);
  commandPacket->AddHeader(macHdr);

  if (Node::ChecksumEnabled()) {
    macTrailer.EnableFcs(true);
    macTrailer.SetFcs(commandPacket);
  }

  commandPacket->AddTrailer(macTrailer);

  Ptr<TxQueueElement> txQElement = Create<TxQueueElement>();
  txQElement->txQPkt = commandPacket;
  EnqueueTxQElement(txQElement);
  CheckQueue();
}

void LrWpanMac::SendAssocResponseCommand(Ptr<Packet> rxDataReqPkt) {
  LrWpanMacHeader receivedMacHdr;
  rxDataReqPkt->RemoveHeader(receivedMacHdr);
  CommandPayloadHeader receivedMacPayload;
  rxDataReqPkt->RemoveHeader(receivedMacPayload);

  NS_ASSERT(receivedMacPayload.GetCommandFrameType() ==
            CommandPayloadHeader::DATA_REQ);

  Ptr<IndTxQueueElement> indTxQElement = Create<IndTxQueueElement>();
  bool elementFound;
  elementFound = DequeueInd(receivedMacHdr.GetExtSrcAddr(), indTxQElement);
  if (elementFound) {
    Ptr<TxQueueElement> txQElement = Create<TxQueueElement>();
    txQElement->txQPkt = indTxQElement->txQPkt;
    m_txQueue.emplace_back(txQElement);
  } else {
    NS_LOG_DEBUG("Requested element not found in pending list");
  }
}

void LrWpanMac::LostAssocRespCommand() {
  m_macPanId = 0xffff;
  m_macCoordShortAddress = Mac16Address("FF:FF");
  m_macCoordExtendedAddress = Mac64Address("ff:ff:ff:ff:ff:ff:ff:ed");

  if (!m_mlmeAssociateConfirmCallback.IsNull()) {
    MlmeAssociateConfirmParams confirmParams;
    confirmParams.m_assocShortAddr = Mac16Address("FF:FF");
    confirmParams.m_status = MLMEASSOC_NO_DATA;
    m_mlmeAssociateConfirmCallback(confirmParams);
  }
}

void LrWpanMac::EndStartRequest() {
  NS_LOG_FUNCTION(this);
  m_pendPrimitive = MLME_NONE;

  if (m_startParams.m_coorRealgn) {
    NS_LOG_ERROR(this << " Coordinator realignment request not supported");
    return;
  } else {
    if (m_startParams.m_panCoor) {
      m_panCoor = true;
    }

    m_coor = true;
    m_macPanId = m_startParams.m_PanId;

    NS_ASSERT(m_startParams.m_PanId != 0xffff);

    m_macBeaconOrder = m_startParams.m_bcnOrd;
    if (m_macBeaconOrder == 15) {
      m_macSuperframeOrder = 15;
      m_fnlCapSlot = 15;
      m_beaconInterval = 0;

      m_csmaCa->Cancel();
      m_capEvent.Cancel();
      m_cfpEvent.Cancel();
      m_incCapEvent.Cancel();
      m_incCfpEvent.Cancel();
      m_trackingEvent.Cancel();
      m_scanEvent.Cancel();
      m_scanOrphanEvent.Cancel();
      m_scanEnergyEvent.Cancel();

      m_csmaCa->SetUnSlottedCsmaCa();

      if (!m_mlmeStartConfirmCallback.IsNull()) {
        MlmeStartConfirmParams confirmParams;
        confirmParams.m_status = MLMESTART_SUCCESS;
        m_mlmeStartConfirmCallback(confirmParams);
      }

      m_phy->PlmeSetTRXStateRequest(IEEE_802_15_4_PHY_RX_ON);
    } else {
      m_macSuperframeOrder = m_startParams.m_sfrmOrd;
      m_csmaCa->SetBatteryLifeExtension(m_startParams.m_battLifeExt);

      m_csmaCa->SetSlottedCsmaCa();

      m_fnlCapSlot = 15;

      m_beaconInterval = (static_cast<uint32_t>(1 << m_macBeaconOrder)) *
                         lrwpan::aBaseSuperframeDuration;
      m_superframeDuration =
          (static_cast<uint32_t>(1 << m_macSuperframeOrder)) *
          lrwpan::aBaseSuperframeDuration;

      m_beaconEvent = Simulator::ScheduleNow(&LrWpanMac::SendOneBeacon, this);
    }
  }
}

void LrWpanMac::EndChannelScan() {
  NS_LOG_FUNCTION(this);

  m_channelScanIndex++;

  bool channelFound = false;

  for (int i = m_channelScanIndex; i <= 26; i++) {
    if ((m_scanParams.m_scanChannels & (1 << m_channelScanIndex)) != 0) {
      channelFound = true;
      break;
    }
    m_channelScanIndex++;
  }

  if (channelFound) {
    Ptr<LrWpanPhyPibAttributes> pibAttr = Create<LrWpanPhyPibAttributes>();
    pibAttr->phyCurrentChannel = m_channelScanIndex;
    m_phy->PlmeSetAttributeRequest(
        LrWpanPibAttributeIdentifier::phyCurrentChannel, pibAttr);
  } else {
    m_macPanId = m_macPanIdScan;
    m_macPanIdScan = 0;

    MlmeScanConfirmParams confirmParams;
    confirmParams.m_chPage = m_scanParams.m_chPage;
    confirmParams.m_scanType = m_scanParams.m_scanType;
    confirmParams.m_energyDetList = {};
    confirmParams.m_unscannedCh = m_unscannedChannels;
    confirmParams.m_resultListSize = m_panDescriptorList.size();

    switch (confirmParams.m_scanType) {
    case MLMESCAN_PASSIVE:
      if (m_macAutoRequest) {
        confirmParams.m_panDescList = m_panDescriptorList;
      }
      confirmParams.m_status = MLMESCAN_SUCCESS;
      break;
    case MLMESCAN_ACTIVE:
      if (m_panDescriptorList.empty()) {
        confirmParams.m_status = MLMESCAN_NO_BEACON;
      } else {
        if (m_macAutoRequest) {
          confirmParams.m_panDescList = m_panDescriptorList;
        }
        confirmParams.m_status = MLMESCAN_SUCCESS;
      }
      break;
    case MLMESCAN_ORPHAN:
      confirmParams.m_panDescList = {};
      confirmParams.m_status = MLMESCAN_NO_BEACON;
      confirmParams.m_resultListSize = 0;
      m_macPanId = 0xffff;
      m_shortAddress = Mac16Address("FF:FF");
      m_macCoordShortAddress = Mac16Address("ff:ff");
      m_macCoordExtendedAddress = Mac64Address("ff:ff:ff:ff:ff:ff:ff:ed");
      break;
    default:
      NS_LOG_ERROR(this << " Invalid scan type");
    }

    m_pendPrimitive = MLME_NONE;
    m_channelScanIndex = 0;
    m_scanParams = {};

    if (!m_mlmeScanConfirmCallback.IsNull()) {
      m_mlmeScanConfirmCallback(confirmParams);
    }
  }
}

void LrWpanMac::EndChannelEnergyScan() {
  NS_LOG_FUNCTION(this);
  m_energyDetectList.emplace_back(m_maxEnergyLevel);
  m_maxEnergyLevel = 0;

  m_channelScanIndex++;

  bool channelFound = false;
  for (int i = m_channelScanIndex; i <= 26; i++) {
    if ((m_scanParams.m_scanChannels & (1 << m_channelScanIndex)) != 0) {
      channelFound = true;
      break;
    }
    m_channelScanIndex++;
  }

  if (channelFound) {
    Ptr<LrWpanPhyPibAttributes> pibAttr = Create<LrWpanPhyPibAttributes>();
    pibAttr->phyCurrentChannel = m_channelScanIndex;
    m_phy->PlmeSetAttributeRequest(
        LrWpanPibAttributeIdentifier::phyCurrentChannel, pibAttr);
  } else {
    m_macPanId = m_macPanIdScan;
    m_macPanIdScan = 0;

    MlmeScanConfirmParams confirmParams;
    confirmParams.m_status = MLMESCAN_SUCCESS;
    confirmParams.m_chPage = m_phy->GetCurrentPage();
    confirmParams.m_scanType = m_scanParams.m_scanType;
    confirmParams.m_energyDetList = m_energyDetectList;
    confirmParams.m_resultListSize = m_energyDetectList.size();

    m_pendPrimitive = MLME_NONE;
    m_channelScanIndex = 0;
    m_scanParams = {};

    if (!m_mlmeScanConfirmCallback.IsNull()) {
      m_mlmeScanConfirmCallback(confirmParams);
    }
  }
}

void LrWpanMac::StartCAP(SuperframeType superframeType) {
  uint32_t activeSlot;
  uint64_t capDuration;
  Time endCapTime;
  uint64_t symbolRate;

  symbolRate = (uint64_t)m_phy->GetDataOrSymbolRate(false);

  if (superframeType == OUTGOING) {
    m_outSuperframeStatus = CAP;
    activeSlot = m_superframeDuration / 16;
    capDuration = activeSlot * (m_fnlCapSlot + 1);
    endCapTime = Seconds((double)capDuration / symbolRate);
    endCapTime -= (Simulator::Now() - m_macBeaconTxTime);

    NS_LOG_DEBUG("Outgoing superframe CAP duration "
                 << (endCapTime.GetSeconds() * symbolRate) << " symbols ("
                 << endCapTime.As(Time::S) << ")");
    NS_LOG_DEBUG("Active Slots duration " << activeSlot << " symbols");

    m_capEvent = Simulator::Schedule(endCapTime, &LrWpanMac::StartCFP, this,
                                     SuperframeType::OUTGOING);
  } else {
    m_incSuperframeStatus = CAP;
    activeSlot = m_incomingSuperframeDuration / 16;
    capDuration = activeSlot * (m_incomingFnlCapSlot + 1);
    endCapTime = Seconds((double)capDuration / symbolRate);
    endCapTime -= (Simulator::Now() - m_macBeaconRxTime);

    NS_LOG_DEBUG("Incoming superframe CAP duration "
                 << (endCapTime.GetSeconds() * symbolRate) << " symbols ("
                 << endCapTime.As(Time::S) << ")");
    NS_LOG_DEBUG("Active Slots duration " << activeSlot << " symbols");

    m_capEvent = Simulator::Schedule(endCapTime, &LrWpanMac::StartCFP, this,
                                     SuperframeType::INCOMING);
  }

  CheckQueue();
}

void LrWpanMac::StartCFP(SuperframeType superframeType) {
  uint32_t activeSlot;
  uint64_t cfpDuration;
  Time endCfpTime;
  uint64_t symbolRate;

  symbolRate = (uint64_t)m_phy->GetDataOrSymbolRate(false);

  if (superframeType == INCOMING) {
    activeSlot = m_incomingSuperframeDuration / 16;
    cfpDuration = activeSlot * (15 - m_incomingFnlCapSlot);
    endCfpTime = Seconds((double)cfpDuration / symbolRate);
    if (cfpDuration > 0) {
      m_incSuperframeStatus = CFP;
    }

    NS_LOG_DEBUG("Incoming superframe CFP duration "
                 << cfpDuration << " symbols (" << endCfpTime.As(Time::S)
                 << ")");

    m_incCfpEvent =
        Simulator::Schedule(endCfpTime, &LrWpanMac::StartInactivePeriod, this,
                            SuperframeType::INCOMING);
  } else {
    activeSlot = m_superframeDuration / 16;
    cfpDuration = activeSlot * (15 - m_fnlCapSlot);
    endCfpTime = Seconds((double)cfpDuration / symbolRate);

    if (cfpDuration > 0) {
      m_outSuperframeStatus = CFP;
    }

    NS_LOG_DEBUG("Outgoing superframe CFP duration "
                 << cfpDuration << " symbols (" << endCfpTime.As(Time::S)
                 << ")");

    m_cfpEvent =
        Simulator::Schedule(endCfpTime, &LrWpanMac::StartInactivePeriod, this,
                            SuperframeType::OUTGOING);
  }
}

void LrWpanMac::StartInactivePeriod(SuperframeType superframeType) {
  uint64_t inactiveDuration;
  Time endInactiveTime;
  uint64_t symbolRate;

  symbolRate = (uint64_t)m_phy->GetDataOrSymbolRate(false);

  if (superframeType == INCOMING) {
    inactiveDuration = m_incomingBeaconInterval - m_incomingSuperframeDuration;
    endInactiveTime = Seconds((double)inactiveDuration / symbolRate);

    if (inactiveDuration > 0) {
      m_incSuperframeStatus = INACTIVE;
    }

    NS_LOG_DEBUG("Incoming superframe Inactive Portion duration "
                 << inactiveDuration << " symbols ("
                 << endInactiveTime.As(Time::S) << ")");
    m_beaconEvent =
        Simulator::Schedule(endInactiveTime, &LrWpanMac::AwaitBeacon, this);
  } else {
    inactiveDuration = m_beaconInterval - m_superframeDuration;
    endInactiveTime = Seconds((double)inactiveDuration / symbolRate);

    if (inactiveDuration > 0) {
      m_outSuperframeStatus = INACTIVE;
    }

    NS_LOG_DEBUG("Outgoing superframe Inactive Portion duration "
                 << inactiveDuration << " symbols ("
                 << endInactiveTime.As(Time::S) << ")");
    m_beaconEvent =
        Simulator::Schedule(endInactiveTime, &LrWpanMac::SendOneBeacon, this);
  }
}

void LrWpanMac::AwaitBeacon() { m_incSuperframeStatus = BEACON; }

void LrWpanMac::BeaconSearchTimeout() {
  auto symbolRate = (uint64_t)m_phy->GetDataOrSymbolRate(false);

  if (m_numLostBeacons > lrwpan::aMaxLostBeacons) {
    MlmeSyncLossIndicationParams syncLossParams;
    syncLossParams.m_lossReason = MLMESYNCLOSS_BEACON_LOST;
    syncLossParams.m_panId = m_macPanId;
    m_mlmeSyncLossIndicationCallback(syncLossParams);

    m_beaconTrackingOn = false;
    m_numLostBeacons = 0;
  } else {
    m_numLostBeacons++;

    uint64_t searchSymbols;
    Time searchBeaconTime;
    searchSymbols = ((uint64_t)1 << m_incomingBeaconOrder) +
                    1 * lrwpan::aBaseSuperframeDuration;
    searchBeaconTime = Seconds((double)searchSymbols / symbolRate);
    m_trackingEvent = Simulator::Schedule(
        searchBeaconTime, &LrWpanMac::BeaconSearchTimeout, this);
  }
}

void LrWpanMac::CheckQueue() {
  NS_LOG_FUNCTION(this);
  if (m_lrWpanMacState == MAC_IDLE && !m_txQueue.empty() &&
      !m_setMacState.IsRunning()) {
    if (m_csmaCa->IsUnSlottedCsmaCa() ||
        (m_outSuperframeStatus == CAP && m_coor) ||
        m_incSuperframeStatus == CAP) {
      if (!m_ifsEvent.IsRunning()) {
        Ptr<TxQueueElement> txQElement = m_txQueue.front();
        m_txPkt = txQElement->txQPkt;

        m_setMacState = Simulator::ScheduleNow(&LrWpanMac::SetLrWpanMacState,
                                               this, MAC_CSMA);
      }
    }
  }
}

SuperframeField LrWpanMac::GetSuperframeField() {
  SuperframeField sfrmSpec;

  sfrmSpec.SetBeaconOrder(m_macBeaconOrder);
  sfrmSpec.SetSuperframeOrder(m_macSuperframeOrder);
  sfrmSpec.SetFinalCapSlot(m_fnlCapSlot);

  if (m_csmaCa->GetBatteryLifeExtension()) {
    sfrmSpec.SetBattLifeExt(true);
  }

  if (m_panCoor) {
    sfrmSpec.SetPanCoor(true);
  }

  if (m_macAssociationPermit) {
    sfrmSpec.SetAssocPermit(true);
  }

  return sfrmSpec;
}

GtsFields LrWpanMac::GetGtsFields() {
  GtsFields gtsFields;

  return gtsFields;
}

PendingAddrFields LrWpanMac::GetPendingAddrFields() {
  PendingAddrFields pndAddrFields;

  return pndAddrFields;
}

void LrWpanMac::SetCsmaCa(Ptr<LrWpanCsmaCa> csmaCa) { m_csmaCa = csmaCa; }

void LrWpanMac::SetPhy(Ptr<LrWpanPhy> phy) { m_phy = phy; }

Ptr<LrWpanPhy> LrWpanMac::GetPhy() { return m_phy; }

void LrWpanMac::SetMcpsDataIndicationCallback(McpsDataIndicationCallback c) {
  m_mcpsDataIndicationCallback = c;
}

void LrWpanMac::SetMlmeAssociateIndicationCallback(
    MlmeAssociateIndicationCallback c) {
  m_mlmeAssociateIndicationCallback = c;
}

void LrWpanMac::SetMlmeCommStatusIndicationCallback(
    MlmeCommStatusIndicationCallback c) {
  m_mlmeCommStatusIndicationCallback = c;
}

void LrWpanMac::SetMlmeOrphanIndicationCallback(
    MlmeOrphanIndicationCallback c) {
  m_mlmeOrphanIndicationCallback = c;
}

void LrWpanMac::SetMcpsDataConfirmCallback(McpsDataConfirmCallback c) {
  m_mcpsDataConfirmCallback = c;
}

void LrWpanMac::SetMlmeStartConfirmCallback(MlmeStartConfirmCallback c) {
  m_mlmeStartConfirmCallback = c;
}

void LrWpanMac::SetMlmeScanConfirmCallback(MlmeScanConfirmCallback c) {
  m_mlmeScanConfirmCallback = c;
}

void LrWpanMac::SetMlmeAssociateConfirmCallback(
    MlmeAssociateConfirmCallback c) {
  m_mlmeAssociateConfirmCallback = c;
}

void LrWpanMac::SetMlmeBeaconNotifyIndicationCallback(
    MlmeBeaconNotifyIndicationCallback c) {
  m_mlmeBeaconNotifyIndicationCallback = c;
}

void LrWpanMac::SetMlmeSyncLossIndicationCallback(
    MlmeSyncLossIndicationCallback c) {
  m_mlmeSyncLossIndicationCallback = c;
}

void LrWpanMac::SetMlmePollConfirmCallback(MlmePollConfirmCallback c) {
  m_mlmePollConfirmCallback = c;
}

void LrWpanMac::SetMlmeSetConfirmCallback(MlmeSetConfirmCallback c) {
  m_mlmeSetConfirmCallback = c;
}

void LrWpanMac::SetMlmeGetConfirmCallback(MlmeGetConfirmCallback c) {
  m_mlmeGetConfirmCallback = c;
}

void LrWpanMac::PdDataIndication(uint32_t psduLength, Ptr<Packet> p,
                                 uint8_t lqi) {
  NS_ASSERT(m_lrWpanMacState == MAC_IDLE ||
            m_lrWpanMacState == MAC_ACK_PENDING ||
            m_lrWpanMacState == MAC_CSMA);
  NS_LOG_FUNCTION(this << psduLength << p << (uint16_t)lqi);

  bool acceptFrame;

  Ptr<Packet> originalPkt = p->Copy();
  auto symbolRate = (uint64_t)m_phy->GetDataOrSymbolRate(false);
  m_promiscSnifferTrace(originalPkt);

  m_macPromiscRxTrace(originalPkt);

  LrWpanMacTrailer receivedMacTrailer;
  p->RemoveTrailer(receivedMacTrailer);
  if (Node::ChecksumEnabled()) {
    receivedMacTrailer.EnableFcs(true);
  }

  if (!receivedMacTrailer.CheckFcs(p)) {
    m_macRxDropTrace(originalPkt);
  } else {
    LrWpanMacHeader receivedMacHdr;
    p->RemoveHeader(receivedMacHdr);

    McpsDataIndicationParams params;
    params.m_dsn = receivedMacHdr.GetSeqNum();
    params.m_mpduLinkQuality = lqi;
    params.m_srcPanId = receivedMacHdr.GetSrcPanId();
    params.m_srcAddrMode = receivedMacHdr.GetSrcAddrMode();
    switch (params.m_srcAddrMode) {
    case SHORT_ADDR:
      params.m_srcAddr = receivedMacHdr.GetShortSrcAddr();
      break;
    case EXT_ADDR:
      params.m_srcExtAddr = receivedMacHdr.GetExtSrcAddr();
      break;
    default:
      break;
    }
    params.m_dstPanId = receivedMacHdr.GetDstPanId();
    params.m_dstAddrMode = receivedMacHdr.GetDstAddrMode();
    switch (params.m_dstAddrMode) {
    case SHORT_ADDR:
      params.m_dstAddr = receivedMacHdr.GetShortDstAddr();
      break;
    case EXT_ADDR:
      params.m_dstExtAddr = receivedMacHdr.GetExtDstAddr();
      break;
    default:
      break;
    }

    if (m_macPromiscuousMode) {
      if (receivedMacHdr.GetDstAddrMode() == SHORT_ADDR) {
        NS_LOG_DEBUG("Packet from " << params.m_srcAddr);
        NS_LOG_DEBUG("Packet to " << params.m_dstAddr);
      } else if (receivedMacHdr.GetDstAddrMode() == EXT_ADDR) {
        NS_LOG_DEBUG("Packet from " << params.m_srcExtAddr);
        NS_LOG_DEBUG("Packet to " << params.m_dstExtAddr);
      }

      if (!m_mcpsDataIndicationCallback.IsNull()) {
        NS_LOG_DEBUG("promiscuous mode, forwarding up");
        m_mcpsDataIndicationCallback(params, p);
      } else {
        NS_LOG_ERROR(this << " Data Indication Callback not initialized");
      }
    } else {
      acceptFrame =
          (receivedMacHdr.GetType() != LrWpanMacHeader::LRWPAN_MAC_RESERVED);

      if (acceptFrame) {
        acceptFrame = (receivedMacHdr.GetFrameVer() <= 1);
      }

      if (acceptFrame && (receivedMacHdr.GetDstAddrMode() > 1)) {

        acceptFrame = ((receivedMacHdr.GetDstPanId() == m_macPanId ||
                        receivedMacHdr.GetDstPanId() == 0xffff) ||
                       (m_macPanId == 0xffff && receivedMacHdr.IsBeacon())) ||
                      (m_macPanId == 0xffff && receivedMacHdr.IsCommand());
      }

      if (acceptFrame && (receivedMacHdr.GetDstAddrMode() == SHORT_ADDR)) {
        if (receivedMacHdr.GetShortDstAddr() == m_shortAddress) {
          acceptFrame = true;
        } else if (receivedMacHdr.GetShortDstAddr().IsBroadcast() ||
                   receivedMacHdr.GetShortDstAddr().IsMulticast()) {
          acceptFrame = !receivedMacHdr.IsAckReq();
        } else {
          acceptFrame = false;
        }
      }

      if (acceptFrame && (receivedMacHdr.GetDstAddrMode() == EXT_ADDR)) {
        acceptFrame = (receivedMacHdr.GetExtDstAddr() == m_selfExt);
      }

      if (acceptFrame && m_scanEvent.IsRunning()) {
        if (!receivedMacHdr.IsBeacon()) {
          acceptFrame = false;
        }
      } else if (acceptFrame && m_scanOrphanEvent.IsRunning()) {
        if (!receivedMacHdr.IsCommand()) {
          acceptFrame = false;
        }
      } else if (m_scanEnergyEvent.IsRunning()) {
        acceptFrame = false;
      }

      if (acceptFrame &&
          (receivedMacHdr.IsCommand() && receivedMacHdr.IsAckReq())) {
        CommandPayloadHeader receivedMacPayload;
        p->PeekHeader(receivedMacPayload);

        if (receivedMacPayload.GetCommandFrameType() ==
                CommandPayloadHeader::ASSOCIATION_REQ &&
            !(m_macAssociationPermit && m_coor)) {
          acceptFrame = false;
        }

        if (acceptFrame &&
            (m_csmaCa->IsSlottedCsmaCa() && m_capEvent.IsRunning())) {
          Time timeLeftInCap = Simulator::GetDelayLeft(m_capEvent);
          uint64_t ackSymbols = lrwpan::aTurnaroundTime +
                                m_phy->GetPhySHRDuration() +
                                ceil(6 * m_phy->GetPhySymbolsPerOctet());
          Time ackTime = Seconds((double)ackSymbols / symbolRate);

          if (ackTime >= timeLeftInCap) {
            NS_LOG_DEBUG(
                "Command frame received but not enough time to transmit ACK "
                "before the end of CAP ");
            acceptFrame = false;
          }
        }
      }

      if (acceptFrame) {
        m_macRxTrace(originalPkt);

        if ((receivedMacHdr.IsData() || receivedMacHdr.IsCommand()) &&
            receivedMacHdr.IsAckReq() &&
            !(receivedMacHdr.GetDstAddrMode() == SHORT_ADDR &&
              (receivedMacHdr.GetShortDstAddr().IsBroadcast() ||
               receivedMacHdr.GetShortDstAddr().IsMulticast()))) {
          if (m_lrWpanMacState == MAC_ACK_PENDING) {
            m_ackWaitTimeout.Cancel();
            PrepareRetransmission();
          } else if (m_lrWpanMacState == MAC_CSMA) {
            NS_LOG_DEBUG(
                "Received a packet with ACK required while in CSMA. Cancel "
                "current CSMA-CA");
            m_csmaCa->Cancel();
          }
          m_setMacState.Cancel();
          ChangeMacState(MAC_IDLE);

          m_rxPkt = originalPkt->Copy();
          m_lastRxFrameLqi = lqi;

          CommandPayloadHeader receivedMacPayload;
          p->PeekHeader(receivedMacPayload);
          switch (receivedMacPayload.GetCommandFrameType()) {
          case CommandPayloadHeader::DATA_REQ:
            NS_LOG_DEBUG("Data Request Command Received; processing ACK");
            break;
          case CommandPayloadHeader::ASSOCIATION_REQ:
            NS_LOG_DEBUG(
                "Association Request Command Received; processing ACK");
            break;
          case CommandPayloadHeader::ASSOCIATION_RESP:
            m_assocResCmdWaitTimeout.Cancel();
            NS_LOG_DEBUG(
                "Association Response Command Received; processing ACK");
            break;
          default:
            break;
          }

          m_setMacState = Simulator::ScheduleNow(&LrWpanMac::SendAck, this,
                                                 receivedMacHdr.GetSeqNum());
        }

        if (receivedMacHdr.GetDstAddrMode() == SHORT_ADDR) {
          NS_LOG_DEBUG("Packet from " << params.m_srcAddr);
          NS_LOG_DEBUG("Packet to " << params.m_dstAddr);
        } else if (receivedMacHdr.GetDstAddrMode() == EXT_ADDR) {
          NS_LOG_DEBUG("Packet from " << params.m_srcExtAddr);
          NS_LOG_DEBUG("Packet to " << params.m_dstExtAddr);
        }

        if (receivedMacHdr.IsBeacon()) {
          m_rxBeaconSymbols =
              m_phy->GetPhySHRDuration() + 1 * m_phy->GetPhySymbolsPerOctet() +
              (originalPkt->GetSize() * m_phy->GetPhySymbolsPerOctet());

          m_macBeaconRxTime = Simulator::Now() -
                              Seconds(double(m_rxBeaconSymbols) / symbolRate);

          NS_LOG_DEBUG("Beacon Received; forwarding up (m_macBeaconRxTime: "
                       << m_macBeaconRxTime.As(Time::S) << ")");

          BeaconPayloadHeader receivedMacPayload;
          p->RemoveHeader(receivedMacPayload);

          PanDescriptor panDescriptor;

          if (receivedMacHdr.GetSrcAddrMode() == SHORT_ADDR) {
            panDescriptor.m_coorAddrMode = SHORT_ADDR;
            panDescriptor.m_coorShortAddr = receivedMacHdr.GetShortSrcAddr();
          } else {
            panDescriptor.m_coorAddrMode = EXT_ADDR;
            panDescriptor.m_coorExtAddr = receivedMacHdr.GetExtSrcAddr();
          }

          panDescriptor.m_coorPanId = receivedMacHdr.GetSrcPanId();
          panDescriptor.m_gtsPermit =
              receivedMacPayload.GetGtsFields().GetGtsPermit();
          panDescriptor.m_linkQuality = lqi;
          panDescriptor.m_logChPage = m_phy->GetCurrentPage();
          panDescriptor.m_logCh = m_phy->GetCurrentChannelNum();
          panDescriptor.m_superframeSpec =
              receivedMacPayload.GetSuperframeSpecField();
          panDescriptor.m_timeStamp = m_macBeaconRxTime;

          if (!m_scanEvent.IsRunning() &&
              m_macPanId == receivedMacHdr.GetDstPanId()) {
            m_csmaCa->Cancel();

            SuperframeField incomingSuperframe;
            incomingSuperframe = receivedMacPayload.GetSuperframeSpecField();

            m_incomingBeaconOrder = incomingSuperframe.GetBeaconOrder();
            m_incomingSuperframeOrder = incomingSuperframe.GetFrameOrder();
            m_incomingFnlCapSlot = incomingSuperframe.GetFinalCapSlot();

            m_incomingBeaconInterval =
                (static_cast<uint32_t>(1 << m_incomingBeaconOrder)) *
                lrwpan::aBaseSuperframeDuration;
            m_incomingSuperframeDuration =
                lrwpan::aBaseSuperframeDuration *
                (static_cast<uint32_t>(1 << m_incomingSuperframeOrder));

            if (incomingSuperframe.IsBattLifeExt()) {
              m_csmaCa->SetBatteryLifeExtension(true);
            } else {
              m_csmaCa->SetBatteryLifeExtension(false);
            }

            if (m_incomingBeaconOrder < 15 && !m_csmaCa->IsSlottedCsmaCa()) {
              m_csmaCa->SetSlottedCsmaCa();
            }

            NS_LOG_DEBUG(
                "Incoming superframe Active Portion (Beacon + CAP + CFP): "
                << m_incomingSuperframeDuration << " symbols");
            m_incCapEvent = Simulator::ScheduleNow(&LrWpanMac::StartCAP, this,
                                                   SuperframeType::INCOMING);
            m_setMacState = Simulator::ScheduleNow(
                &LrWpanMac::SetLrWpanMacState, this, MAC_IDLE);
          } else if (!m_scanEvent.IsRunning() && m_macPanId == 0xFFFF) {
            NS_LOG_DEBUG(this
                         << " Device not associated, cannot process beacon");
          }

          if (m_macAutoRequest) {
            if (p->GetSize() > 0) {
              if (!m_mlmeBeaconNotifyIndicationCallback.IsNull()) {
                MlmeBeaconNotifyIndicationParams beaconParams;
                beaconParams.m_bsn = receivedMacHdr.GetSeqNum();
                beaconParams.m_panDescriptor = panDescriptor;
                beaconParams.m_sduLength = p->GetSize();
                beaconParams.m_sdu = p;
                m_mlmeBeaconNotifyIndicationCallback(beaconParams);
              }
            }

            if (m_scanEvent.IsRunning()) {
              bool descriptorExists = false;

              for (const auto &descriptor : m_panDescriptorList) {
                if (descriptor.m_coorAddrMode == SHORT_ADDR) {
                  descriptorExists =
                      (descriptor.m_coorShortAddr ==
                           panDescriptor.m_coorShortAddr &&
                       descriptor.m_coorPanId == panDescriptor.m_coorPanId);
                } else {
                  descriptorExists =
                      (descriptor.m_coorExtAddr ==
                           panDescriptor.m_coorExtAddr &&
                       descriptor.m_coorPanId == panDescriptor.m_coorPanId);
                }

                if (descriptorExists) {
                  break;
                }
              }

              if (!descriptorExists) {
                m_panDescriptorList.emplace_back(panDescriptor);
              }
              return;
            } else if (m_trackingEvent.IsRunning()) {
              m_trackingEvent.Cancel();
              m_numLostBeacons = 0;

              if (m_beaconTrackingOn) {
                uint64_t searchSymbols;
                Time searchBeaconTime;

                searchSymbols =
                    (static_cast<uint64_t>(1 << m_incomingBeaconOrder)) +
                    1 * lrwpan::aBaseSuperframeDuration;
                searchBeaconTime =
                    Seconds(static_cast<double>(searchSymbols / symbolRate));
                m_trackingEvent = Simulator::Schedule(
                    searchBeaconTime, &LrWpanMac::BeaconSearchTimeout, this);
              }

              PendingAddrFields pndAddrFields;
              pndAddrFields = receivedMacPayload.GetPndAddrFields();
            }
          } else {
            if (!m_mlmeBeaconNotifyIndicationCallback.IsNull()) {
              MlmeBeaconNotifyIndicationParams beaconParams;
              beaconParams.m_bsn = receivedMacHdr.GetSeqNum();
              beaconParams.m_panDescriptor = panDescriptor;
              beaconParams.m_sduLength = p->GetSize();
              beaconParams.m_sdu = p;
              m_mlmeBeaconNotifyIndicationCallback(beaconParams);
            }
          }
        } else if (receivedMacHdr.IsCommand()) {
          CommandPayloadHeader receivedMacPayload;
          p->PeekHeader(receivedMacPayload);

          switch (receivedMacPayload.GetCommandFrameType()) {
          case CommandPayloadHeader::BEACON_REQ:
            if (m_csmaCa->IsUnSlottedCsmaCa() && m_coor) {
              SendOneBeacon();
            } else {
              m_macRxDropTrace(originalPkt);
            }
            break;
          case CommandPayloadHeader::ORPHAN_NOTIF:
            if (!m_mlmeOrphanIndicationCallback.IsNull()) {
              if (m_coor) {
                MlmeOrphanIndicationParams orphanParams;
                orphanParams.m_orphanAddr = receivedMacHdr.GetExtSrcAddr();
                m_mlmeOrphanIndicationCallback(orphanParams);
              }
            }
            break;
          case CommandPayloadHeader::COOR_REALIGN:
            if (m_scanOrphanEvent.IsRunning()) {
              m_scanOrphanEvent.Cancel();

              m_macPanIdScan = 0;
              m_pendPrimitive = MLME_NONE;
              m_channelScanIndex = 0;

              m_macPanId = receivedMacPayload.GetPanId();
              m_shortAddress = receivedMacPayload.GetShortAddr();
              m_macCoordExtendedAddress = receivedMacHdr.GetExtSrcAddr();
              m_macCoordShortAddress = receivedMacPayload.GetCoordShortAddr();

              if (!m_mlmeScanConfirmCallback.IsNull()) {
                MlmeScanConfirmParams confirmParams;
                confirmParams.m_scanType = m_scanParams.m_scanType;
                confirmParams.m_chPage = m_scanParams.m_chPage;
                confirmParams.m_status = MLMESCAN_SUCCESS;
                m_mlmeScanConfirmCallback(confirmParams);
              }
              m_scanParams = {};
            }
            break;
          default:
            m_macRxDropTrace(originalPkt);
            break;
          }
        } else if (receivedMacHdr.IsData() &&
                   !m_mcpsDataIndicationCallback.IsNull()) {
          NS_LOG_DEBUG("Data Packet is for me; forwarding up");
          m_mcpsDataIndicationCallback(params, p);
        } else if (receivedMacHdr.IsAcknowledgment() && m_txPkt &&
                   m_lrWpanMacState == MAC_ACK_PENDING) {
          LrWpanMacHeader peekedMacHdr;
          m_txPkt->PeekHeader(peekedMacHdr);
          if (receivedMacHdr.GetSeqNum() == peekedMacHdr.GetSeqNum()) {
            m_ackWaitTimeout.Cancel();
            m_macTxOkTrace(m_txPkt);

            Time ifsWaitTime = Seconds((double)GetIfsSize() / symbolRate);

            if (peekedMacHdr.IsCommand()) {
              Ptr<Packet> pkt = m_txPkt->Copy();
              LrWpanMacHeader macHdr;
              CommandPayloadHeader cmdPayload;
              pkt->RemoveHeader(macHdr);
              pkt->RemoveHeader(cmdPayload);

              switch (cmdPayload.GetCommandFrameType()) {
              case CommandPayloadHeader::ASSOCIATION_REQ: {
                double symbolRate = m_phy->GetDataOrSymbolRate(false);
                Time waitTime = Seconds(
                    static_cast<double>(m_macResponseWaitTime) / symbolRate);
                if (!m_beaconTrackingOn) {
                  m_respWaitTimeout = Simulator::Schedule(
                      waitTime, &LrWpanMac::SendDataRequestCommand, this);
                } else {
                }
                break;
              }

              case CommandPayloadHeader::ASSOCIATION_RESP: {
                if (!m_mlmeCommStatusIndicationCallback.IsNull()) {
                  MlmeCommStatusIndicationParams commStatusParams;
                  commStatusParams.m_panId = m_macPanId;
                  commStatusParams.m_srcAddrMode = LrWpanMacHeader::EXTADDR;
                  commStatusParams.m_srcExtAddr = macHdr.GetExtSrcAddr();
                  commStatusParams.m_dstAddrMode = LrWpanMacHeader::EXTADDR;
                  commStatusParams.m_dstExtAddr = macHdr.GetExtDstAddr();
                  commStatusParams.m_status =
                      LrWpanMlmeCommStatus::MLMECOMMSTATUS_SUCCESS;
                  m_mlmeCommStatusIndicationCallback(commStatusParams);
                }
                RemovePendTxQElement(m_txPkt->Copy());
                break;
              }

              case CommandPayloadHeader::DATA_REQ: {
                double symbolRate = m_phy->GetDataOrSymbolRate(false);
                Time waitTime = Seconds(
                    static_cast<double>(m_assocRespCmdWaitTime) / symbolRate);
                m_assocResCmdWaitTimeout = Simulator::Schedule(
                    waitTime, &LrWpanMac::LostAssocRespCommand, this);

                if (!m_mlmePollConfirmCallback.IsNull()) {
                  MlmePollConfirmParams pollConfirmParams;
                  pollConfirmParams.m_status =
                      LrWpanMlmePollConfirmStatus::MLMEPOLL_SUCCESS;
                  m_mlmePollConfirmCallback(pollConfirmParams);
                }
                break;
              }

              case CommandPayloadHeader::COOR_REALIGN: {
                if (!m_mlmeCommStatusIndicationCallback.IsNull()) {
                  MlmeCommStatusIndicationParams commStatusParams;
                  commStatusParams.m_panId = m_macPanId;
                  commStatusParams.m_srcAddrMode = LrWpanMacHeader::EXTADDR;
                  commStatusParams.m_srcExtAddr = macHdr.GetExtSrcAddr();
                  commStatusParams.m_dstAddrMode = LrWpanMacHeader::EXTADDR;
                  commStatusParams.m_dstExtAddr = macHdr.GetExtDstAddr();
                  commStatusParams.m_status =
                      LrWpanMlmeCommStatus::MLMECOMMSTATUS_SUCCESS;
                  m_mlmeCommStatusIndicationCallback(commStatusParams);
                }
              }

              default: {
                break;
              }
              }
            } else {
              if (!m_mcpsDataConfirmCallback.IsNull()) {
                Ptr<TxQueueElement> txQElement = m_txQueue.front();
                McpsDataConfirmParams confirmParams;
                confirmParams.m_msduHandle = txQElement->txQMsduHandle;
                confirmParams.m_status = IEEE_802_15_4_SUCCESS;
                m_mcpsDataConfirmCallback(confirmParams);
              }
            }

            RemoveFirstTxQElement();
            m_setMacState.Cancel();
            m_setMacState = Simulator::ScheduleNow(
                &LrWpanMac::SetLrWpanMacState, this, MAC_IDLE);
            m_ifsEvent = Simulator::Schedule(
                ifsWaitTime, &LrWpanMac::IfsWaitTimeout, this, ifsWaitTime);
          } else {
            m_ackWaitTimeout.Cancel();
            if (!PrepareRetransmission()) {
              m_setMacState.Cancel();
              m_setMacState = Simulator::ScheduleNow(
                  &LrWpanMac::SetLrWpanMacState, this, MAC_IDLE);
            } else {
              m_setMacState.Cancel();
              m_setMacState = Simulator::ScheduleNow(
                  &LrWpanMac::SetLrWpanMacState, this, MAC_CSMA);
            }
          }
        }
      } else {
        m_macRxDropTrace(originalPkt);
      }
    }
  }
}

void LrWpanMac::SendAck(uint8_t seqno) {
  NS_LOG_FUNCTION(this << static_cast<uint32_t>(seqno));

  NS_ASSERT(m_lrWpanMacState == MAC_IDLE);

  LrWpanMacHeader macHdr(LrWpanMacHeader::LRWPAN_MAC_ACKNOWLEDGMENT, seqno);
  LrWpanMacTrailer macTrailer;
  Ptr<Packet> ackPacket = Create<Packet>(0);
  ackPacket->AddHeader(macHdr);
  if (Node::ChecksumEnabled()) {
    macTrailer.EnableFcs(true);
    macTrailer.SetFcs(ackPacket);
  }
  ackPacket->AddTrailer(macTrailer);

  m_txPkt = ackPacket;

  ChangeMacState(MAC_SENDING);
  m_phy->PlmeSetTRXStateRequest(IEEE_802_15_4_PHY_TX_ON);
}

void LrWpanMac::EnqueueTxQElement(Ptr<TxQueueElement> txQElement) {
  if (m_txQueue.size() < m_maxTxQueueSize) {
    m_txQueue.emplace_back(txQElement);
    m_macTxEnqueueTrace(txQElement->txQPkt);
  } else {
    if (!m_mcpsDataConfirmCallback.IsNull()) {
      McpsDataConfirmParams confirmParams;
      confirmParams.m_msduHandle = txQElement->txQMsduHandle;
      confirmParams.m_status = IEEE_802_15_4_TRANSACTION_OVERFLOW;
      m_mcpsDataConfirmCallback(confirmParams);
    }
    NS_LOG_DEBUG("TX Queue with size " << m_txQueue.size()
                                       << " is full, dropping packet");
    m_macTxDropTrace(txQElement->txQPkt);
  }
}

void LrWpanMac::RemoveFirstTxQElement() {
  Ptr<TxQueueElement> txQElement = m_txQueue.front();
  Ptr<const Packet> p = txQElement->txQPkt;
  m_numCsmacaRetry += m_csmaCa->GetNB() + 1;

  Ptr<Packet> pkt = p->Copy();
  LrWpanMacHeader hdr;
  pkt->RemoveHeader(hdr);
  if (!hdr.GetShortDstAddr().IsBroadcast() &&
      !hdr.GetShortDstAddr().IsMulticast()) {
    m_sentPktTrace(p, m_retransmission + 1, m_numCsmacaRetry);
  }

  txQElement->txQPkt = nullptr;
  txQElement = nullptr;
  m_txQueue.pop_front();
  m_txPkt = nullptr;
  m_retransmission = 0;
  m_numCsmacaRetry = 0;
  m_macTxDequeueTrace(p);
}

void LrWpanMac::AckWaitTimeout() {
  NS_LOG_FUNCTION(this);

  if (!PrepareRetransmission()) {
    SetLrWpanMacState(MAC_IDLE);
  } else {
    SetLrWpanMacState(MAC_CSMA);
  }
}

void LrWpanMac::IfsWaitTimeout(Time ifsTime) {
  auto symbolRate = (uint64_t)m_phy->GetDataOrSymbolRate(false);
  Time lifsTime = Seconds((double)m_macLIFSPeriod / symbolRate);
  Time sifsTime = Seconds((double)m_macSIFSPeriod / symbolRate);

  if (ifsTime == lifsTime) {
    NS_LOG_DEBUG("LIFS of " << m_macLIFSPeriod << " symbols ("
                            << ifsTime.As(Time::S) << ") completed ");
  } else if (ifsTime == sifsTime) {
    NS_LOG_DEBUG("SIFS of " << m_macSIFSPeriod << " symbols ("
                            << ifsTime.As(Time::S) << ") completed ");
  } else {
    NS_LOG_DEBUG("Unknown IFS size (" << ifsTime.As(Time::S) << ") completed ");
  }

  m_macIfsEndTrace(ifsTime);
  CheckQueue();
}

bool LrWpanMac::PrepareRetransmission() {
  NS_LOG_FUNCTION(this);

  if (m_retransmission >= m_macMaxFrameRetries) {
    LrWpanMacHeader peekedMacHdr;
    m_txPkt->PeekHeader(peekedMacHdr);

    if (peekedMacHdr.IsCommand()) {
      m_macTxDropTrace(m_txPkt);

      Ptr<Packet> pkt = m_txPkt->Copy();
      LrWpanMacHeader macHdr;
      CommandPayloadHeader cmdPayload;
      pkt->RemoveHeader(macHdr);
      pkt->RemoveHeader(cmdPayload);

      switch (cmdPayload.GetCommandFrameType()) {
      case CommandPayloadHeader::ASSOCIATION_REQ: {
        m_macPanId = 0xffff;
        m_macCoordShortAddress = Mac16Address("FF:FF");
        m_macCoordExtendedAddress = Mac64Address("ff:ff:ff:ff:ff:ff:ff:ed");
        m_incCapEvent.Cancel();
        m_incCfpEvent.Cancel();
        m_csmaCa->SetUnSlottedCsmaCa();
        m_incomingBeaconOrder = 15;
        m_incomingSuperframeOrder = 15;

        if (!m_mlmeAssociateConfirmCallback.IsNull()) {
          MlmeAssociateConfirmParams confirmParams;
          confirmParams.m_assocShortAddr = Mac16Address("FF:FF");
          confirmParams.m_status = MLMEASSOC_NO_ACK;
          m_mlmeAssociateConfirmCallback(confirmParams);
        }
        break;
      }
      case CommandPayloadHeader::ASSOCIATION_RESP: {
        if (!m_mlmeCommStatusIndicationCallback.IsNull()) {
          MlmeCommStatusIndicationParams commStatusParams;
          commStatusParams.m_panId = m_macPanId;
          commStatusParams.m_srcAddrMode = LrWpanMacHeader::EXTADDR;
          commStatusParams.m_srcExtAddr = macHdr.GetExtSrcAddr();
          commStatusParams.m_dstAddrMode = LrWpanMacHeader::EXTADDR;
          commStatusParams.m_dstExtAddr = macHdr.GetExtDstAddr();
          commStatusParams.m_status =
              LrWpanMlmeCommStatus::MLMECOMMSTATUS_NO_ACK;
          m_mlmeCommStatusIndicationCallback(commStatusParams);
        }
        RemovePendTxQElement(m_txPkt->Copy());
        break;
      }
      case CommandPayloadHeader::DATA_REQ: {
        m_macPanId = 0xffff;
        m_macCoordShortAddress = Mac16Address("FF:FF");
        m_macCoordExtendedAddress = Mac64Address("ff:ff:ff:ff:ff:ff:ff:ed");
        m_incCapEvent.Cancel();
        m_incCfpEvent.Cancel();
        m_csmaCa->SetUnSlottedCsmaCa();
        m_incomingBeaconOrder = 15;
        m_incomingSuperframeOrder = 15;

        if (!m_mlmePollConfirmCallback.IsNull()) {
          MlmePollConfirmParams pollConfirmParams;
          pollConfirmParams.m_status =
              LrWpanMlmePollConfirmStatus::MLMEPOLL_NO_ACK;
          m_mlmePollConfirmCallback(pollConfirmParams);
        }
        break;
      }
      default: {
        break;
      }
      }
    } else {
      Ptr<TxQueueElement> txQElement = m_txQueue.front();
      m_macTxDropTrace(txQElement->txQPkt);
      if (!m_mcpsDataConfirmCallback.IsNull()) {
        McpsDataConfirmParams confirmParams;
        confirmParams.m_msduHandle = txQElement->txQMsduHandle;
        confirmParams.m_status = IEEE_802_15_4_NO_ACK;
        m_mcpsDataConfirmCallback(confirmParams);
      }
    }

    RemoveFirstTxQElement();
    return false;
  } else {
    m_retransmission++;
    m_numCsmacaRetry += m_csmaCa->GetNB() + 1;
    return true;
  }
}

void LrWpanMac::EnqueueInd(Ptr<Packet> p) {
  Ptr<IndTxQueueElement> indTxQElement = Create<IndTxQueueElement>();
  LrWpanMacHeader peekedMacHdr;
  p->PeekHeader(peekedMacHdr);

  PurgeInd();

  NS_ASSERT(peekedMacHdr.GetDstAddrMode() == SHORT_ADDR ||
            peekedMacHdr.GetDstAddrMode() == EXT_ADDR);

  if (peekedMacHdr.GetDstAddrMode() == SHORT_ADDR) {
    indTxQElement->dstShortAddress = peekedMacHdr.GetShortDstAddr();
  } else {
    indTxQElement->dstExtAddress = peekedMacHdr.GetExtDstAddr();
  }

  indTxQElement->seqNum = peekedMacHdr.GetSeqNum();

  uint32_t unit = 0;
  if (m_macBeaconOrder == 15) {
    unit = lrwpan::aBaseSuperframeDuration * m_macTransactionPersistenceTime;
  } else {
    unit = ((static_cast<uint32_t>(1) << m_macBeaconOrder) *
            lrwpan::aBaseSuperframeDuration) *
           m_macTransactionPersistenceTime;
  }

  if (m_indTxQueue.size() < m_maxIndTxQueueSize) {
    double symbolRate = m_phy->GetDataOrSymbolRate(false);
    Time expireTime = Seconds(unit / symbolRate);
    expireTime += Simulator::Now();
    indTxQElement->expireTime = expireTime;
    indTxQElement->txQPkt = p;
    m_indTxQueue.emplace_back(indTxQElement);
    m_macIndTxEnqueueTrace(p);
  } else {
    if (!m_mlmeCommStatusIndicationCallback.IsNull()) {
      LrWpanMacHeader peekedMacHdr;
      indTxQElement->txQPkt->PeekHeader(peekedMacHdr);
      MlmeCommStatusIndicationParams commStatusParams;
      commStatusParams.m_panId = m_macPanId;
      commStatusParams.m_srcAddrMode = LrWpanMacHeader::EXTADDR;
      commStatusParams.m_srcExtAddr = peekedMacHdr.GetExtSrcAddr();
      commStatusParams.m_dstAddrMode = LrWpanMacHeader::EXTADDR;
      commStatusParams.m_dstExtAddr = peekedMacHdr.GetExtDstAddr();
      commStatusParams.m_status = MLMECOMMSTATUS_TRANSACTION_OVERFLOW;
      m_mlmeCommStatusIndicationCallback(commStatusParams);
    }
    m_macIndTxDropTrace(p);
  }
}

bool LrWpanMac::DequeueInd(Mac64Address dst, Ptr<IndTxQueueElement> entry) {
  PurgeInd();

  for (auto iter = m_indTxQueue.begin(); iter != m_indTxQueue.end(); iter++) {
    if ((*iter)->dstExtAddress == dst) {
      *entry = **iter;
      m_macIndTxDequeueTrace((*iter)->txQPkt->Copy());
      m_indTxQueue.erase(iter);
      return true;
    }
  }
  return false;
}

void LrWpanMac::PurgeInd() {
  for (uint32_t i = 0; i < m_indTxQueue.size();) {
    if (Simulator::Now() > m_indTxQueue[i]->expireTime) {
      LrWpanMacHeader peekedMacHdr;
      m_indTxQueue[i]->txQPkt->PeekHeader(peekedMacHdr);

      if (peekedMacHdr.IsCommand()) {
        if (!m_mlmeCommStatusIndicationCallback.IsNull()) {
          MlmeCommStatusIndicationParams commStatusParams;
          commStatusParams.m_panId = m_macPanId;
          commStatusParams.m_srcAddrMode = LrWpanMacHeader::EXTADDR;
          commStatusParams.m_srcExtAddr = peekedMacHdr.GetExtSrcAddr();
          commStatusParams.m_dstAddrMode = LrWpanMacHeader::EXTADDR;
          commStatusParams.m_dstExtAddr = peekedMacHdr.GetExtDstAddr();
          commStatusParams.m_status =
              LrWpanMlmeCommStatus::MLMECOMMSTATUS_TRANSACTION_EXPIRED;
          m_mlmeCommStatusIndicationCallback(commStatusParams);
        }
      } else if (peekedMacHdr.IsData()) {
        if (!m_mcpsDataConfirmCallback.IsNull()) {
          McpsDataConfirmParams confParams;
          confParams.m_status = IEEE_802_15_4_TRANSACTION_EXPIRED;
          m_mcpsDataConfirmCallback(confParams);
        }
      }
      m_macIndTxDropTrace(m_indTxQueue[i]->txQPkt->Copy());
      m_indTxQueue.erase(m_indTxQueue.begin() + i);
    } else {
      i++;
    }
  }
}

void LrWpanMac::PrintPendingTxQueue(std::ostream &os) const {
  LrWpanMacHeader peekedMacHdr;

  os << "Pending Transaction List [" << GetShortAddress() << " | "
     << GetExtendedAddress()
     << "] | CurrentTime: " << Simulator::Now().As(Time::S) << "\n"
     << "    Destination    |"
     << "    Sequence Number |"
     << "    Frame type    |"
     << "    Expire time\n";

  for (auto transaction : m_indTxQueue) {
    transaction->txQPkt->PeekHeader(peekedMacHdr);
    os << transaction->dstExtAddress << "           "
       << static_cast<uint32_t>(transaction->seqNum) << "          ";

    if (peekedMacHdr.IsCommand()) {
      os << " Command Frame   ";
    } else if (peekedMacHdr.IsData()) {
      os << " Data Frame      ";
    } else {
      os << " Unknown Frame   ";
    }

    os << transaction->expireTime.As(Time::S) << "\n";
  }
}

void LrWpanMac::PrintTxQueue(std::ostream &os) const {
  LrWpanMacHeader peekedMacHdr;

  os << "\nTx Queue [" << GetShortAddress() << " | " << GetExtendedAddress()
     << "] | CurrentTime: " << Simulator::Now().As(Time::S) << "\n"
     << "    Destination    |"
     << "    Sequence Number    |"
     << "    Dst PAN id    |"
     << "    Frame type    |\n";

  for (auto transaction : m_txQueue) {
    transaction->txQPkt->PeekHeader(peekedMacHdr);

    os << "[" << peekedMacHdr.GetShortDstAddr() << "]"
       << ", [" << peekedMacHdr.GetExtDstAddr() << "]        "
       << static_cast<uint32_t>(peekedMacHdr.GetSeqNum()) << "               "
       << peekedMacHdr.GetDstPanId() << "          ";

    if (peekedMacHdr.IsCommand()) {
      os << " Command Frame   ";
    } else if (peekedMacHdr.IsData()) {
      os << " Data Frame      ";
    } else {
      os << " Unknown Frame   ";
    }

    os << "\n";
  }
  os << "\n";
}

void LrWpanMac::RemovePendTxQElement(Ptr<Packet> p) {
  LrWpanMacHeader peekedMacHdr;
  p->PeekHeader(peekedMacHdr);

  for (auto it = m_indTxQueue.begin(); it != m_indTxQueue.end(); it++) {
    if (peekedMacHdr.GetDstAddrMode() == EXT_ADDR) {
      if (((*it)->dstExtAddress == peekedMacHdr.GetExtDstAddr()) &&
          ((*it)->seqNum == peekedMacHdr.GetSeqNum())) {
        m_macIndTxDequeueTrace(p);
        m_indTxQueue.erase(it);
        break;
      }
    } else if (peekedMacHdr.GetDstAddrMode() == SHORT_ADDR) {
      if (((*it)->dstShortAddress == peekedMacHdr.GetShortDstAddr()) &&
          ((*it)->seqNum == peekedMacHdr.GetSeqNum())) {
        m_macIndTxDequeueTrace(p);
        m_indTxQueue.erase(it);
        break;
      }
    }
  }

  p = nullptr;
}

void LrWpanMac::PdDataConfirm(LrWpanPhyEnumeration status) {
  NS_ASSERT(m_lrWpanMacState == MAC_SENDING);
  NS_LOG_FUNCTION(this << status << m_txQueue.size());

  LrWpanMacHeader macHdr;
  Time ifsWaitTime;
  double symbolRate;

  symbolRate = m_phy->GetDataOrSymbolRate(false);

  m_txPkt->PeekHeader(macHdr);

  if (status == IEEE_802_15_4_PHY_SUCCESS) {
    if (!macHdr.IsAcknowledgment()) {
      if (macHdr.IsBeacon()) {
        if (m_csmaCa->IsSlottedCsmaCa()) {
          uint64_t beaconSymbols =
              m_phy->GetPhySHRDuration() + 1 * m_phy->GetPhySymbolsPerOctet() +
              (m_txPkt->GetSize() * m_phy->GetPhySymbolsPerOctet());

          m_macBeaconTxTime =
              Simulator::Now() -
              Seconds(static_cast<double>(beaconSymbols) / symbolRate);

          m_capEvent = Simulator::ScheduleNow(&LrWpanMac::StartCAP, this,
                                              SuperframeType::OUTGOING);
          NS_LOG_DEBUG("Beacon Sent (m_macBeaconTxTime: "
                       << m_macBeaconTxTime.As(Time::S) << ")");

          if (!m_mlmeStartConfirmCallback.IsNull()) {
            MlmeStartConfirmParams mlmeConfirmParams;
            mlmeConfirmParams.m_status = MLMESTART_SUCCESS;
            m_mlmeStartConfirmCallback(mlmeConfirmParams);
          }
        }

        ifsWaitTime = Seconds(static_cast<double>(GetIfsSize()) / symbolRate);
        m_txPkt = nullptr;
      } else if (macHdr.IsAckReq()) {
        Time waitTime =
            Seconds(static_cast<double>(GetMacAckWaitDuration()) / symbolRate);
        NS_ASSERT(m_ackWaitTimeout.IsExpired());
        m_ackWaitTimeout =
            Simulator::Schedule(waitTime, &LrWpanMac::AckWaitTimeout, this);
        m_setMacState.Cancel();
        m_setMacState = Simulator::ScheduleNow(&LrWpanMac::SetLrWpanMacState,
                                               this, MAC_ACK_PENDING);
        return;
      } else if (macHdr.IsCommand()) {

        Ptr<Packet> txOriginalPkt = m_txPkt->Copy();
        LrWpanMacHeader txMacHdr;
        txOriginalPkt->RemoveHeader(txMacHdr);
        CommandPayloadHeader txMacPayload;
        txOriginalPkt->RemoveHeader(txMacPayload);

        if (txMacPayload.GetCommandFrameType() ==
            CommandPayloadHeader::COOR_REALIGN) {
          if (!m_mlmeCommStatusIndicationCallback.IsNull()) {
            MlmeCommStatusIndicationParams commStatusParams;
            commStatusParams.m_panId = m_macPanId;

            commStatusParams.m_srcAddrMode = macHdr.GetSrcAddrMode();
            commStatusParams.m_srcExtAddr = macHdr.GetExtSrcAddr();
            commStatusParams.m_srcShortAddr = macHdr.GetShortSrcAddr();

            commStatusParams.m_dstAddrMode = macHdr.GetDstAddrMode();
            commStatusParams.m_dstExtAddr = macHdr.GetExtDstAddr();
            commStatusParams.m_dstShortAddr = macHdr.GetShortDstAddr();

            commStatusParams.m_status =
                LrWpanMlmeCommStatus::MLMECOMMSTATUS_SUCCESS;
            m_mlmeCommStatusIndicationCallback(commStatusParams);
          }
        }

        ifsWaitTime = Seconds(static_cast<double>(GetIfsSize()) / symbolRate);
        RemoveFirstTxQElement();
      } else {
        m_macTxOkTrace(m_txPkt);
        if (!m_mcpsDataConfirmCallback.IsNull()) {
          McpsDataConfirmParams confirmParams;
          NS_ASSERT_MSG(!m_txQueue.empty(), "TxQsize = 0");
          Ptr<TxQueueElement> txQElement = m_txQueue.front();
          confirmParams.m_msduHandle = txQElement->txQMsduHandle;
          confirmParams.m_status = IEEE_802_15_4_SUCCESS;
          m_mcpsDataConfirmCallback(confirmParams);
        }
        ifsWaitTime = Seconds(static_cast<double>(GetIfsSize()) / symbolRate);
        RemoveFirstTxQElement();
      }
    } else {

      Ptr<Packet> recvOriginalPkt = m_rxPkt->Copy();
      LrWpanMacHeader receivedMacHdr;
      recvOriginalPkt->RemoveHeader(receivedMacHdr);

      if (receivedMacHdr.IsCommand()) {
        CommandPayloadHeader receivedMacPayload;
        recvOriginalPkt->RemoveHeader(receivedMacPayload);

        if (receivedMacPayload.GetCommandFrameType() ==
            CommandPayloadHeader::ASSOCIATION_REQ) {
          if (!m_mlmeAssociateIndicationCallback.IsNull()) {
            MlmeAssociateIndicationParams associateParams;
            associateParams.capabilityInfo =
                receivedMacPayload.GetCapabilityField();
            associateParams.m_extDevAddr = receivedMacHdr.GetExtSrcAddr();
            associateParams.lqi = m_lastRxFrameLqi;
            m_mlmeAssociateIndicationCallback(associateParams);
          }

          m_rxPkt = nullptr;
        } else if (receivedMacPayload.GetCommandFrameType() ==
                   CommandPayloadHeader::ASSOCIATION_RESP) {
          MlmeAssociateConfirmParams confirmParams;

          switch (receivedMacPayload.GetAssociationStatus()) {
          case CommandPayloadHeader::SUCCESSFUL:
            SetShortAddress(receivedMacPayload.GetShortAddr());
            m_macPanId = receivedMacHdr.GetSrcPanId();

            confirmParams.m_status =
                LrWpanMlmeAssociateConfirmStatus::MLMEASSOC_SUCCESS;
            confirmParams.m_assocShortAddr = GetShortAddress();
            break;
          case CommandPayloadHeader::FULL_CAPACITY:
            confirmParams.m_status =
                LrWpanMlmeAssociateConfirmStatus::MLMEASSOC_FULL_CAPACITY;
            m_macPanId = 0xffff;
            m_macCoordShortAddress = Mac16Address("FF:FF");
            m_macCoordExtendedAddress = Mac64Address("ff:ff:ff:ff:ff:ff:ff:ed");
            m_incCapEvent.Cancel();
            m_incCfpEvent.Cancel();
            m_csmaCa->SetUnSlottedCsmaCa();
            m_incomingBeaconOrder = 15;
            m_incomingSuperframeOrder = 15;
            break;
          case CommandPayloadHeader::ACCESS_DENIED:
            confirmParams.m_status =
                LrWpanMlmeAssociateConfirmStatus::MLMEASSOC_ACCESS_DENIED;
            m_macPanId = 0xffff;
            m_macCoordShortAddress = Mac16Address("FF:FF");
            m_macCoordExtendedAddress = Mac64Address("ff:ff:ff:ff:ff:ff:ff:ed");
            m_incCapEvent.Cancel();
            m_incCfpEvent.Cancel();
            m_csmaCa->SetUnSlottedCsmaCa();
            m_incomingBeaconOrder = 15;
            m_incomingSuperframeOrder = 15;
            break;
          }

          if (!m_mlmeAssociateConfirmCallback.IsNull()) {
            m_mlmeAssociateConfirmCallback(confirmParams);
          }
        } else if (receivedMacPayload.GetCommandFrameType() ==
                   CommandPayloadHeader::DATA_REQ) {
          SendAssocResponseCommand(m_rxPkt->Copy());
        }
      }

      m_txPkt = nullptr;
    }
  } else if (status == IEEE_802_15_4_PHY_UNSPECIFIED) {
    if (!macHdr.IsAcknowledgment()) {
      NS_ASSERT_MSG(!m_txQueue.empty(), "TxQsize = 0");
      Ptr<TxQueueElement> txQElement = m_txQueue.front();
      m_macTxDropTrace(txQElement->txQPkt);
      if (!m_mcpsDataConfirmCallback.IsNull()) {
        McpsDataConfirmParams confirmParams;
        confirmParams.m_msduHandle = txQElement->txQMsduHandle;
        confirmParams.m_status = IEEE_802_15_4_FRAME_TOO_LONG;
        m_mcpsDataConfirmCallback(confirmParams);
      }
      RemoveFirstTxQElement();
    } else {
      NS_LOG_ERROR("Unable to send ACK");
    }
  } else {
    NS_FATAL_ERROR("Transmission attempt failed with PHY status " << status);
  }

  if (!ifsWaitTime.IsZero()) {
    m_ifsEvent = Simulator::Schedule(ifsWaitTime, &LrWpanMac::IfsWaitTimeout,
                                     this, ifsWaitTime);
  }

  m_setMacState.Cancel();
  m_setMacState =
      Simulator::ScheduleNow(&LrWpanMac::SetLrWpanMacState, this, MAC_IDLE);
}

void LrWpanMac::PlmeCcaConfirm(LrWpanPhyEnumeration status) {
  NS_LOG_FUNCTION(this << status);
  m_csmaCa->PlmeCcaConfirm(status);
}

void LrWpanMac::PlmeEdConfirm(LrWpanPhyEnumeration status,
                              uint8_t energyLevel) {
  NS_LOG_FUNCTION(this << status << energyLevel);

  if (energyLevel > m_maxEnergyLevel) {
    m_maxEnergyLevel = energyLevel;
  }

  if (Simulator::GetDelayLeft(m_scanEnergyEvent) >
      Seconds(8.0 / m_phy->GetDataOrSymbolRate(false))) {
    m_phy->PlmeEdRequest();
  }
}

void LrWpanMac::PlmeGetAttributeConfirm(LrWpanPhyEnumeration status,
                                        LrWpanPibAttributeIdentifier id,
                                        Ptr<LrWpanPhyPibAttributes> attribute) {
  NS_LOG_FUNCTION(this << status << id << attribute);
}

void LrWpanMac::PlmeSetTRXStateConfirm(LrWpanPhyEnumeration status) {
  NS_LOG_FUNCTION(this << status);

  if (m_lrWpanMacState == MAC_SENDING &&
      (status == IEEE_802_15_4_PHY_TX_ON ||
       status == IEEE_802_15_4_PHY_SUCCESS)) {
    NS_ASSERT(m_txPkt);

    m_promiscSnifferTrace(m_txPkt);
    m_snifferTrace(m_txPkt);
    m_macTxTrace(m_txPkt);
    m_phy->PdDataRequest(m_txPkt->GetSize(), m_txPkt);
  } else if (m_lrWpanMacState == MAC_CSMA &&
             (status == IEEE_802_15_4_PHY_RX_ON ||
              status == IEEE_802_15_4_PHY_SUCCESS)) {
    m_csmaCa->Start();
  } else if (m_lrWpanMacState == MAC_IDLE) {
    NS_ASSERT(status == IEEE_802_15_4_PHY_RX_ON ||
              status == IEEE_802_15_4_PHY_SUCCESS ||
              status == IEEE_802_15_4_PHY_TRX_OFF);

    if (status == IEEE_802_15_4_PHY_RX_ON && m_scanEnergyEvent.IsRunning()) {
      m_phy->PlmeEdRequest();
    } else if (status == IEEE_802_15_4_PHY_RX_ON ||
               status == IEEE_802_15_4_PHY_SUCCESS) {
      CheckQueue();
    }
  } else if (m_lrWpanMacState == MAC_ACK_PENDING) {
    NS_ASSERT(status == IEEE_802_15_4_PHY_RX_ON ||
              status == IEEE_802_15_4_PHY_SUCCESS);
  } else {
    NS_FATAL_ERROR("Error changing transceiver state");
  }
}

void LrWpanMac::PlmeSetAttributeConfirm(LrWpanPhyEnumeration status,
                                        LrWpanPibAttributeIdentifier id) {
  NS_LOG_FUNCTION(this << status << id);
  if (id == LrWpanPibAttributeIdentifier::phyCurrentPage &&
      m_pendPrimitive == MLME_SCAN_REQ) {
    if (status == LrWpanPhyEnumeration::IEEE_802_15_4_PHY_SUCCESS) {
      bool channelFound = false;
      for (int i = m_channelScanIndex; i <= 26; i++) {
        if ((m_scanParams.m_scanChannels & (1 << m_channelScanIndex)) != 0) {
          channelFound = true;
          break;
        }
        m_channelScanIndex++;
      }

      if (channelFound) {
        Ptr<LrWpanPhyPibAttributes> pibAttr = Create<LrWpanPhyPibAttributes>();
        pibAttr->phyCurrentChannel = m_channelScanIndex;
        m_phy->PlmeSetAttributeRequest(
            LrWpanPibAttributeIdentifier::phyCurrentChannel, pibAttr);
      }
    } else {
      if (!m_mlmeScanConfirmCallback.IsNull()) {
        MlmeScanConfirmParams confirmParams;
        confirmParams.m_scanType = m_scanParams.m_scanType;
        confirmParams.m_chPage = m_scanParams.m_chPage;
        confirmParams.m_status = MLMESCAN_INVALID_PARAMETER;
        m_mlmeScanConfirmCallback(confirmParams);
      }
      NS_LOG_ERROR(this << "Channel Scan: Invalid channel page");
    }
  } else if (id == LrWpanPibAttributeIdentifier::phyCurrentChannel &&
             m_pendPrimitive == MLME_SCAN_REQ) {
    if (status == LrWpanPhyEnumeration::IEEE_802_15_4_PHY_SUCCESS) {
      auto symbolRate =
          static_cast<uint64_t>(m_phy->GetDataOrSymbolRate(false));
      Time nextScanTime;

      if (m_scanParams.m_scanType == MLMESCAN_ORPHAN) {
        nextScanTime =
            Seconds(static_cast<double>(m_macResponseWaitTime) / symbolRate);
      } else {
        uint64_t scanDurationSym = lrwpan::aBaseSuperframeDuration *
                                   (pow(2, m_scanParams.m_scanDuration) + 1);

        nextScanTime =
            Seconds(static_cast<double>(scanDurationSym) / symbolRate);
      }

      switch (m_scanParams.m_scanType) {
      case MLMESCAN_ED:
        m_maxEnergyLevel = 0;
        m_scanEnergyEvent = Simulator::Schedule(
            nextScanTime, &LrWpanMac::EndChannelEnergyScan, this);
        m_phy->PlmeSetTRXStateRequest(IEEE_802_15_4_PHY_RX_ON);
        break;
      case MLMESCAN_ACTIVE:
        m_scanEvent =
            Simulator::Schedule(nextScanTime, &LrWpanMac::EndChannelScan, this);
        SendBeaconRequestCommand();
        break;
      case MLMESCAN_PASSIVE:
        m_scanEvent =
            Simulator::Schedule(nextScanTime, &LrWpanMac::EndChannelScan, this);
        m_phy->PlmeSetTRXStateRequest(IEEE_802_15_4_PHY_RX_ON);
        break;
      case MLMESCAN_ORPHAN:
        m_scanOrphanEvent =
            Simulator::Schedule(nextScanTime, &LrWpanMac::EndChannelScan, this);
        SendOrphanNotificationCommand();
        break;

      default:
        MlmeScanConfirmParams confirmParams;
        confirmParams.m_scanType = m_scanParams.m_scanType;
        confirmParams.m_chPage = m_scanParams.m_chPage;
        confirmParams.m_status = MLMESCAN_INVALID_PARAMETER;
        if (!m_mlmeScanConfirmCallback.IsNull()) {
          m_mlmeScanConfirmCallback(confirmParams);
        }
        NS_LOG_ERROR("Scan Type currently not supported");
        return;
      }
    } else {
      if (!m_mlmeScanConfirmCallback.IsNull()) {
        MlmeScanConfirmParams confirmParams;
        confirmParams.m_scanType = m_scanParams.m_scanType;
        confirmParams.m_chPage = m_scanParams.m_chPage;
        confirmParams.m_status = MLMESCAN_INVALID_PARAMETER;
        m_mlmeScanConfirmCallback(confirmParams);
      }
      NS_LOG_ERROR("Channel " << m_channelScanIndex
                              << " could not be set in the current page");
    }
  } else if (id == LrWpanPibAttributeIdentifier::phyCurrentPage &&
             m_pendPrimitive == MLME_START_REQ) {
    if (status == LrWpanPhyEnumeration::IEEE_802_15_4_PHY_SUCCESS) {
      Ptr<LrWpanPhyPibAttributes> pibAttr = Create<LrWpanPhyPibAttributes>();
      pibAttr->phyCurrentChannel = m_startParams.m_logCh;
      m_phy->PlmeSetAttributeRequest(
          LrWpanPibAttributeIdentifier::phyCurrentChannel, pibAttr);
    } else {
      if (!m_mlmeStartConfirmCallback.IsNull()) {
        MlmeStartConfirmParams confirmParams;
        confirmParams.m_status = MLMESTART_INVALID_PARAMETER;
        m_mlmeStartConfirmCallback(confirmParams);
      }
      NS_LOG_ERROR("Invalid page parameter in MLME-start");
    }
  } else if (id == LrWpanPibAttributeIdentifier::phyCurrentChannel &&
             m_pendPrimitive == MLME_START_REQ) {
    if (status == LrWpanPhyEnumeration::IEEE_802_15_4_PHY_SUCCESS) {
      EndStartRequest();
    } else {
      if (!m_mlmeStartConfirmCallback.IsNull()) {
        MlmeStartConfirmParams confirmParams;
        confirmParams.m_status = MLMESTART_INVALID_PARAMETER;
        m_mlmeStartConfirmCallback(confirmParams);
      }
      NS_LOG_ERROR("Invalid channel parameter in MLME-start");
    }
  } else if (id == LrWpanPibAttributeIdentifier::phyCurrentPage &&
             m_pendPrimitive == MLME_ASSOC_REQ) {
    if (status == LrWpanPhyEnumeration::IEEE_802_15_4_PHY_SUCCESS) {
      Ptr<LrWpanPhyPibAttributes> pibAttr = Create<LrWpanPhyPibAttributes>();
      pibAttr->phyCurrentChannel = m_associateParams.m_chNum;
      m_phy->PlmeSetAttributeRequest(
          LrWpanPibAttributeIdentifier::phyCurrentChannel, pibAttr);
    } else {
      m_macPanId = 0xffff;
      m_macCoordShortAddress = Mac16Address("FF:FF");
      m_macCoordExtendedAddress = Mac64Address("ff:ff:ff:ff:ff:ff:ff:ed");
      m_incCapEvent.Cancel();
      m_incCfpEvent.Cancel();
      m_csmaCa->SetUnSlottedCsmaCa();
      m_incomingBeaconOrder = 15;
      m_incomingSuperframeOrder = 15;

      if (!m_mlmeAssociateConfirmCallback.IsNull()) {
        MlmeAssociateConfirmParams confirmParams;
        confirmParams.m_assocShortAddr = Mac16Address("FF:FF");
        confirmParams.m_status = MLMEASSOC_INVALID_PARAMETER;
        m_mlmeAssociateConfirmCallback(confirmParams);
      }
      NS_LOG_ERROR("Invalid page parameter in MLME-associate");
    }
  } else if (id == LrWpanPibAttributeIdentifier::phyCurrentChannel &&
             m_pendPrimitive == MLME_ASSOC_REQ) {
    if (status == LrWpanPhyEnumeration::IEEE_802_15_4_PHY_SUCCESS) {
      EndAssociateRequest();
    } else {
      m_macPanId = 0xffff;
      m_macCoordShortAddress = Mac16Address("FF:FF");
      m_macCoordExtendedAddress = Mac64Address("ff:ff:ff:ff:ff:ff:ff:ed");
      m_incCapEvent.Cancel();
      m_incCfpEvent.Cancel();
      m_csmaCa->SetUnSlottedCsmaCa();
      m_incomingBeaconOrder = 15;
      m_incomingSuperframeOrder = 15;

      if (!m_mlmeAssociateConfirmCallback.IsNull()) {
        MlmeAssociateConfirmParams confirmParams;
        confirmParams.m_assocShortAddr = Mac16Address("FF:FF");
        confirmParams.m_status = MLMEASSOC_INVALID_PARAMETER;
        m_mlmeAssociateConfirmCallback(confirmParams);
      }
      NS_LOG_ERROR("Invalid channel parameter in MLME-associate");
    }
  }
}

void LrWpanMac::SetLrWpanMacState(LrWpanMacState macState) {
  NS_LOG_FUNCTION(this << "mac state = " << macState);

  if (macState == MAC_IDLE) {
    ChangeMacState(MAC_IDLE);
    if (m_macRxOnWhenIdle) {
      m_phy->PlmeSetTRXStateRequest(IEEE_802_15_4_PHY_RX_ON);
    } else {
      m_phy->PlmeSetTRXStateRequest(IEEE_802_15_4_PHY_TRX_OFF);
    }
  } else if (macState == MAC_ACK_PENDING) {
    ChangeMacState(MAC_ACK_PENDING);
    m_phy->PlmeSetTRXStateRequest(IEEE_802_15_4_PHY_RX_ON);
  } else if (macState == MAC_CSMA) {
    NS_ASSERT(m_lrWpanMacState == MAC_IDLE ||
              m_lrWpanMacState == MAC_ACK_PENDING);
    ChangeMacState(MAC_CSMA);
    m_phy->PlmeSetTRXStateRequest(IEEE_802_15_4_PHY_RX_ON);
  } else if (m_lrWpanMacState == MAC_CSMA && macState == CHANNEL_IDLE) {
    ChangeMacState(MAC_SENDING);
    m_phy->PlmeSetTRXStateRequest(IEEE_802_15_4_PHY_TX_ON);
  } else if (m_lrWpanMacState == MAC_CSMA &&
             macState == CHANNEL_ACCESS_FAILURE) {
    NS_ASSERT(m_txPkt);

    NS_LOG_DEBUG(this << " cannot find clear channel");

    m_macTxDropTrace(m_txPkt);

    Ptr<Packet> pkt = m_txPkt->Copy();
    LrWpanMacHeader macHdr;
    pkt->RemoveHeader(macHdr);

    if (macHdr.IsCommand()) {
      CommandPayloadHeader cmdPayload;
      pkt->RemoveHeader(cmdPayload);

      switch (cmdPayload.GetCommandFrameType()) {
      case CommandPayloadHeader::ASSOCIATION_REQ: {
        m_macPanId = 0xffff;
        m_macCoordShortAddress = Mac16Address("FF:FF");
        m_macCoordExtendedAddress = Mac64Address("ff:ff:ff:ff:ff:ff:ff:ed");
        m_incCapEvent.Cancel();
        m_incCfpEvent.Cancel();
        m_csmaCa->SetUnSlottedCsmaCa();
        m_incomingBeaconOrder = 15;
        m_incomingSuperframeOrder = 15;

        if (!m_mlmeAssociateConfirmCallback.IsNull()) {
          MlmeAssociateConfirmParams confirmParams;
          confirmParams.m_assocShortAddr = Mac16Address("FF:FF");
          confirmParams.m_status = MLMEASSOC_CHANNEL_ACCESS_FAILURE;
          m_mlmeAssociateConfirmCallback(confirmParams);
        }
        break;
      }
      case CommandPayloadHeader::ASSOCIATION_RESP: {
        if (!m_mlmeCommStatusIndicationCallback.IsNull()) {
          MlmeCommStatusIndicationParams commStatusParams;
          commStatusParams.m_panId = m_macPanId;
          commStatusParams.m_srcAddrMode = LrWpanMacHeader::EXTADDR;
          commStatusParams.m_srcExtAddr = macHdr.GetExtSrcAddr();
          commStatusParams.m_dstAddrMode = LrWpanMacHeader::EXTADDR;
          commStatusParams.m_dstExtAddr = macHdr.GetExtDstAddr();
          commStatusParams.m_status =
              LrWpanMlmeCommStatus::MLMECOMMSTATUS_CHANNEL_ACCESS_FAILURE;
          m_mlmeCommStatusIndicationCallback(commStatusParams);
        }
        RemovePendTxQElement(m_txPkt->Copy());
        break;
      }
      case CommandPayloadHeader::DATA_REQ: {
        m_macPanId = 0xffff;
        m_macCoordShortAddress = Mac16Address("FF:FF");
        m_macCoordExtendedAddress = Mac64Address("ff:ff:ff:ff:ff:ff:ff:ed");
        m_incCapEvent.Cancel();
        m_incCfpEvent.Cancel();
        m_csmaCa->SetUnSlottedCsmaCa();
        m_incomingBeaconOrder = 15;
        m_incomingSuperframeOrder = 15;

        if (!m_mlmePollConfirmCallback.IsNull()) {
          MlmePollConfirmParams pollConfirmParams;
          pollConfirmParams.m_status =
              LrWpanMlmePollConfirmStatus::MLMEPOLL_CHANNEL_ACCESS_FAILURE;
          m_mlmePollConfirmCallback(pollConfirmParams);
        }
        break;
      }
      case CommandPayloadHeader::COOR_REALIGN: {
        if (!m_mlmeCommStatusIndicationCallback.IsNull()) {
          MlmeCommStatusIndicationParams commStatusParams;
          commStatusParams.m_panId = m_macPanId;
          commStatusParams.m_srcAddrMode = LrWpanMacHeader::EXTADDR;
          commStatusParams.m_srcExtAddr = macHdr.GetExtSrcAddr();
          commStatusParams.m_dstAddrMode = LrWpanMacHeader::EXTADDR;
          commStatusParams.m_dstExtAddr = macHdr.GetExtDstAddr();
          commStatusParams.m_status =
              LrWpanMlmeCommStatus::MLMECOMMSTATUS_CHANNEL_ACCESS_FAILURE;
          m_mlmeCommStatusIndicationCallback(commStatusParams);
        }
        break;
      }
      case CommandPayloadHeader::ORPHAN_NOTIF: {
        if (m_scanOrphanEvent.IsRunning()) {
          m_unscannedChannels.emplace_back(m_phy->GetCurrentChannelNum());
        }
        break;
      }
      case CommandPayloadHeader::BEACON_REQ: {
        if (m_scanEvent.IsRunning()) {
          m_unscannedChannels.emplace_back(m_phy->GetCurrentChannelNum());
        }
        break;
      }
      default: {
        break;
      }
      }
      RemoveFirstTxQElement();
    } else if (macHdr.IsData()) {
      if (!m_mcpsDataConfirmCallback.IsNull()) {
        McpsDataConfirmParams confirmParams;
        confirmParams.m_msduHandle = m_txQueue.front()->txQMsduHandle;
        confirmParams.m_status = IEEE_802_15_4_CHANNEL_ACCESS_FAILURE;
        m_mcpsDataConfirmCallback(confirmParams);
      }
      RemoveFirstTxQElement();
    } else {
      m_txPkt = nullptr;
      m_retransmission = 0;
      m_numCsmacaRetry = 0;
    }

    ChangeMacState(MAC_IDLE);
    if (m_macRxOnWhenIdle) {
      m_phy->PlmeSetTRXStateRequest(IEEE_802_15_4_PHY_RX_ON);
    } else {
      m_phy->PlmeSetTRXStateRequest(IEEE_802_15_4_PHY_TRX_OFF);
    }
  } else if (m_lrWpanMacState == MAC_CSMA && macState == MAC_CSMA_DEFERRED) {
    ChangeMacState(MAC_IDLE);
    m_txPkt = nullptr;

    NS_LOG_DEBUG("****** PACKET DEFERRED to the next superframe *****");
  }
}

LrWpanAssociationStatus LrWpanMac::GetAssociationStatus() const {
  return m_associationStatus;
}

void LrWpanMac::SetAssociationStatus(LrWpanAssociationStatus status) {
  m_associationStatus = status;
}

void LrWpanMac::SetTxQMaxSize(uint32_t queueSize) {
  m_maxTxQueueSize = queueSize;
}

void LrWpanMac::SetIndTxQMaxSize(uint32_t queueSize) {
  m_maxIndTxQueueSize = queueSize;
}

uint16_t LrWpanMac::GetPanId() const { return m_macPanId; }

Mac16Address LrWpanMac::GetCoordShortAddress() const {
  return m_macCoordShortAddress;
}

Mac64Address LrWpanMac::GetCoordExtAddress() const {
  return m_macCoordExtendedAddress;
}

void LrWpanMac::SetPanId(uint16_t panId) { m_macPanId = panId; }

void LrWpanMac::ChangeMacState(LrWpanMacState newState) {
  NS_LOG_LOGIC(this << " change lrwpan mac state from " << m_lrWpanMacState
                    << " to " << newState);
  m_macStateLogger(m_lrWpanMacState, newState);
  m_lrWpanMacState = newState;
}

uint64_t LrWpanMac::GetMacAckWaitDuration() const {
  return lrwpan::aUnitBackoffPeriod + lrwpan::aTurnaroundTime +
         m_phy->GetPhySHRDuration() + ceil(6 * m_phy->GetPhySymbolsPerOctet());
}

uint8_t LrWpanMac::GetMacMaxFrameRetries() const {
  return m_macMaxFrameRetries;
}

void LrWpanMac::PrintTransmitQueueSize() {
  NS_LOG_DEBUG("Transmit Queue Size: " << m_txQueue.size());
}

void LrWpanMac::SetMacMaxFrameRetries(uint8_t retries) {
  m_macMaxFrameRetries = retries;
}

bool LrWpanMac::isCoordDest() {
  NS_ASSERT(m_txPkt);
  LrWpanMacHeader macHdr;
  m_txPkt->PeekHeader(macHdr);

  if (m_coor) {
    return false;
  } else if (m_macCoordShortAddress == macHdr.GetShortDstAddr() ||
             m_macCoordExtendedAddress == macHdr.GetExtDstAddr()) {
    return true;
  } else {
    NS_LOG_DEBUG("ERROR: Packet not for the coordinator!");
    return false;
  }
}

uint32_t LrWpanMac::GetIfsSize() {
  NS_ASSERT(m_txPkt);

  if (m_txPkt->GetSize() <= lrwpan::aMaxSIFSFrameSize) {
    return m_macSIFSPeriod;
  } else {
    return m_macLIFSPeriod;
  }
}

void LrWpanMac::SetAssociatedCoor(Mac16Address mac) {
  m_macCoordShortAddress = mac;
}

void LrWpanMac::SetAssociatedCoor(Mac64Address mac) {
  m_macCoordExtendedAddress = mac;
}

uint64_t LrWpanMac::GetTxPacketSymbols() {
  NS_ASSERT(m_txPkt);
  return (m_phy->GetPhySHRDuration() + 1 * m_phy->GetPhySymbolsPerOctet() +
          (m_txPkt->GetSize() * m_phy->GetPhySymbolsPerOctet()));
}

bool LrWpanMac::isTxAckReq() {
  NS_ASSERT(m_txPkt);
  LrWpanMacHeader macHdr;
  m_txPkt->PeekHeader(macHdr);

  return macHdr.IsAckReq();
}

} // namespace ns3
