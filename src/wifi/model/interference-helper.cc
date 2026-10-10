
#include "interference-helper.h"

#include "error-rate-model.h"
#include "wifi-phy-operating-channel.h"
#include "wifi-phy.h"
#include "wifi-psdu.h"
#include "wifi-utils.h"

#include "ns3/he-ppdu.h"
#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"

#include <algorithm>
#include <numeric>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("InterferenceHelper");

NS_OBJECT_ENSURE_REGISTERED(InterferenceHelper);

Event::Event(Ptr<const WifiPpdu> ppdu, Time duration,
             RxPowerWattPerChannelBand &&rxPower)
    : m_ppdu(ppdu), m_startTime(Simulator::Now()),
      m_endTime(m_startTime + duration), m_rxPowerW(std::move(rxPower)) {}

Event::~Event() {
  m_ppdu = nullptr;
  m_rxPowerW.clear();
}

Ptr<const WifiPpdu> Event::GetPpdu() const { return m_ppdu; }

Time Event::GetStartTime() const { return m_startTime; }

Time Event::GetEndTime() const { return m_endTime; }

Time Event::GetDuration() const { return m_endTime - m_startTime; }

double Event::GetRxPowerW() const {
  NS_ASSERT(!m_rxPowerW.empty());
  auto it = std::max_element(
      m_rxPowerW.cbegin(), m_rxPowerW.cend(),
      [](const auto &p1, const auto &p2) { return p1.second < p2.second; });
  return it->second;
}

double Event::GetRxPowerW(const WifiSpectrumBandInfo &band) const {
  const auto it = m_rxPowerW.find(band);
  NS_ASSERT(it != m_rxPowerW.cend());
  return it->second;
}

const RxPowerWattPerChannelBand &Event::GetRxPowerWPerBand() const {
  return m_rxPowerW;
}

void Event::UpdateRxPowerW(const RxPowerWattPerChannelBand &rxPower) {
  NS_ASSERT(rxPower.size() == m_rxPowerW.size());
  for (auto &currentRxPowerW : m_rxPowerW) {
    auto band = currentRxPowerW.first;
    auto it = rxPower.find(band);
    if (it != rxPower.end()) {
      currentRxPowerW.second += it->second;
    }
  }
}

void Event::UpdatePpdu(Ptr<const WifiPpdu> ppdu) { m_ppdu = ppdu; }

std::ostream &operator<<(std::ostream &os, const Event &event) {
  os << "start=" << event.GetStartTime() << ", end=" << event.GetEndTime()
     << ", power=" << event.GetRxPowerW() << "W"
     << ", PPDU=" << event.GetPpdu();
  return os;
}

InterferenceHelper::NiChange::NiChange(double power, Ptr<Event> event)
    : m_power(power), m_event(event) {}

InterferenceHelper::NiChange::~NiChange() { m_event = nullptr; }

double InterferenceHelper::NiChange::GetPower() const { return m_power; }

void InterferenceHelper::NiChange::AddPower(double power) { m_power += power; }

Ptr<Event> InterferenceHelper::NiChange::GetEvent() const { return m_event; }

InterferenceHelper::InterferenceHelper()
    : m_errorRateModel(nullptr), m_numRxAntennas(1), m_rxing(false) {
  NS_LOG_FUNCTION(this);
}

InterferenceHelper::~InterferenceHelper() { NS_LOG_FUNCTION(this); }

TypeId InterferenceHelper::GetTypeId() {
  static TypeId tid = TypeId("ns3::InterferenceHelper")
                          .SetParent<ns3::Object>()
                          .SetGroupName("Wifi")
                          .AddConstructor<InterferenceHelper>();
  return tid;
}

void InterferenceHelper::DoDispose() {
  NS_LOG_FUNCTION(this);
  for (auto it : m_niChanges) {
    it.second.clear();
  }
  m_niChanges.clear();
  m_firstPowers.clear();
  m_errorRateModel = nullptr;
}

Ptr<Event> InterferenceHelper::Add(Ptr<const WifiPpdu> ppdu, Time duration,
                                   RxPowerWattPerChannelBand &rxPowerW,
                                   bool isStartHePortionRxing) {
  Ptr<Event> event = Create<Event>(ppdu, duration, std::move(rxPowerW));
  AppendEvent(event, isStartHePortionRxing);
  return event;
}

