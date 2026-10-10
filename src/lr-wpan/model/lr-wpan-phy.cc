#include "lr-wpan-phy.h"

#include "lr-wpan-constants.h"
#include "lr-wpan-error-model.h"
#include "lr-wpan-lqi-tag.h"
#include "lr-wpan-net-device.h"
#include "lr-wpan-spectrum-signal-parameters.h"
#include "lr-wpan-spectrum-value-helper.h"

#include <ns3/abort.h>
#include <ns3/antenna-model.h>
#include <ns3/double.h>
#include <ns3/error-model.h>
#include <ns3/log.h>
#include <ns3/mobility-model.h>
#include <ns3/net-device.h>
#include <ns3/node.h>
#include <ns3/packet-burst.h>
#include <ns3/packet.h>
#include <ns3/pointer.h>
#include <ns3/random-variable-stream.h>
#include <ns3/simulator.h>
#include <ns3/spectrum-channel.h>
#include <ns3/spectrum-value.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("LrWpanPhy");

NS_OBJECT_ENSURE_REGISTERED(LrWpanPhy);

static const LrWpanPhyDataAndSymbolRates
    dataSymbolRates[IEEE_802_15_4_INVALID_PHY_OPTION]{
        {20.0, 20.0},  {40.0, 40.0},  {20.0, 20.0},
        {250.0, 12.5}, {250.0, 50.0}, {250.0, 62.5},
        {100.0, 25.0}, {250.0, 62.5}, {250.0, 62.5},
    };

const LrWpanPhyPpduHeaderSymbolNumber
    ppduHeaderSymbolNumbers[IEEE_802_15_4_INVALID_PHY_OPTION]{
        {32.0, 8.0, 8.0}, {32.0, 8.0, 8.0}, {32.0, 8.0, 8.0},
        {2.0, 1.0, 0.4},  {6.0, 1.0, 1.6},  {8.0, 2.0, 2.0},
        {8.0, 2.0, 2.0},  {8.0, 2.0, 2.0},  {8.0, 2.0, 2.0},
    };

TypeId LrWpanPhy::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::LrWpanPhy")
          .SetParent<SpectrumPhy>()
          .SetGroupName("LrWpan")
          .AddConstructor<LrWpanPhy>()
          .AddAttribute(
              "PostReceptionErrorModel",
              "An optional packet error model can be added to the receive "
              "packet process after any propagation-based (SNR-based) error "
              "models have been applied. Typically this is used to force "
              "specific packet drops, for testing purposes.",
              PointerValue(),
              MakePointerAccessor(&LrWpanPhy::m_postReceptionErrorModel),
              MakePointerChecker<ErrorModel>())
          .AddTraceSource("TrxStateValue", "The state of the transceiver",
                          MakeTraceSourceAccessor(&LrWpanPhy::m_trxState),
                          "ns3::TracedValueCallback::LrWpanPhyEnumeration")
          .AddTraceSource("TrxState", "The state of the transceiver",
                          MakeTraceSourceAccessor(&LrWpanPhy::m_trxStateLogger),
                          "ns3::LrWpanPhy::StateTracedCallback")
          .AddTraceSource(
              "PhyTxBegin",
              "Trace source indicating a packet has "
              "begun transmitting over the channel medium",
              MakeTraceSourceAccessor(&LrWpanPhy::m_phyTxBeginTrace),
              "ns3::Packet::TracedCallback")
          .AddTraceSource("PhyTxEnd",
                          "Trace source indicating a packet has been "
                          "completely transmitted over the channel.",
                          MakeTraceSourceAccessor(&LrWpanPhy::m_phyTxEndTrace),
                          "ns3::Packet::TracedCallback")
          .AddTraceSource("PhyTxDrop",
                          "Trace source indicating a packet has been "
                          "dropped by the device during transmission",
                          MakeTraceSourceAccessor(&LrWpanPhy::m_phyTxDropTrace),
                          "ns3::Packet::TracedCallback")
          .AddTraceSource(
              "PhyRxBegin",
              "Trace source indicating a packet has begun "
              "being received from the channel medium by the device",
              MakeTraceSourceAccessor(&LrWpanPhy::m_phyRxBeginTrace),
              "ns3::Packet::TracedCallback")
          .AddTraceSource("PhyRxEnd",
                          "Trace source indicating a packet has been "
                          "completely received from the channel medium "
                          "by the device",
                          MakeTraceSourceAccessor(&LrWpanPhy::m_phyRxEndTrace),
                          "ns3::Packet::SinrTracedCallback")
          .AddTraceSource("PhyRxDrop",
                          "Trace source indicating a packet has been "
                          "dropped by the device during reception",
                          MakeTraceSourceAccessor(&LrWpanPhy::m_phyRxDropTrace),
                          "ns3::Packet::TracedCallback");
  return tid;
}

LrWpanPhy::LrWpanPhy() : m_edRequest(), m_setTRXState() {
  m_trxState = IEEE_802_15_4_PHY_TRX_OFF;
  m_trxStatePending = IEEE_802_15_4_PHY_IDLE;

  m_phyPIBAttributes.phyTransmitPower = 0;
  m_phyPIBAttributes.phyCCAMode = 1;

  SetPhyOption(IEEE_802_15_4_2_4GHZ_OQPSK);

  m_random = CreateObject<UniformRandomVariable>();
  m_random->SetAttribute("Min", DoubleValue(0.0));
  m_random->SetAttribute("Max", DoubleValue(1.0));

  m_isRxCanceled = false;
  ChangeTrxState(IEEE_802_15_4_PHY_TRX_OFF);
}

LrWpanPhy::~LrWpanPhy() {}

void LrWpanPhy::DoInitialize() {
  NS_LOG_FUNCTION(this);

  if (!m_mobility) {
    NS_ABORT_MSG_UNLESS(
        m_device && m_device->GetNode(),
        "Either install a MobilityModel on this object or ensure that this "
        "object is part of a Node and NetDevice");
    m_mobility = m_device->GetNode()->GetObject<MobilityModel>();
    if (!m_mobility) {
      NS_LOG_WARN(
          "Mobility not found, propagation models might not work properly");
    }
  }
}

void LrWpanPhy::DoDispose() {
  NS_LOG_FUNCTION(this);

  m_setTRXState.Cancel();
  m_trxState = IEEE_802_15_4_PHY_TRX_OFF;
  m_trxStatePending = IEEE_802_15_4_PHY_IDLE;

  m_mobility = nullptr;
  m_device = nullptr;
  m_channel = nullptr;
  m_antenna = nullptr;
  m_txPsd = nullptr;
  m_noise = nullptr;
  m_signal = nullptr;
  m_errorModel = nullptr;
  m_currentRxPacket.first = nullptr;
  m_currentTxPacket.first = nullptr;
  m_postReceptionErrorModel = nullptr;

  m_ccaRequest.Cancel();
  m_edRequest.Cancel();
  m_setTRXState.Cancel();
  m_pdDataRequest.Cancel();

  m_random = nullptr;
  m_pdDataIndicationCallback =
      MakeNullCallback<void, uint32_t, Ptr<Packet>, uint8_t>();
  m_pdDataConfirmCallback = MakeNullCallback<void, LrWpanPhyEnumeration>();
  m_plmeCcaConfirmCallback = MakeNullCallback<void, LrWpanPhyEnumeration>();
  m_plmeEdConfirmCallback =
      MakeNullCallback<void, LrWpanPhyEnumeration, uint8_t>();
  m_plmeGetAttributeConfirmCallback =
      MakeNullCallback<void, LrWpanPhyEnumeration, LrWpanPibAttributeIdentifier,
                       Ptr<LrWpanPhyPibAttributes>>();
  m_plmeSetTRXStateConfirmCallback =
      MakeNullCallback<void, LrWpanPhyEnumeration>();
  m_plmeSetAttributeConfirmCallback =
      MakeNullCallback<void, LrWpanPhyEnumeration,
                       LrWpanPibAttributeIdentifier>();

  SpectrumPhy::DoDispose();
}

