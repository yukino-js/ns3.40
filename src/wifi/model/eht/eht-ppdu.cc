
#include "eht-ppdu.h"

#include "eht-phy.h"

#include "ns3/log.h"
#include "ns3/wifi-phy-operating-channel.h"
#include "ns3/wifi-psdu.h"

#include <numeric>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("EhtPpdu");

EhtPpdu::EhtPpdu(const WifiConstPsduMap &psdus, const WifiTxVector &txVector,
                 const WifiPhyOperatingChannel &channel, Time ppduDuration,
                 uint64_t uid, TxPsdFlag flag)
    : HePpdu(psdus, txVector, channel, ppduDuration, uid, flag) {
  NS_LOG_FUNCTION(this << psdus << txVector << channel << ppduDuration << uid
                       << flag);
  SetPhyHeaders(txVector, ppduDuration);
}

void EhtPpdu::SetPhyHeaders(const WifiTxVector &txVector, Time ppduDuration) {
  NS_LOG_FUNCTION(this << txVector << ppduDuration);
  SetEhtPhyHeader(txVector);
}

void EhtPpdu::SetEhtPhyHeader(const WifiTxVector &txVector) {
  const auto bssColor = txVector.GetBssColor();
  NS_ASSERT(bssColor < 64);
  if (ns3::IsDlMu(m_preamble)) {
    const auto p20Index = m_operatingChannel.GetPrimaryChannelIndex(20);
    m_ehtPhyHeader.emplace<EhtMuPhyHeader>(EhtMuPhyHeader{
        .m_bandwidth =
            GetChannelWidthEncodingFromMhz(txVector.GetChannelWidth()),
        .m_bssColor = bssColor,
        .m_ppduType = txVector.GetEhtPpduType(),
        .m_ehtSigMcs = txVector.GetSigBMode().GetMcsValue(),
        .m_giLtfSize =
            GetGuardIntervalAndNltfEncoding(txVector.GetGuardInterval(), 2),
        .m_ruAllocationA =
            txVector.IsMu() ? std::optional{txVector.GetRuAllocation(p20Index)}
                            : std::nullopt,
        .m_contentChannels = GetEhtSigContentChannels(txVector, p20Index)});
  } else if (ns3::IsUlMu(m_preamble)) {
    m_ehtPhyHeader.emplace<EhtTbPhyHeader>(EhtTbPhyHeader{
        .m_bandwidth =
            GetChannelWidthEncodingFromMhz(txVector.GetChannelWidth()),
        .m_bssColor = bssColor,
        .m_ppduType = txVector.GetEhtPpduType()});
  }
}

WifiPpduType EhtPpdu::GetType() const {
  if (m_psdus.count(SU_STA_ID) > 0) {
    return WIFI_PPDU_TYPE_SU;
  }
  switch (m_preamble) {
  case WIFI_PREAMBLE_EHT_MU:
    return WIFI_PPDU_TYPE_DL_MU;
  case WIFI_PREAMBLE_EHT_TB:
    return WIFI_PPDU_TYPE_UL_MU;
  default:
    NS_ASSERT_MSG(false, "invalid preamble " << m_preamble);
    return WIFI_PPDU_TYPE_SU;
  }
}

bool EhtPpdu::IsDlMu() const {
  return (m_preamble == WIFI_PREAMBLE_EHT_MU) &&
         (m_psdus.count(SU_STA_ID) == 0);
}

bool EhtPpdu::IsUlMu() const {
  return (m_preamble == WIFI_PREAMBLE_EHT_TB) &&
         (m_psdus.count(SU_STA_ID) == 0);
}

