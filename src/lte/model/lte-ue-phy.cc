
#include "lte-ue-phy.h"

#include "ff-mac-common.h"
#include "lte-amc.h"
#include "lte-common.h"
#include "lte-net-device.h"
#include "lte-spectrum-value-helper.h"
#include "lte-ue-net-device.h"
#include "lte-ue-power-control.h"

#include <ns3/boolean.h>
#include <ns3/double.h>
#include <ns3/log.h>
#include <ns3/node.h>
#include <ns3/object-factory.h>
#include <ns3/pointer.h>
#include <ns3/simulator.h>

#include <cfloat>
#include <cmath>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("LteUePhy");

static const Time UL_DATA_DURATION = NanoSeconds(1e6 - 71429 - 1);

static const Time UL_SRS_DELAY_FROM_SUBFRAME_START = NanoSeconds(1e6 - 71429);

class UeMemberLteUePhySapProvider : public LteUePhySapProvider {
public:
  UeMemberLteUePhySapProvider(LteUePhy *phy);

  void SendMacPdu(Ptr<Packet> p) override;
  void SendLteControlMessage(Ptr<LteControlMessage> msg) override;
  void SendRachPreamble(uint32_t prachId, uint32_t raRnti) override;
  void NotifyConnectionSuccessful() override;

private:
  LteUePhy *m_phy;
};

UeMemberLteUePhySapProvider::UeMemberLteUePhySapProvider(LteUePhy *phy)
    : m_phy(phy) {}

void UeMemberLteUePhySapProvider::SendMacPdu(Ptr<Packet> p) {
  m_phy->DoSendMacPdu(p);
}

void UeMemberLteUePhySapProvider::SendLteControlMessage(
    Ptr<LteControlMessage> msg) {
  m_phy->DoSendLteControlMessage(msg);
}

void UeMemberLteUePhySapProvider::SendRachPreamble(uint32_t prachId,
                                                   uint32_t raRnti) {
  m_phy->DoSendRachPreamble(prachId, raRnti);
}

void UeMemberLteUePhySapProvider::NotifyConnectionSuccessful() {
  m_phy->DoNotifyConnectionSuccessful();
}

static const std::string g_uePhyStateName[LteUePhy::NUM_STATES] = {
    "CELL_SEARCH",
    "SYNCHRONIZED",
};

static inline const std::string &ToString(LteUePhy::State s) {
  return g_uePhyStateName[s];
}

NS_OBJECT_ENSURE_REGISTERED(LteUePhy);

LteUePhy::LteUePhy() {
  NS_LOG_FUNCTION(this);
  NS_FATAL_ERROR("This constructor should not be called");
}

LteUePhy::LteUePhy(Ptr<LteSpectrumPhy> dlPhy, Ptr<LteSpectrumPhy> ulPhy)
    : LtePhy(dlPhy, ulPhy), m_uePhySapUser(nullptr), m_ueCphySapUser(nullptr),
      m_state(CELL_SEARCH), m_subframeNo(0), m_rsReceivedPowerUpdated(false),
      m_rsInterferencePowerUpdated(false),
      m_dataInterferencePowerUpdated(false), m_pssReceived(false),
      m_ueMeasurementsFilterPeriod(MilliSeconds(200)),
      m_ueMeasurementsFilterLast(MilliSeconds(0)), m_rsrpSinrSampleCounter(0),
      m_imsi(0) {
  m_amc = CreateObject<LteAmc>();
  m_powerControl = CreateObject<LteUePowerControl>();
  m_uePhySapProvider = new UeMemberLteUePhySapProvider(this);
  m_ueCphySapProvider = new MemberLteUeCphySapProvider<LteUePhy>(this);
  m_macChTtiDelay = UL_PUSCH_TTIS_DELAY;

  NS_ASSERT_MSG(Simulator::Now().GetNanoSeconds() == 0,
                "Cannot create UE devices after simulation started");
  Simulator::Schedule(m_ueMeasurementsFilterPeriod,
                      &LteUePhy::ReportUeMeasurements, this);

  DoReset();
}

LteUePhy::~LteUePhy() { m_txModeGain.clear(); }

void LteUePhy::DoDispose() {
  NS_LOG_FUNCTION(this);
  delete m_uePhySapProvider;
  delete m_ueCphySapProvider;
  LtePhy::DoDispose();
}

