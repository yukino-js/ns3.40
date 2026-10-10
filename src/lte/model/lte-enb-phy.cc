
#include "lte-enb-phy.h"

#include "lte-common.h"
#include "lte-control-messages.h"
#include "lte-net-device.h"
#include "lte-spectrum-value-helper.h"
#include "lte-vendor-specific-parameters.h"

#include <ns3/attribute-accessor-helper.h>
#include <ns3/double.h>
#include <ns3/log.h>
#include <ns3/object-factory.h>
#include <ns3/simulator.h>

#include <cfloat>
#include <cmath>

#include <ns3/node-list.h>
#include <ns3/node.h>
#include <ns3/pointer.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("LteEnbPhy");

NS_OBJECT_ENSURE_REGISTERED(LteEnbPhy);

static const Time DL_DATA_DURATION = NanoSeconds(785714 - 1);

static const Time DL_CTRL_DELAY_FROM_SUBFRAME_START = NanoSeconds(214286);

class EnbMemberLteEnbPhySapProvider : public LteEnbPhySapProvider {
public:
  EnbMemberLteEnbPhySapProvider(LteEnbPhy *phy);

  void SendMacPdu(Ptr<Packet> p) override;
  void SendLteControlMessage(Ptr<LteControlMessage> msg) override;
  uint8_t GetMacChTtiDelay() override;
  virtual void SetBandwidth(uint16_t ulBandwidth, uint16_t dlBandwidth);
  virtual void SetCellId(uint16_t cellId);

private:
  LteEnbPhy *m_phy;
};

EnbMemberLteEnbPhySapProvider::EnbMemberLteEnbPhySapProvider(LteEnbPhy *phy)
    : m_phy(phy) {}

void EnbMemberLteEnbPhySapProvider::SendMacPdu(Ptr<Packet> p) {
  m_phy->DoSendMacPdu(p);
}

void EnbMemberLteEnbPhySapProvider::SetBandwidth(uint16_t ulBandwidth,
                                                 uint16_t dlBandwidth) {
  m_phy->DoSetBandwidth(ulBandwidth, dlBandwidth);
}

void EnbMemberLteEnbPhySapProvider::SetCellId(uint16_t cellId) {
  m_phy->DoSetCellId(cellId);
}

void EnbMemberLteEnbPhySapProvider::SendLteControlMessage(
    Ptr<LteControlMessage> msg) {
  m_phy->DoSendLteControlMessage(msg);
}

uint8_t EnbMemberLteEnbPhySapProvider::GetMacChTtiDelay() {
  return (m_phy->DoGetMacChTtiDelay());
}

LteEnbPhy::LteEnbPhy() {
  NS_LOG_FUNCTION(this);
  NS_FATAL_ERROR("This constructor should not be called");
}

LteEnbPhy::LteEnbPhy(Ptr<LteSpectrumPhy> dlPhy, Ptr<LteSpectrumPhy> ulPhy)
    : LtePhy(dlPhy, ulPhy), m_enbPhySapUser(nullptr), m_enbCphySapUser(nullptr),
      m_nrFrames(0), m_nrSubFrames(0), m_srsPeriodicity(0),
      m_srsStartTime(Seconds(0)), m_currentSrsOffset(0),
      m_interferenceSampleCounter(0) {
  m_enbPhySapProvider = new EnbMemberLteEnbPhySapProvider(this);
  m_enbCphySapProvider = new MemberLteEnbCphySapProvider<LteEnbPhy>(this);
  m_harqPhyModule = Create<LteHarqPhy>();
  m_downlinkSpectrumPhy->SetHarqPhyModule(m_harqPhyModule);
  m_uplinkSpectrumPhy->SetHarqPhyModule(m_harqPhyModule);
}

