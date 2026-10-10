
#include "wifi-tx-parameters.h"

#include "mpdu-aggregator.h"
#include "msdu-aggregator.h"
#include "wifi-acknowledgment.h"
#include "wifi-mac-trailer.h"
#include "wifi-mpdu.h"
#include "wifi-protection.h"

#include "ns3/log.h"
#include "ns3/packet.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("WifiTxParameters");

WifiTxParameters::WifiTxParameters() {}

WifiTxParameters::WifiTxParameters(const WifiTxParameters &txParams) {
  m_txVector = txParams.m_txVector;
  m_protection =
      (txParams.m_protection ? txParams.m_protection->Copy() : nullptr);
  m_acknowledgment =
      (txParams.m_acknowledgment ? txParams.m_acknowledgment->Copy() : nullptr);
  m_txDuration = txParams.m_txDuration;
  m_info = txParams.m_info;
}

WifiTxParameters &
WifiTxParameters::operator=(const WifiTxParameters &txParams) {
  if (&txParams == this) {
    return *this;
  }

  m_txVector = txParams.m_txVector;
  m_protection =
      (txParams.m_protection ? txParams.m_protection->Copy() : nullptr);
  m_acknowledgment =
      (txParams.m_acknowledgment ? txParams.m_acknowledgment->Copy() : nullptr);
  m_txDuration = txParams.m_txDuration;
  m_info = txParams.m_info;

  return *this;
}

void WifiTxParameters::Clear() {
  NS_LOG_FUNCTION(this);

  m_info.clear();
  m_txVector = WifiTxVector();
  m_protection.reset(nullptr);
  m_acknowledgment.reset(nullptr);
  m_txDuration = Time::Min();
}

const WifiTxParameters::PsduInfo *
WifiTxParameters::GetPsduInfo(Mac48Address receiver) const {
  auto infoIt = m_info.find(receiver);

  if (infoIt == m_info.end()) {
    return nullptr;
  }
  return &infoIt->second;
}

const WifiTxParameters::PsduInfoMap &WifiTxParameters::GetPsduInfoMap() const {
  return m_info;
}

void WifiTxParameters::AddMpdu(Ptr<const WifiMpdu> mpdu) {
  NS_LOG_FUNCTION(this << *mpdu);

  const WifiMacHeader &hdr = mpdu->GetHeader();

  auto infoIt = m_info.find(hdr.GetAddr1());

  if (infoIt == m_info.end()) {
    std::map<uint8_t, std::set<uint16_t>> seqNumbers;
    if (hdr.IsQosData()) {
      seqNumbers[hdr.GetQosTid()] = {hdr.GetSequenceNumber()};
    }

    m_info.emplace(hdr.GetAddr1(),
                   PsduInfo{hdr, mpdu->GetPacketSize(), 0, seqNumbers});
    return;
  }

  NS_ASSERT_MSG((hdr.IsQosData() && !hdr.HasData()) ||
                    infoIt->second.amsduSize > 0,
                "An MPDU can only be aggregated to an existing (A-)MPDU");

  infoIt->second.ampduSize = MpduAggregator::GetSizeIfAggregated(
      infoIt->second.header.GetSize() + infoIt->second.amsduSize +
          WIFI_MAC_FCS_LENGTH,
      infoIt->second.ampduSize);
  infoIt->second.header = hdr;
  infoIt->second.amsduSize = mpdu->GetPacketSize();

  if (hdr.IsQosData()) {
    auto ret = infoIt->second.seqNumbers.emplace(
        hdr.GetQosTid(), std::set<uint16_t>{hdr.GetSequenceNumber()});

    if (!ret.second) {
      ret.first->second.insert(hdr.GetSequenceNumber());
    }
  }
}

uint32_t WifiTxParameters::GetSizeIfAddMpdu(Ptr<const WifiMpdu> mpdu) const {
  NS_LOG_FUNCTION(this << *mpdu);

  auto infoIt = m_info.find(mpdu->GetHeader().GetAddr1());

  if (infoIt == m_info.end()) {
    if (m_txVector.GetModulationClass() >= WIFI_MOD_CLASS_VHT) {
      return MpduAggregator::GetSizeIfAggregated(mpdu->GetSize(), 0);
    }
    return mpdu->GetSize();
  }

  uint32_t ampduSize = MpduAggregator::GetSizeIfAggregated(
      infoIt->second.header.GetSize() + infoIt->second.amsduSize +
          WIFI_MAC_FCS_LENGTH,
      infoIt->second.ampduSize);
  return MpduAggregator::GetSizeIfAggregated(mpdu->GetSize(), ampduSize);
}