TypeId LteUePhy::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::LteUePhy")
          .SetParent<LtePhy>()
          .SetGroupName("Lte")
          .AddConstructor<LteUePhy>()
          .AddAttribute(
              "TxPower", "Transmission power in dBm", DoubleValue(10.0),
              MakeDoubleAccessor(&LteUePhy::SetTxPower, &LteUePhy::GetTxPower),
              MakeDoubleChecker<double>())
          .AddAttribute("NoiseFigure",
                        "Loss (dB) in the Signal-to-Noise-Ratio due to "
                        "non-idealities in the receiver."
                        " According to Wikipedia "
                        "(http://en.wikipedia.org/wiki/Noise_figure), this is "
                        "\"the difference in decibels (dB) between"
                        " the noise output of the actual receiver to the noise "
                        "output of an "
                        " ideal receiver with the same overall gain and "
                        "bandwidth when the receivers "
                        " are connected to sources at the standard noise "
                        "temperature T0.\" "
                        "In this model, we consider T0 = 290K.",
                        DoubleValue(9.0),
                        MakeDoubleAccessor(&LteUePhy::SetNoiseFigure,
                                           &LteUePhy::GetNoiseFigure),
                        MakeDoubleChecker<double>())
          .AddAttribute("TxMode1Gain", "Transmission mode 1 gain in dB",
                        DoubleValue(0.0),
                        MakeDoubleAccessor(&LteUePhy::SetTxMode1Gain),
                        MakeDoubleChecker<double>())
          .AddAttribute("TxMode2Gain", "Transmission mode 2 gain in dB",
                        DoubleValue(4.2),
                        MakeDoubleAccessor(&LteUePhy::SetTxMode2Gain),
                        MakeDoubleChecker<double>())
          .AddAttribute("TxMode3Gain", "Transmission mode 3 gain in dB",
                        DoubleValue(-2.8),
                        MakeDoubleAccessor(&LteUePhy::SetTxMode3Gain),
                        MakeDoubleChecker<double>())
          .AddAttribute("TxMode4Gain", "Transmission mode 4 gain in dB",
                        DoubleValue(0.0),
                        MakeDoubleAccessor(&LteUePhy::SetTxMode4Gain),
                        MakeDoubleChecker<double>())
          .AddAttribute("TxMode5Gain", "Transmission mode 5 gain in dB",
                        DoubleValue(0.0),
                        MakeDoubleAccessor(&LteUePhy::SetTxMode5Gain),
                        MakeDoubleChecker<double>())
          .AddAttribute("TxMode6Gain", "Transmission mode 6 gain in dB",
                        DoubleValue(0.0),
                        MakeDoubleAccessor(&LteUePhy::SetTxMode6Gain),
                        MakeDoubleChecker<double>())
          .AddAttribute("TxMode7Gain", "Transmission mode 7 gain in dB",
                        DoubleValue(0.0),
                        MakeDoubleAccessor(&LteUePhy::SetTxMode7Gain),
                        MakeDoubleChecker<double>())
          .AddTraceSource("ReportCurrentCellRsrpSinr",
                          "RSRP and SINR statistics.",
                          MakeTraceSourceAccessor(
                              &LteUePhy::m_reportCurrentCellRsrpSinrTrace),
                          "ns3::LteUePhy::RsrpSinrTracedCallback")
          .AddAttribute("RsrpSinrSamplePeriod",
                        "The sampling period for reporting RSRP-SINR stats "
                        "(default value 1)",
                        UintegerValue(1),
                        MakeUintegerAccessor(&LteUePhy::m_rsrpSinrSamplePeriod),
                        MakeUintegerChecker<uint16_t>())
          .AddTraceSource(
              "ReportUlPhyResourceBlocks",
              "UL transmission PHY layer resource blocks.",
              MakeTraceSourceAccessor(&LteUePhy::m_reportUlPhyResourceBlocks),
              "ns3::LteUePhy::UlPhyResourceBlocksTracedCallback")
          .AddTraceSource(
              "ReportPowerSpectralDensity", "Power Spectral Density data.",
              MakeTraceSourceAccessor(&LteUePhy::m_reportPowerSpectralDensity),
              "ns3::LteUePhy::PowerSpectralDensityTracedCallback")
          .AddTraceSource(
              "UlPhyTransmission", "DL transmission PHY layer statistics.",
              MakeTraceSourceAccessor(&LteUePhy::m_ulPhyTransmission),
              "ns3::PhyTransmissionStatParameters::TracedCallback")
          .AddAttribute("DlSpectrumPhy",
                        "The downlink LteSpectrumPhy associated to this LtePhy",
                        TypeId::ATTR_GET, PointerValue(),
                        MakePointerAccessor(&LteUePhy::GetDlSpectrumPhy),
                        MakePointerChecker<LteSpectrumPhy>())
          .AddAttribute("UlSpectrumPhy",
                        "The uplink LteSpectrumPhy associated to this LtePhy",
                        TypeId::ATTR_GET, PointerValue(),
                        MakePointerAccessor(&LteUePhy::GetUlSpectrumPhy),
                        MakePointerChecker<LteSpectrumPhy>())
          .AddAttribute("RsrqUeMeasThreshold",
                        "Receive threshold for PSS on RSRQ [dB]",
                        DoubleValue(-1000.0),
                        MakeDoubleAccessor(&LteUePhy::m_pssReceptionThreshold),
                        MakeDoubleChecker<double>())
          .AddAttribute(
              "UeMeasurementsFilterPeriod",
              "Time period for reporting UE measurements, i.e., the"
              "length of layer-1 filtering.",
              TimeValue(MilliSeconds(200)),
              MakeTimeAccessor(&LteUePhy::m_ueMeasurementsFilterPeriod),
              MakeTimeChecker())
          .AddAttribute("DownlinkCqiPeriodicity",
                        "Periodicity in milliseconds for reporting the"
                        "wideband and subband downlink CQIs to the eNB",
                        TimeValue(MilliSeconds(1)),
                        MakeTimeAccessor(&LteUePhy::SetDownlinkCqiPeriodicity),
                        MakeTimeChecker())
          .AddTraceSource(
              "ReportUeMeasurements",
              "Report UE measurements RSRP (dBm) and RSRQ (dB).",
              MakeTraceSourceAccessor(&LteUePhy::m_reportUeMeasurements),
              "ns3::LteUePhy::RsrpRsrqTracedCallback")
          .AddTraceSource(
              "StateTransition",
              "Trace fired upon every UE PHY state transition",
              MakeTraceSourceAccessor(&LteUePhy::m_stateTransitionTrace),
              "ns3::LteUePhy::StateTracedCallback")
          .AddAttribute(
              "EnableUplinkPowerControl",
              "If true, Uplink Power Control will be enabled.",
              BooleanValue(true),
              MakeBooleanAccessor(&LteUePhy::m_enableUplinkPowerControl),
              MakeBooleanChecker())
          .AddAttribute("Qout",
                        "corresponds to 10% block error rate of a hypothetical "
                        "PDCCH transmission"
                        "taking into account the PCFICH errors with "
                        "transmission parameters."
                        "see 3GPP TS 36.213 4.2.1 and TS 36.133 7.6",
                        DoubleValue(-5), MakeDoubleAccessor(&LteUePhy::m_qOut),
                        MakeDoubleChecker<double>())
          .AddAttribute("Qin",
                        "corresponds to 2% block error rate of a hypothetical "
                        "PDCCH transmission"
                        "taking into account the PCFICH errors with "
                        "transmission parameters."
                        "see 3GPP TS 36.213 4.2.1 and TS 36.133 7.6",
                        DoubleValue(-3.9), MakeDoubleAccessor(&LteUePhy::m_qIn),
                        MakeDoubleChecker<double>())
          .AddAttribute(
              "NumQoutEvalSf",
              "This specifies the total number of consecutive subframes"
              "which corresponds to the Qout evaluation period",
              UintegerValue(200),
              MakeUintegerAccessor(&LteUePhy::SetNumQoutEvalSf,
                                   &LteUePhy::GetNumQoutEvalSf),
              MakeUintegerChecker<uint16_t>())
          .AddAttribute(
              "NumQinEvalSf",
              "This specifies the total number of consecutive subframes"
              "which corresponds to the Qin evaluation period",
              UintegerValue(100),
              MakeUintegerAccessor(&LteUePhy::SetNumQinEvalSf,
                                   &LteUePhy::GetNumQinEvalSf),
              MakeUintegerChecker<uint16_t>())
          .AddAttribute("EnableRlfDetection",
                        "If true, RLF detection will be enabled.",
                        BooleanValue(true),
                        MakeBooleanAccessor(&LteUePhy::m_enableRlfDetection),
                        MakeBooleanChecker());
  return tid;
}

void LteUePhy::DoInitialize() {
  NS_LOG_FUNCTION(this);

  NS_ABORT_MSG_IF(!m_netDevice, "LteNetDevice is not available in LteUePhy");
  Ptr<Node> node = m_netDevice->GetNode();
  NS_ABORT_MSG_IF(!node,
                  "Node is not available in the LteNetDevice of LteUePhy");
  uint32_t nodeId = node->GetId();

  Simulator::ScheduleWithContext(nodeId, Seconds(0),
                                 &LteUePhy::SubframeIndication, this, 1, 1);

  LtePhy::DoInitialize();
}

void LteUePhy::SetLteUePhySapUser(LteUePhySapUser *s) {
  NS_LOG_FUNCTION(this);
  m_uePhySapUser = s;
}