TypeId LteEnbPhy::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::LteEnbPhy")
          .SetParent<LtePhy>()
          .SetGroupName("Lte")
          .AddConstructor<LteEnbPhy>()
          .AddAttribute("TxPower", "Transmission power in dBm",
                        DoubleValue(30.0),
                        MakeDoubleAccessor(&LteEnbPhy::SetTxPower,
                                           &LteEnbPhy::GetTxPower),
                        MakeDoubleChecker<double>())
          .AddAttribute(
              "NoiseFigure",
              "Loss (dB) in the Signal-to-Noise-Ratio due to "
              "non-idealities in the receiver.  According to Wikipedia "
              "(http://en.wikipedia.org/wiki/Noise_figure), this is "
              "\"the difference in decibels (dB) between"
              " the noise output of the actual receiver to "
              "the noise output of an ideal receiver with "
              "the same overall gain and bandwidth when the receivers "
              "are connected to sources at the standard noise "
              "temperature T0.\"  In this model, we consider T0 = 290K.",
              DoubleValue(5.0),
              MakeDoubleAccessor(&LteEnbPhy::SetNoiseFigure,
                                 &LteEnbPhy::GetNoiseFigure),
              MakeDoubleChecker<double>())
          .AddAttribute("MacToChannelDelay",
                        "The delay in TTI units that occurs between "
                        "a scheduling decision in the MAC and the actual "
                        "start of the transmission by the PHY. This is "
                        "intended to be used to model the latency of real PHY "
                        "and MAC implementations.",
                        UintegerValue(2),
                        MakeUintegerAccessor(&LteEnbPhy::SetMacChDelay,
                                             &LteEnbPhy::GetMacChDelay),
                        MakeUintegerChecker<uint8_t>())
          .AddTraceSource("ReportUeSinr", "Report UEs' averaged linear SINR",
                          MakeTraceSourceAccessor(&LteEnbPhy::m_reportUeSinr),
                          "ns3::LteEnbPhy::ReportUeSinrTracedCallback")
          .AddAttribute("UeSinrSamplePeriod",
                        "The sampling period for reporting UEs' SINR stats.",
                        UintegerValue(1),
                        MakeUintegerAccessor(&LteEnbPhy::m_srsSamplePeriod),
                        MakeUintegerChecker<uint16_t>())
          .AddTraceSource(
              "ReportInterference",
              "Report linear interference power per PHY RB",
              MakeTraceSourceAccessor(&LteEnbPhy::m_reportInterferenceTrace),
              "ns3::LteEnbPhy::ReportInterferenceTracedCallback")
          .AddAttribute(
              "InterferenceSamplePeriod",
              "The sampling period for reporting interference stats",
              UintegerValue(1),
              MakeUintegerAccessor(&LteEnbPhy::m_interferenceSamplePeriod),
              MakeUintegerChecker<uint16_t>())
          .AddTraceSource(
              "DlPhyTransmission", "DL transmission PHY layer statistics.",
              MakeTraceSourceAccessor(&LteEnbPhy::m_dlPhyTransmission),
              "ns3::PhyTransmissionStatParameters::TracedCallback")
          .AddAttribute("DlSpectrumPhy",
                        "The downlink LteSpectrumPhy associated to this LtePhy",
                        TypeId::ATTR_GET, PointerValue(),
                        MakePointerAccessor(&LteEnbPhy::GetDlSpectrumPhy),
                        MakePointerChecker<LteSpectrumPhy>())
          .AddAttribute("UlSpectrumPhy",
                        "The uplink LteSpectrumPhy associated to this LtePhy",
                        TypeId::ATTR_GET, PointerValue(),
                        MakePointerAccessor(&LteEnbPhy::GetUlSpectrumPhy),
                        MakePointerChecker<LteSpectrumPhy>());
  return tid;
}

LteEnbPhy::~LteEnbPhy() {}

void LteEnbPhy::DoDispose() {
  NS_LOG_FUNCTION(this);
  m_ueAttached.clear();
  m_srsUeOffset.clear();
  delete m_enbPhySapProvider;
  delete m_enbCphySapProvider;
  LtePhy::DoDispose();
}

void LteEnbPhy::DoInitialize() {
  NS_LOG_FUNCTION(this);

  NS_ABORT_MSG_IF(!m_netDevice, "LteEnbDevice is not available in LteEnbPhy");
  Ptr<Node> node = m_netDevice->GetNode();
  NS_ABORT_MSG_IF(!node,
                  "Node is not available in the LteNetDevice of LteEnbPhy");
  uint32_t nodeId = node->GetId();

  Simulator::ScheduleWithContext(nodeId, Seconds(0), &LteEnbPhy::StartFrame,
                                 this);

  Ptr<SpectrumValue> noisePsd =
      LteSpectrumValueHelper::CreateNoisePowerSpectralDensity(
          m_ulEarfcn, m_ulBandwidth, m_noiseFigure);
  m_uplinkSpectrumPhy->SetNoisePowerSpectralDensity(noisePsd);
  LtePhy::DoInitialize();
}

void LteEnbPhy::SetLteEnbPhySapUser(LteEnbPhySapUser *s) {
  m_enbPhySapUser = s;
}

LteEnbPhySapProvider *LteEnbPhy::GetLteEnbPhySapProvider() {
  return (m_enbPhySapProvider);
}

void LteEnbPhy::SetLteEnbCphySapUser(LteEnbCphySapUser *s) {
  NS_LOG_FUNCTION(this);
  m_enbCphySapUser = s;
}

LteEnbCphySapProvider *LteEnbPhy::GetLteEnbCphySapProvider() {
  NS_LOG_FUNCTION(this);
  return (m_enbCphySapProvider);
}