Ptr<NetDevice> LrWpanPhy::GetDevice() const {
  NS_LOG_FUNCTION(this);
  return m_device;
}

Ptr<MobilityModel> LrWpanPhy::GetMobility() const {
  NS_LOG_FUNCTION(this);
  return m_mobility;
}

void LrWpanPhy::SetDevice(Ptr<NetDevice> d) {
  NS_LOG_FUNCTION(this << d);
  m_device = d;
}

void LrWpanPhy::SetMobility(Ptr<MobilityModel> m) {
  NS_LOG_FUNCTION(this << m);
  m_mobility = m;
}

void LrWpanPhy::SetChannel(Ptr<SpectrumChannel> c) {
  NS_LOG_FUNCTION(this << c);
  m_channel = c;
}

Ptr<SpectrumChannel> LrWpanPhy::GetChannel() {
  NS_LOG_FUNCTION(this);
  return m_channel;
}

Ptr<const SpectrumModel> LrWpanPhy::GetRxSpectrumModel() const {
  NS_LOG_FUNCTION(this);
  if (m_txPsd) {
    return m_txPsd->GetSpectrumModel();
  } else {
    return nullptr;
  }
}

Ptr<Object> LrWpanPhy::GetAntenna() const {
  NS_LOG_FUNCTION(this);
  return m_antenna;
}

void LrWpanPhy::SetAntenna(Ptr<AntennaModel> a) {
  NS_LOG_FUNCTION(this << a);
  m_antenna = a;
}

void LrWpanPhy::StartRx(Ptr<SpectrumSignalParameters> spectrumRxParams) {
  NS_LOG_FUNCTION(this << spectrumRxParams);

  if (!m_edRequest.IsExpired()) {
    Time now = Simulator::Now();
    m_edPower.averagePower +=
        LrWpanSpectrumValueHelper::TotalAvgPower(
            m_signal->GetSignalPsd(), m_phyPIBAttributes.phyCurrentChannel) *
        (now - m_edPower.lastUpdate).GetTimeStep() /
        m_edPower.measurementLength.GetTimeStep();
    m_edPower.lastUpdate = now;
  }

  Ptr<LrWpanSpectrumSignalParameters> lrWpanRxParams =
      DynamicCast<LrWpanSpectrumSignalParameters>(spectrumRxParams);

  if (!lrWpanRxParams) {
    CheckInterference();
    m_signal->AddSignal(spectrumRxParams->psd);

    if (!m_ccaRequest.IsExpired()) {
      double power = LrWpanSpectrumValueHelper::TotalAvgPower(
          m_signal->GetSignalPsd(), m_phyPIBAttributes.phyCurrentChannel);
      if (m_ccaPeakPower < power) {
        m_ccaPeakPower = power;
      }
    }

    Simulator::Schedule(spectrumRxParams->duration, &LrWpanPhy::EndRx, this,
                        spectrumRxParams);
    return;
  }

  Ptr<Packet> p = (lrWpanRxParams->packetBurst->GetPackets()).front();
  NS_ASSERT(p);

  if (m_trxState == IEEE_802_15_4_PHY_RX_ON && !m_setTRXState.IsRunning()) {

    NS_LOG_DEBUG(this << " receiving packet with power: "
                      << 10 * log10(LrWpanSpectrumValueHelper::TotalAvgPower(
                                  lrWpanRxParams->psd,
                                  m_phyPIBAttributes.phyCurrentChannel)) +
                             30
                      << "dBm");
    m_signal->AddSignal(lrWpanRxParams->psd);
    Ptr<SpectrumValue> interferenceAndNoise = m_signal->GetSignalPsd();
    *interferenceAndNoise -= *lrWpanRxParams->psd;
    *interferenceAndNoise += *m_noise;
    double sinr =
        LrWpanSpectrumValueHelper::TotalAvgPower(
            lrWpanRxParams->psd, m_phyPIBAttributes.phyCurrentChannel) /
        LrWpanSpectrumValueHelper::TotalAvgPower(
            interferenceAndNoise, m_phyPIBAttributes.phyCurrentChannel);

    if (10 * log10(sinr) > -5) {
      ChangeTrxState(IEEE_802_15_4_PHY_BUSY_RX);
      m_currentRxPacket = std::make_pair(lrWpanRxParams, false);
      m_phyRxBeginTrace(p);

      m_rxLastUpdate = Simulator::Now();
    } else {
      m_phyRxDropTrace(p);
    }
  } else if (m_trxState == IEEE_802_15_4_PHY_BUSY_RX) {
    NS_LOG_DEBUG(this << " packet collision");
    m_phyRxDropTrace(p);

    CheckInterference();

    m_signal->AddSignal(lrWpanRxParams->psd);
  } else {
    NS_LOG_DEBUG(this << " transceiver not in RX state");
    m_phyRxDropTrace(p);

    m_signal->AddSignal(lrWpanRxParams->psd);
  }

  if (!m_ccaRequest.IsExpired()) {
    double power = LrWpanSpectrumValueHelper::TotalAvgPower(
        m_signal->GetSignalPsd(), m_phyPIBAttributes.phyCurrentChannel);
    if (m_ccaPeakPower < power) {
      m_ccaPeakPower = power;
    }
  }

  Simulator::Schedule(spectrumRxParams->duration, &LrWpanPhy::EndRx, this,
                      spectrumRxParams);
}

void LrWpanPhy::CheckInterference() {
  LrWpanSpectrumValueHelper psdHelper;
  Ptr<LrWpanSpectrumSignalParameters> currentRxParams = m_currentRxPacket.first;

  if (m_trxState == IEEE_802_15_4_PHY_BUSY_RX) {

    Ptr<Packet> currentPacket =
        currentRxParams->packetBurst->GetPackets().front();
    if (m_errorModel) {
      double t = (Simulator::Now() - m_rxLastUpdate).ToDouble(Time::MS);
      uint32_t chunkSize = ceil(t * (GetDataOrSymbolRate(true) / 1000));
      Ptr<SpectrumValue> interferenceAndNoise = m_signal->GetSignalPsd();
      *interferenceAndNoise -= *currentRxParams->psd;
      *interferenceAndNoise += *m_noise;
      double sinr =
          LrWpanSpectrumValueHelper::TotalAvgPower(
              currentRxParams->psd, m_phyPIBAttributes.phyCurrentChannel) /
          LrWpanSpectrumValueHelper::TotalAvgPower(
              interferenceAndNoise, m_phyPIBAttributes.phyCurrentChannel);
      double per = 1.0 - m_errorModel->GetChunkSuccessRate(sinr, chunkSize);

      LrWpanLqiTag tag(std::numeric_limits<uint8_t>::max());
      currentPacket->PeekPacketTag(tag);
      uint8_t lqi = tag.Get();
      tag.Set(lqi - (per * lqi));
      currentPacket->ReplacePacketTag(tag);

      if (m_random->GetValue() < per) {
        m_currentRxPacket.second = true;
      }
    } else {
      NS_LOG_WARN("Missing ErrorModel");
    }
  }
  m_rxLastUpdate = Simulator::Now();
}