LteUePhySapProvider *LteUePhy::GetLteUePhySapProvider() {
  NS_LOG_FUNCTION(this);
  return (m_uePhySapProvider);
}

void LteUePhy::SetLteUeCphySapUser(LteUeCphySapUser *s) {
  NS_LOG_FUNCTION(this);
  m_ueCphySapUser = s;
}

LteUeCphySapProvider *LteUePhy::GetLteUeCphySapProvider() {
  NS_LOG_FUNCTION(this);
  return (m_ueCphySapProvider);
}

void LteUePhy::SetNoiseFigure(double nf) {
  NS_LOG_FUNCTION(this << nf);
  m_noiseFigure = nf;
}

double LteUePhy::GetNoiseFigure() const {
  NS_LOG_FUNCTION(this);
  return m_noiseFigure;
}

void LteUePhy::SetTxPower(double pow) {
  NS_LOG_FUNCTION(this << pow);
  m_txPower = pow;
  m_powerControl->SetTxPower(pow);
}

double LteUePhy::GetTxPower() const {
  NS_LOG_FUNCTION(this);
  return m_txPower;
}

Ptr<LteUePowerControl> LteUePhy::GetUplinkPowerControl() const {
  NS_LOG_FUNCTION(this);
  return m_powerControl;
}

uint8_t LteUePhy::GetMacChDelay() const { return (m_macChTtiDelay); }

Ptr<LteSpectrumPhy> LteUePhy::GetDlSpectrumPhy() const {
  return m_downlinkSpectrumPhy;
}

Ptr<LteSpectrumPhy> LteUePhy::GetUlSpectrumPhy() const {
  return m_uplinkSpectrumPhy;
}

void LteUePhy::SetNumQoutEvalSf(uint16_t numSubframes) {
  NS_LOG_FUNCTION(this << numSubframes);
  NS_ABORT_MSG_IF(numSubframes % 10 != 0, "Number of subframes used for Qout "
                                          "evaluation must be multiple of 10");
  m_numOfQoutEvalSf = numSubframes;
}

void LteUePhy::SetNumQinEvalSf(uint16_t numSubframes) {
  NS_LOG_FUNCTION(this << numSubframes);
  NS_ABORT_MSG_IF(numSubframes % 10 != 0, "Number of subframes used for Qin "
                                          "evaluation must be multiple of 10");
  m_numOfQinEvalSf = numSubframes;
}

uint16_t LteUePhy::GetNumQoutEvalSf() const {
  NS_LOG_FUNCTION(this);
  return m_numOfQoutEvalSf;
}

uint16_t LteUePhy::GetNumQinEvalSf() const {
  NS_LOG_FUNCTION(this);
  return m_numOfQinEvalSf;
}

void LteUePhy::DoSendMacPdu(Ptr<Packet> p) {
  NS_LOG_FUNCTION(this);

  SetMacPdu(p);
}

void LteUePhy::PhyPduReceived(Ptr<Packet> p) {
  m_uePhySapUser->ReceivePhyPdu(p);
}

void LteUePhy::SetSubChannelsForTransmission(std::vector<int> mask) {
  NS_LOG_FUNCTION(this);

  m_subChannelsForTransmission = mask;

  Ptr<SpectrumValue> txPsd = CreateTxPowerSpectralDensity();
  m_uplinkSpectrumPhy->SetTxPowerSpectralDensity(txPsd);
}

void LteUePhy::SetSubChannelsForReception(std::vector<int> mask) {
  NS_LOG_FUNCTION(this);
  m_subChannelsForReception = mask;
}

std::vector<int> LteUePhy::GetSubChannelsForTransmission() {
  NS_LOG_FUNCTION(this);
  return m_subChannelsForTransmission;
}

std::vector<int> LteUePhy::GetSubChannelsForReception() {
  NS_LOG_FUNCTION(this);
  return m_subChannelsForReception;
}

Ptr<SpectrumValue> LteUePhy::CreateTxPowerSpectralDensity() {
  NS_LOG_FUNCTION(this);
  Ptr<SpectrumValue> psd =
      LteSpectrumValueHelper::CreateUlTxPowerSpectralDensity(
          m_ulEarfcn, m_ulBandwidth, m_txPower,
          GetSubChannelsForTransmission());
  m_reportPowerSpectralDensity(m_rnti, psd);

  return psd;
}

void LteUePhy::GenerateCtrlCqiReport(const SpectrumValue &sinr) {
  NS_LOG_FUNCTION(this);
  if (m_cellId == 0) {
    return;
  }
  m_ctrlSinrForRlf = sinr;
  GenerateCqiRsrpRsrq(sinr);
}