void LteEnbPhy::SetTxPower(double pow) {
  NS_LOG_FUNCTION(this << pow);
  m_txPower = pow;
}

double LteEnbPhy::GetTxPower() const {
  NS_LOG_FUNCTION(this);
  return m_txPower;
}

int8_t LteEnbPhy::DoGetReferenceSignalPower() const {
  NS_LOG_FUNCTION(this);
  return m_txPower;
}

void LteEnbPhy::SetNoiseFigure(double nf) {
  NS_LOG_FUNCTION(this << nf);
  m_noiseFigure = nf;
}

double LteEnbPhy::GetNoiseFigure() const {
  NS_LOG_FUNCTION(this);
  return m_noiseFigure;
}

void LteEnbPhy::SetMacChDelay(uint8_t delay) {
  NS_LOG_FUNCTION(this);
  m_macChTtiDelay = delay;
  for (int i = 0; i < m_macChTtiDelay; i++) {
    Ptr<PacketBurst> pb = CreateObject<PacketBurst>();
    m_packetBurstQueue.push_back(pb);
    std::list<Ptr<LteControlMessage>> l;
    m_controlMessagesQueue.push_back(l);
    std::list<UlDciLteControlMessage> l1;
    m_ulDciQueue.push_back(l1);
  }
  for (int i = 0; i < UL_PUSCH_TTIS_DELAY; i++) {
    std::list<UlDciLteControlMessage> l1;
    m_ulDciQueue.push_back(l1);
  }
}

uint8_t LteEnbPhy::GetMacChDelay() const { return (m_macChTtiDelay); }

Ptr<LteSpectrumPhy> LteEnbPhy::GetDlSpectrumPhy() const {
  return m_downlinkSpectrumPhy;
}

Ptr<LteSpectrumPhy> LteEnbPhy::GetUlSpectrumPhy() const {
  return m_uplinkSpectrumPhy;
}

bool LteEnbPhy::AddUePhy(uint16_t rnti) {
  NS_LOG_FUNCTION(this << rnti);
  auto it = m_ueAttached.find(rnti);
  if (it == m_ueAttached.end()) {
    m_ueAttached.insert(rnti);
    return (true);
  } else {
    NS_LOG_ERROR("UE already attached");
    return (false);
  }
}

bool LteEnbPhy::DeleteUePhy(uint16_t rnti) {
  NS_LOG_FUNCTION(this << rnti);
  auto it = m_ueAttached.find(rnti);
  if (it == m_ueAttached.end()) {
    NS_LOG_ERROR("UE not attached");
    return (false);
  } else {
    m_ueAttached.erase(it);
    return (true);
  }
}

void LteEnbPhy::DoSendMacPdu(Ptr<Packet> p) {
  NS_LOG_FUNCTION(this);
  SetMacPdu(p);
}

uint8_t LteEnbPhy::DoGetMacChTtiDelay() { return (m_macChTtiDelay); }

void LteEnbPhy::PhyPduReceived(Ptr<Packet> p) {
  NS_LOG_FUNCTION(this);
  m_enbPhySapUser->ReceivePhyPdu(p);
}

void LteEnbPhy::SetDownlinkSubChannels(std::vector<int> mask) {
  NS_LOG_FUNCTION(this);
  m_listOfDownlinkSubchannel = mask;
  Ptr<SpectrumValue> txPsd = CreateTxPowerSpectralDensity();
  m_downlinkSpectrumPhy->SetTxPowerSpectralDensity(txPsd);
}

void LteEnbPhy::SetDownlinkSubChannelsWithPowerAllocation(
    std::vector<int> mask) {
  NS_LOG_FUNCTION(this);
  m_listOfDownlinkSubchannel = mask;
  Ptr<SpectrumValue> txPsd = CreateTxPowerSpectralDensityWithPowerAllocation();
  m_downlinkSpectrumPhy->SetTxPowerSpectralDensity(txPsd);
}

std::vector<int> LteEnbPhy::GetDownlinkSubChannels() {
  NS_LOG_FUNCTION(this);
  return m_listOfDownlinkSubchannel;
}

void LteEnbPhy::GeneratePowerAllocationMap(uint16_t rnti, int rbId) {
  NS_LOG_FUNCTION(this);
  double rbgTxPower = m_txPower;

  auto it = m_paMap.find(rnti);
  if (it != m_paMap.end()) {
    rbgTxPower = m_txPower + it->second;
  }

  m_dlPowerAllocationMap.insert(std::pair<int, double>(rbId, rbgTxPower));
}