void EhtPpdu::SetTxVectorFromPhyHeaders(WifiTxVector &txVector) const {
  txVector.SetLength(m_lSig.GetLength());
  txVector.SetAggregation(m_psdus.size() > 1 ||
                          m_psdus.begin()->second->IsAggregate());
  if (ns3::IsDlMu(m_preamble)) {
    auto ehtPhyHeader = std::get_if<EhtMuPhyHeader>(&m_ehtPhyHeader);
    NS_ASSERT(ehtPhyHeader);
    txVector.SetChannelWidth(
        GetChannelWidthMhzFromEncoding(ehtPhyHeader->m_bandwidth));
    txVector.SetBssColor(ehtPhyHeader->m_bssColor);
    txVector.SetEhtPpduType(ehtPhyHeader->m_ppduType);
    txVector.SetSigBMode(HePhy::GetVhtMcs(ehtPhyHeader->m_ehtSigMcs));
    txVector.SetGuardInterval(
        GetGuardIntervalFromEncoding(ehtPhyHeader->m_giLtfSize));
    const auto ruAllocation = ehtPhyHeader->m_ruAllocationA;
    if (const auto p20Index = m_operatingChannel.GetPrimaryChannelIndex(20);
        ruAllocation.has_value()) {
      txVector.SetRuAllocation(ruAllocation.value(), p20Index);
      const auto isMuMimo = (ehtPhyHeader->m_ppduType == 2);
      const auto muMimoUsers =
          isMuMimo ? std::accumulate(ehtPhyHeader->m_contentChannels.cbegin(),
                                     ehtPhyHeader->m_contentChannels.cend(), 0,
                                     [](uint8_t prev, const auto &cc) {
                                       return prev + cc.size();
                                     })
                   : 0;
      SetHeMuUserInfos(txVector, ruAllocation.value(),
                       ehtPhyHeader->m_contentChannels,
                       ehtPhyHeader->m_ppduType == 2, muMimoUsers);
    }
    if (ehtPhyHeader->m_ppduType == 1) {
      NS_ASSERT(ehtPhyHeader->m_contentChannels.size() == 1 &&
                ehtPhyHeader->m_contentChannels.front().size() == 1);
      txVector.SetMode(EhtPhy::GetEhtMcs(
          ehtPhyHeader->m_contentChannels.front().front().mcs));
      txVector.SetNss(ehtPhyHeader->m_contentChannels.front().front().nss);
    }
  } else if (ns3::IsUlMu(m_preamble)) {
    auto ehtPhyHeader = std::get_if<EhtTbPhyHeader>(&m_ehtPhyHeader);
    NS_ASSERT(ehtPhyHeader);
    txVector.SetChannelWidth(
        GetChannelWidthMhzFromEncoding(ehtPhyHeader->m_bandwidth));
    txVector.SetBssColor(ehtPhyHeader->m_bssColor);
    txVector.SetEhtPpduType(ehtPhyHeader->m_ppduType);
  }
}

std::pair<std::size_t, std::size_t> EhtPpdu::GetNumRusPerEhtSigBContentChannel(
    uint16_t channelWidth, uint8_t ehtPpduType,
    const RuAllocation &ruAllocation, bool compression,
    std::size_t numMuMimoUsers) {
  if (ehtPpduType == 1) {
    return {1, 0};
  }
  return HePpdu::GetNumRusPerHeSigBContentChannel(channelWidth, ruAllocation,
                                                  compression, numMuMimoUsers);
}

HePpdu::HeSigBContentChannels
EhtPpdu::GetEhtSigContentChannels(const WifiTxVector &txVector,
                                  uint8_t p20Index) {
  if (txVector.GetEhtPpduType() == 1) {
    return HeSigBContentChannels{
        {{0, txVector.GetNss(), txVector.GetMode().GetMcsValue()}}};
  }
  return HePpdu::GetHeSigBContentChannels(txVector, p20Index);
}

uint32_t EhtPpdu::GetEhtSigFieldSize(uint16_t channelWidth,
                                     const RuAllocation &ruAllocation,
                                     uint8_t ehtPpduType, bool compression,
                                     std::size_t numMuMimoUsers) {
  uint32_t commonFieldSize = 0;
  if (!compression) {
    commonFieldSize = 4 + 6;
    if (channelWidth <= 40) {
      commonFieldSize += 8;
    } else {
      commonFieldSize += 8 * (channelWidth / 40) + 1;
    }
  }

  auto numRusPerContentChannel = GetNumRusPerEhtSigBContentChannel(
      channelWidth, ehtPpduType, ruAllocation, compression, numMuMimoUsers);
  auto maxNumRusPerContentChannel =
      std::max(numRusPerContentChannel.first, numRusPerContentChannel.second);
  auto maxNumUserBlockFields = maxNumRusPerContentChannel / 2;
  std::size_t userSpecificFieldSize = maxNumUserBlockFields * (2 * 21 + 4 + 6);
  if (maxNumRusPerContentChannel % 2 != 0) {
    userSpecificFieldSize += 21 + 4 + 6;
  }

  return commonFieldSize + userSpecificFieldSize;
}

Ptr<WifiPpdu> EhtPpdu::Copy() const {
  return Ptr<WifiPpdu>(new EhtPpdu(*this), false);
}

} // namespace ns3