void LteUePhy::GenerateCqiRsrpRsrq(const SpectrumValue &sinr) {
  NS_LOG_FUNCTION(this << sinr);

  NS_ASSERT(m_state != CELL_SEARCH);
  NS_ASSERT(m_cellId > 0);

  if (m_dlConfigured && m_ulConfigured && (m_rnti > 0)) {
    if (Simulator::Now() > m_p10CqiLast + m_p10CqiPeriodicity) {
      NS_LOG_DEBUG("Reporting P10 CQI at : " << Simulator::Now().As(Time::MS)
                                             << ". Last reported at : "
                                             << m_p10CqiLast.As(Time::MS));
      Ptr<LteUeNetDevice> thisDevice = GetDevice()->GetObject<LteUeNetDevice>();
      Ptr<DlCqiLteControlMessage> msg = CreateDlCqiFeedbackMessage(sinr);
      if (msg) {
        DoSendLteControlMessage(msg);
      }
      m_p10CqiLast = Simulator::Now();
    }
    if (Simulator::Now() > m_a30CqiLast + m_a30CqiPeriodicity) {
      NS_LOG_DEBUG("Reporting A30 CQI at : " << Simulator::Now().As(Time::MS)
                                             << ". Last reported at : "
                                             << m_a30CqiLast.As(Time::MS));
      Ptr<LteUeNetDevice> thisDevice = GetDevice()->GetObject<LteUeNetDevice>();
      Ptr<DlCqiLteControlMessage> msg = CreateDlCqiFeedbackMessage(sinr);
      if (msg) {
        DoSendLteControlMessage(msg);
      }
      m_a30CqiLast = Simulator::Now();
    }
  }

  m_rsrpSinrSampleCounter++;
  if (m_rsrpSinrSampleCounter == m_rsrpSinrSamplePeriod) {
    NS_ASSERT_MSG(m_rsReceivedPowerUpdated, " RS received power info obsolete");
    double sum = 0.0;
    uint8_t rbNum = 0;
    for (auto it = m_rsReceivedPower.ConstValuesBegin();
         it != m_rsReceivedPower.ConstValuesEnd(); it++) {
      double powerTxW = ((*it) * 180000.0) / 12.0;
      sum += powerTxW;
      rbNum++;
    }
    double rsrp = (rbNum > 0) ? (sum / rbNum) : DBL_MAX;
    double avSinr = ComputeAvgSinr(sinr);

    NS_LOG_INFO(this << " cellId " << m_cellId << " rnti " << m_rnti << " RSRP "
                     << rsrp << " SINR " << avSinr << " ComponentCarrierId "
                     << (uint16_t)m_componentCarrierId);
    if (m_isConnected && m_enableRlfDetection) {
      double avrgSinrForRlf = ComputeAvgSinr(m_ctrlSinrForRlf);
      RlfDetection(10 * log10(avrgSinrForRlf));
    }

    m_reportCurrentCellRsrpSinrTrace(m_cellId, m_rnti, rsrp, avSinr,
                                     (uint16_t)m_componentCarrierId);
    m_rsrpSinrSampleCounter = 0;
  }

  if (m_pssReceived) {
    NS_ASSERT_MSG(m_rsInterferencePowerUpdated,
                  " RS interference power info obsolete");

    auto itPss = m_pssList.begin();
    while (itPss != m_pssList.end()) {
      uint16_t rbNum = 0;
      double rssiSum = 0.0;

      auto itIntN = m_rsInterferencePower.ConstValuesBegin();
      auto itPj = m_rsReceivedPower.ConstValuesBegin();
      for (itPj = m_rsReceivedPower.ConstValuesBegin();
           itPj != m_rsReceivedPower.ConstValuesEnd(); itIntN++, itPj++) {
        rbNum++;
        double interfPlusNoisePowerTxW = ((*itIntN) * 180000.0) / 12.0;
        double signalPowerTxW = ((*itPj) * 180000.0) / 12.0;
        rssiSum += (2 * (interfPlusNoisePowerTxW + signalPowerTxW));
      }

      NS_ASSERT(rbNum == (*itPss).nRB);
      double rsrq_dB = 10 * log10((*itPss).pssPsdSum / rssiSum);

      if (rsrq_dB > m_pssReceptionThreshold) {
        NS_LOG_INFO(this << " PSS RNTI " << m_rnti << " cellId " << m_cellId
                         << " has RSRQ " << rsrq_dB << " and RBnum " << rbNum);
        auto itMeasMap = m_ueMeasurementsMap.find((*itPss).cellId);
        if (itMeasMap != m_ueMeasurementsMap.end()) {
          (*itMeasMap).second.rsrqSum += rsrq_dB;
          (*itMeasMap).second.rsrqNum++;
        } else {
          NS_LOG_WARN("race condition of bug 2091 occurred");
        }
      }

      itPss++;
    }

    m_pssList.clear();
  }
}

double LteUePhy::ComputeAvgSinr(const SpectrumValue &sinr) {
  NS_LOG_FUNCTION(this);

  double sum = 0.0;
  uint8_t rbNum = 0;

  for (auto it = sinr.ConstValuesBegin(); it != sinr.ConstValuesEnd(); it++) {
    sum += (*it);
    rbNum++;
  }

  double avrgSinr = (rbNum > 0) ? (sum / rbNum) : DBL_MAX;

  return avrgSinr;
}

void LteUePhy::GenerateDataCqiReport(const SpectrumValue &sinr) {}

void LteUePhy::GenerateMixedCqiReport(const SpectrumValue &sinr) {
  NS_LOG_FUNCTION(this);

  if (m_cellId == 0) {
    return;
  }

  NS_ASSERT(m_state != CELL_SEARCH);
  m_ctrlSinrForRlf = sinr;

  SpectrumValue mixedSinr = (m_rsReceivedPower * m_paLinear);
  if (m_dataInterferencePowerUpdated) {
    mixedSinr /= m_dataInterferencePower;
    m_dataInterferencePowerUpdated = false;
    NS_LOG_LOGIC("data interf measurement available, SINR = " << mixedSinr);
  } else {
    mixedSinr /= (*m_noisePsd);
    NS_LOG_LOGIC("no data interf measurement available, SINR = " << mixedSinr);
  }

  uint32_t rbgSize = GetRbgSize();
  uint32_t modulo = m_dlBandwidth % rbgSize;
  double avgMixedSinr = 0;
  uint32_t usedRbgNum = 0;
  for (uint32_t i = 0; i < (m_dlBandwidth - 1 - modulo); i++) {
    usedRbgNum++;
    avgMixedSinr += mixedSinr[i];
  }
  avgMixedSinr = avgMixedSinr / usedRbgNum;
  for (uint32_t i = 0; i < modulo; i++) {
    mixedSinr[m_dlBandwidth - 1 - i] = avgMixedSinr;
  }

  GenerateCqiRsrpRsrq(mixedSinr);
}

void LteUePhy::ReportInterference(const SpectrumValue &interf) {
  NS_LOG_FUNCTION(this << interf);
  m_rsInterferencePowerUpdated = true;
  m_rsInterferencePower = interf;
}

void LteUePhy::ReportDataInterference(const SpectrumValue &interf) {
  NS_LOG_FUNCTION(this << interf);

  m_dataInterferencePowerUpdated = true;
  m_dataInterferencePower = interf;
}

void LteUePhy::ReportRsReceivedPower(const SpectrumValue &power) {
  NS_LOG_FUNCTION(this << power);
  m_rsReceivedPowerUpdated = true;
  m_rsReceivedPower = power;

  if (m_enableUplinkPowerControl) {
    double sum = 0;
    for (auto it = m_rsReceivedPower.ConstValuesBegin();
         it != m_rsReceivedPower.ConstValuesEnd(); it++) {
      double powerTxW = ((*it) * 180000);
      sum += powerTxW;
    }
    double rsrp = 10 * log10(sum) + 30;

    NS_LOG_INFO("RSRP: " << rsrp);
    m_powerControl->SetRsrp(rsrp);
  }
}