Ptr<SpectrumValue> LteEnbPhy::CreateTxPowerSpectralDensity() {
  NS_LOG_FUNCTION(this);

  Ptr<SpectrumValue> psd = LteSpectrumValueHelper::CreateTxPowerSpectralDensity(
      m_dlEarfcn, m_dlBandwidth, m_txPower, GetDownlinkSubChannels());

  return psd;
}

Ptr<SpectrumValue>
LteEnbPhy::CreateTxPowerSpectralDensityWithPowerAllocation() {
  NS_LOG_FUNCTION(this);

  Ptr<SpectrumValue> psd = LteSpectrumValueHelper::CreateTxPowerSpectralDensity(
      m_dlEarfcn, m_dlBandwidth, m_txPower, m_dlPowerAllocationMap,
      GetDownlinkSubChannels());

  return psd;
}

void LteEnbPhy::CalcChannelQualityForUe(std::vector<double> sinr,
                                        Ptr<LteSpectrumPhy> ue) {
  NS_LOG_FUNCTION(this);
}

void LteEnbPhy::DoSendLteControlMessage(Ptr<LteControlMessage> msg) {
  NS_LOG_FUNCTION(this << msg);
  SetControlMessages(msg);
}

void LteEnbPhy::ReceiveLteControlMessage(Ptr<LteControlMessage> msg) {
  NS_FATAL_ERROR("Obsolete function");
  NS_LOG_FUNCTION(this << msg);
  m_enbPhySapUser->ReceiveLteControlMessage(msg);
}

void LteEnbPhy::ReceiveLteControlMessageList(
    std::list<Ptr<LteControlMessage>> msgList) {
  NS_LOG_FUNCTION(this);
  for (auto it = msgList.begin(); it != msgList.end(); it++) {
    switch ((*it)->GetMessageType()) {
    case LteControlMessage::RACH_PREAMBLE: {
      Ptr<RachPreambleLteControlMessage> rachPreamble =
          DynamicCast<RachPreambleLteControlMessage>(*it);
      m_enbPhySapUser->ReceiveRachPreamble(rachPreamble->GetRapId());
    } break;
    case LteControlMessage::DL_CQI: {
      Ptr<DlCqiLteControlMessage> dlcqiMsg =
          DynamicCast<DlCqiLteControlMessage>(*it);
      CqiListElement_s dlcqi = dlcqiMsg->GetDlCqi();
      if (m_ueAttached.find(dlcqi.m_rnti) != m_ueAttached.end()) {
        m_enbPhySapUser->ReceiveLteControlMessage(*it);
      }
    } break;
    case LteControlMessage::BSR: {
      Ptr<BsrLteControlMessage> bsrMsg = DynamicCast<BsrLteControlMessage>(*it);
      MacCeListElement_s bsr = bsrMsg->GetBsr();
      if (m_ueAttached.find(bsr.m_rnti) != m_ueAttached.end()) {
        m_enbPhySapUser->ReceiveLteControlMessage(*it);
      }
    } break;
    case LteControlMessage::DL_HARQ: {
      Ptr<DlHarqFeedbackLteControlMessage> dlharqMsg =
          DynamicCast<DlHarqFeedbackLteControlMessage>(*it);
      DlInfoListElement_s dlharq = dlharqMsg->GetDlHarqFeedback();
      if (m_ueAttached.find(dlharq.m_rnti) != m_ueAttached.end()) {
        m_enbPhySapUser->ReceiveLteControlMessage(*it);
      }
    } break;
    default:
      NS_FATAL_ERROR("Unexpected LteControlMessage type");
      break;
    }
  }
}

void LteEnbPhy::StartFrame() {
  NS_LOG_FUNCTION(this);

  ++m_nrFrames;
  NS_LOG_INFO("-----frame " << m_nrFrames << "-----");
  m_nrSubFrames = 0;

  m_mib.systemFrameNumber = m_nrSubFrames;
  Ptr<MibLteControlMessage> mibMsg = Create<MibLteControlMessage>();
  mibMsg->SetMib(m_mib);
  m_controlMessagesQueue.at(0).emplace_back(mibMsg);

  StartSubFrame();
}