void LrWpanPhy::EndRx(Ptr<SpectrumSignalParameters> par) {
  NS_LOG_FUNCTION(this);

  Ptr<LrWpanSpectrumSignalParameters> params =
      DynamicCast<LrWpanSpectrumSignalParameters>(par);

  if (!m_edRequest.IsExpired()) {
    Time now = Simulator::Now();
    m_edPower.averagePower +=
        LrWpanSpectrumValueHelper::TotalAvgPower(
            m_signal->GetSignalPsd(), m_phyPIBAttributes.phyCurrentChannel) *
        (now - m_edPower.lastUpdate).GetTimeStep() /
        m_edPower.measurementLength.GetTimeStep();
    m_edPower.lastUpdate = now;
  }

  Ptr<LrWpanSpectrumSignalParameters> currentRxParams = m_currentRxPacket.first;
  if (currentRxParams == params) {
    CheckInterference();
  }

  m_signal->RemoveSignal(par->psd);

  if (!params) {
    NS_LOG_LOGIC("Node: " << m_device->GetAddress()
                          << " Removing interferent: " << *(par->psd));
    return;
  }

  if (currentRxParams == params) {
    Ptr<Packet> currentPacket =
        currentRxParams->packetBurst->GetPackets().front();
    NS_ASSERT(currentPacket);

    if (m_postReceptionErrorModel &&
        m_postReceptionErrorModel->IsCorrupt(currentPacket->Copy())) {
      NS_LOG_DEBUG("Reception failed due to post-rx error model");
      m_currentRxPacket.second = true;
    }

    LrWpanLqiTag tag(std::numeric_limits<uint8_t>::max());
    currentPacket->PeekPacketTag(tag);
    m_phyRxEndTrace(currentPacket, tag.Get());

    if (!m_currentRxPacket.second) {
      if (!m_pdDataIndicationCallback.IsNull()) {
        m_pdDataIndicationCallback(currentPacket->GetSize(), currentPacket,
                                   tag.Get());
      }
    } else {
      m_phyRxDropTrace(currentPacket);
    }
    Ptr<LrWpanSpectrumSignalParameters> none = nullptr;
    m_currentRxPacket = std::make_pair(none, true);

    if (!m_isRxCanceled) {
      if (m_trxStatePending != IEEE_802_15_4_PHY_IDLE) {
        if (!m_setTRXState.IsRunning()) {
          NS_LOG_LOGIC("Apply pending state change to " << m_trxStatePending);
          ChangeTrxState(m_trxStatePending);
          m_trxStatePending = IEEE_802_15_4_PHY_IDLE;
          if (!m_plmeSetTRXStateConfirmCallback.IsNull()) {
            m_plmeSetTRXStateConfirmCallback(IEEE_802_15_4_PHY_SUCCESS);
          }
        }
      } else {
        ChangeTrxState(IEEE_802_15_4_PHY_RX_ON);
      }
    } else {
      m_isRxCanceled = false;
    }
  }
}

void LrWpanPhy::PdDataRequest(const uint32_t psduLength, Ptr<Packet> p) {
  NS_LOG_FUNCTION(this << psduLength << p);

  if (psduLength > lrwpan::aMaxPhyPacketSize) {
    if (!m_pdDataConfirmCallback.IsNull()) {
      m_pdDataConfirmCallback(IEEE_802_15_4_PHY_UNSPECIFIED);
    }
    NS_LOG_DEBUG("Drop packet because psduLength too long: " << psduLength);
    return;
  }

  if (!m_setTRXState.IsRunning()) {
    if (m_trxState == IEEE_802_15_4_PHY_TX_ON) {
      NS_ASSERT(m_channel);

      LrWpanLqiTag lqiTag;
      p->RemovePacketTag(lqiTag);

      m_phyTxBeginTrace(p);
      m_currentTxPacket.first = p;
      m_currentTxPacket.second = false;

      Ptr<LrWpanSpectrumSignalParameters> txParams =
          Create<LrWpanSpectrumSignalParameters>();
      txParams->duration = CalculateTxTime(p);
      txParams->txPhy = GetObject<SpectrumPhy>();
      txParams->psd = m_txPsd;
      txParams->txAntenna = m_antenna;
      Ptr<PacketBurst> pb = CreateObject<PacketBurst>();
      pb->AddPacket(p);
      txParams->packetBurst = pb;
      m_channel->StartTx(txParams);
      m_pdDataRequest =
          Simulator::Schedule(txParams->duration, &LrWpanPhy::EndTx, this);
      ChangeTrxState(IEEE_802_15_4_PHY_BUSY_TX);
      return;
    } else if ((m_trxState == IEEE_802_15_4_PHY_RX_ON) ||
               (m_trxState == IEEE_802_15_4_PHY_TRX_OFF) ||
               (m_trxState == IEEE_802_15_4_PHY_BUSY_TX)) {
      if (!m_pdDataConfirmCallback.IsNull()) {
        m_pdDataConfirmCallback(m_trxState);
      }
      m_phyTxDropTrace(p);
      return;
    } else {
      NS_FATAL_ERROR("This should be unreachable, or else state "
                     << m_trxState << " should be added as a case");
    }
  } else {
    if (!m_pdDataConfirmCallback.IsNull()) {
      m_pdDataConfirmCallback(IEEE_802_15_4_PHY_UNSPECIFIED);
    }
    m_phyTxDropTrace(p);
    return;
  }
}

void LrWpanPhy::PlmeCcaRequest() {
  NS_LOG_FUNCTION(this);

  if (m_trxState == IEEE_802_15_4_PHY_RX_ON ||
      m_trxState == IEEE_802_15_4_PHY_BUSY_RX) {
    m_ccaPeakPower = 0.0;
    Time ccaTime = Seconds(8.0 / GetDataOrSymbolRate(false));
    m_ccaRequest = Simulator::Schedule(ccaTime, &LrWpanPhy::EndCca, this);
  } else {
    if (!m_plmeCcaConfirmCallback.IsNull()) {
      if (m_trxState == IEEE_802_15_4_PHY_TRX_OFF) {
        m_plmeCcaConfirmCallback(IEEE_802_15_4_PHY_TRX_OFF);
      } else {
        m_plmeCcaConfirmCallback(IEEE_802_15_4_PHY_BUSY);
      }
    }
  }
}

void LrWpanPhy::CcaCancel() {
  NS_LOG_FUNCTION(this);
  m_ccaRequest.Cancel();
}

void LrWpanPhy::PlmeEdRequest() {
  NS_LOG_FUNCTION(this);
  if (m_trxState == IEEE_802_15_4_PHY_RX_ON ||
      m_trxState == IEEE_802_15_4_PHY_BUSY_RX) {
    m_edPower.averagePower = 0;
    m_edPower.lastUpdate = Simulator::Now();
    m_edPower.measurementLength = Seconds(8.0 / GetDataOrSymbolRate(false));
    m_edRequest = Simulator::Schedule(m_edPower.measurementLength,
                                      &LrWpanPhy::EndEd, this);
  } else {
    LrWpanPhyEnumeration result = m_trxState;
    if (m_trxState == IEEE_802_15_4_PHY_BUSY_TX) {
      result = IEEE_802_15_4_PHY_TX_ON;
    }

    if (!m_plmeEdConfirmCallback.IsNull()) {
      m_plmeEdConfirmCallback(result, 0);
    }
  }
}

