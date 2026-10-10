
#include "ht-phy.h"

#include "ht-ppdu.h"

#include "ns3/assert.h"
#include "ns3/interference-helper.h"
#include "ns3/log.h"
#include "ns3/wifi-phy.h"
#include "ns3/wifi-psdu.h"
#include "ns3/wifi-utils.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("HtPhy");

// clang-format off

const PhyEntity::PpduFormats HtPhy::m_htPpduFormats {
    { WIFI_PREAMBLE_HT_MF, { WIFI_PPDU_FIELD_PREAMBLE,
                             WIFI_PPDU_FIELD_NON_HT_HEADER,
                             WIFI_PPDU_FIELD_HT_SIG,
                             WIFI_PPDU_FIELD_TRAINING,
                             WIFI_PPDU_FIELD_DATA } }
};

// clang-format on

HtPhy::HtPhy(uint8_t maxNss, bool buildModeList)
    : OfdmPhy(OFDM_PHY_DEFAULT, false) {
  NS_LOG_FUNCTION(this << +maxNss << buildModeList);
  m_maxSupportedNss = maxNss;
  m_bssMembershipSelector = HT_PHY;
  m_maxMcsIndexPerSs = 7;
  m_maxSupportedMcsIndexPerSs = m_maxMcsIndexPerSs;
  if (buildModeList) {
    NS_ABORT_MSG_IF(maxNss == 0 || maxNss > HT_MAX_NSS,
                    "Unsupported max Nss " << +maxNss << " for HT PHY");
    BuildModeList();
  }
}

HtPhy::~HtPhy() { NS_LOG_FUNCTION(this); }

void HtPhy::BuildModeList() {
  NS_LOG_FUNCTION(this);
  NS_ASSERT(m_modeList.empty());
  NS_ASSERT(m_bssMembershipSelector == HT_PHY);

  uint8_t index = 0;
  for (uint8_t nss = 1; nss <= m_maxSupportedNss; ++nss) {
    for (uint8_t i = 0; i <= m_maxSupportedMcsIndexPerSs; ++i) {
      NS_LOG_LOGIC("Add HtMcs" << +index << " to list");
      m_modeList.emplace_back(CreateHtMcs(index));
      ++index;
    }
    index = 8 * nss;
  }
}

WifiMode HtPhy::GetMcs(uint8_t index) const {
  for (const auto &mcs : m_modeList) {
    if (mcs.GetMcsValue() == index) {
      return mcs;
    }
  }

  NS_ABORT_MSG("Unsupported MCS index " << +index << " for this PHY entity");
  return WifiMode();
}

bool HtPhy::IsMcsSupported(uint8_t index) const {
  for (const auto &mcs : m_modeList) {
    if (mcs.GetMcsValue() == index) {
      return true;
    }
  }
  return false;
}

bool HtPhy::HandlesMcsModes() const { return true; }

const PhyEntity::PpduFormats &HtPhy::GetPpduFormats() const {
  return m_htPpduFormats;
}

WifiMode HtPhy::GetSigMode(WifiPpduField field,
                           const WifiTxVector &txVector) const {
  switch (field) {
  case WIFI_PPDU_FIELD_PREAMBLE:
  case WIFI_PPDU_FIELD_NON_HT_HEADER:
    return GetLSigMode();
  case WIFI_PPDU_FIELD_TRAINING:
  case WIFI_PPDU_FIELD_HT_SIG:
    return GetHtSigMode();
  default:
    return OfdmPhy::GetSigMode(field, txVector);
  }
}

WifiMode HtPhy::GetLSigMode() { return GetOfdmRate6Mbps(); }

WifiMode HtPhy::GetHtSigMode() const { return GetLSigMode(); }

uint8_t HtPhy::GetBssMembershipSelector() const {
  return m_bssMembershipSelector;
}

void HtPhy::SetMaxSupportedMcsIndexPerSs(uint8_t maxIndex) {
  NS_LOG_FUNCTION(this << +maxIndex);
  NS_ABORT_MSG_IF(maxIndex > m_maxMcsIndexPerSs,
                  "Provided max MCS index "
                      << +maxIndex
                      << " per SS greater than max standard-defined value "
                      << +m_maxMcsIndexPerSs);
  if (maxIndex != m_maxSupportedMcsIndexPerSs) {
    NS_LOG_LOGIC(
        "Rebuild mode list since max MCS index per spatial stream has changed");
    m_maxSupportedMcsIndexPerSs = maxIndex;
    m_modeList.clear();
    BuildModeList();
  }
}