void InterferenceHelper::AddForeignSignal(Time duration,
                                          RxPowerWattPerChannelBand &rxPowerW) {
  WifiMacHeader hdr;
  hdr.SetType(WIFI_MAC_QOSDATA);
  hdr.SetQosTid(0);
  Ptr<WifiPpdu> fakePpdu =
      Create<WifiPpdu>(Create<WifiPsdu>(Create<Packet>(0), hdr), WifiTxVector(),
                       WifiPhyOperatingChannel());
  Add(fakePpdu, duration, rxPowerW);
}

bool InterferenceHelper::HasBands() const { return !m_niChanges.empty(); }

bool InterferenceHelper::HasBand(const WifiSpectrumBandInfo &band) const {
  return (m_niChanges.count(band) > 0);
}

void InterferenceHelper::AddBand(const WifiSpectrumBandInfo &band) {
  NS_LOG_FUNCTION(this << band);
  NS_ASSERT(m_niChanges.count(band) == 0);
  NS_ASSERT(m_firstPowers.count(band) == 0);
  NiChanges niChanges;
  auto result = m_niChanges.insert({band, niChanges});
  NS_ASSERT(result.second);
  AddNiChangeEvent(Time(0), NiChange(0.0, nullptr), result.first);
  m_firstPowers.insert({band, 0.0});
}

void InterferenceHelper::UpdateBands(
    const std::vector<WifiSpectrumBandInfo> &bands,
    const FrequencyRange &freqRange) {
  NS_LOG_FUNCTION(this << freqRange);
  for (auto it = m_niChanges.begin(); it != m_niChanges.end();) {
    if (!IsBandInFrequencyRange(it->first, freqRange)) {
      it++;
      continue;
    }
    const auto frequencies = it->first.frequencies;
    const auto found = std::find_if(bands.cbegin(), bands.cend(),
                                    [frequencies](const auto &item) {
                                      return frequencies == item.frequencies;
                                    }) != std::end(bands);
    if (!found) {
      m_firstPowers.erase(it->first);
      it->second.clear();
      it = m_niChanges.erase(it);
    } else {
      it++;
    }
  }
  for (const auto &band : bands) {
    if (!HasBand(band)) {
      AddBand(band);
    }
  }
}

void InterferenceHelper::SetNoiseFigure(double value) { m_noiseFigure = value; }

void InterferenceHelper::SetErrorRateModel(const Ptr<ErrorRateModel> rate) {
  m_errorRateModel = rate;
}

Ptr<ErrorRateModel> InterferenceHelper::GetErrorRateModel() const {
  return m_errorRateModel;
}

void InterferenceHelper::SetNumberOfReceiveAntennas(uint8_t rx) {
  m_numRxAntennas = rx;
}

Time InterferenceHelper::GetEnergyDuration(double energyW,
                                           const WifiSpectrumBandInfo &band) {
  NS_LOG_FUNCTION(this << energyW << band);
  Time now = Simulator::Now();
  auto niIt = m_niChanges.find(band);
  NS_ABORT_IF(niIt == m_niChanges.end());
  auto i = GetPreviousPosition(now, niIt);
  Time end = i->first;
  for (; i != niIt->second.end(); ++i) {
    double noiseInterferenceW = i->second.GetPower();
    end = i->first;
    if (noiseInterferenceW < energyW) {
      break;
    }
  }
  return end > now ? end - now : MicroSeconds(0);
}

void InterferenceHelper::AppendEvent(Ptr<Event> event,
                                     bool isStartHePortionRxing) {
  NS_LOG_FUNCTION(this << event << isStartHePortionRxing);
  for (const auto &[band, power] : event->GetRxPowerWPerBand()) {
    auto niIt = m_niChanges.find(band);
    NS_ABORT_IF(niIt == m_niChanges.end());
    double previousPowerStart = 0;
    double previousPowerEnd = 0;
    auto previousPowerPosition =
        GetPreviousPosition(event->GetStartTime(), niIt);
    previousPowerStart = previousPowerPosition->second.GetPower();
    previousPowerEnd =
        GetPreviousPosition(event->GetEndTime(), niIt)->second.GetPower();
    if (!m_rxing) {
      m_firstPowers.find(band)->second = previousPowerStart;
      niIt->second.erase(++(niIt->second.begin()), ++previousPowerPosition);
    } else if (isStartHePortionRxing) {
      m_firstPowers.find(band)->second = previousPowerStart;
    }
    auto first = AddNiChangeEvent(event->GetStartTime(),
                                  NiChange(previousPowerStart, event), niIt);
    auto last = AddNiChangeEvent(event->GetEndTime(),
                                 NiChange(previousPowerEnd, event), niIt);
    for (auto i = first; i != last; ++i) {
      i->second.AddPower(power);
    }
  }
}