void LrWpanPhy::PlmeGetAttributeRequest(LrWpanPibAttributeIdentifier id) {
  NS_LOG_FUNCTION(this << id);
  LrWpanPhyEnumeration status = IEEE_802_15_4_PHY_SUCCESS;
  Ptr<LrWpanPhyPibAttributes> attributes = Create<LrWpanPhyPibAttributes>();

  switch (id) {
  case phyCurrentChannel:
    attributes->phyCurrentChannel = m_phyPIBAttributes.phyCurrentChannel;
    break;
  case phyCurrentPage:
    attributes->phyCurrentPage = m_phyPIBAttributes.phyCurrentPage;
    break;
  case phySHRDuration:
    attributes->phySHRDuration = GetPhySHRDuration();
    break;
  case phySymbolsPerOctet:
    attributes->phySymbolsPerOctet = GetPhySymbolsPerOctet();
    break;
  default:
    status = IEEE_802_15_4_PHY_UNSUPPORTED_ATTRIBUTE;
    break;
  }

  if (!m_plmeGetAttributeConfirmCallback.IsNull()) {
    m_plmeGetAttributeConfirmCallback(status, id, attributes);
  }
}

void LrWpanPhy::PlmeSetTRXStateRequest(LrWpanPhyEnumeration state) {
  NS_LOG_FUNCTION(this << state);

  NS_ABORT_IF((state != IEEE_802_15_4_PHY_RX_ON) &&
              (state != IEEE_802_15_4_PHY_TRX_OFF) &&
              (state != IEEE_802_15_4_PHY_FORCE_TRX_OFF) &&
              (state != IEEE_802_15_4_PHY_TX_ON));

  NS_LOG_LOGIC("Trying to set m_trxState from " << m_trxState << " to "
                                                << state);
  if (!m_setTRXState.IsExpired()) {
    if (m_trxStatePending == state) {
      return;
    } else {
      NS_LOG_DEBUG("Cancel m_setTRXState");
      m_setTRXState.Cancel();
    }
  }
  if (m_trxStatePending != IEEE_802_15_4_PHY_IDLE) {
    m_trxStatePending = IEEE_802_15_4_PHY_IDLE;
  }

  if (state == m_trxState) {
    if (!m_plmeSetTRXStateConfirmCallback.IsNull()) {
      m_plmeSetTRXStateConfirmCallback(state);
    }
    return;
  }

  if (((state == IEEE_802_15_4_PHY_RX_ON) ||
       (state == IEEE_802_15_4_PHY_TRX_OFF)) &&
      (m_trxState == IEEE_802_15_4_PHY_BUSY_TX)) {
    NS_LOG_DEBUG("Phy is busy; setting state pending to " << state);
    m_trxStatePending = state;
    return;
  }

  if (state == IEEE_802_15_4_PHY_TRX_OFF) {
    CancelEd(state);

    if ((m_trxState == IEEE_802_15_4_PHY_BUSY_RX) &&
        (m_currentRxPacket.first) && (!m_currentRxPacket.second)) {
      NS_LOG_DEBUG("Receiver has valid SFD; defer state change");
      m_trxStatePending = state;
      return;
    } else if (m_trxState == IEEE_802_15_4_PHY_RX_ON ||
               m_trxState == IEEE_802_15_4_PHY_TX_ON) {
      ChangeTrxState(IEEE_802_15_4_PHY_TRX_OFF);
      if (!m_plmeSetTRXStateConfirmCallback.IsNull()) {
        m_plmeSetTRXStateConfirmCallback(state);
      }
      return;
    }
  }

  if (state == IEEE_802_15_4_PHY_TX_ON) {
    CancelEd(state);

    NS_LOG_DEBUG("turn on PHY_TX_ON");
    if ((m_trxState == IEEE_802_15_4_PHY_BUSY_RX) ||
        (m_trxState == IEEE_802_15_4_PHY_RX_ON)) {
      if (m_currentRxPacket.first) {
        NS_LOG_DEBUG("force TX_ON, terminate reception");
        m_currentRxPacket.second = true;
        m_isRxCanceled = true;
      }

      if (!m_ccaRequest.IsExpired()) {
        m_ccaRequest.Cancel();
        if (!m_plmeCcaConfirmCallback.IsNull()) {
          m_plmeCcaConfirmCallback(IEEE_802_15_4_PHY_BUSY);
        }
      }

      m_trxStatePending = IEEE_802_15_4_PHY_TX_ON;

      Time setTime =
          Seconds((double)lrwpan::aTurnaroundTime / GetDataOrSymbolRate(false));
      m_setTRXState =
          Simulator::Schedule(setTime, &LrWpanPhy::EndSetTRXState, this);
      return;
    } else if (m_trxState == IEEE_802_15_4_PHY_BUSY_TX ||
               m_trxState == IEEE_802_15_4_PHY_TX_ON) {
      if (!m_plmeSetTRXStateConfirmCallback.IsNull()) {
        m_plmeSetTRXStateConfirmCallback(IEEE_802_15_4_PHY_TX_ON);
      }
      return;
    } else if (m_trxState == IEEE_802_15_4_PHY_TRX_OFF) {
      ChangeTrxState(IEEE_802_15_4_PHY_TX_ON);
      if (!m_plmeSetTRXStateConfirmCallback.IsNull()) {
        m_plmeSetTRXStateConfirmCallback(IEEE_802_15_4_PHY_TX_ON);
      }
      return;
    }
  }

  if (state == IEEE_802_15_4_PHY_FORCE_TRX_OFF) {
    if (m_trxState == IEEE_802_15_4_PHY_TRX_OFF) {
      NS_LOG_DEBUG("force TRX_OFF, was already off");
    } else {
      NS_LOG_DEBUG("force TRX_OFF, SUCCESS");
      if (m_currentRxPacket.first) {
        NS_LOG_DEBUG("force TRX_OFF, terminate reception");
        m_currentRxPacket.second = true;
        m_isRxCanceled = true;
      }
      if (m_trxState == IEEE_802_15_4_PHY_BUSY_TX) {
        NS_LOG_DEBUG("force TRX_OFF, terminate transmission");
        m_currentTxPacket.second = true;
      }
      ChangeTrxState(IEEE_802_15_4_PHY_TRX_OFF);
      m_trxStatePending = IEEE_802_15_4_PHY_IDLE;
    }
    if (!m_plmeSetTRXStateConfirmCallback.IsNull()) {
      m_plmeSetTRXStateConfirmCallback(IEEE_802_15_4_PHY_SUCCESS);
    }
    return;
  }

  if (state == IEEE_802_15_4_PHY_RX_ON) {
    if (m_trxState == IEEE_802_15_4_PHY_TX_ON ||
        m_trxState == IEEE_802_15_4_PHY_TRX_OFF) {
      m_trxStatePending = IEEE_802_15_4_PHY_RX_ON;

      Time setTime =
          Seconds((double)lrwpan::aTurnaroundTime / GetDataOrSymbolRate(false));
      m_setTRXState =
          Simulator::Schedule(setTime, &LrWpanPhy::EndSetTRXState, this);
      return;
    } else if (m_trxState == IEEE_802_15_4_PHY_BUSY_RX) {
      if (!m_plmeSetTRXStateConfirmCallback.IsNull()) {
        m_plmeSetTRXStateConfirmCallback(IEEE_802_15_4_PHY_RX_ON);
      }
      return;
    }
  }

  NS_FATAL_ERROR("Unexpected transition from state " << m_trxState
                                                     << " to state " << state);
}