uint8_t HtPhy::GetMaxSupportedMcsIndexPerSs() const {
  return m_maxSupportedMcsIndexPerSs;
}

void HtPhy::SetMaxSupportedNss(uint8_t maxNss) {
  NS_LOG_FUNCTION(this << +maxNss);
  NS_ASSERT(m_bssMembershipSelector == HT_PHY);
  maxNss = std::min(HT_MAX_NSS, maxNss);
  if (maxNss != m_maxSupportedNss) {
    NS_LOG_LOGIC(
        "Rebuild mode list since max number of spatial streams has changed");
    m_maxSupportedNss = maxNss;
    m_modeList.clear();
    BuildModeList();
  }
}

Time HtPhy::GetDuration(WifiPpduField field,
                        const WifiTxVector &txVector) const {
  switch (field) {
  case WIFI_PPDU_FIELD_PREAMBLE:
    return MicroSeconds(16);
  case WIFI_PPDU_FIELD_NON_HT_HEADER:
    return GetLSigDuration(txVector.GetPreambleType());
  case WIFI_PPDU_FIELD_TRAINING: {
    uint8_t nDataLtf = 8;
    uint8_t nss = txVector.GetNssMax();
    if (nss < 3) {
      nDataLtf = nss;
    } else if (nss < 5) {
      nDataLtf = 4;
    } else if (nss < 7) {
      nDataLtf = 6;
    }

    uint8_t nExtensionLtf = (txVector.GetNess() < 3) ? txVector.GetNess() : 4;

    return GetTrainingDuration(txVector, nDataLtf, nExtensionLtf);
  }
  case WIFI_PPDU_FIELD_HT_SIG:
    return GetHtSigDuration();
  default:
    return OfdmPhy::GetDuration(field, txVector);
  }
}

Time HtPhy::GetLSigDuration(WifiPreamble preamble) const {
  return MicroSeconds(4);
}

Time HtPhy::GetTrainingDuration(const WifiTxVector &txVector, uint8_t nDataLtf,
                                uint8_t nExtensionLtf) const {
  NS_ABORT_MSG_IF(nDataLtf == 0 || nDataLtf > 4 || nExtensionLtf > 4 ||
                      (nDataLtf + nExtensionLtf) > 5,
                  "Unsupported combination of data ("
                      << +nDataLtf << ")  and extension (" << +nExtensionLtf
                      << ")  LTFs numbers for HT");
  Time duration = MicroSeconds(4) * (nDataLtf + nExtensionLtf);
  return MicroSeconds(4) * (1 + nDataLtf + nExtensionLtf);
}

Time HtPhy::GetHtSigDuration() const { return MicroSeconds(8); }

Time HtPhy::GetPayloadDuration(uint32_t size, const WifiTxVector &txVector,
                               WifiPhyBand band, MpduType mpdutype,
                               bool incFlag, uint32_t &totalAmpduSize,
                               double &totalAmpduNumSymbols,
                               uint16_t staId) const {
  WifiMode payloadMode = txVector.GetMode(staId);
  uint8_t stbc = txVector.IsStbc() ? 2 : 1;
  uint8_t nes = GetNumberBccEncoders(txVector);
  Time symbolDuration = GetSymbolDuration(txVector);

  double numDataBitsPerSymbol = payloadMode.GetDataRate(txVector, staId) *
                                symbolDuration.GetNanoSeconds() / 1e9;
  uint8_t service = GetNumberServiceBits();

  double numSymbols = 0;
  switch (mpdutype) {
  case FIRST_MPDU_IN_AGGREGATE: {
    numSymbols = (stbc * (service + size * 8.0 + 6 * nes) /
                  (stbc * numDataBitsPerSymbol));
    if (incFlag) {
      totalAmpduSize += size;
      totalAmpduNumSymbols += numSymbols;
    }
    break;
  }
  case MIDDLE_MPDU_IN_AGGREGATE: {
    numSymbols = (stbc * size * 8.0) / (stbc * numDataBitsPerSymbol);
    if (incFlag) {
      totalAmpduSize += size;
      totalAmpduNumSymbols += numSymbols;
    }
    break;
  }
  case LAST_MPDU_IN_AGGREGATE: {
    uint32_t totalSize = totalAmpduSize + size;
    numSymbols = lrint(stbc * ceil((service + totalSize * 8.0 + 6 * nes) /
                                   (stbc * numDataBitsPerSymbol)));
    NS_ASSERT(totalAmpduNumSymbols <= numSymbols);
    numSymbols -= totalAmpduNumSymbols;
    if (incFlag) {
      totalAmpduSize = 0;
      totalAmpduNumSymbols = 0;
    }
    break;
  }
  case NORMAL_MPDU:
  case SINGLE_MPDU: {
    numSymbols = lrint(stbc * ceil((service + size * 8.0 + 6.0 * nes) /
                                   (stbc * numDataBitsPerSymbol)));
    break;
  }
  default:
    NS_FATAL_ERROR("Unknown MPDU type");
  }

  Time payloadDuration = FemtoSeconds(
      static_cast<uint64_t>(numSymbols * symbolDuration.GetFemtoSeconds()));
  if (mpdutype == NORMAL_MPDU || mpdutype == SINGLE_MPDU ||
      mpdutype == LAST_MPDU_IN_AGGREGATE) {
    payloadDuration += GetSignalExtension(band);
  }
  return payloadDuration;
}