void WifiTxParameters::AggregateMsdu(Ptr<const WifiMpdu> msdu) {
  NS_LOG_FUNCTION(this << *msdu);

  auto infoIt = m_info.find(msdu->GetHeader().GetAddr1());
  NS_ASSERT_MSG(infoIt != m_info.end(),
                "There must be already an MPDU addressed to the same receiver");

  infoIt->second.amsduSize = GetSizeIfAggregateMsdu(msdu).first;
  infoIt->second.header.SetQosAmsdu();
}

std::pair<uint32_t, uint32_t>
WifiTxParameters::GetSizeIfAggregateMsdu(Ptr<const WifiMpdu> msdu) const {
  NS_LOG_FUNCTION(this << *msdu);

  NS_ASSERT_MSG(msdu->GetHeader().IsQosData(),
                "Can only aggregate a QoS data frame to an A-MSDU");

  auto infoIt = m_info.find(msdu->GetHeader().GetAddr1());
  NS_ASSERT_MSG(infoIt != m_info.end(),
                "There must be already an MPDU addressed to the same receiver");

  NS_ASSERT_MSG(
      infoIt->second.amsduSize > 0,
      "The amsduSize should be set to the size of the previous MSDU(s)");
  NS_ASSERT_MSG(
      infoIt->second.header.IsQosData(),
      "The MPDU being built for this receiver must be a QoS data frame");
  NS_ASSERT_MSG(infoIt->second.header.GetQosTid() ==
                    msdu->GetHeader().GetQosTid(),
                "The MPDU being built must belong to the same TID as the MSDU "
                "to aggregate");
  NS_ASSERT_MSG(
      infoIt->second.seqNumbers.find(msdu->GetHeader().GetQosTid()) !=
          infoIt->second.seqNumbers.end(),
      "At least one MPDU with the same TID must have been added previously");

  uint32_t currAmsduSize = infoIt->second.amsduSize;

  if (!infoIt->second.header.IsQosAmsdu()) {
    currAmsduSize = MsduAggregator::GetSizeIfAggregated(currAmsduSize, 0);
  }

  uint32_t newAmsduSize = MsduAggregator::GetSizeIfAggregated(
      msdu->GetPacket()->GetSize(), currAmsduSize);
  uint32_t newMpduSize =
      infoIt->second.header.GetSize() + newAmsduSize + WIFI_MAC_FCS_LENGTH;

  if (infoIt->second.ampduSize > 0 ||
      m_txVector.GetModulationClass() >= WIFI_MOD_CLASS_VHT) {
    return {newAmsduSize, MpduAggregator::GetSizeIfAggregated(
                              newMpduSize, infoIt->second.ampduSize)};
  }

  return {newAmsduSize, newMpduSize};
}

uint32_t WifiTxParameters::GetSize(Mac48Address receiver) const {
  NS_LOG_FUNCTION(this << receiver);

  auto infoIt = m_info.find(receiver);

  if (infoIt == m_info.end()) {
    return 0;
  }

  uint32_t newMpduSize = infoIt->second.header.GetSize() +
                         infoIt->second.amsduSize + WIFI_MAC_FCS_LENGTH;

  if (infoIt->second.ampduSize > 0 ||
      m_txVector.GetModulationClass() >= WIFI_MOD_CLASS_VHT) {
    return MpduAggregator::GetSizeIfAggregated(newMpduSize,
                                               infoIt->second.ampduSize);
  }

  return newMpduSize;
}

void WifiTxParameters::Print(std::ostream &os) const {
  os << "TXVECTOR=" << m_txVector;
  if (m_protection) {
    os << ", Protection=" << m_protection.get();
  }
  if (m_acknowledgment) {
    os << ", Acknowledgment=" << m_acknowledgment.get();
  }
  os << ", PSDUs:";
  for (const auto &info : m_info) {
    os << " [To=" << info.second.header.GetAddr1()
       << ", A-MSDU size=" << info.second.amsduSize
       << ", A-MPDU size=" << info.second.ampduSize << "]";
  }
}

std::ostream &operator<<(std::ostream &os, const WifiTxParameters *txParams) {
  txParams->Print(os);
  return os;
}

} // namespace ns3