void InterferenceHelper::UpdateEvent(Ptr<Event> event,
                                     const RxPowerWattPerChannelBand &rxPower) {
  NS_LOG_FUNCTION(this << event);
  for (const auto &[band, power] : rxPower) {
    auto niIt = m_niChanges.find(band);
    NS_ABORT_IF(niIt == m_niChanges.end());
    auto first = GetPreviousPosition(event->GetStartTime(), niIt);
    auto last = GetPreviousPosition(event->GetEndTime(), niIt);
    for (auto i = first; i != last; ++i) {
      i->second.AddPower(power);
    }
  }
  event->UpdateRxPowerW(rxPower);
}

double InterferenceHelper::CalculateSnr(double signal, double noiseInterference,
                                        uint16_t channelWidth,
                                        uint8_t nss) const {
  NS_LOG_FUNCTION(this << signal << noiseInterference << channelWidth << +nss);
  static const double BOLTZMANN = 1.3803e-23;
  double Nt = BOLTZMANN * 290 * channelWidth * 1e6;
  double noiseFloor = m_noiseFigure * Nt;
  double noise = noiseFloor + noiseInterference;
  double snr = signal / noise;
  NS_LOG_DEBUG("bandwidth(MHz)=" << channelWidth << ", signal(W)= " << signal
                                 << ", noise(W)=" << noiseFloor
                                 << ", interference(W)=" << noiseInterference
                                 << ", snr=" << RatioToDb(snr) << "dB");
  if (m_errorRateModel->IsAwgn()) {
    double gain = 1;
    if (m_numRxAntennas > nss) {
      gain = static_cast<double>(m_numRxAntennas) / nss;
    }
    NS_LOG_DEBUG("SNR improvement thanks to diversity: "
                 << 10 * std::log10(gain) << "dB");
    snr *= gain;
  }
  return snr;
}

double InterferenceHelper::CalculateNoiseInterferenceW(
    Ptr<Event> event, NiChangesPerBand &nis,
    const WifiSpectrumBandInfo &band) const {
  NS_LOG_FUNCTION(this << band);
  auto firstPower_it = m_firstPowers.find(band);
  NS_ABORT_IF(firstPower_it == m_firstPowers.end());
  double noiseInterferenceW = firstPower_it->second;
  auto niIt = m_niChanges.find(band);
  NS_ABORT_IF(niIt == m_niChanges.end());
  auto it = niIt->second.find(event->GetStartTime());
  double muMimoPowerW = (event->GetPpdu()->GetType() == WIFI_PPDU_TYPE_UL_MU)
                            ? CalculateMuMimoPowerW(event, band)
                            : 0.0;
  for (; it != niIt->second.end() && it->first < Simulator::Now(); ++it) {
    if (IsSameMuMimoTransmission(event, it->second.GetEvent()) &&
        (event != it->second.GetEvent())) {
      continue;
    }
    noiseInterferenceW =
        it->second.GetPower() - event->GetRxPowerW(band) - muMimoPowerW;
    if (std::abs(noiseInterferenceW) < std::numeric_limits<double>::epsilon()) {
      noiseInterferenceW = 0.0;
    }
  }
  it = niIt->second.find(event->GetStartTime());
  NS_ABORT_IF(it == niIt->second.end());
  for (; it != niIt->second.end() && it->second.GetEvent() != event; ++it) {
    ;
  }
  NiChanges ni;
  ni.emplace(event->GetStartTime(), NiChange(0, event));
  while (++it != niIt->second.end() && it->second.GetEvent() != event) {
    ni.insert(*it);
  }
  ni.emplace(event->GetEndTime(), NiChange(0, event));
  nis.insert({band, ni});
  NS_ASSERT_MSG(noiseInterferenceW >= 0.0,
                "CalculateNoiseInterferenceW returns negative value "
                    << noiseInterferenceW);
  return noiseInterferenceW;
}