uint8_t HtPhy::GetNumberBccEncoders(const WifiTxVector &txVector) const {
  double maxRatePerCoder = (txVector.GetGuardInterval() == 800) ? 320e6 : 350e6;
  return ceil(txVector.GetMode().GetDataRate(txVector) / maxRatePerCoder);
}

Time HtPhy::GetSymbolDuration(const WifiTxVector &txVector) const {
  uint16_t gi = txVector.GetGuardInterval();
  NS_ASSERT(gi == 400 || gi == 800);
  return NanoSeconds(3200 + gi);
}

Ptr<WifiPpdu> HtPhy::BuildPpdu(const WifiConstPsduMap &psdus,
                               const WifiTxVector &txVector,
                               Time ppduDuration) {
  NS_LOG_FUNCTION(this << psdus << txVector << ppduDuration);
  return Create<HtPpdu>(psdus.begin()->second, txVector,
                        m_wifiPhy->GetOperatingChannel(), ppduDuration,
                        ObtainNextUid(txVector));
}

PhyEntity::PhyFieldRxStatus HtPhy::DoEndReceiveField(WifiPpduField field,
                                                     Ptr<Event> event) {
  NS_LOG_FUNCTION(this << field << *event);
  switch (field) {
  case WIFI_PPDU_FIELD_HT_SIG:
    return EndReceiveHtSig(event);
  case WIFI_PPDU_FIELD_TRAINING:
    return PhyFieldRxStatus(true);
  case WIFI_PPDU_FIELD_NON_HT_HEADER:
  default:
    return OfdmPhy::DoEndReceiveField(field, event);
  }
}

PhyEntity::PhyFieldRxStatus HtPhy::EndReceiveHtSig(Ptr<Event> event) {
  NS_LOG_FUNCTION(this << *event);
  NS_ASSERT(event->GetPpdu()->GetTxVector().GetPreambleType() ==
            WIFI_PREAMBLE_HT_MF);
  SnrPer snrPer = GetPhyHeaderSnrPer(WIFI_PPDU_FIELD_HT_SIG, event);
  NS_LOG_DEBUG("HT-SIG: SNR(dB)=" << RatioToDb(snrPer.snr)
                                  << ", PER=" << snrPer.per);
  PhyFieldRxStatus status(GetRandomValue() > snrPer.per);
  if (status.isSuccess) {
    NS_LOG_DEBUG("Received HT-SIG");
    if (!IsAllConfigSupported(WIFI_PPDU_FIELD_HT_SIG, event->GetPpdu())) {
      status = PhyFieldRxStatus(false, UNSUPPORTED_SETTINGS, DROP);
    }
  } else {
    NS_LOG_DEBUG("Drop packet because HT-SIG reception failed");
    status.reason = HT_SIG_FAILURE;
    status.actionIfFailure = DROP;
  }
  return status;
}

bool HtPhy::IsAllConfigSupported(WifiPpduField field,
                                 Ptr<const WifiPpdu> ppdu) const {
  if (field == WIFI_PPDU_FIELD_NON_HT_HEADER) {
    return true;
  }
  return OfdmPhy::IsAllConfigSupported(field, ppdu);
}

bool HtPhy::IsConfigSupported(Ptr<const WifiPpdu> ppdu) const {
  const auto &txVector = ppdu->GetTxVector();
  if (txVector.GetNss() > m_wifiPhy->GetMaxSupportedRxSpatialStreams()) {
    NS_LOG_DEBUG(
        "Packet reception could not be started because not enough RX antennas");
    return false;
  }
  if (!IsModeSupported(txVector.GetMode())) {
    NS_LOG_DEBUG("Drop packet because it was sent using an unsupported mode ("
                 << txVector.GetMode() << ")");
    return false;
  }
  return true;
}