void LteEnbPhy::StartSubFrame() {
  NS_LOG_FUNCTION(this);

  ++m_nrSubFrames;

  if ((m_nrSubFrames == 6) && ((m_nrFrames % 2) == 1)) {
    Ptr<Sib1LteControlMessage> msg = Create<Sib1LteControlMessage>();
    msg->SetSib1(m_sib1);
    m_controlMessagesQueue.at(0).emplace_back(msg);
  }

  if (m_srsPeriodicity > 0) {
    NS_ASSERT_MSG(m_nrFrames > 1,
                  "the SRS index check code assumes that frameNo starts at 1");
    NS_ASSERT_MSG(
        m_nrSubFrames > 0 && m_nrSubFrames <= 10,
        "the SRS index check code assumes that subframeNo starts at 1");
    m_currentSrsOffset =
        (((m_nrFrames - 1) * 10 + (m_nrSubFrames - 1)) % m_srsPeriodicity);
  }
  NS_LOG_INFO("-----sub frame " << m_nrSubFrames << "-----");
  m_harqPhyModule->SubframeIndication(m_nrFrames, m_nrSubFrames);

  std::list<UlDciLteControlMessage> uldcilist = DequeueUlDci();
  NS_LOG_DEBUG(this << " eNB Expected TBs " << uldcilist.size());
  for (auto dciIt = uldcilist.begin(); dciIt != uldcilist.end(); dciIt++) {
    auto it2 = m_ueAttached.find((*dciIt).GetDci().m_rnti);

    if (it2 == m_ueAttached.end()) {
      NS_LOG_ERROR("UE not attached");
    } else {
      std::vector<int> rbMap;
      for (int i = (*dciIt).GetDci().m_rbStart;
           i < (*dciIt).GetDci().m_rbStart + (*dciIt).GetDci().m_rbLen; i++) {
        rbMap.push_back(i);
      }
      m_uplinkSpectrumPhy->AddExpectedTb(
          (*dciIt).GetDci().m_rnti, (*dciIt).GetDci().m_ndi,
          (*dciIt).GetDci().m_tbSize, (*dciIt).GetDci().m_mcs, rbMap, 0, 0, 0,
          false);
      if ((*dciIt).GetDci().m_ndi == 1) {
        NS_LOG_DEBUG(this << " RNTI " << (*dciIt).GetDci().m_rnti << " NEW TB");
      } else {
        NS_LOG_DEBUG(this << " RNTI " << (*dciIt).GetDci().m_rnti
                          << " HARQ RETX");
      }
    }
  }

  std::list<Ptr<LteControlMessage>> ctrlMsg = GetControlMessages();
  m_dlDataRbMap.clear();
  m_dlPowerAllocationMap.clear();
  if (!ctrlMsg.empty()) {
    auto it = ctrlMsg.begin();
    while (it != ctrlMsg.end()) {
      Ptr<LteControlMessage> msg = (*it);
      if (msg->GetMessageType() == LteControlMessage::DL_DCI) {
        Ptr<DlDciLteControlMessage> dci =
            DynamicCast<DlDciLteControlMessage>(msg);
        uint32_t mask = 0x1;
        for (int i = 0; i < 32; i++) {
          if (((dci->GetDci().m_rbBitmap & mask) >> i) == 1) {
            for (int k = 0; k < GetRbgSize(); k++) {
              m_dlDataRbMap.push_back((i * GetRbgSize()) + k);
              GeneratePowerAllocationMap(dci->GetDci().m_rnti,
                                         (i * GetRbgSize()) + k);
            }
          }
          mask = (mask << 1);
        }
        for (std::size_t i = 0; i < dci->GetDci().m_mcs.size(); i++) {
          PhyTransmissionStatParameters params;
          params.m_cellId = m_cellId;
          params.m_imsi = 0;
          params.m_timestamp = Simulator::Now().GetMilliSeconds();
          params.m_rnti = dci->GetDci().m_rnti;
          params.m_txMode = 0;
          params.m_layer = i;
          params.m_mcs = dci->GetDci().m_mcs.at(i);
          params.m_size = dci->GetDci().m_tbsSize.at(i);
          params.m_rv = dci->GetDci().m_rv.at(i);
          params.m_ndi = dci->GetDci().m_ndi.at(i);
          params.m_ccId = m_componentCarrierId;
          m_dlPhyTransmission(params);
        }
      } else if (msg->GetMessageType() == LteControlMessage::UL_DCI) {
        Ptr<UlDciLteControlMessage> dci =
            DynamicCast<UlDciLteControlMessage>(msg);
        QueueUlDci(*dci);
      } else if (msg->GetMessageType() == LteControlMessage::RAR) {
        Ptr<RarLteControlMessage> rarMsg =
            DynamicCast<RarLteControlMessage>(msg);
        for (auto it = rarMsg->RarListBegin(); it != rarMsg->RarListEnd();
             ++it) {
          if (it->rarPayload.m_grant.m_ulDelay) {
            NS_FATAL_ERROR(" RAR delay is not yet implemented");
          }
          UlGrant_s ulGrant = it->rarPayload.m_grant;
          UlDciListElement_s dci;
          dci.m_rnti = ulGrant.m_rnti;
          dci.m_rbStart = ulGrant.m_rbStart;
          dci.m_rbLen = ulGrant.m_rbLen;
          dci.m_tbSize = ulGrant.m_tbSize;
          dci.m_mcs = ulGrant.m_mcs;
          dci.m_hopping = ulGrant.m_hopping;
          dci.m_tpc = ulGrant.m_tpc;
          dci.m_cqiRequest = ulGrant.m_cqiRequest;
          dci.m_ndi = 1;
          UlDciLteControlMessage msg;
          msg.SetDci(dci);
          QueueUlDci(msg);
        }
      }
      it++;
    }
  }

  SendControlChannels(ctrlMsg);

  Ptr<PacketBurst> pb = GetPacketBurst();
  if (pb) {
    Simulator::Schedule(DL_CTRL_DELAY_FROM_SUBFRAME_START,
                        &LteEnbPhy::SendDataChannels, this, pb);
  }

  m_enbPhySapUser->SubframeIndication(m_nrFrames, m_nrSubFrames);

  Simulator::Schedule(Seconds(GetTti()), &LteEnbPhy::EndSubFrame, this);
}

