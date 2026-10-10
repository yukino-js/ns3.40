
#include "dsss-phy.h"

#include "dsss-ppdu.h"

#include "ns3/interference-helper.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/wifi-phy.h"
#include "ns3/wifi-psdu.h"
#include "ns3/wifi-utils.h"

#include <array>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("DsssPhy");

// clang-format off

const PhyEntity::PpduFormats DsssPhy::m_dsssPpduFormats {
    { WIFI_PREAMBLE_LONG,  { WIFI_PPDU_FIELD_PREAMBLE,
                             WIFI_PPDU_FIELD_NON_HT_HEADER,
                             WIFI_PPDU_FIELD_DATA } },
    { WIFI_PREAMBLE_SHORT, { WIFI_PPDU_FIELD_PREAMBLE,
                             WIFI_PPDU_FIELD_NON_HT_HEADER,
                             WIFI_PPDU_FIELD_DATA } },
};

const PhyEntity::ModulationLookupTable DsssPhy::m_dsssModulationLookupTable {
  { "DsssRate1Mbps",   { WIFI_CODE_RATE_UNDEFINED, 2 } },
  { "DsssRate2Mbps",   { WIFI_CODE_RATE_UNDEFINED, 4 } },
  { "DsssRate5_5Mbps", { WIFI_CODE_RATE_UNDEFINED, 16 } },
  { "DsssRate11Mbps",  { WIFI_CODE_RATE_UNDEFINED, 256 } },
};

// clang-format on

static const std::array<uint64_t, 4> s_dsssRatesBpsList = {1000000, 2000000,
                                                           5500000, 11000000};

const std::array<uint64_t, 4> &GetDsssRatesBpsList() {
  return s_dsssRatesBpsList;
};

DsssPhy::DsssPhy() {
  NS_LOG_FUNCTION(this);
  for (const auto &rate : GetDsssRatesBpsList()) {
    WifiMode mode = GetDsssRate(rate);
    NS_LOG_LOGIC("Add " << mode << " to list");
    m_modeList.emplace_back(mode);
  }
}

DsssPhy::~DsssPhy() { NS_LOG_FUNCTION(this); }

WifiMode DsssPhy::GetSigMode(WifiPpduField field,
                             const WifiTxVector &txVector) const {
  switch (field) {
  case WIFI_PPDU_FIELD_PREAMBLE:
  case WIFI_PPDU_FIELD_NON_HT_HEADER:
    return GetHeaderMode(txVector);
  default:
    return PhyEntity::GetSigMode(field, txVector);
  }
}

WifiMode DsssPhy::GetHeaderMode(const WifiTxVector &txVector) const {
  if (txVector.GetPreambleType() == WIFI_PREAMBLE_LONG ||
      txVector.GetMode() == GetDsssRate1Mbps()) {
    return GetDsssRate1Mbps();
  } else {
    return GetDsssRate2Mbps();
  }
}

const PhyEntity::PpduFormats &DsssPhy::GetPpduFormats() const {
  return m_dsssPpduFormats;
}

Time DsssPhy::GetDuration(WifiPpduField field,
                          const WifiTxVector &txVector) const {
  if (field == WIFI_PPDU_FIELD_PREAMBLE) {
    return GetPreambleDuration(txVector);
  } else if (field == WIFI_PPDU_FIELD_NON_HT_HEADER) {
    return GetHeaderDuration(txVector);
  } else {
    return PhyEntity::GetDuration(field, txVector);
  }
}

Time DsssPhy::GetPreambleDuration(const WifiTxVector &txVector) const {
  if (txVector.GetPreambleType() == WIFI_PREAMBLE_SHORT &&
      (txVector.GetMode().GetDataRate(22) > 1000000)) {
    return MicroSeconds(72);
  } else {
    return MicroSeconds(144);
  }
}

Time DsssPhy::GetHeaderDuration(const WifiTxVector &txVector) const {
  if (txVector.GetPreambleType() == WIFI_PREAMBLE_SHORT &&
      (txVector.GetMode().GetDataRate(22) > 1000000)) {
    return MicroSeconds(24);
  } else {
    return MicroSeconds(48);
  }
}