double InterferenceHelper::CalculateMuMimoPowerW(
    Ptr<const Event> event, const WifiSpectrumBandInfo &band) const {
  auto niIt = m_niChanges.find(band);
  NS_ASSERT(niIt != m_niChanges.end());
  auto it = niIt->second.begin();
  ++it;
  double muMimoPowerW = 0.0;
  for (; it != niIt->second.end() && it->first < Simulator::Now(); ++it) {
    if (IsSameMuMimoTransmission(event, it->second.GetEvent())) {
      auto hePpdu =
          DynamicCast<HePpdu>(it->second.GetEvent()->GetPpdu()->Copy());
      NS_ASSERT(hePpdu);
      HePpdu::TxPsdFlag psdFlag = hePpdu->GetTxPsdFlag();
      if (psdFlag == HePpdu::PSD_HE_PORTION) {
        const auto staId = event->GetPpdu()
                               ->GetTxVector()
                               .GetHeMuUserInfoMap()
                               .cbegin()
                               ->first;
        const auto otherStaId = it->second.GetEvent()
                                    ->GetPpdu()
                                    ->GetTxVector()
                                    .GetHeMuUserInfoMap()
                                    .cbegin()
                                    ->first;
        if (staId == otherStaId) {
          break;
        }
        muMimoPowerW += it->second.GetEvent()->GetRxPowerW(band);
      }
    }
  }
  return muMimoPowerW;
}

double InterferenceHelper::CalculateChunkSuccessRate(
    double snir, Time duration, WifiMode mode, const WifiTxVector &txVector,
    WifiPpduField field) const {
  if (duration.IsZero()) {
    return 1.0;
  }
  uint64_t rate = mode.GetDataRate(txVector.GetChannelWidth());
  auto nbits = static_cast<uint64_t>(rate * duration.GetSeconds());
  double csr = m_errorRateModel->GetChunkSuccessRate(
      mode, txVector, snir, nbits, m_numRxAntennas, field);
  return csr;
}

double InterferenceHelper::CalculatePayloadChunkSuccessRate(
    double snir, Time duration, const WifiTxVector &txVector,
    uint16_t staId) const {
  if (duration.IsZero()) {
    return 1.0;
  }
  WifiMode mode = txVector.GetMode(staId);
  uint64_t rate = mode.GetDataRate(txVector, staId);
  auto nbits = static_cast<uint64_t>(rate * duration.GetSeconds());
  nbits /= txVector.GetNss(staId);
  double csr = m_errorRateModel->GetChunkSuccessRate(
      mode, txVector, snir, nbits, m_numRxAntennas, WIFI_PPDU_FIELD_DATA,
      staId);
  return csr;
}

double InterferenceHelper::CalculatePayloadPer(
    Ptr<const Event> event, uint16_t channelWidth, NiChangesPerBand *nis,
    const WifiSpectrumBandInfo &band, uint16_t staId,
    std::pair<Time, Time> window) const {
  NS_LOG_FUNCTION(this << channelWidth << band << staId << window.first
                       << window.second);
  double psr = 1.0;
  const auto &niIt = nis->find(band)->second;
  auto j = niIt.cbegin();
  Time previous = j->first;
  double muMimoPowerW = 0.0;
  WifiMode payloadMode = event->GetPpdu()->GetTxVector().GetMode(staId);
  Time phyPayloadStart = j->first;
  if (event->GetPpdu()->GetType() != WIFI_PPDU_TYPE_UL_MU &&
      event->GetPpdu()->GetType() != WIFI_PPDU_TYPE_DL_MU) {
    phyPayloadStart = j->first + WifiPhy::CalculatePhyPreambleAndHeaderDuration(
                                     event->GetPpdu()->GetTxVector());
  } else {
    muMimoPowerW = CalculateMuMimoPowerW(event, band);
  }
  Time windowStart = phyPayloadStart + window.first;
  Time windowEnd = phyPayloadStart + window.second;
  NS_ABORT_IF(m_firstPowers.count(band) == 0);
  double noiseInterferenceW = m_firstPowers.at(band);
  double powerW = event->GetRxPowerW(band);
  while (++j != niIt.cend()) {
    Time current = j->first;
    NS_LOG_DEBUG("previous= " << previous << ", current=" << current);
    NS_ASSERT(current >= previous);
    double snr = CalculateSnr(powerW, noiseInterferenceW, channelWidth,
                              event->GetPpdu()->GetTxVector().GetNss(staId));
    if (previous >= windowStart) {
      psr *= CalculatePayloadChunkSuccessRate(
          snr, Min(windowEnd, current) - previous,
          event->GetPpdu()->GetTxVector(), staId);
      NS_LOG_DEBUG(
          "Both previous and current point to the windowed payload: mode="
          << payloadMode << ", psr=" << psr);
    } else if (current >= windowStart) {
      psr *= CalculatePayloadChunkSuccessRate(
          snr, Min(windowEnd, current) - windowStart,
          event->GetPpdu()->GetTxVector(), staId);
      NS_LOG_DEBUG("previous is before windowed payload and current is in the "
                   "windowed payload: mode="
                   << payloadMode << ", psr=" << psr);
    }
    noiseInterferenceW = j->second.GetPower() - powerW;
    if (IsSameMuMimoTransmission(event, j->second.GetEvent())) {
      muMimoPowerW += j->second.GetEvent()->GetRxPowerW(band);
      NS_LOG_DEBUG("PPDU belongs to same MU-MIMO transmission: muMimoPowerW="
                   << muMimoPowerW);
    }
    noiseInterferenceW -= muMimoPowerW;
    previous = j->first;
    if (previous > windowEnd) {
      NS_LOG_DEBUG("Stop: new previous="
                   << previous << " after time window end=" << windowEnd);
      break;
    }
  }
  double per = 1 - psr;
  return per;
}