bool LrWpanPhy::ChannelSupported(uint8_t channel) {
  NS_LOG_FUNCTION(this << channel);
  bool retValue = false;

  if ((m_phyPIBAttributes
           .phyChannelsSupported[m_phyPIBAttributes.phyCurrentPage] &
       (1 << channel)) != 0) {
    return retValue = true;
  } else {
    return retValue;
  }
}

bool LrWpanPhy::PageSupported(uint8_t page) {
  NS_LOG_FUNCTION(this << +page);
  bool retValue = false;

  NS_ABORT_MSG_UNLESS(page == 0, " Only Page 0 (2.4Ghz O-QPSK supported).");

  uint8_t supportedPage =
      (m_phyPIBAttributes.phyChannelsSupported[page] >> 27) & (0x1F);

  if (page == supportedPage) {
    retValue = true;
  }

  return retValue;
}

void LrWpanPhy::PlmeSetAttributeRequest(LrWpanPibAttributeIdentifier id,
                                        Ptr<LrWpanPhyPibAttributes> attribute) {
  NS_LOG_FUNCTION(this << id << attribute);
  NS_ASSERT(attribute);
  LrWpanPhyEnumeration status = IEEE_802_15_4_PHY_SUCCESS;

  switch (id) {
  case phyCurrentPage: {
    if (!PageSupported(attribute->phyCurrentPage)) {
      status = IEEE_802_15_4_PHY_INVALID_PARAMETER;
    } else if (m_phyPIBAttributes.phyCurrentPage != attribute->phyCurrentPage) {
      m_trxState = IEEE_802_15_4_PHY_TRX_OFF;
      if (m_trxStatePending != IEEE_802_15_4_PHY_IDLE) {
        m_trxStatePending = IEEE_802_15_4_PHY_IDLE;
        m_setTRXState.Cancel();
        if (!m_plmeSetTRXStateConfirmCallback.IsNull()) {
          m_plmeSetTRXStateConfirmCallback(IEEE_802_15_4_PHY_TRX_OFF);
        }
      }

      if (m_currentRxPacket.first) {
        m_currentRxPacket.second = true;
      }
      if (PhyIsBusy()) {
        m_currentTxPacket.second = true;
        m_pdDataRequest.Cancel();
        m_currentTxPacket.first = nullptr;
        if (!m_pdDataConfirmCallback.IsNull()) {
          m_pdDataConfirmCallback(IEEE_802_15_4_PHY_TRX_OFF);
        }
      }

      if (attribute->phyCurrentPage == 0) {
        if (m_phyPIBAttributes.phyCurrentChannel == 0) {
          m_phyOption = IEEE_802_15_4_868MHZ_BPSK;
          NS_LOG_INFO("Page 0, 868 MHz BPSK PHY SET");
        } else if (m_phyPIBAttributes.phyCurrentChannel <= 10) {
          m_phyOption = IEEE_802_15_4_915MHZ_BPSK;
          NS_LOG_INFO("Page " << (uint32_t)attribute->phyCurrentPage
                              << ",915 MHz BPSK PHY SET");
        } else if (m_phyPIBAttributes.phyCurrentChannel <= 26) {
          m_phyOption = IEEE_802_15_4_2_4GHZ_OQPSK;
          NS_LOG_INFO("Page " << (uint32_t)attribute->phyCurrentPage
                              << ", 2.4 Ghz O-QPSK PHY SET");
        }
      } else if (attribute->phyCurrentPage == 1) {
        if (m_phyPIBAttributes.phyCurrentChannel == 0) {
          m_phyOption = IEEE_802_15_4_868MHZ_ASK;
          NS_LOG_INFO("Page " << (uint32_t)attribute->phyCurrentPage
                              << ", 868 MHz ASK PHY SET");
        } else if (m_phyPIBAttributes.phyCurrentChannel <= 10) {
          m_phyOption = IEEE_802_15_4_915MHZ_ASK;
          NS_LOG_INFO("Page " << (uint32_t)attribute->phyCurrentPage
                              << ", 915 MHz ASK PHY SET");
        } else {
          m_phyOption = IEEE_802_15_4_868MHZ_ASK;
          m_phyPIBAttributes.phyCurrentChannel = 0;
          NS_LOG_INFO("Channel no longer valid in new page "
                      << (uint32_t)attribute->phyCurrentPage
                      << ", setting new default channel "
                      << (uint32_t)m_phyPIBAttributes.phyCurrentChannel);
          NS_LOG_INFO("868 MHz ASK PHY SET");
        }
      } else if (attribute->phyCurrentPage == 2) {
        if (m_phyPIBAttributes.phyCurrentChannel == 0) {
          m_phyOption = IEEE_802_15_4_868MHZ_OQPSK;
          NS_LOG_INFO("Page " << (uint32_t)attribute->phyCurrentPage
                              << ", 868 MHz O-QPSK PHY SET");
        } else if (m_phyPIBAttributes.phyCurrentChannel <= 10) {
          m_phyOption = IEEE_802_15_4_915MHZ_OQPSK;
          NS_LOG_INFO("Page " << (uint32_t)attribute->phyCurrentPage
                              << ", 915 MHz O-QPSK PHY SET");
        } else {
          m_phyOption = IEEE_802_15_4_868MHZ_OQPSK;
          m_phyPIBAttributes.phyCurrentChannel = 0;
          NS_LOG_INFO("Channel no longer valid in new page "
                      << (uint32_t)attribute->phyCurrentPage
                      << ", setting new default channel "
                      << (uint32_t)m_phyPIBAttributes.phyCurrentChannel);
          NS_LOG_INFO("868 MHz O-QPSK PHY SET");
        }
      } else if (attribute->phyCurrentPage == 5) {
        if (m_phyPIBAttributes.phyCurrentChannel <= 3) {
          m_phyOption = IEEE_802_15_4_780MHZ_OQPSK;
          NS_LOG_INFO("Page " << (uint32_t)attribute->phyCurrentPage
                              << ", 915 MHz O-QPSK PHY SET");
        } else {
          m_phyOption = IEEE_802_15_4_780MHZ_OQPSK;
          m_phyPIBAttributes.phyCurrentChannel = 0;
          NS_LOG_INFO("Channel no longer valid in new page "
                      << (uint32_t)attribute->phyCurrentPage
                      << ", setting new default channel "
                      << (uint32_t)m_phyPIBAttributes.phyCurrentChannel);
          NS_LOG_INFO("780 MHz O-QPSK PHY SET");
        }
      } else if (attribute->phyCurrentPage == 6) {
        if (m_phyPIBAttributes.phyCurrentChannel <= 9) {
          m_phyOption = IEEE_802_15_4_950MHZ_BPSK;
          NS_LOG_INFO("Page " << (uint32_t)attribute->phyCurrentPage
                              << ", 950 MHz BPSK PHY SET");
        } else {
          m_phyOption = IEEE_802_15_4_950MHZ_BPSK;
          m_phyPIBAttributes.phyCurrentChannel = 0;
          NS_LOG_INFO("Channel no longer valid in new page "
                      << (uint32_t)attribute->phyCurrentPage
                      << ", setting new default channel "
                      << (uint32_t)m_phyPIBAttributes.phyCurrentChannel);
          NS_LOG_INFO("950 MHz BPSK PHY SET");
        }
      }

      m_phyPIBAttributes.phyCurrentPage = attribute->phyCurrentPage;

      SetRxSensitivity(-106.58);
    }
    break;
  }
  case phyCurrentChannel: {
    if (!ChannelSupported(attribute->phyCurrentChannel)) {
      status = IEEE_802_15_4_PHY_INVALID_PARAMETER;
    }
    if (m_phyPIBAttributes.phyCurrentChannel != attribute->phyCurrentChannel) {
      m_trxState = IEEE_802_15_4_PHY_TRX_OFF;
      if (m_trxStatePending != IEEE_802_15_4_PHY_IDLE) {
        m_trxStatePending = IEEE_802_15_4_PHY_IDLE;
        m_setTRXState.Cancel();
        if (!m_plmeSetTRXStateConfirmCallback.IsNull()) {
          m_plmeSetTRXStateConfirmCallback(IEEE_802_15_4_PHY_TRX_OFF);
        }
      }

      if (m_currentRxPacket.first) {
        m_currentRxPacket.second = true;
      }
      if (PhyIsBusy()) {
        m_currentTxPacket.second = true;
        m_pdDataRequest.Cancel();
        m_currentTxPacket.first = nullptr;
        if (!m_pdDataConfirmCallback.IsNull()) {
          m_pdDataConfirmCallback(IEEE_802_15_4_PHY_TRX_OFF);
        }
      }

      m_phyPIBAttributes.phyCurrentChannel = attribute->phyCurrentChannel;

      SetRxSensitivity(WToDbm(m_rxSensitivity));
    }
    break;
  }
  case phyChannelsSupported: {
    if ((attribute->phyChannelsSupported[0] & 0xf8000000) != 0) {
      status = IEEE_802_15_4_PHY_INVALID_PARAMETER;
    } else {
      m_phyPIBAttributes.phyChannelsSupported[0] =
          attribute->phyChannelsSupported[0];
    }
    break;
  }
  case phyTransmitPower: {
    if (attribute->phyTransmitPower & 0xC0) {
      NS_LOG_LOGIC(
          "LrWpanPhy::PlmeSetAttributeRequest error - can not change read-only "
          "attribute bits.");
      status = IEEE_802_15_4_PHY_INVALID_PARAMETER;
    } else {
      m_phyPIBAttributes.phyTransmitPower = attribute->phyTransmitPower;
      LrWpanSpectrumValueHelper psdHelper;
      m_txPsd = psdHelper.CreateTxPowerSpectralDensity(
          GetNominalTxPowerFromPib(m_phyPIBAttributes.phyTransmitPower),
          m_phyPIBAttributes.phyCurrentChannel);
    }
    break;
  }
  case phyCCAMode: {
    if ((attribute->phyCCAMode < 1) || (attribute->phyCCAMode > 3)) {
      status = IEEE_802_15_4_PHY_INVALID_PARAMETER;
    } else {
      m_phyPIBAttributes.phyCCAMode = attribute->phyCCAMode;
    }
    break;
  }
  default: {
    status = IEEE_802_15_4_PHY_UNSUPPORTED_ATTRIBUTE;
    break;
  }
  }

  if (!m_plmeSetAttributeConfirmCallback.IsNull()) {
    m_plmeSetAttributeConfirmCallback(status, id);
  }
}