Time DsssPhy::GetPayloadDuration(uint32_t size, const WifiTxVector &txVector,
                                 WifiPhyBand, MpduType, bool, uint32_t &,
                                 double &, uint16_t) const {
  return MicroSeconds(
      lrint(ceil((size * 8.0) / (txVector.GetMode().GetDataRate(22) / 1.0e6))));
}

Ptr<WifiPpdu> DsssPhy::BuildPpdu(const WifiConstPsduMap &psdus,
                                 const WifiTxVector &txVector,
                                 Time ppduDuration) {
  NS_LOG_FUNCTION(this << psdus << txVector << ppduDuration);
  return Create<DsssPpdu>(psdus.begin()->second, txVector,
                          m_wifiPhy->GetOperatingChannel(), ppduDuration,
                          ObtainNextUid(txVector));
}

PhyEntity::PhyFieldRxStatus DsssPhy::DoEndReceiveField(WifiPpduField field,
                                                       Ptr<Event> event) {
  NS_LOG_FUNCTION(this << field << *event);
  if (field == WIFI_PPDU_FIELD_NON_HT_HEADER) {
    return EndReceiveHeader(event);
  }
  return PhyEntity::DoEndReceiveField(field, event);
}

PhyEntity::PhyFieldRxStatus DsssPhy::EndReceiveHeader(Ptr<Event> event) {
  NS_LOG_FUNCTION(this << *event);
  SnrPer snrPer = GetPhyHeaderSnrPer(WIFI_PPDU_FIELD_NON_HT_HEADER, event);
  NS_LOG_DEBUG("Long/Short PHY header: SNR(dB)=" << RatioToDb(snrPer.snr)
                                                 << ", PER=" << snrPer.per);
  PhyFieldRxStatus status(GetRandomValue() > snrPer.per);
  if (status.isSuccess) {
    NS_LOG_DEBUG("Received long/short PHY header");
    if (!IsConfigSupported(event->GetPpdu())) {
      status = PhyFieldRxStatus(false, UNSUPPORTED_SETTINGS, DROP);
    }
  } else {
    NS_LOG_DEBUG(
        "Abort reception because long/short PHY header reception failed");
    status.reason = L_SIG_FAILURE;
    status.actionIfFailure = ABORT;
  }
  return status;
}

uint16_t DsssPhy::GetRxChannelWidth(const WifiTxVector &txVector) const {
  if (m_wifiPhy->GetChannelWidth() > 20) {
    return 20;
  }
  return PhyEntity::GetRxChannelWidth(txVector);
}

uint16_t
DsssPhy::GetMeasurementChannelWidth(const Ptr<const WifiPpdu> ppdu) const {
  return ppdu ? GetRxChannelWidth(ppdu->GetTxVector()) : 22;
}

Ptr<SpectrumValue>
DsssPhy::GetTxPowerSpectralDensity(double txPowerW,
                                   Ptr<const WifiPpdu> ppdu) const {
  const auto &txVector = ppdu->GetTxVector();
  uint16_t centerFrequency = GetCenterFrequencyForChannelWidth(txVector);
  uint16_t channelWidth = txVector.GetChannelWidth();
  NS_LOG_FUNCTION(this << centerFrequency << channelWidth << txPowerW);
  NS_ABORT_MSG_IF(channelWidth != 22, "Invalid channel width for DSSS");
  Ptr<SpectrumValue> v =
      WifiSpectrumValueHelper::CreateDsssTxPowerSpectralDensity(
          centerFrequency, txPowerW, GetGuardBandwidth(channelWidth));
  return v;
}

void DsssPhy::InitializeModes() {
  for (const auto &rate : GetDsssRatesBpsList()) {
    GetDsssRate(rate);
  }
}

WifiMode DsssPhy::GetDsssRate(uint64_t rate) {
  switch (rate) {
  case 1000000:
    return GetDsssRate1Mbps();
  case 2000000:
    return GetDsssRate2Mbps();
  case 5500000:
    return GetDsssRate5_5Mbps();
  case 11000000:
    return GetDsssRate11Mbps();
  default:
    NS_ABORT_MSG("Inexistent rate (" << rate << " bps) requested for HR/DSSS");
    return WifiMode();
  }
}