Ptr<SpectrumValue>
HtPhy::GetTxPowerSpectralDensity(double txPowerW,
                                 Ptr<const WifiPpdu> ppdu) const {
  const auto &txVector = ppdu->GetTxVector();
  uint16_t centerFrequency = GetCenterFrequencyForChannelWidth(txVector);
  uint16_t channelWidth = txVector.GetChannelWidth();
  NS_LOG_FUNCTION(this << centerFrequency << channelWidth << txPowerW);
  const auto &txMaskRejectionParams = GetTxMaskRejectionParams();
  Ptr<SpectrumValue> v =
      WifiSpectrumValueHelper::CreateHtOfdmTxPowerSpectralDensity(
          centerFrequency, channelWidth, txPowerW,
          GetGuardBandwidth(channelWidth), std::get<0>(txMaskRejectionParams),
          std::get<1>(txMaskRejectionParams),
          std::get<2>(txMaskRejectionParams));
  return v;
}

void HtPhy::InitializeModes() {
  for (uint8_t i = 0; i < 32; ++i) {
    GetHtMcs(i);
  }
}

WifiMode HtPhy::GetHtMcs(uint8_t index) {
#define CASE(x)                                                                \
  case x:                                                                      \
    return GetHtMcs##x();

  switch (index) {
    CASE(0)
    CASE(1)
    CASE(2)
    CASE(3)
    CASE(4)
    CASE(5)
    CASE(6)
    CASE(7)
    CASE(8)
    CASE(9)
    CASE(10)
    CASE(11)
    CASE(12)
    CASE(13)
    CASE(14)
    CASE(15)
    CASE(16)
    CASE(17)
    CASE(18)
    CASE(19)
    CASE(20)
    CASE(21)
    CASE(22)
    CASE(23)
    CASE(24)
    CASE(25)
    CASE(26)
    CASE(27)
    CASE(28)
    CASE(29)
    CASE(30)
    CASE(31)
  default:
    NS_ABORT_MSG("Inexistent (or not supported) index ("
                 << +index << ") requested for HT");
    return WifiMode();
  }
#undef CASE
}

#define GET_HT_MCS(x)                                                          \
  WifiMode HtPhy::GetHtMcs##x() {                                              \
    static WifiMode mcs = CreateHtMcs(x);                                      \
    return mcs;                                                                \
  };

GET_HT_MCS(0)
GET_HT_MCS(1)
GET_HT_MCS(2)
GET_HT_MCS(3)
GET_HT_MCS(4)
GET_HT_MCS(5)
GET_HT_MCS(6)
GET_HT_MCS(7)
GET_HT_MCS(8)
GET_HT_MCS(9)
GET_HT_MCS(10)
GET_HT_MCS(11)
GET_HT_MCS(12)
GET_HT_MCS(13)
GET_HT_MCS(14)
GET_HT_MCS(15)
GET_HT_MCS(16)
GET_HT_MCS(17)
GET_HT_MCS(18)
GET_HT_MCS(19)
GET_HT_MCS(20)
GET_HT_MCS(21)
GET_HT_MCS(22)
GET_HT_MCS(23)
GET_HT_MCS(24)
GET_HT_MCS(25)
GET_HT_MCS(26)
GET_HT_MCS(27)
GET_HT_MCS(28)
GET_HT_MCS(29)
GET_HT_MCS(30)
GET_HT_MCS(31)
#undef GET_HT_MCS

WifiMode HtPhy::CreateHtMcs(uint8_t index) {
  NS_ASSERT_MSG(index <= 31, "HtMcs index must be <= 31!");
  return WifiModeFactory::CreateWifiMcs(
      "HtMcs" + std::to_string(index), index, WIFI_MOD_CLASS_HT, false,
      MakeBoundCallback(&GetHtCodeRate, index),
      MakeBoundCallback(&GetHtConstellationSize, index),
      MakeCallback(&GetPhyRateFromTxVector),
      MakeCallback(&GetDataRateFromTxVector),
      MakeBoundCallback(&GetNonHtReferenceRate, index),
      MakeCallback(&IsAllowed));
}

WifiCodeRate HtPhy::GetHtCodeRate(uint8_t mcsValue) {
  return GetCodeRate(mcsValue % 8);
}