double InterferenceHelper::CalculatePhyHeaderSectionPsr(
    Ptr<const Event> event, NiChangesPerBand *nis, uint16_t channelWidth,
    const WifiSpectrumBandInfo &band,
    PhyEntity::PhyHeaderSections phyHeaderSections) const {
  NS_LOG_FUNCTION(this << band);
  double psr = 1.0;
  auto niIt = nis->find(band)->second;
  auto j = niIt.begin();

  NS_ASSERT(!phyHeaderSections.empty());
  Time stopLastSection = Seconds(0);
  for (const auto &section : phyHeaderSections) {
    stopLastSection = Max(stopLastSection, section.second.first.second);
  }

  Time previous = j->first;
  NS_ABORT_IF(m_firstPowers.count(band) == 0);
  double noiseInterferenceW = m_firstPowers.at(band);
  double powerW = event->GetRxPowerW(band);
  while (++j != niIt.end()) {
    Time current = j->first;
    NS_LOG_DEBUG("previous= " << previous << ", current=" << current);
    NS_ASSERT(current >= previous);
    double snr = CalculateSnr(powerW, noiseInterferenceW, channelWidth, 1);
    for (const auto &section : phyHeaderSections) {
      Time start = section.second.first.first;
      Time stop = section.second.first.second;

      if (previous <= stop || current >= start) {
        Time duration = Min(stop, current) - Max(start, previous);
        if (duration.IsStrictlyPositive()) {
          psr *= CalculateChunkSuccessRate(snr, duration, section.second.second,
                                           event->GetPpdu()->GetTxVector(),
                                           section.first);
          NS_LOG_DEBUG("Current NI change in "
                       << section.first << " [" << start << ", " << stop
                       << "] for " << duration.As(Time::NS) << ": mode="
                       << section.second.second << ", psr=" << psr);
        }
      }
    }
    noiseInterferenceW = j->second.GetPower() - powerW;
    previous = j->first;
    if (previous > stopLastSection) {
      NS_LOG_DEBUG("Stop: new previous=" << previous
                                         << " after stop of last section="
                                         << stopLastSection);
      break;
    }
  }
  return psr;
}

double InterferenceHelper::CalculatePhyHeaderPer(
    Ptr<const Event> event, NiChangesPerBand *nis, uint16_t channelWidth,
    const WifiSpectrumBandInfo &band, WifiPpduField header) const {
  NS_LOG_FUNCTION(this << band << header);
  auto niIt = nis->find(band)->second;
  auto phyEntity = WifiPhy::GetStaticPhyEntity(
      event->GetPpdu()->GetTxVector().GetModulationClass());

  PhyEntity::PhyHeaderSections sections;
  for (const auto &section : phyEntity->GetPhyHeaderSections(
           event->GetPpdu()->GetTxVector(), niIt.begin()->first)) {
    if (section.first == header) {
      sections[header] = section.second;
    }
  }

  double psr = 1.0;
  if (!sections.empty()) {
    psr =
        CalculatePhyHeaderSectionPsr(event, nis, channelWidth, band, sections);
  }
  return 1 - psr;
}

PhyEntity::SnrPer InterferenceHelper::CalculatePayloadSnrPer(
    Ptr<Event> event, uint16_t channelWidth, const WifiSpectrumBandInfo &band,
    uint16_t staId, std::pair<Time, Time> relativeMpduStartStop) const {
  NS_LOG_FUNCTION(this << channelWidth << band << staId
                       << relativeMpduStartStop.first
                       << relativeMpduStartStop.second);
  NiChangesPerBand ni;
  double noiseInterferenceW = CalculateNoiseInterferenceW(event, ni, band);
  double snr =
      CalculateSnr(event->GetRxPowerW(band), noiseInterferenceW, channelWidth,
                   event->GetPpdu()->GetTxVector().GetNss(staId));

  double per = CalculatePayloadPer(event, channelWidth, &ni, band, staId,
                                   relativeMpduStartStop);

  return PhyEntity::SnrPer(snr, per);
}