Ptr<DlCqiLteControlMessage>
LteUePhy::CreateDlCqiFeedbackMessage(const SpectrumValue &sinr) {
  NS_LOG_FUNCTION(this);

  NS_ASSERT(m_transmissionMode < m_txModeGain.size());
  SpectrumValue newSinr = sinr;
  newSinr *= m_txModeGain.at(m_transmissionMode);

  Ptr<DlCqiLteControlMessage> msg = Create<DlCqiLteControlMessage>();
  CqiListElement_s dlcqi;
  std::vector<int> cqi;
  if (Simulator::Now() > m_p10CqiLast + m_p10CqiPeriodicity) {
    cqi = m_amc->CreateCqiFeedbacks(newSinr, m_dlBandwidth);

    auto nLayer = TransmissionModesLayers::TxMode2LayerNum(m_transmissionMode);
    auto nbSubChannels = cqi.size();
    double cqiSum = 0.0;
    int activeSubChannels = 0;
    for (std::size_t i = 0; i < nbSubChannels; i++) {
      if (cqi.at(i) != -1) {
        cqiSum += cqi.at(i);
        activeSubChannels++;
      }
      NS_LOG_DEBUG(this << " subch " << i << " cqi " << cqi.at(i));
    }
    dlcqi.m_rnti = m_rnti;
    dlcqi.m_ri = 1;
    dlcqi.m_cqiType = CqiListElement_s::P10;
    NS_ASSERT_MSG(nLayer > 0, " nLayer negative");
    NS_ASSERT_MSG(nLayer < 3, " nLayer limit is 2s");
    for (uint8_t i = 0; i < nLayer; i++) {
      if (activeSubChannels > 0) {
        dlcqi.m_wbCqi.push_back((uint16_t)cqiSum / activeSubChannels);
      } else {
        dlcqi.m_wbCqi.push_back(1);
      }
    }
    dlcqi.m_wbPmi = 0;
  } else if (Simulator::Now() > m_a30CqiLast + m_a30CqiPeriodicity) {
    cqi = m_amc->CreateCqiFeedbacks(newSinr, GetRbgSize());
    auto nLayer = TransmissionModesLayers::TxMode2LayerNum(m_transmissionMode);
    auto nbSubChannels = cqi.size();
    int rbgSize = GetRbgSize();
    double cqiSum = 0.0;
    int cqiNum = 0;
    SbMeasResult_s rbgMeas;
    for (std::size_t i = 0; i < nbSubChannels; i++) {
      if (cqi.at(i) != -1) {
        cqiSum += cqi.at(i);
      }
      cqiNum++;
      if (cqiNum == rbgSize) {
        HigherLayerSelected_s hlCqi;
        hlCqi.m_sbPmi = 0;
        for (uint8_t i = 0; i < nLayer; i++) {
          hlCqi.m_sbCqi.push_back((uint16_t)cqiSum / rbgSize);
        }
        rbgMeas.m_higherLayerSelected.push_back(hlCqi);
        cqiSum = 0.0;
        cqiNum = 0;
      }
    }
    dlcqi.m_rnti = m_rnti;
    dlcqi.m_ri = 1;
    dlcqi.m_cqiType = CqiListElement_s::A30;
    dlcqi.m_wbPmi = 0;
    dlcqi.m_sbMeasResult = rbgMeas;
  }

  msg->SetDlCqi(dlcqi);
  return msg;
}

void LteUePhy::ReportUeMeasurements() {
  NS_LOG_FUNCTION(this << Simulator::Now());
  NS_LOG_DEBUG(this << " Report UE Measurements ");

  LteUeCphySapUser::UeMeasurementsParameters ret;

  for (auto it = m_ueMeasurementsMap.begin(); it != m_ueMeasurementsMap.end();
       it++) {
    double avg_rsrp = (*it).second.rsrpSum / (double)(*it).second.rsrpNum;
    double avg_rsrq = (*it).second.rsrqSum / (double)(*it).second.rsrqNum;
    NS_LOG_DEBUG(this << " CellId " << (*it).first << " RSRP " << avg_rsrp
                      << " (nSamples " << (uint16_t)(*it).second.rsrpNum << ")"
                      << " RSRQ " << avg_rsrq << " (nSamples "
                      << (uint16_t)(*it).second.rsrqNum << ")"
                      << " ComponentCarrierID "
                      << (uint16_t)m_componentCarrierId);

    LteUeCphySapUser::UeMeasurementsElement newEl;
    newEl.m_cellId = (*it).first;
    newEl.m_rsrp = avg_rsrp;
    newEl.m_rsrq = avg_rsrq;
    ret.m_ueMeasurementsList.push_back(newEl);
    ret.m_componentCarrierId = m_componentCarrierId;

    m_reportUeMeasurements(m_rnti, (*it).first, avg_rsrp, avg_rsrq,
                           (*it).first == m_cellId, m_componentCarrierId);
  }

  m_ueCphySapUser->ReportUeMeasurements(ret);

  m_ueMeasurementsMap.clear();
  Simulator::Schedule(m_ueMeasurementsFilterPeriod,
                      &LteUePhy::ReportUeMeasurements, this);
}

void LteUePhy::SetDownlinkCqiPeriodicity(Time cqiPeriodicity) {
  NS_LOG_FUNCTION(this << cqiPeriodicity);
  m_a30CqiPeriodicity = cqiPeriodicity;
  m_p10CqiPeriodicity = cqiPeriodicity;
}

void LteUePhy::DoSendLteControlMessage(Ptr<LteControlMessage> msg) {
  NS_LOG_FUNCTION(this << msg);

  SetControlMessages(msg);
}

void LteUePhy::DoSendRachPreamble(uint32_t raPreambleId, uint32_t raRnti) {
  NS_LOG_FUNCTION(this << raPreambleId);

  Ptr<RachPreambleLteControlMessage> msg =
      Create<RachPreambleLteControlMessage>();
  msg->SetRapId(raPreambleId);
  m_raPreambleId = raPreambleId;
  m_raRnti = raRnti;
  m_controlMessagesQueue.at(0).emplace_back(msg);
}

void LteUePhy::DoNotifyConnectionSuccessful() {
  if (m_componentCarrierId == 0) {
    m_isConnected = true;
    InitializeRlfParams();
  }
}