WifiCodeRate HtPhy::GetCodeRate(uint8_t mcsValue) {
  switch (mcsValue) {
  case 0:
  case 1:
  case 3:
    return WIFI_CODE_RATE_1_2;
  case 2:
  case 4:
  case 6:
    return WIFI_CODE_RATE_3_4;
  case 5:
    return WIFI_CODE_RATE_2_3;
  case 7:
    return WIFI_CODE_RATE_5_6;
  default:
    return WIFI_CODE_RATE_UNDEFINED;
  }
}

uint16_t HtPhy::GetHtConstellationSize(uint8_t mcsValue) {
  return GetConstellationSize(mcsValue % 8);
}

uint16_t HtPhy::GetConstellationSize(uint8_t mcsValue) {
  switch (mcsValue) {
  case 0:
    return 2;
  case 1:
  case 2:
    return 4;
  case 3:
  case 4:
    return 16;
  case 5:
  case 6:
  case 7:
    return 64;
  default:
    return 0;
  }
}

uint64_t HtPhy::GetPhyRate(uint8_t mcsValue, uint16_t channelWidth,
                           uint16_t guardInterval, uint8_t nss) {
  WifiCodeRate codeRate = GetHtCodeRate(mcsValue);
  uint64_t dataRate = GetDataRate(mcsValue, channelWidth, guardInterval, nss);
  return CalculatePhyRate(codeRate, dataRate);
}

uint64_t HtPhy::CalculatePhyRate(WifiCodeRate codeRate, uint64_t dataRate) {
  return (dataRate / GetCodeRatio(codeRate));
}

uint64_t HtPhy::GetPhyRateFromTxVector(const WifiTxVector &txVector, uint16_t) {
  return GetPhyRate(txVector.GetMode().GetMcsValue(),
                    txVector.GetChannelWidth(), txVector.GetGuardInterval(),
                    txVector.GetNss());
}

double HtPhy::GetCodeRatio(WifiCodeRate codeRate) {
  switch (codeRate) {
  case WIFI_CODE_RATE_5_6:
    return (5.0 / 6.0);
  default:
    return OfdmPhy::GetCodeRatio(codeRate);
  }
}

uint64_t HtPhy::GetDataRateFromTxVector(const WifiTxVector &txVector,
                                        uint16_t) {
  return GetDataRate(txVector.GetMode().GetMcsValue(),
                     txVector.GetChannelWidth(), txVector.GetGuardInterval(),
                     txVector.GetNss());
}

uint64_t HtPhy::GetDataRate(uint8_t mcsValue, uint16_t channelWidth,
                            uint16_t guardInterval, uint8_t nss) {
  NS_ASSERT(guardInterval == 800 || guardInterval == 400);
  NS_ASSERT(nss <= 4);
  return CalculateDataRate(
      GetSymbolDuration(NanoSeconds(guardInterval)),
      GetUsableSubcarriers(channelWidth),
      static_cast<uint16_t>(log2(GetHtConstellationSize(mcsValue))),
      GetCodeRatio(GetHtCodeRate(mcsValue)), nss);
}

uint64_t HtPhy::CalculateDataRate(Time symbolDuration,
                                  uint16_t usableSubCarriers,
                                  uint16_t numberOfBitsPerSubcarrier,
                                  double codingRate, uint8_t nss) {
  return nss * OfdmPhy::CalculateDataRate(symbolDuration, usableSubCarriers,
                                          numberOfBitsPerSubcarrier,
                                          codingRate);
}

uint16_t HtPhy::GetUsableSubcarriers(uint16_t channelWidth) {
  return (channelWidth == 40) ? 108 : 52;
}

Time HtPhy::GetSymbolDuration(Time guardInterval) {
  return NanoSeconds(3200) + guardInterval;
}

uint64_t HtPhy::GetNonHtReferenceRate(uint8_t mcsValue) {
  WifiCodeRate codeRate = GetHtCodeRate(mcsValue);
  uint16_t constellationSize = GetHtConstellationSize(mcsValue);
  return CalculateNonHtReferenceRate(codeRate, constellationSize);
}