double
InterferenceHelper::CalculateSnr(Ptr<Event> event, uint16_t channelWidth,
                                 uint8_t nss,
                                 const WifiSpectrumBandInfo &band) const {
  NiChangesPerBand ni;
  double noiseInterferenceW = CalculateNoiseInterferenceW(event, ni, band);
  double snr = CalculateSnr(event->GetRxPowerW(band), noiseInterferenceW,
                            channelWidth, nss);
  return snr;
}

PhyEntity::SnrPer InterferenceHelper::CalculatePhyHeaderSnrPer(
    Ptr<Event> event, uint16_t channelWidth, const WifiSpectrumBandInfo &band,
    WifiPpduField header) const {
  NS_LOG_FUNCTION(this << band << header);
  NiChangesPerBand ni;
  double noiseInterferenceW = CalculateNoiseInterferenceW(event, ni, band);
  double snr = CalculateSnr(event->GetRxPowerW(band), noiseInterferenceW,
                            channelWidth, 1);

  double per = CalculatePhyHeaderPer(event, &ni, channelWidth, band, header);

  return PhyEntity::SnrPer(snr, per);
}

InterferenceHelper::NiChanges::iterator
InterferenceHelper::GetNextPosition(Time moment,
                                    NiChangesPerBand::iterator niIt) {
  return niIt->second.upper_bound(moment);
}

InterferenceHelper::NiChanges::iterator
InterferenceHelper::GetPreviousPosition(Time moment,
                                        NiChangesPerBand::iterator niIt) {
  auto it = GetNextPosition(moment, niIt);
  --it;
  return it;
}

InterferenceHelper::NiChanges::iterator
InterferenceHelper::AddNiChangeEvent(Time moment, NiChange change,
                                     NiChangesPerBand::iterator niIt) {
  return niIt->second.insert(GetNextPosition(moment, niIt),
                             std::make_pair(moment, change));
}

void InterferenceHelper::NotifyRxStart() {
  NS_LOG_FUNCTION(this);
  m_rxing = true;
}

void InterferenceHelper::NotifyRxEnd(Time endTime,
                                     const FrequencyRange &freqRange) {
  NS_LOG_FUNCTION(this << endTime << freqRange);
  m_rxing = false;
  for (auto niIt = m_niChanges.begin(); niIt != m_niChanges.end(); ++niIt) {
    if (!IsBandInFrequencyRange(niIt->first, freqRange)) {
      continue;
    }
    NS_ASSERT(niIt->second.size() > 1);
    auto it = GetPreviousPosition(endTime, niIt);
    it--;
    m_firstPowers.find(niIt->first)->second = it->second.GetPower();
  }
}

bool InterferenceHelper::IsBandInFrequencyRange(
    const WifiSpectrumBandInfo &band, const FrequencyRange &freqRange) const {
  return ((band.frequencies.second > (freqRange.minFrequency * 1e6)) &&
          (band.frequencies.first < (freqRange.maxFrequency * 1e6)));
}

bool InterferenceHelper::IsSameMuMimoTransmission(
    Ptr<const Event> currentEvent, Ptr<const Event> otherEvent) const {
  if ((currentEvent->GetPpdu()->GetType() == WIFI_PPDU_TYPE_UL_MU) &&
      (otherEvent->GetPpdu()->GetType() == WIFI_PPDU_TYPE_UL_MU) &&
      (currentEvent->GetPpdu()->GetUid() == otherEvent->GetPpdu()->GetUid())) {
    const auto currentTxVector = currentEvent->GetPpdu()->GetTxVector();
    const auto otherTxVector = otherEvent->GetPpdu()->GetTxVector();
    NS_ASSERT(currentTxVector.GetHeMuUserInfoMap().size() == 1);
    NS_ASSERT(otherTxVector.GetHeMuUserInfoMap().size() == 1);
    const auto currentUserInfo = currentTxVector.GetHeMuUserInfoMap().cbegin();
    const auto otherUserInfo = otherTxVector.GetHeMuUserInfoMap().cbegin();
    return (currentUserInfo->second.ru == otherUserInfo->second.ru);
  }
  return false;
}

} // namespace ns3