void LteEnbPhy::SendControlChannels(
    std::list<Ptr<LteControlMessage>> ctrlMsgList) {
  NS_LOG_FUNCTION(this << " eNB " << m_cellId << " start tx ctrl frame");
  std::vector<int> dlRb;
  for (uint16_t i = 0; i < m_dlBandwidth; i++) {
    dlRb.push_back(i);
  }
  SetDownlinkSubChannels(dlRb);
  NS_LOG_LOGIC(this << " eNB start TX CTRL");
  bool pss = false;
  if ((m_nrSubFrames == 1) || (m_nrSubFrames == 6)) {
    pss = true;
  }
  m_downlinkSpectrumPhy->StartTxDlCtrlFrame(ctrlMsgList, pss);
}

void LteEnbPhy::SendDataChannels(Ptr<PacketBurst> pb) {
  SetDownlinkSubChannelsWithPowerAllocation(m_dlDataRbMap);
  NS_LOG_LOGIC(this << " eNB start TX DATA");
  std::list<Ptr<LteControlMessage>> ctrlMsgList;
  ctrlMsgList.clear();
  m_downlinkSpectrumPhy->StartTxDataFrame(pb, ctrlMsgList, DL_DATA_DURATION);
}

void LteEnbPhy::EndSubFrame() {
  NS_LOG_FUNCTION(this << Simulator::Now().As(Time::S));
  if (m_nrSubFrames == 10) {
    Simulator::ScheduleNow(&LteEnbPhy::EndFrame, this);
  } else {
    Simulator::ScheduleNow(&LteEnbPhy::StartSubFrame, this);
  }
}

void LteEnbPhy::EndFrame() {
  NS_LOG_FUNCTION(this << Simulator::Now().As(Time::S));
  Simulator::ScheduleNow(&LteEnbPhy::StartFrame, this);
}

void LteEnbPhy::GenerateCtrlCqiReport(const SpectrumValue &sinr) {
  NS_LOG_FUNCTION(this << sinr << Simulator::Now() << m_srsStartTime);
  if (Simulator::Now() > m_srsStartTime) {
    FfMacSchedSapProvider::SchedUlCqiInfoReqParameters ulcqi =
        CreateSrsCqiReport(sinr);
    m_enbPhySapUser->UlCqiReport(ulcqi);
  }
}

void LteEnbPhy::GenerateDataCqiReport(const SpectrumValue &sinr) {
  NS_LOG_FUNCTION(this << sinr);
  FfMacSchedSapProvider::SchedUlCqiInfoReqParameters ulcqi =
      CreatePuschCqiReport(sinr);
  m_enbPhySapUser->UlCqiReport(ulcqi);
}

void LteEnbPhy::ReportInterference(const SpectrumValue &interf) {
  NS_LOG_FUNCTION(this << interf);
  Ptr<SpectrumValue> interfCopy = Create<SpectrumValue>(interf);
  m_interferenceSampleCounter++;
  if (m_interferenceSampleCounter == m_interferenceSamplePeriod) {
    m_reportInterferenceTrace(m_cellId, interfCopy);
    m_interferenceSampleCounter = 0;
  }
}

void LteEnbPhy::ReportRsReceivedPower(const SpectrumValue &power) {}

FfMacSchedSapProvider::SchedUlCqiInfoReqParameters
LteEnbPhy::CreatePuschCqiReport(const SpectrumValue &sinr) {
  NS_LOG_FUNCTION(this << sinr);
  FfMacSchedSapProvider::SchedUlCqiInfoReqParameters ulcqi;
  ulcqi.m_ulCqi.m_type = UlCqi_s::PUSCH;
  for (auto it = sinr.ConstValuesBegin(); it != sinr.ConstValuesEnd(); it++) {
    double sinrdb = 10 * std::log10((*it));
    int16_t sinrFp = LteFfConverter::double2fpS11dot3(sinrdb);
    ulcqi.m_ulCqi.m_sinr.push_back(sinrFp);
  }
  return ulcqi;
}