void LrWpanPhy::SetPdDataIndicationCallback(PdDataIndicationCallback c) {
  NS_LOG_FUNCTION(this);
  m_pdDataIndicationCallback = c;
}

void LrWpanPhy::SetPdDataConfirmCallback(PdDataConfirmCallback c) {
  NS_LOG_FUNCTION(this);
  m_pdDataConfirmCallback = c;
}

void LrWpanPhy::SetPlmeCcaConfirmCallback(PlmeCcaConfirmCallback c) {
  NS_LOG_FUNCTION(this);
  m_plmeCcaConfirmCallback = c;
}

void LrWpanPhy::SetPlmeEdConfirmCallback(PlmeEdConfirmCallback c) {
  NS_LOG_FUNCTION(this);
  m_plmeEdConfirmCallback = c;
}

void LrWpanPhy::SetPlmeGetAttributeConfirmCallback(
    PlmeGetAttributeConfirmCallback c) {
  NS_LOG_FUNCTION(this);
  m_plmeGetAttributeConfirmCallback = c;
}

void LrWpanPhy::SetPlmeSetTRXStateConfirmCallback(
    PlmeSetTRXStateConfirmCallback c) {
  NS_LOG_FUNCTION(this);
  m_plmeSetTRXStateConfirmCallback = c;
}

void LrWpanPhy::SetPlmeSetAttributeConfirmCallback(
    PlmeSetAttributeConfirmCallback c) {
  NS_LOG_FUNCTION(this);
  m_plmeSetAttributeConfirmCallback = c;
}

void LrWpanPhy::ChangeTrxState(LrWpanPhyEnumeration newState) {
  NS_LOG_LOGIC(this << " state: " << m_trxState << " -> " << newState);
  m_trxStateLogger(Simulator::Now(), m_trxState, newState);
  m_trxState = newState;
}

bool LrWpanPhy::PhyIsBusy() const {
  NS_LOG_FUNCTION(this << m_trxState);
  return ((m_trxState == IEEE_802_15_4_PHY_BUSY_TX) ||
          (m_trxState == IEEE_802_15_4_PHY_BUSY_RX) ||
          (m_trxState == IEEE_802_15_4_PHY_BUSY));
}

void LrWpanPhy::CancelEd(LrWpanPhyEnumeration state) {
  NS_LOG_FUNCTION(this);
  NS_ASSERT(state == IEEE_802_15_4_PHY_TRX_OFF ||
            state == IEEE_802_15_4_PHY_TX_ON);

  if (!m_edRequest.IsExpired()) {
    m_edRequest.Cancel();
    if (!m_plmeEdConfirmCallback.IsNull()) {
      m_plmeEdConfirmCallback(state, 0);
    }
  }
}

void LrWpanPhy::EndEd() {
  NS_LOG_FUNCTION(this);

  m_edPower.averagePower +=
      LrWpanSpectrumValueHelper::TotalAvgPower(
          m_signal->GetSignalPsd(), m_phyPIBAttributes.phyCurrentChannel) *
      (Simulator::Now() - m_edPower.lastUpdate).GetTimeStep() /
      m_edPower.measurementLength.GetTimeStep();

  uint8_t energyLevel;

  double ratio = m_edPower.averagePower / m_rxSensitivity;
  ratio = 10.0 * log10(ratio);
  if (ratio <= 10.0) {
    energyLevel = 0;
  } else if (ratio >= 40.0) {
    energyLevel = 255;
  } else {
    energyLevel = static_cast<uint8_t>(((ratio - 10.0) / 30.0) * 255.0);
  }

  if (!m_plmeEdConfirmCallback.IsNull()) {
    m_plmeEdConfirmCallback(IEEE_802_15_4_PHY_SUCCESS, energyLevel);
  }
}

