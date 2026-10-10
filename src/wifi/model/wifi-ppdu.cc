
#include "wifi-ppdu.h"

#include "wifi-phy-operating-channel.h"
#include "wifi-psdu.h"

#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("WifiPpdu");

WifiPpdu::WifiPpdu(Ptr<const WifiPsdu> psdu, const WifiTxVector &txVector,
                   const WifiPhyOperatingChannel &channel, uint64_t uid)
    : m_preamble(txVector.GetPreambleType()),
      m_modulation(txVector.IsValid() ? txVector.GetModulationClass()
                                      : WIFI_MOD_CLASS_UNKNOWN),
      m_txCenterFreq(channel.IsSet() ? channel.GetPrimaryChannelCenterFrequency(
                                           txVector.GetChannelWidth())
                                     : 0),
      m_uid(uid), m_txVector(txVector), m_operatingChannel(channel),
      m_truncatedTx(false), m_txPowerLevel(txVector.GetTxPowerLevel()),
      m_txAntennas(txVector.GetNTx()),
      m_txChannelWidth(txVector.GetChannelWidth()) {
  NS_LOG_FUNCTION(this << *psdu << txVector << channel << uid);
  m_psdus.insert(std::make_pair(SU_STA_ID, psdu));
}

WifiPpdu::WifiPpdu(const WifiConstPsduMap &psdus, const WifiTxVector &txVector,
                   const WifiPhyOperatingChannel &channel, uint64_t uid)
    : m_preamble(txVector.GetPreambleType()),
      m_modulation(
          txVector.IsValid()
              ? txVector.GetMode(psdus.begin()->first).GetModulationClass()
              : WIFI_MOD_CLASS_UNKNOWN),
      m_txCenterFreq(channel.IsSet() ? channel.GetPrimaryChannelCenterFrequency(
                                           txVector.GetChannelWidth())
                                     : 0),
      m_uid(uid), m_txVector(txVector), m_operatingChannel(channel),
      m_truncatedTx(false), m_txPowerLevel(txVector.GetTxPowerLevel()),
      m_txAntennas(txVector.GetNTx()),
      m_txChannelWidth(txVector.GetChannelWidth()) {
  NS_LOG_FUNCTION(this << psdus << txVector << channel << uid);
  m_psdus = psdus;
}

WifiPpdu::~WifiPpdu() {
  for (auto &psdu : m_psdus) {
    psdu.second = nullptr;
  }
  m_psdus.clear();
}

const WifiTxVector &WifiPpdu::GetTxVector() const {
  if (!m_txVector.has_value()) {
    m_txVector = DoGetTxVector();
    m_txVector->SetTxPowerLevel(m_txPowerLevel);
    m_txVector->SetNTx(m_txAntennas);
    m_txVector->SetChannelWidth(m_txChannelWidth);
  }
  return m_txVector.value();
}

WifiTxVector WifiPpdu::DoGetTxVector() const {
  NS_FATAL_ERROR(
      "This method should not be called for the base WifiPpdu class. Use the "
      "overloaded version in the amendment-specific PPDU subclasses instead!");
  return WifiTxVector();
}

void WifiPpdu::ResetTxVector() const {
  NS_LOG_FUNCTION(this);
  m_txVector.reset();
}

void WifiPpdu::UpdateTxVector(const WifiTxVector &updatedTxVector) const {
  NS_LOG_FUNCTION(this << updatedTxVector);
  ResetTxVector();
  m_txVector = updatedTxVector;
}

Ptr<const WifiPsdu> WifiPpdu::GetPsdu() const {
  return m_psdus.begin()->second;
}

bool WifiPpdu::IsTruncatedTx() const { return m_truncatedTx; }

void WifiPpdu::SetTruncatedTx() {
  NS_LOG_FUNCTION(this);
  m_truncatedTx = true;
}

WifiModulationClass WifiPpdu::GetModulation() const { return m_modulation; }

uint16_t WifiPpdu::GetTxChannelWidth() const { return m_txChannelWidth; }

uint16_t WifiPpdu::GetTxCenterFreq() const { return m_txCenterFreq; }

bool WifiPpdu::DoesOverlapChannel(uint16_t minFreq, uint16_t maxFreq) const {
  NS_LOG_FUNCTION(this << m_txCenterFreq << minFreq << maxFreq);
  uint16_t minTxFreq = m_txCenterFreq - m_txChannelWidth / 2;
  uint16_t maxTxFreq = m_txCenterFreq + m_txChannelWidth / 2;
  return minTxFreq < maxFreq && maxTxFreq > minFreq;
}

uint64_t WifiPpdu::GetUid() const { return m_uid; }

WifiPreamble WifiPpdu::GetPreamble() const { return m_preamble; }

WifiPpduType WifiPpdu::GetType() const { return WIFI_PPDU_TYPE_SU; }

uint16_t WifiPpdu::GetStaId() const { return SU_STA_ID; }

Time WifiPpdu::GetTxDuration() const {
  NS_FATAL_ERROR(
      "This method should not be called for the base WifiPpdu class. Use the "
      "overloaded version in the amendment-specific PPDU subclasses instead!");
  return MicroSeconds(0);
}

void WifiPpdu::Print(std::ostream &os) const {
  os << "[ preamble=" << m_preamble << ", modulation=" << m_modulation
     << ", truncatedTx=" << (m_truncatedTx ? "Y" : "N") << ", UID=" << m_uid
     << ", " << PrintPayload() << "]";
}

std::string WifiPpdu::PrintPayload() const {
  std::ostringstream ss;
  ss << "PSDU=" << GetPsdu() << " ";
  return ss.str();
}

Ptr<WifiPpdu> WifiPpdu::Copy() const {
  NS_FATAL_ERROR(
      "This method should not be called for the base WifiPpdu class. Use the "
      "overloaded version in the amendment-specific PPDU subclasses instead!");
  return Ptr<WifiPpdu>(new WifiPpdu(*this), false);
}

std::ostream &operator<<(std::ostream &os, const Ptr<const WifiPpdu> &ppdu) {
  ppdu->Print(os);
  return os;
}

std::ostream &operator<<(std::ostream &os, const WifiConstPsduMap &psdus) {
  for (const auto &psdu : psdus) {
    os << "PSDU for STA_ID=" << psdu.first << " (" << *psdu.second << ") ";
  }
  return os;
}

} // namespace ns3