void LteEnbPhy::DoSetBandwidth(uint16_t ulBandwidth, uint16_t dlBandwidth) {
  NS_LOG_FUNCTION(this << (uint32_t)ulBandwidth << (uint32_t)dlBandwidth);
  m_ulBandwidth = ulBandwidth;
  m_dlBandwidth = dlBandwidth;

  static const int Type0AllocationRbg[4] = {
      10,
      26,
      63,
      110,
  };
  for (int i = 0; i < 4; i++) {
    if (dlBandwidth < Type0AllocationRbg[i]) {
      m_rbgSize = i + 1;
      break;
    }
  }
}

void LteEnbPhy::DoSetEarfcn(uint32_t ulEarfcn, uint32_t dlEarfcn) {
  NS_LOG_FUNCTION(this << ulEarfcn << dlEarfcn);
  m_ulEarfcn = ulEarfcn;
  m_dlEarfcn = dlEarfcn;
}

void LteEnbPhy::DoAddUe(uint16_t rnti) {
  NS_LOG_FUNCTION(this << rnti);

  bool success = AddUePhy(rnti);
  NS_ASSERT_MSG(success, "AddUePhy() failed");

  DoSetPa(rnti, 0);
}

void LteEnbPhy::DoRemoveUe(uint16_t rnti) {
  NS_LOG_FUNCTION(this << rnti);

  bool success = DeleteUePhy(rnti);
  NS_ASSERT_MSG(success, "DeleteUePhy() failed");

  auto it = m_paMap.find(rnti);
  if (it != m_paMap.end()) {
    m_paMap.erase(it);
  }

  m_uplinkSpectrumPhy->RemoveExpectedTb(rnti);
  auto sit = m_srsSampleCounterMap.find(rnti);
  if (sit != m_srsSampleCounterMap.end()) {
    m_srsSampleCounterMap.erase(rnti);
  }

  for (auto &ctrlMessageList : m_controlMessagesQueue) {
    auto ctrlMsgListIt = ctrlMessageList.begin();
    while (ctrlMsgListIt != ctrlMessageList.end()) {
      Ptr<LteControlMessage> msg = (*ctrlMsgListIt);
      if (msg->GetMessageType() == LteControlMessage::DL_DCI) {
        auto dci = DynamicCast<DlDciLteControlMessage>(msg);
        if (dci->GetDci().m_rnti == rnti) {
          NS_LOG_INFO("DL_DCI to be sent from cell id : "
                      << m_cellId << " to RNTI : " << rnti << " is deleted");
          ctrlMsgListIt = ctrlMessageList.erase(ctrlMsgListIt);
        } else {
          ++ctrlMsgListIt;
        }
      } else if (msg->GetMessageType() == LteControlMessage::UL_DCI) {
        auto dci = DynamicCast<UlDciLteControlMessage>(msg);
        if (dci->GetDci().m_rnti == rnti) {
          NS_LOG_INFO("UL_DCI to be sent from cell id : "
                      << m_cellId << " to RNTI : " << rnti << " is deleted");
          ctrlMsgListIt = ctrlMessageList.erase(ctrlMsgListIt);
        } else {
          ++ctrlMsgListIt;
        }
      } else {
        ++ctrlMsgListIt;
      }
    }
  }
}

void LteEnbPhy::DoSetPa(uint16_t rnti, double pa) {
  NS_LOG_FUNCTION(this << rnti);

  auto it = m_paMap.find(rnti);

  if (it == m_paMap.end()) {
    m_paMap.insert(std::pair<uint16_t, double>(rnti, pa));
  } else {
    it->second = pa;
  }
}