void LrWpanPhy::EndCca() {
  NS_LOG_FUNCTION(this);
  LrWpanPhyEnumeration sensedChannelState = IEEE_802_15_4_PHY_UNSPECIFIED;

  double power = LrWpanSpectrumValueHelper::TotalAvgPower(
      m_signal->GetSignalPsd(), m_phyPIBAttributes.phyCurrentChannel);
  if (m_ccaPeakPower < power) {
    m_ccaPeakPower = power;
  }

  if (PhyIsBusy()) {
    sensedChannelState = IEEE_802_15_4_PHY_BUSY;
  } else if (m_phyPIBAttributes.phyCCAMode == 1) {
    if (10 * log10(m_ccaPeakPower / m_rxSensitivity) >= 10.0) {
      sensedChannelState = IEEE_802_15_4_PHY_BUSY;
    } else {
      sensedChannelState = IEEE_802_15_4_PHY_IDLE;
    }
  } else if (m_phyPIBAttributes.phyCCAMode == 2) {
    if (m_trxState == IEEE_802_15_4_PHY_BUSY_RX) {
      sensedChannelState = IEEE_802_15_4_PHY_BUSY;
    } else {
      sensedChannelState = IEEE_802_15_4_PHY_IDLE;
    }
  } else if (m_phyPIBAttributes.phyCCAMode == 3) {
    if ((10 * log10(m_ccaPeakPower / m_rxSensitivity) >= 10.0) &&
        m_trxState == IEEE_802_15_4_PHY_BUSY_RX) {
      sensedChannelState = IEEE_802_15_4_PHY_BUSY;
    } else {
      sensedChannelState = IEEE_802_15_4_PHY_IDLE;
    }
  } else {
    NS_ASSERT_MSG(false, "Invalid CCA mode");
  }

  NS_LOG_LOGIC(this << "channel sensed state: " << sensedChannelState);

  if (!m_plmeCcaConfirmCallback.IsNull()) {
    m_plmeCcaConfirmCallback(sensedChannelState);
  }
}

void LrWpanPhy::EndSetTRXState() {
  NS_LOG_FUNCTION(this);

  NS_ABORT_IF((m_trxStatePending != IEEE_802_15_4_PHY_RX_ON) &&
              (m_trxStatePending != IEEE_802_15_4_PHY_TX_ON));
  ChangeTrxState(m_trxStatePending);
  m_trxStatePending = IEEE_802_15_4_PHY_IDLE;

  if (!m_plmeSetTRXStateConfirmCallback.IsNull()) {
    m_plmeSetTRXStateConfirmCallback(m_trxState);
  }
}

void LrWpanPhy::EndTx() {
  NS_LOG_FUNCTION(this);

  NS_ABORT_IF((m_trxState != IEEE_802_15_4_PHY_BUSY_TX) &&
              (m_trxState != IEEE_802_15_4_PHY_TRX_OFF));

  if (!m_currentTxPacket.second) {
    NS_LOG_DEBUG("Packet successfully transmitted");
    m_phyTxEndTrace(m_currentTxPacket.first);
    if (!m_pdDataConfirmCallback.IsNull()) {
      m_pdDataConfirmCallback(IEEE_802_15_4_PHY_SUCCESS);
    }
  } else {
    NS_LOG_DEBUG("Packet transmission aborted");
    m_phyTxDropTrace(m_currentTxPacket.first);
    if (!m_pdDataConfirmCallback.IsNull()) {
      NS_ASSERT(m_trxState == IEEE_802_15_4_PHY_TRX_OFF);
      m_pdDataConfirmCallback(m_trxState);
    }
  }
  m_currentTxPacket.first = nullptr;
  m_currentTxPacket.second = false;

  if (m_trxStatePending != IEEE_802_15_4_PHY_IDLE) {
    if (!m_setTRXState.IsRunning()) {
      NS_LOG_LOGIC("Apply pending state change to " << m_trxStatePending);
      ChangeTrxState(m_trxStatePending);
      m_trxStatePending = IEEE_802_15_4_PHY_IDLE;
      if (!m_plmeSetTRXStateConfirmCallback.IsNull()) {
        m_plmeSetTRXStateConfirmCallback(IEEE_802_15_4_PHY_SUCCESS);
      }
    }
  } else {
    if (m_trxState != IEEE_802_15_4_PHY_TRX_OFF) {
      ChangeTrxState(IEEE_802_15_4_PHY_TX_ON);
    }
  }
}

Time LrWpanPhy::CalculateTxTime(Ptr<const Packet> packet) {
  NS_LOG_FUNCTION(this << packet);

  bool isData = true;
  Time txTime = GetPpduHeaderTxTime();

  txTime += Seconds(packet->GetSize() * 8.0 / GetDataOrSymbolRate(isData));

  return txTime;
}

uint8_t LrWpanPhy::GetCurrentPage() const {
  return m_phyPIBAttributes.phyCurrentPage;
}

uint8_t LrWpanPhy::GetCurrentChannelNum() const {
  return m_phyPIBAttributes.phyCurrentChannel;
}

double LrWpanPhy::GetDataOrSymbolRate(bool isData) {
  NS_LOG_FUNCTION(this << isData);

  double rate = 0.0;

  NS_ASSERT(m_phyOption < IEEE_802_15_4_INVALID_PHY_OPTION);

  if (isData) {
    rate = dataSymbolRates[m_phyOption].bitRate;
  } else {
    rate = dataSymbolRates[m_phyOption].symbolRate;
  }

  return (rate * 1000.0);
}

Time LrWpanPhy::GetPpduHeaderTxTime() {
  NS_LOG_FUNCTION(this);

  bool isData = false;
  double totalPpduHdrSymbols;

  NS_ASSERT(m_phyOption < IEEE_802_15_4_INVALID_PHY_OPTION);

  totalPpduHdrSymbols = ppduHeaderSymbolNumbers[m_phyOption].shrPreamble +
                        ppduHeaderSymbolNumbers[m_phyOption].shrSfd +
                        ppduHeaderSymbolNumbers[m_phyOption].phr;

  return Seconds(totalPpduHdrSymbols / GetDataOrSymbolRate(isData));
}