void LteUePhy::ReceiveLteControlMessageList(
    std::list<Ptr<LteControlMessage>> msgList) {
  NS_LOG_FUNCTION(this);

  NS_LOG_DEBUG(this << " I am rnti = " << m_rnti << " and I received msgs "
                    << (uint16_t)msgList.size());
  for (auto it = msgList.begin(); it != msgList.end(); it++) {
    Ptr<LteControlMessage> msg = (*it);

    if (msg->GetMessageType() == LteControlMessage::DL_DCI) {
      Ptr<DlDciLteControlMessage> msg2 =
          DynamicCast<DlDciLteControlMessage>(msg);

      DlDciListElement_s dci = msg2->GetDci();
      if (dci.m_rnti != m_rnti) {
        continue;
      }

      if (dci.m_resAlloc != 0) {
        NS_FATAL_ERROR("Resource Allocation type not implemented");
      }

      std::vector<int> dlRb;

      uint32_t mask = 0x1;
      for (int i = 0; i < 32; i++) {
        if (((dci.m_rbBitmap & mask) >> i) == 1) {
          for (int k = 0; k < GetRbgSize(); k++) {
            dlRb.push_back((i * GetRbgSize()) + k);
          }
        }
        mask = (mask << 1);
      }
      if (m_enableUplinkPowerControl) {
        m_powerControl->ReportTpc(dci.m_tpc);
      }

      NS_LOG_DEBUG(this << " UE " << m_rnti << " DL-DCI " << dci.m_rnti
                        << " bitmap " << dci.m_rbBitmap);
      for (std::size_t i = 0; i < dci.m_tbsSize.size(); i++) {
        m_downlinkSpectrumPhy->AddExpectedTb(
            dci.m_rnti, dci.m_ndi.at(i), dci.m_tbsSize.at(i), dci.m_mcs.at(i),
            dlRb, i, dci.m_harqProcess, dci.m_rv.at(i), true);
      }

      SetSubChannelsForReception(dlRb);
    } else if (msg->GetMessageType() == LteControlMessage::UL_DCI) {
      Ptr<UlDciLteControlMessage> msg2 =
          DynamicCast<UlDciLteControlMessage>(msg);
      UlDciListElement_s dci = msg2->GetDci();
      if (dci.m_rnti != m_rnti) {
        continue;
      }
      NS_LOG_INFO(this << " UL DCI");
      std::vector<int> ulRb;
      ulRb.reserve(dci.m_rbLen);
      for (int i = 0; i < dci.m_rbLen; i++) {
        ulRb.push_back(i + dci.m_rbStart);
      }
      m_reportUlPhyResourceBlocks(m_rnti, ulRb);
      QueueSubChannelsForTransmission(ulRb);
      HarqProcessInfoList_t harqInfoList =
          m_harqPhyModule->GetHarqProcessInfoUl(m_rnti, 0);
      PhyTransmissionStatParameters params;
      params.m_cellId = m_cellId;
      params.m_imsi = 0;
      params.m_timestamp =
          Simulator::Now().GetMilliSeconds() + UL_PUSCH_TTIS_DELAY;
      params.m_rnti = m_rnti;
      params.m_txMode = 0;
      params.m_layer = 0;
      params.m_mcs = dci.m_mcs;
      params.m_size = dci.m_tbSize;
      params.m_rv = harqInfoList.size();
      params.m_ndi = dci.m_ndi;
      params.m_ccId = m_componentCarrierId;
      m_ulPhyTransmission(params);
      m_uePhySapUser->ReceiveLteControlMessage(msg);
    } else if (msg->GetMessageType() == LteControlMessage::RAR) {
      Ptr<RarLteControlMessage> rarMsg = DynamicCast<RarLteControlMessage>(msg);
      if (rarMsg->GetRaRnti() == m_raRnti) {
        for (auto it = rarMsg->RarListBegin(); it != rarMsg->RarListEnd();
             ++it) {
          if (it->rapId != m_raPreambleId) {
            continue;
          } else {
            NS_LOG_INFO("received RAR RNTI " << m_raRnti);
            std::vector<int> ulRb;
            ulRb.reserve(it->rarPayload.m_grant.m_rbLen);
            for (int i = 0; i < it->rarPayload.m_grant.m_rbLen; i++) {
              ulRb.push_back(i + it->rarPayload.m_grant.m_rbStart);
            }

            QueueSubChannelsForTransmission(ulRb);
            m_uePhySapUser->ReceiveLteControlMessage(msg);
            m_raPreambleId = 255;
            m_raRnti = 11;
          }
        }
      }
    } else if (msg->GetMessageType() == LteControlMessage::MIB) {
      NS_LOG_INFO("received MIB");
      NS_ASSERT(m_cellId > 0);
      Ptr<MibLteControlMessage> msg2 = DynamicCast<MibLteControlMessage>(msg);
      m_ueCphySapUser->RecvMasterInformationBlock(m_cellId, msg2->GetMib());
    } else if (msg->GetMessageType() == LteControlMessage::SIB1) {
      NS_LOG_INFO("received SIB1");
      NS_ASSERT(m_cellId > 0);
      Ptr<Sib1LteControlMessage> msg2 = DynamicCast<Sib1LteControlMessage>(msg);
      m_ueCphySapUser->RecvSystemInformationBlockType1(m_cellId,
                                                       msg2->GetSib1());
    } else {
      m_uePhySapUser->ReceiveLteControlMessage(msg);
    }
  }
}

void LteUePhy::ReceivePss(uint16_t cellId, Ptr<SpectrumValue> p) {
  NS_LOG_FUNCTION(this << cellId << (*p));

  double sum = 0.0;
  uint16_t nRB = 0;
  for (auto itPi = p->ConstValuesBegin(); itPi != p->ConstValuesEnd(); itPi++) {
    double powerTxW = ((*itPi) * 180000.0) / 12.0;
    sum += powerTxW;
    nRB++;
  }

  double rsrp_dBm = 10 * log10(1000 * (sum / (double)nRB));
  NS_LOG_INFO(this << " PSS RNTI " << m_rnti << " cellId " << m_cellId
                   << " has RSRP " << rsrp_dBm << " and RBnum " << nRB);

  auto itMeasMap = m_ueMeasurementsMap.find(cellId);
  if (itMeasMap == m_ueMeasurementsMap.end()) {
    UeMeasurementsElement newEl;
    newEl.rsrpSum = rsrp_dBm;
    newEl.rsrpNum = 1;
    newEl.rsrqSum = 0;
    newEl.rsrqNum = 0;
    m_ueMeasurementsMap.insert(
        std::pair<uint16_t, UeMeasurementsElement>(cellId, newEl));
  } else {
    (*itMeasMap).second.rsrpSum += rsrp_dBm;
    (*itMeasMap).second.rsrpNum++;
  }

  m_pssReceived = true;
  PssElement el;
  el.cellId = cellId;
  el.pssPsdSum = sum;
  el.nRB = nRB;
  m_pssList.push_back(el);
}

void LteUePhy::QueueSubChannelsForTransmission(std::vector<int> rbMap) {
  m_subChannelsForTransmissionQueue.at(m_macChTtiDelay - 1) = rbMap;
}