FfMacSchedSapProvider::SchedUlCqiInfoReqParameters
LteEnbPhy::CreateSrsCqiReport(const SpectrumValue &sinr) {
  NS_LOG_FUNCTION(this << sinr);
  FfMacSchedSapProvider::SchedUlCqiInfoReqParameters ulcqi;
  ulcqi.m_ulCqi.m_type = UlCqi_s::SRS;
  int i = 0;
  double srsSum = 0.0;
  for (auto it = sinr.ConstValuesBegin(); it != sinr.ConstValuesEnd(); it++) {
    double sinrdb = 10 * log10((*it));
    int16_t sinrFp = LteFfConverter::double2fpS11dot3(sinrdb);
    srsSum += (*it);
    ulcqi.m_ulCqi.m_sinr.push_back(sinrFp);
    i++;
  }
  NS_LOG_DEBUG(this << " ENB RX UL-CQI of "
                    << m_srsUeOffset.at(m_currentSrsOffset));
  VendorSpecificListElement_s vsp;
  vsp.m_type = SRS_CQI_RNTI_VSP;
  vsp.m_length = sizeof(SrsCqiRntiVsp);
  Ptr<SrsCqiRntiVsp> rnti =
      Create<SrsCqiRntiVsp>(m_srsUeOffset.at(m_currentSrsOffset));
  vsp.m_value = rnti;
  ulcqi.m_vendorSpecificList.push_back(vsp);
  CreateSrsReport(m_srsUeOffset.at(m_currentSrsOffset),
                  (i > 0) ? (srsSum / i) : DBL_MAX);
  return (ulcqi);
}

void LteEnbPhy::CreateSrsReport(uint16_t rnti, double srs) {
  NS_LOG_FUNCTION(this << rnti << srs);
  auto it = m_srsSampleCounterMap.find(rnti);
  if (it == m_srsSampleCounterMap.end()) {
    m_srsSampleCounterMap.insert(std::pair<uint16_t, uint16_t>(rnti, 0));
    it = m_srsSampleCounterMap.find(rnti);
  }
  (*it).second++;
  if ((*it).second == m_srsSamplePeriod) {
    m_reportUeSinr(m_cellId, rnti, srs, (uint16_t)m_componentCarrierId);
    (*it).second = 0;
  }
}

void LteEnbPhy::DoSetTransmissionMode(uint16_t rnti, uint8_t txMode) {
  NS_LOG_FUNCTION(this << rnti << (uint16_t)txMode);
}

void LteEnbPhy::QueueUlDci(UlDciLteControlMessage m) {
  NS_LOG_FUNCTION(this);
  m_ulDciQueue.at(UL_PUSCH_TTIS_DELAY - 1).push_back(m);
}

std::list<UlDciLteControlMessage> LteEnbPhy::DequeueUlDci() {
  NS_LOG_FUNCTION(this);
  if (!m_ulDciQueue.at(0).empty()) {
    std::list<UlDciLteControlMessage> ret = m_ulDciQueue.at(0);
    m_ulDciQueue.erase(m_ulDciQueue.begin());
    std::list<UlDciLteControlMessage> l;
    m_ulDciQueue.push_back(l);
    return (ret);
  } else {
    m_ulDciQueue.erase(m_ulDciQueue.begin());
    std::list<UlDciLteControlMessage> l;
    m_ulDciQueue.push_back(l);
    std::list<UlDciLteControlMessage> emptylist;
    return (emptylist);
  }
}

void LteEnbPhy::DoSetSrsConfigurationIndex(uint16_t rnti, uint16_t srcCi) {
  NS_LOG_FUNCTION(this);
  uint16_t p = GetSrsPeriodicity(srcCi);
  if (p != m_srsPeriodicity) {
    m_srsUeOffset.clear();
    m_srsUeOffset.resize(p, 0);
    m_srsPeriodicity = p;
    m_srsStartTime =
        Simulator::Now() + MilliSeconds(m_macChTtiDelay) + MilliSeconds(0);
  }

  NS_LOG_DEBUG(this << " ENB SRS P " << m_srsPeriodicity << " RNTI " << rnti
                    << " offset " << GetSrsSubframeOffset(srcCi) << " CI "
                    << srcCi);
  auto it = m_srsCounter.find(rnti);
  if (it != m_srsCounter.end()) {
    (*it).second = GetSrsSubframeOffset(srcCi) + 1;
  } else {
    m_srsCounter.insert(
        std::pair<uint16_t, uint16_t>(rnti, GetSrsSubframeOffset(srcCi) + 1));
  }
  m_srsUeOffset.at(GetSrsSubframeOffset(srcCi)) = rnti;
}

void LteEnbPhy::DoSetMasterInformationBlock(
    LteRrcSap::MasterInformationBlock mib) {
  NS_LOG_FUNCTION(this);
  m_mib = mib;
}

void LteEnbPhy::DoSetSystemInformationBlockType1(
    LteRrcSap::SystemInformationBlockType1 sib1) {
  NS_LOG_FUNCTION(this);
  m_sib1 = sib1;
}

void LteEnbPhy::SetHarqPhyModule(Ptr<LteHarqPhy> harq) {
  m_harqPhyModule = harq;
}

void LteEnbPhy::ReportUlHarqFeedback(UlInfoListElement_s mes) {
  NS_LOG_FUNCTION(this);
  m_enbPhySapUser->UlInfoListElementHarqFeedback(mes);
}

} // namespace ns3