uint64_t HtPhy::CalculateNonHtReferenceRate(WifiCodeRate codeRate,
                                            uint16_t constellationSize) {
  uint64_t dataRate;
  switch (constellationSize) {
  case 2:
    if (codeRate == WIFI_CODE_RATE_1_2) {
      dataRate = 6000000;
    } else if (codeRate == WIFI_CODE_RATE_3_4) {
      dataRate = 9000000;
    } else {
      NS_FATAL_ERROR(
          "Trying to get reference rate for a MCS with wrong combination of "
          "coding rate and modulation");
    }
    break;
  case 4:
    if (codeRate == WIFI_CODE_RATE_1_2) {
      dataRate = 12000000;
    } else if (codeRate == WIFI_CODE_RATE_3_4) {
      dataRate = 18000000;
    } else {
      NS_FATAL_ERROR(
          "Trying to get reference rate for a MCS with wrong combination of "
          "coding rate and modulation");
    }
    break;
  case 16:
    if (codeRate == WIFI_CODE_RATE_1_2) {
      dataRate = 24000000;
    } else if (codeRate == WIFI_CODE_RATE_3_4) {
      dataRate = 36000000;
    } else {
      NS_FATAL_ERROR(
          "Trying to get reference rate for a MCS with wrong combination of "
          "coding rate and modulation");
    }
    break;
  case 64:
    if (codeRate == WIFI_CODE_RATE_1_2 || codeRate == WIFI_CODE_RATE_2_3) {
      dataRate = 48000000;
    } else if (codeRate == WIFI_CODE_RATE_3_4 ||
               codeRate == WIFI_CODE_RATE_5_6) {
      dataRate = 54000000;
    } else {
      NS_FATAL_ERROR(
          "Trying to get reference rate for a MCS with wrong combination of "
          "coding rate and modulation");
    }
    break;
  default:
    NS_FATAL_ERROR("Wrong constellation size");
  }
  return dataRate;
}

bool HtPhy::IsAllowed(const WifiTxVector &) { return true; }

uint32_t HtPhy::GetMaxPsduSize() const { return 65535; }

PhyEntity::CcaIndication
HtPhy::GetCcaIndication(const Ptr<const WifiPpdu> ppdu) {
  NS_LOG_FUNCTION(this);
  if (m_wifiPhy->GetChannelWidth() < 40) {
    return OfdmPhy::GetCcaIndication(ppdu);
  }
  double ccaThresholdDbm = GetCcaThreshold(ppdu, WIFI_CHANLIST_PRIMARY);
  Time delayUntilCcaEnd =
      GetDelayUntilCcaEnd(ccaThresholdDbm, GetPrimaryBand(20));
  if (delayUntilCcaEnd.IsStrictlyPositive()) {
    return std::make_pair(delayUntilCcaEnd, WIFI_CHANLIST_PRIMARY);
  }
  if (ppdu) {
    const uint16_t primaryWidth = 20;
    uint16_t p20MinFreq =
        m_wifiPhy->GetOperatingChannel().GetPrimaryChannelCenterFrequency(
            primaryWidth) -
        (primaryWidth / 2);
    uint16_t p20MaxFreq =
        m_wifiPhy->GetOperatingChannel().GetPrimaryChannelCenterFrequency(
            primaryWidth) +
        (primaryWidth / 2);
    if (ppdu->DoesOverlapChannel(p20MinFreq, p20MaxFreq)) {
      return std::nullopt;
    }
  }

  const uint16_t secondaryWidth = 20;
  uint16_t s20MinFreq =
      m_wifiPhy->GetOperatingChannel().GetSecondaryChannelCenterFrequency(
          secondaryWidth) -
      (secondaryWidth / 2);
  uint16_t s20MaxFreq =
      m_wifiPhy->GetOperatingChannel().GetSecondaryChannelCenterFrequency(
          secondaryWidth) +
      (secondaryWidth / 2);
  if (!ppdu || ppdu->DoesOverlapChannel(s20MinFreq, s20MaxFreq)) {
    ccaThresholdDbm = GetCcaThreshold(ppdu, WIFI_CHANLIST_SECONDARY);
    delayUntilCcaEnd =
        GetDelayUntilCcaEnd(ccaThresholdDbm, GetSecondaryBand(20));
    if (delayUntilCcaEnd.IsStrictlyPositive()) {
      return std::make_pair(delayUntilCcaEnd, WIFI_CHANLIST_SECONDARY);
    }
  }

  return std::nullopt;
}

} // namespace ns3

namespace {

class ConstructorHt {
public:
  ConstructorHt() {
    ns3::HtPhy::InitializeModes();
    ns3::WifiPhy::AddStaticPhyEntity(ns3::WIFI_MOD_CLASS_HT,
                                     ns3::Create<ns3::HtPhy>());
  }
} g_constructor_ht;

} // namespace