void LteUePhy::SubframeIndication(uint32_t frameNo, uint32_t subframeNo) {
  NS_LOG_FUNCTION(this << frameNo << subframeNo);

  NS_ASSERT_MSG(frameNo > 0,
                "the SRS index check code assumes that frameNo starts at 1");

  m_rsReceivedPowerUpdated = false;
  m_rsInterferencePowerUpdated = false;
  m_pssReceived = false;

  if (m_ulConfigured) {
    std::vector<int> rbMask = m_subChannelsForTransmissionQueue.at(0);
    SetSubChannelsForTransmission(m_subChannelsForTransmissionQueue.at(0));

    for (uint8_t i = 1; i < m_macChTtiDelay; i++) {
      m_subChannelsForTransmissionQueue.at(i - 1) =
          m_subChannelsForTransmissionQueue.at(i);
    }
    m_subChannelsForTransmissionQueue.at(m_macChTtiDelay - 1).clear();

    if (m_srsConfigured && (m_srsStartTime <= Simulator::Now())) {
      NS_ASSERT_MSG(
          subframeNo > 0 && subframeNo <= 10,
          "the SRS index check code assumes that subframeNo starts at 1");
      if ((((frameNo - 1) * 10 + (subframeNo - 1)) % m_srsPeriodicity) ==
          m_srsSubframeOffset) {
        NS_LOG_INFO("frame " << frameNo << " subframe " << subframeNo
                             << " sending SRS (offset=" << m_srsSubframeOffset
                             << ", period=" << m_srsPeriodicity << ")");
        m_sendSrsEvent = Simulator::Schedule(UL_SRS_DELAY_FROM_SUBFRAME_START,
                                             &LteUePhy::SendSrs, this);
      }
    }

    std::list<Ptr<LteControlMessage>> ctrlMsg = GetControlMessages();
    NS_LOG_LOGIC(this << " UE - start slot for PUSCH + PUCCH - RNTI " << m_rnti
                      << " CELLID " << m_cellId);
    Ptr<PacketBurst> pb = GetPacketBurst();
    if (pb) {
      if (m_enableUplinkPowerControl) {
        m_txPower = m_powerControl->GetPuschTxPower(rbMask);
        SetSubChannelsForTransmission(rbMask);
      }
      m_uplinkSpectrumPhy->StartTxDataFrame(pb, ctrlMsg, UL_DATA_DURATION);
    } else {
      if (!ctrlMsg.empty()) {
        NS_LOG_LOGIC(this << " UE - start TX PUCCH (NO PUSCH)");
        std::vector<int> dlRb;

        if (m_enableUplinkPowerControl) {
          m_txPower = m_powerControl->GetPucchTxPower(dlRb);
        }

        SetSubChannelsForTransmission(dlRb);
        m_uplinkSpectrumPhy->StartTxDataFrame(pb, ctrlMsg, UL_DATA_DURATION);
      } else {
        NS_LOG_LOGIC(this << " UE - UL NOTHING TO SEND");
      }
    }
  }

  m_uePhySapUser->SubframeIndication(frameNo, subframeNo);

  m_subframeNo = subframeNo;
  ++subframeNo;
  if (subframeNo > 10) {
    ++frameNo;
    subframeNo = 1;
  }

  Simulator::Schedule(Seconds(GetTti()), &LteUePhy::SubframeIndication, this,
                      frameNo, subframeNo);
}

void LteUePhy::SendSrs() {
  NS_LOG_FUNCTION(this << " UE " << m_rnti << " start tx SRS, cell Id "
                       << (uint32_t)m_cellId);
  NS_ASSERT(m_cellId > 0);
  std::vector<int> dlRb;
  for (uint16_t i = 0; i < m_ulBandwidth; i++) {
    dlRb.push_back(i);
  }

  if (m_enableUplinkPowerControl) {
    m_txPower = m_powerControl->GetSrsTxPower(dlRb);
  }

  SetSubChannelsForTransmission(dlRb);
  m_uplinkSpectrumPhy->StartTxUlSrsFrame();
}

void LteUePhy::DoReset() {
  NS_LOG_FUNCTION(this);

  m_rnti = 0;
  m_cellId = 0;
  m_isConnected = false;
  m_transmissionMode = 0;
  m_srsPeriodicity = 0;
  m_srsConfigured = false;
  m_dlConfigured = false;
  m_ulConfigured = false;
  m_raPreambleId = 255;
  m_raRnti = 11;
  m_rsrpSinrSampleCounter = 0;
  m_p10CqiLast = Simulator::Now();
  m_a30CqiLast = Simulator::Now();
  m_paLinear = 1;

  m_rsReceivedPowerUpdated = false;
  m_rsInterferencePowerUpdated = false;
  m_dataInterferencePowerUpdated = false;

  m_packetBurstQueue.clear();
  m_controlMessagesQueue.clear();
  m_subChannelsForTransmissionQueue.clear();
  for (int i = 0; i < m_macChTtiDelay; i++) {
    Ptr<PacketBurst> pb = CreateObject<PacketBurst>();
    m_packetBurstQueue.push_back(pb);
    std::list<Ptr<LteControlMessage>> l;
    m_controlMessagesQueue.push_back(l);
  }
  std::vector<int> ulRb;
  m_subChannelsForTransmissionQueue.resize(m_macChTtiDelay, ulRb);

  m_sendSrsEvent.Cancel();
  m_downlinkSpectrumPhy->Reset();
  m_uplinkSpectrumPhy->Reset();
  m_pssList.clear();
  m_downlinkSpectrumPhy->m_interferenceCtrl->EndRx();
  m_downlinkSpectrumPhy->m_interferenceData->EndRx();
}

void LteUePhy::DoStartCellSearch(uint32_t dlEarfcn) {
  NS_LOG_FUNCTION(this << dlEarfcn);
  m_dlEarfcn = dlEarfcn;
  DoSetDlBandwidth(6);
  SwitchToState(CELL_SEARCH);
}

void LteUePhy::DoSynchronizeWithEnb(uint16_t cellId, uint32_t dlEarfcn) {
  NS_LOG_FUNCTION(this << cellId << dlEarfcn);
  m_dlEarfcn = dlEarfcn;
  DoSynchronizeWithEnb(cellId);
}

void LteUePhy::DoSynchronizeWithEnb(uint16_t cellId) {
  NS_LOG_FUNCTION(this << cellId);

  if (cellId == 0) {
    NS_FATAL_ERROR("Cell ID shall not be zero");
  }

  m_cellId = cellId;
  m_downlinkSpectrumPhy->SetCellId(cellId);
  m_uplinkSpectrumPhy->SetCellId(cellId);

  DoSetDlBandwidth(6);

  m_dlConfigured = false;
  m_ulConfigured = false;

  SwitchToState(SYNCHRONIZED);
}

uint16_t LteUePhy::DoGetCellId() { return m_cellId; }

uint32_t LteUePhy::DoGetDlEarfcn() { return m_dlEarfcn; }

void LteUePhy::DoSetDlBandwidth(uint16_t dlBandwidth) {
  NS_LOG_FUNCTION(this << (uint32_t)dlBandwidth);
  if (m_dlBandwidth != dlBandwidth or !m_dlConfigured) {
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

    m_noisePsd = LteSpectrumValueHelper::CreateNoisePowerSpectralDensity(
        m_dlEarfcn, m_dlBandwidth, m_noiseFigure);
    m_downlinkSpectrumPhy->SetNoisePowerSpectralDensity(m_noisePsd);
    m_downlinkSpectrumPhy->GetChannel()->AddRx(m_downlinkSpectrumPhy);
  }
  m_dlConfigured = true;
}

void LteUePhy::DoConfigureUplink(uint32_t ulEarfcn, uint16_t ulBandwidth) {
  m_ulEarfcn = ulEarfcn;
  m_ulBandwidth = ulBandwidth;
  m_ulConfigured = true;
}

void LteUePhy::DoConfigureReferenceSignalPower(int8_t referenceSignalPower) {
  NS_LOG_FUNCTION(this);
  m_powerControl->ConfigureReferenceSignalPower(referenceSignalPower);
}

void LteUePhy::DoSetRnti(uint16_t rnti) {
  NS_LOG_FUNCTION(this << rnti);
  m_rnti = rnti;

  m_powerControl->SetCellId(m_cellId);
  m_powerControl->SetRnti(m_rnti);
}