#define GET_DSSS_MODE(x, m)                                                    \
  WifiMode DsssPhy::Get##x() {                                                 \
    static WifiMode mode = CreateDsssMode(#x, WIFI_MOD_CLASS_##m);             \
    return mode;                                                               \
  };

GET_DSSS_MODE(DsssRate1Mbps, DSSS)
GET_DSSS_MODE(DsssRate2Mbps, DSSS)
GET_DSSS_MODE(DsssRate5_5Mbps, HR_DSSS)
GET_DSSS_MODE(DsssRate11Mbps, HR_DSSS)
#undef GET_DSSS_MODE

WifiMode DsssPhy::CreateDsssMode(std::string uniqueName,
                                 WifiModulationClass modClass) {
  const auto it = m_dsssModulationLookupTable.find(uniqueName);
  NS_ASSERT_MSG(it != m_dsssModulationLookupTable.end(),
                "DSSS or HR/DSSS mode cannot be created because it is not in "
                "the lookup table!");
  NS_ASSERT_MSG(modClass == WIFI_MOD_CLASS_DSSS ||
                    modClass == WIFI_MOD_CLASS_HR_DSSS,
                "DSSS or HR/DSSS mode must be either WIFI_MOD_CLASS_DSSS or "
                "WIFI_MOD_CLASS_HR_DSSS!");

  return WifiModeFactory::CreateWifiMode(
      uniqueName, modClass, true, MakeBoundCallback(&GetCodeRate, uniqueName),
      MakeBoundCallback(&GetConstellationSize, uniqueName),
      MakeCallback(&GetDataRateFromTxVector),
      MakeCallback(&GetDataRateFromTxVector), MakeCallback(&IsAllowed));
}

WifiCodeRate DsssPhy::GetCodeRate(const std::string &name) {
  return m_dsssModulationLookupTable.at(name).first;
}

uint16_t DsssPhy::GetConstellationSize(const std::string &name) {
  return m_dsssModulationLookupTable.at(name).second;
}

uint64_t DsssPhy::GetDataRateFromTxVector(const WifiTxVector &txVector,
                                          uint16_t) {
  WifiMode mode = txVector.GetMode();
  return DsssPhy::GetDataRate(mode.GetUniqueName(), mode.GetModulationClass());
}

uint64_t DsssPhy::GetDataRate(const std::string &name,
                              WifiModulationClass modClass) {
  uint16_t constellationSize = GetConstellationSize(name);
  uint16_t divisor = 0;
  if (modClass == WIFI_MOD_CLASS_DSSS) {
    divisor = 11;
  } else if (modClass == WIFI_MOD_CLASS_HR_DSSS) {
    divisor = 8;
  } else {
    NS_FATAL_ERROR("Incorrect modulation class, must specify either "
                   "WIFI_MOD_CLASS_DSSS or "
                   "WIFI_MOD_CLASS_HR_DSSS!");
  }
  auto numberOfBitsPerSubcarrier =
      static_cast<uint16_t>(log2(constellationSize));
  uint64_t dataRate = ((11000000 / divisor) * numberOfBitsPerSubcarrier);
  return dataRate;
}

bool DsssPhy::IsAllowed(const WifiTxVector &) { return true; }

uint32_t DsssPhy::GetMaxPsduSize() const { return 4095; }

} // namespace ns3

namespace {

class ConstructorDsss {
public:
  ConstructorDsss() {
    ns3::DsssPhy::InitializeModes();
    ns3::Ptr<ns3::DsssPhy> phyEntity = ns3::Create<ns3::DsssPhy>();
    ns3::WifiPhy::AddStaticPhyEntity(ns3::WIFI_MOD_CLASS_HR_DSSS, phyEntity);
    ns3::WifiPhy::AddStaticPhyEntity(ns3::WIFI_MOD_CLASS_DSSS, phyEntity);
  }
} g_constructor_dsss;

} // namespace