void LrWpanPhy::SetPhyOption(LrWpanPhyOption phyOption) {
  NS_LOG_FUNCTION(this);

  m_phyOption = IEEE_802_15_4_INVALID_PHY_OPTION;

  NS_ABORT_MSG_UNLESS(phyOption == IEEE_802_15_4_2_4GHZ_OQPSK,
                      " Only 2.4Ghz O-QPSK supported.");

  switch (phyOption) {
  case IEEE_802_15_4_868MHZ_BPSK:
    m_phyPIBAttributes.phyCurrentPage = 0;
    m_phyPIBAttributes.phyCurrentChannel = 0;
    break;
  case IEEE_802_15_4_915MHZ_BPSK:
    m_phyPIBAttributes.phyCurrentPage = 0;
    m_phyPIBAttributes.phyCurrentChannel = 1;
    break;
  case IEEE_802_15_4_950MHZ_BPSK:
    m_phyPIBAttributes.phyCurrentPage = 6;
    m_phyPIBAttributes.phyCurrentChannel = 0;
    break;
  case IEEE_802_15_4_868MHZ_ASK:
    m_phyPIBAttributes.phyCurrentPage = 1;
    m_phyPIBAttributes.phyCurrentChannel = 0;
    break;
  case IEEE_802_15_4_915MHZ_ASK:
    m_phyPIBAttributes.phyCurrentPage = 1;
    m_phyPIBAttributes.phyCurrentChannel = 1;
    break;
  case IEEE_802_15_4_780MHZ_OQPSK:
    m_phyPIBAttributes.phyCurrentPage = 5;
    m_phyPIBAttributes.phyCurrentChannel = 0;
    break;
  case IEEE_802_15_4_868MHZ_OQPSK:
    m_phyPIBAttributes.phyCurrentPage = 2;
    m_phyPIBAttributes.phyCurrentChannel = 0;
    break;
  case IEEE_802_15_4_915MHZ_OQPSK:
    m_phyPIBAttributes.phyCurrentPage = 2;
    m_phyPIBAttributes.phyCurrentChannel = 1;
    break;
  case IEEE_802_15_4_2_4GHZ_OQPSK:
    m_phyPIBAttributes.phyCurrentPage = 0;
    m_phyPIBAttributes.phyCurrentChannel = 11;
    break;
  case IEEE_802_15_4_INVALID_PHY_OPTION:
    m_phyPIBAttributes.phyCurrentPage = 31;
    m_phyPIBAttributes.phyCurrentChannel = 26;
    break;
  }

  NS_ASSERT(phyOption != IEEE_802_15_4_INVALID_PHY_OPTION);

  m_phyOption = phyOption;
  m_phyPIBAttributes.phyChannelsSupported[0] = 0x7FFF800;

  for (int i = 1; i <= 31; i++) {
    m_phyPIBAttributes.phyChannelsSupported[i] = 0xF8000000;
  }

  m_edPower.averagePower = 0.0;
  m_edPower.lastUpdate = Seconds(0.0);
  m_edPower.measurementLength = Seconds(0.0);

  SetRxSensitivity(-106.58);

  m_rxLastUpdate = Seconds(0);
  Ptr<Packet> none_packet = nullptr;
  Ptr<LrWpanSpectrumSignalParameters> none_params = nullptr;
  m_currentRxPacket = std::make_pair(none_params, true);
  m_currentTxPacket = std::make_pair(none_packet, true);
  m_errorModel = nullptr;
}

void LrWpanPhy::SetRxSensitivity(double dbmSensitivity) {
  NS_LOG_FUNCTION(this << dbmSensitivity << "dBm");

  if (m_phyOption == IEEE_802_15_4_915MHZ_BPSK ||
      m_phyOption == IEEE_802_15_4_950MHZ_BPSK) {
    if (dbmSensitivity > -92) {
      NS_ABORT_MSG("The minimum Rx sensitivity for this band should be at "
                   "least -92 dBm");
    }
  } else {
    if (dbmSensitivity > -85) {
      NS_ABORT_MSG("The minimum Rx sensitivity for this band should be at "
                   "least -85 dBm");
    }
  }

  double maxRxSensitivityW = DbmToW(-106.58);

  LrWpanSpectrumValueHelper psdHelper;
  m_txPsd = psdHelper.CreateTxPowerSpectralDensity(
      GetNominalTxPowerFromPib(m_phyPIBAttributes.phyTransmitPower),
      m_phyPIBAttributes.phyCurrentChannel);
  long double noiseFactor = DbmToW(dbmSensitivity) / maxRxSensitivityW;
  psdHelper.SetNoiseFactor(noiseFactor);
  m_noise = psdHelper.CreateNoisePowerSpectralDensity(
      m_phyPIBAttributes.phyCurrentChannel);

  m_signal = Create<LrWpanInterferenceHelper>(m_noise->GetSpectrumModel());
  m_rxSensitivity = DbmToW(dbmSensitivity);
}

double LrWpanPhy::GetRxSensitivity() {
  NS_LOG_FUNCTION(this);
  return WToDbm(m_rxSensitivity);
}

LrWpanPhyOption LrWpanPhy::GetMyPhyOption() {
  NS_LOG_FUNCTION(this);
  return m_phyOption;
}

void LrWpanPhy::SetTxPowerSpectralDensity(Ptr<SpectrumValue> txPsd) {
  NS_LOG_FUNCTION(this << txPsd);
  NS_ASSERT(txPsd);
  m_txPsd = txPsd;
  NS_LOG_INFO("\t computed tx_psd: " << *txPsd
                                     << "\t stored tx_psd: " << *m_txPsd);
}

void LrWpanPhy::SetNoisePowerSpectralDensity(
    Ptr<const SpectrumValue> noisePsd) {
  NS_LOG_FUNCTION(this << noisePsd);
  NS_LOG_INFO("\t computed noise_psd: " << *noisePsd);
  NS_ASSERT(noisePsd);
  m_noise = noisePsd;
}

Ptr<const SpectrumValue> LrWpanPhy::GetNoisePowerSpectralDensity() {
  NS_LOG_FUNCTION(this);
  return m_noise;
}

void LrWpanPhy::SetErrorModel(Ptr<LrWpanErrorModel> e) {
  NS_LOG_FUNCTION(this << e);
  NS_ASSERT(e);
  m_errorModel = e;
}

Ptr<LrWpanErrorModel> LrWpanPhy::GetErrorModel() const {
  NS_LOG_FUNCTION(this);
  return m_errorModel;
}

uint64_t LrWpanPhy::GetPhySHRDuration() const {
  NS_LOG_FUNCTION(this);
  NS_ASSERT(m_phyOption < IEEE_802_15_4_INVALID_PHY_OPTION);

  return ppduHeaderSymbolNumbers[m_phyOption].shrPreamble +
         ppduHeaderSymbolNumbers[m_phyOption].shrSfd;
}

double LrWpanPhy::GetPhySymbolsPerOctet() const {
  NS_LOG_FUNCTION(this);
  NS_ASSERT(m_phyOption < IEEE_802_15_4_INVALID_PHY_OPTION);

  return dataSymbolRates[m_phyOption].symbolRate /
         (dataSymbolRates[m_phyOption].bitRate / 8);
}

double LrWpanPhy::GetCurrentSignalPsd() {
  double powerWatts = LrWpanSpectrumValueHelper::TotalAvgPower(
      m_signal->GetSignalPsd(), m_phyPIBAttributes.phyCurrentChannel);
  return WToDbm(powerWatts);
}

int8_t LrWpanPhy::GetNominalTxPowerFromPib(uint8_t phyTransmitPower) {
  NS_LOG_FUNCTION(this << +phyTransmitPower);

  int8_t nominalTxPower = phyTransmitPower & 0x1F;

  if (phyTransmitPower & 0x20) {
    nominalTxPower -= 32;
  }
  return nominalTxPower;
}

double LrWpanPhy::WToDbm(double watt) { return (10 * log10(1000 * watt)); }

double LrWpanPhy::DbmToW(double dbm) {
  return (pow(10.0, dbm / 10.0) / 1000.0);
}

int64_t LrWpanPhy::AssignStreams(int64_t stream) {
  NS_LOG_FUNCTION(this);
  m_random->SetStream(stream);
  return 1;
}

void LrWpanPhy::SetPostReceptionErrorModel(const Ptr<ErrorModel> em) {
  NS_LOG_FUNCTION(this << em);
  m_postReceptionErrorModel = em;
}

} // namespace ns3