void LteUePhy::DoSetTransmissionMode(uint8_t txMode) {
  NS_LOG_FUNCTION(this << (uint16_t)txMode);
  m_transmissionMode = txMode;
  m_downlinkSpectrumPhy->SetTransmissionMode(txMode);
}

void LteUePhy::DoSetSrsConfigurationIndex(uint16_t srcCi) {
  NS_LOG_FUNCTION(this << srcCi);
  m_srsPeriodicity = GetSrsPeriodicity(srcCi);
  m_srsSubframeOffset = GetSrsSubframeOffset(srcCi);
  m_srsConfigured = true;

  m_srsStartTime = Simulator::Now() + MilliSeconds(0);
  NS_LOG_DEBUG(this << " UE SRS P " << m_srsPeriodicity << " RNTI " << m_rnti
                    << " offset " << m_srsSubframeOffset << " cellId "
                    << m_cellId << " CI " << srcCi);
}

void LteUePhy::DoSetPa(double pa) {
  NS_LOG_FUNCTION(this << pa);
  m_paLinear = pow(10, (pa / 10));
}

void LteUePhy::DoSetRsrpFilterCoefficient(uint8_t rsrpFilterCoefficient) {
  NS_LOG_FUNCTION(this << (uint16_t)(rsrpFilterCoefficient));
  m_powerControl->SetRsrpFilterCoefficient(rsrpFilterCoefficient);
}

void LteUePhy::DoResetPhyAfterRlf() {
  NS_LOG_FUNCTION(this);
  m_downlinkSpectrumPhy->m_harqPhyModule->ClearDlHarqBuffer(m_rnti);
  m_dataInterferencePowerUpdated = false;
  m_rsInterferencePowerUpdated = false;
  m_pssReceived = false;
  DoReset();
}

void LteUePhy::DoResetRlfParams() {
  NS_LOG_FUNCTION(this);

  InitializeRlfParams();
}

void LteUePhy::DoStartInSyncDetection() {
  NS_LOG_FUNCTION(this);
  m_downlinkInSync = false;
}

void LteUePhy::DoSetImsi(uint64_t imsi) {
  NS_LOG_FUNCTION(this);
  m_imsi = imsi;
}

void LteUePhy::InitializeRlfParams() {
  NS_LOG_FUNCTION(this);
  m_numOfSubframes = 0;
  m_sinrDbFrame = 0;
  m_numOfFrames = 0;
  m_downlinkInSync = true;
}

void LteUePhy::RlfDetection(double sinrDb) {
  NS_LOG_FUNCTION(this << sinrDb);
  m_sinrDbFrame += sinrDb;
  m_numOfSubframes++;
  NS_LOG_LOGIC("No of Subframes: " << m_numOfSubframes
                                   << " UE synchronized: " << m_downlinkInSync);
  if (m_downlinkInSync && m_numOfSubframes == 10) {
    if ((m_sinrDbFrame / m_numOfSubframes) < m_qOut) {
      m_numOfFrames++;
      NS_LOG_LOGIC("No of Frames which cannot be decoded: " << m_numOfFrames);
    } else {
      NS_LOG_INFO(
          "Resetting frame counter at phy. Current value = " << m_numOfFrames);
      m_numOfFrames = 0;
      m_ueCphySapUser->ResetSyncIndicationCounter();
    }
    m_numOfSubframes = 0;
    m_sinrDbFrame = 0;
  }
  if (m_downlinkInSync && (m_numOfFrames * 10) == m_numOfQoutEvalSf) {
    NS_LOG_LOGIC(
        "At " << Simulator::Now().As(Time::MS)
              << " ms UE PHY sending out of sync indication to UE RRC layer");
    m_ueCphySapUser->NotifyOutOfSync();
    m_numOfFrames = 0;
  }
  if (!m_downlinkInSync && m_numOfSubframes == 10) {
    if ((m_sinrDbFrame / m_numOfSubframes) > m_qIn) {
      m_numOfFrames++;
      NS_LOG_LOGIC("No of Frames successfully decoded: " << m_numOfFrames);
    } else {
      m_numOfFrames = 0;
      m_ueCphySapUser->ResetSyncIndicationCounter();
    }
    m_numOfSubframes = 0;
    m_sinrDbFrame = 0;
  }
  if (!m_downlinkInSync && (m_numOfFrames * 10) == m_numOfQinEvalSf) {
    NS_LOG_LOGIC(
        "At " << Simulator::Now().As(Time::MS)
              << " ms UE PHY sending in sync indication to UE RRC layer");
    m_ueCphySapUser->NotifyInSync();
    m_numOfFrames = 0;
  }
}

void LteUePhy::SetTxMode1Gain(double gain) { SetTxModeGain(1, gain); }

void LteUePhy::SetTxMode2Gain(double gain) { SetTxModeGain(2, gain); }

void LteUePhy::SetTxMode3Gain(double gain) { SetTxModeGain(3, gain); }

void LteUePhy::SetTxMode4Gain(double gain) { SetTxModeGain(4, gain); }

void LteUePhy::SetTxMode5Gain(double gain) { SetTxModeGain(5, gain); }

void LteUePhy::SetTxMode6Gain(double gain) { SetTxModeGain(6, gain); }

void LteUePhy::SetTxMode7Gain(double gain) { SetTxModeGain(7, gain); }

void LteUePhy::SetTxModeGain(uint8_t txMode, double gain) {
  NS_LOG_FUNCTION(this << gain);
  if (txMode > 0) {
    double gainLin = std::pow(10.0, (gain / 10.0));
    if (m_txModeGain.size() < txMode) {
      m_txModeGain.resize(txMode);
    }
    m_txModeGain.at(txMode - 1) = gainLin;
  }
  m_downlinkSpectrumPhy->SetTxModeGain(txMode, gain);
}

void LteUePhy::EnqueueDlHarqFeedback(DlInfoListElement_s m) {
  NS_LOG_FUNCTION(this);
  Ptr<DlHarqFeedbackLteControlMessage> msg =
      Create<DlHarqFeedbackLteControlMessage>();
  msg->SetDlHarqFeedback(m);
  SetControlMessages(msg);
}

void LteUePhy::SetHarqPhyModule(Ptr<LteHarqPhy> harq) {
  m_harqPhyModule = harq;
}

LteUePhy::State LteUePhy::GetState() const {
  NS_LOG_FUNCTION(this);
  return m_state;
}

void LteUePhy::SwitchToState(State newState) {
  NS_LOG_FUNCTION(this << newState);
  State oldState = m_state;
  m_state = newState;
  NS_LOG_INFO(this << " cellId=" << m_cellId << " rnti=" << m_rnti << " UePhy "
                   << ToString(oldState) << " --> " << ToString(newState));
  m_stateTransitionTrace(m_cellId, m_rnti, oldState, newState);
}

} // namespace ns3
