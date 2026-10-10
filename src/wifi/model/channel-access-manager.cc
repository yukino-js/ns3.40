
#include "channel-access-manager.h"

#include "txop.h"
#include "wifi-phy-listener.h"
#include "wifi-phy.h"

#include "ns3/eht-frame-exchange-manager.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

#include <sstream>

#undef NS_LOG_APPEND_CONTEXT
#define NS_LOG_APPEND_CONTEXT std::clog << "[link=" << +m_linkId << "] "

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("ChannelAccessManager");

class PhyListener : public ns3::WifiPhyListener {
public:
  PhyListener(ns3::ChannelAccessManager *cam) : m_cam(cam), m_active(true) {}

  ~PhyListener() override {}

  void SetActive(bool active) { m_active = active; }

  bool IsActive() const { return m_active; }

  void NotifyRxStart(Time duration) override {
    if (m_active) {
      m_cam->NotifyRxStartNow(duration);
    }
  }

  void NotifyRxEndOk() override {
    if (m_active) {
      m_cam->NotifyRxEndOkNow();
    }
  }

  void NotifyRxEndError() override {
    if (m_active) {
      m_cam->NotifyRxEndErrorNow();
    }
  }

  void NotifyTxStart(Time duration, double txPowerDbm) override {
    if (m_active) {
      m_cam->NotifyTxStartNow(duration);
    }
  }

  void NotifyCcaBusyStart(Time duration, WifiChannelListType channelType,
                          const std::vector<Time> &per20MhzDurations) override {
    if (m_active) {
      m_cam->NotifyCcaBusyStartNow(duration, channelType, per20MhzDurations);
    }
  }

  void NotifySwitchingStart(Time duration) override {
    m_cam->NotifySwitchingStartNow(this, duration);
  }

  void NotifySleep() override {
    if (m_active) {
      m_cam->NotifySleepNow();
    }
  }

  void NotifyOff() override {
    if (m_active) {
      m_cam->NotifyOffNow();
    }
  }

  void NotifyWakeup() override {
    if (m_active) {
      m_cam->NotifyWakeupNow();
    }
  }

  void NotifyOn() override {
    if (m_active) {
      m_cam->NotifyOnNow();
    }
  }

private:
  ns3::ChannelAccessManager *m_cam;
  bool m_active;
};

ChannelAccessManager::ChannelAccessManager()
    : m_lastAckTimeoutEnd(0), m_lastCtsTimeoutEnd(0), m_lastNavEnd(0),
      m_lastRx({MicroSeconds(0), MicroSeconds(0)}), m_lastRxReceivedOk(true),
      m_lastTxEnd(0), m_lastSwitchingEnd(0), m_usingOtherEmlsrLink(false),
      m_lastUsingOtherEmlsrLinkEnd(0), m_sleeping(false), m_off(false),
      m_linkId(0) {
  NS_LOG_FUNCTION(this);
  InitLastBusyStructs();
}

ChannelAccessManager::~ChannelAccessManager() { NS_LOG_FUNCTION(this); }

void ChannelAccessManager::DoInitialize() {
  NS_LOG_FUNCTION(this);
  InitLastBusyStructs();
}

void ChannelAccessManager::DoDispose() {
  NS_LOG_FUNCTION(this);
  for (Ptr<Txop> i : m_txops) {
    i->Dispose();
    i = nullptr;
  }
  m_phy = nullptr;
  m_feManager = nullptr;
  m_phyListeners.clear();
}

PhyListener *ChannelAccessManager::GetPhyListener(Ptr<WifiPhy> phy) const {
  if (auto listenerIt = m_phyListeners.find(phy);
      listenerIt != m_phyListeners.end()) {
    return listenerIt->second.get();
  }
  return nullptr;
}

void ChannelAccessManager::SetupPhyListener(Ptr<WifiPhy> phy) {
  NS_LOG_FUNCTION(this << phy);

  auto phyListener = GetPhyListener(phy);

  if (phyListener) {
    NS_ASSERT_MSG(
        !phyListener->IsActive(),
        "There is already an active listener registered for given PHY");
    NS_ASSERT_MSG(!m_phy,
                  "Cannot reactivate a listener if another PHY is active");
    phyListener->SetActive(true);
  } else {
    phyListener = new PhyListener(this);
    m_phyListeners.emplace(phy, phyListener);
    phy->RegisterListener(phyListener);
  }
  if (m_phy) {
    DeactivatePhyListener(m_phy);
  }
  m_phy = phy;
  InitLastBusyStructs();
  if (phy->IsStateSwitching()) {
    auto duration = phy->GetDelayUntilIdle();
    NS_LOG_DEBUG("switching start for " << duration);
    m_lastSwitchingEnd = Simulator::Now() + duration;
  }
}

void ChannelAccessManager::RemovePhyListener(Ptr<WifiPhy> phy) {
  NS_LOG_FUNCTION(this << phy);
  if (auto phyListener = GetPhyListener(phy)) {
    phy->UnregisterListener(phyListener);
    m_phyListeners.erase(phy);
    if (m_phy == phy) {
      m_phy = nullptr;
    }
  }
}

void ChannelAccessManager::DeactivatePhyListener(Ptr<WifiPhy> phy) {
  NS_LOG_FUNCTION(this << phy);
  if (auto listener = GetPhyListener(phy)) {
    listener->SetActive(false);
  }
  if (m_phy == phy) {
    m_phy = nullptr;
  }
}

void ChannelAccessManager::NotifySwitchingEmlsrLink(
    Ptr<WifiPhy> phy, const WifiPhyOperatingChannel &channel, uint8_t linkId) {
  NS_LOG_FUNCTION(this << phy << channel << linkId);
  NS_ASSERT_MSG(m_switchingEmlsrLinks.count(phy) == 0,
                "The given PHY is already expected to switch channel");
  m_switchingEmlsrLinks.emplace(phy, EmlsrLinkSwitchInfo{channel, linkId});
}

void ChannelAccessManager::SetLinkId(uint8_t linkId) {
  NS_LOG_FUNCTION(this << +linkId);
  m_linkId = linkId;
}

void ChannelAccessManager::SetupFrameExchangeManager(
    Ptr<FrameExchangeManager> feManager) {
  NS_LOG_FUNCTION(this << feManager);
  m_feManager = feManager;
  m_feManager->SetChannelAccessManager(this);
}

Time ChannelAccessManager::GetSlot() const { return m_phy->GetSlot(); }

Time ChannelAccessManager::GetSifs() const { return m_phy->GetSifs(); }

Time ChannelAccessManager::GetEifsNoDifs() const {
  return m_phy->GetSifs() + m_phy->GetAckTxTime();
}

void ChannelAccessManager::Add(Ptr<Txop> txop) {
  NS_LOG_FUNCTION(this << txop);
  m_txops.push_back(txop);
}

void ChannelAccessManager::InitLastBusyStructs() {
  NS_LOG_FUNCTION(this);
  Time now = Simulator::Now();
  m_lastBusyEnd.clear();
  m_lastPer20MHzBusyEnd.clear();
  m_lastIdle.clear();
  m_lastBusyEnd[WIFI_CHANLIST_PRIMARY] = now;
  m_lastIdle[WIFI_CHANLIST_PRIMARY] = {now, now};

  if (!m_phy || !m_phy->GetOperatingChannel().IsOfdm()) {
    return;
  }

  uint16_t width = m_phy->GetChannelWidth();

  if (width >= 40) {
    m_lastBusyEnd[WIFI_CHANLIST_SECONDARY] = now;
    m_lastIdle[WIFI_CHANLIST_SECONDARY] = {now, now};
  }
  if (width >= 80) {
    m_lastBusyEnd[WIFI_CHANLIST_SECONDARY40] = now;
    m_lastIdle[WIFI_CHANLIST_SECONDARY40] = {now, now};
  }
  if (width >= 160) {
    m_lastBusyEnd[WIFI_CHANLIST_SECONDARY80] = now;
    m_lastIdle[WIFI_CHANLIST_SECONDARY80] = {now, now};
  }

  if (m_phy->GetStandard() >= WIFI_STANDARD_80211ax && width > 20) {
    m_lastPer20MHzBusyEnd.assign(width / 20, now);
  }
}

bool ChannelAccessManager::IsBusy() const {
  NS_LOG_FUNCTION(this);
  Time now = Simulator::Now();
  return (m_lastRx.end > now) || (m_lastTxEnd > now) || (m_lastNavEnd > now) ||
         (m_lastBusyEnd.at(WIFI_CHANLIST_PRIMARY) > now);
}

bool ChannelAccessManager::NeedBackoffUponAccess(Ptr<Txop> txop) {
  NS_LOG_FUNCTION(this << txop);

  if (m_sleeping || m_off || m_usingOtherEmlsrLink) {
    return false;
  }

  UpdateBackoff();

  if (!txop->HasFramesToTransmit(m_linkId) &&
      txop->GetAccessStatus(m_linkId) != Txop::GRANTED &&
      txop->GetBackoffSlots(m_linkId) == 0) {
    if (!IsBusy()) {
      Time delay = (txop->IsQosTxop()
                        ? Seconds(0)
                        : GetSifs() + txop->GetAifsn(m_linkId) * GetSlot());
      txop->UpdateBackoffSlotsNow(0, Simulator::Now() + delay, m_linkId);
    } else {
      return true;
    }
  }
  return false;
}

void ChannelAccessManager::RequestAccess(Ptr<Txop> txop) {
  NS_LOG_FUNCTION(this << txop);
  if (m_phy) {
    m_phy->NotifyChannelAccessRequested();
  }
  if (m_usingOtherEmlsrLink) {
    NS_LOG_DEBUG(
        "Channel access cannot be requested while using another EMLSR link");
    return;
  }
  if (m_sleeping || m_off) {
    return;
  }
  Time accessGrantStart =
      GetAccessGrantStart() + (txop->GetAifsn(m_linkId) * GetSlot());

  if (txop->IsQosTxop() && txop->GetBackoffStart(m_linkId) > accessGrantStart) {
    Time diff = txop->GetBackoffStart(m_linkId) - accessGrantStart;
    uint32_t nIntSlots = (diff / GetSlot()).GetHigh() + 1;
    txop->UpdateBackoffSlotsNow(0, accessGrantStart + (nIntSlots * GetSlot()),
                                m_linkId);
  }

  UpdateBackoff();
  NS_ASSERT(txop->GetAccessStatus(m_linkId) != Txop::REQUESTED);
  txop->NotifyAccessRequested(m_linkId);
  DoGrantDcfAccess();
  DoRestartAccessTimeoutIfNeeded();
}

void ChannelAccessManager::DoGrantDcfAccess() {
  NS_LOG_FUNCTION(this);
  uint32_t k = 0;
  Time now = Simulator::Now();
  for (auto i = m_txops.begin(); i != m_txops.end(); k++) {
    Ptr<Txop> txop = *i;
    if (txop->GetAccessStatus(m_linkId) == Txop::REQUESTED &&
        (!txop->IsQosTxop() ||
         !StaticCast<QosTxop>(txop)->EdcaDisabled(m_linkId)) &&
        GetBackoffEndFor(txop) <= now) {
      NS_LOG_DEBUG(
          "dcf " << k
                 << " needs access. backoff expired. access granted. slots="
                 << txop->GetBackoffSlots(m_linkId));
      i++;
      k++;
      std::vector<Ptr<Txop>> internalCollisionTxops;
      for (auto j = i; j != m_txops.end(); j++, k++) {
        Ptr<Txop> otherTxop = *j;
        if (otherTxop->GetAccessStatus(m_linkId) == Txop::REQUESTED &&
            GetBackoffEndFor(otherTxop) <= now) {
          NS_LOG_DEBUG(
              "dcf "
              << k
              << " needs access. backoff expired. internal collision. slots="
              << otherTxop->GetBackoffSlots(m_linkId));
          internalCollisionTxops.push_back(otherTxop);
        }
      }

      NS_ASSERT(m_feManager);
      auto interval = (m_phy->GetPhyBand() == WIFI_PHY_BAND_2_4GHZ)
                          ? GetSifs() + 2 * GetSlot()
                          : m_phy->GetPifs();
      auto width = (m_phy->GetOperatingChannel().IsOfdm() &&
                    m_phy->GetChannelWidth() > 20)
                       ? GetLargestIdlePrimaryChannel(interval, now)
                       : m_phy->GetChannelWidth();
      if (m_feManager->StartTransmission(txop, width)) {
        for (auto &collidingTxop : internalCollisionTxops) {
          m_feManager->NotifyInternalCollision(collidingTxop);
        }
        break;
      } else {
        i--;
        k = std::distance(m_txops.begin(), i);
      }
    }
    i++;
  }
}

void ChannelAccessManager::AccessTimeout() {
  NS_LOG_FUNCTION(this);
  UpdateBackoff();
  DoGrantDcfAccess();
  DoRestartAccessTimeoutIfNeeded();
}

Time ChannelAccessManager::GetAccessGrantStart(bool ignoreNav) const {
  NS_LOG_FUNCTION(this);
  const Time &sifs = GetSifs();
  Time rxAccessStart = m_lastRx.end + sifs;
  if ((m_lastRx.end <= Simulator::Now()) && !m_lastRxReceivedOk) {
    rxAccessStart += GetEifsNoDifs();
  }
  Time busyAccessStart = m_lastBusyEnd.at(WIFI_CHANLIST_PRIMARY) + sifs;
  Time txAccessStart = m_lastTxEnd + sifs;
  Time navAccessStart = m_lastNavEnd + sifs;
  Time ackTimeoutAccessStart = m_lastAckTimeoutEnd + sifs;
  Time ctsTimeoutAccessStart = m_lastCtsTimeoutEnd + sifs;
  Time switchingAccessStart = m_lastSwitchingEnd + sifs;
  Time usingOtherEmlsrLinkAccessStart =
      (m_usingOtherEmlsrLink ? Simulator::Now()
                             : m_lastUsingOtherEmlsrLinkEnd) +
      sifs;
  Time accessGrantedStart;
  if (ignoreNav) {
    accessGrantedStart =
        std::max({rxAccessStart, busyAccessStart, txAccessStart,
                  ackTimeoutAccessStart, ctsTimeoutAccessStart,
                  usingOtherEmlsrLinkAccessStart, switchingAccessStart});
  } else {
    accessGrantedStart =
        std::max({rxAccessStart, busyAccessStart, txAccessStart, navAccessStart,
                  ackTimeoutAccessStart, ctsTimeoutAccessStart,
                  usingOtherEmlsrLinkAccessStart, switchingAccessStart});
  }
  std::stringstream ss;
  if (m_usingOtherEmlsrLink) {
    ss << ", using other EMLSR link access start="
       << usingOtherEmlsrLinkAccessStart;
  }
  NS_LOG_INFO("access grant start="
              << accessGrantedStart << ", rx access start=" << rxAccessStart
              << ", busy access start=" << busyAccessStart
              << ", tx access start=" << txAccessStart
              << ", nav access start=" << navAccessStart << ss.str());
  return accessGrantedStart;
}

Time ChannelAccessManager::GetBackoffStartFor(Ptr<Txop> txop) {
  NS_LOG_FUNCTION(this << txop);
  Time mostRecentEvent = std::max(
      {txop->GetBackoffStart(m_linkId),
       GetAccessGrantStart() + (txop->GetAifsn(m_linkId) * GetSlot())});
  NS_LOG_DEBUG("Backoff start: " << mostRecentEvent.As(Time::US));

  return mostRecentEvent;
}

Time ChannelAccessManager::GetBackoffEndFor(Ptr<Txop> txop) {
  NS_LOG_FUNCTION(this << txop);
  Time backoffEnd =
      GetBackoffStartFor(txop) + (txop->GetBackoffSlots(m_linkId) * GetSlot());
  NS_LOG_DEBUG("Backoff end: " << backoffEnd.As(Time::US));

  return backoffEnd;
}

void ChannelAccessManager::UpdateBackoff() {
  NS_LOG_FUNCTION(this);
  uint32_t k = 0;
  for (auto txop : m_txops) {
    Time backoffStart = GetBackoffStartFor(txop);
    if (backoffStart <= Simulator::Now()) {
      uint32_t nIntSlots =
          ((Simulator::Now() - backoffStart) / GetSlot()).GetHigh();
      if (txop->IsQosTxop()) {
        nIntSlots++;
      }
      uint32_t n = std::min(nIntSlots, txop->GetBackoffSlots(m_linkId));
      NS_LOG_DEBUG("dcf " << k << " dec backoff slots=" << n);
      Time backoffUpdateBound = backoffStart + (n * GetSlot());
      txop->UpdateBackoffSlotsNow(n, backoffUpdateBound, m_linkId);
    }
    ++k;
  }
}

void ChannelAccessManager::DoRestartAccessTimeoutIfNeeded() {
  NS_LOG_FUNCTION(this);
  bool accessTimeoutNeeded = false;
  Time expectedBackoffEnd = Simulator::GetMaximumSimulationTime();
  for (auto txop : m_txops) {
    if (txop->GetAccessStatus(m_linkId) == Txop::REQUESTED) {
      Time tmp = GetBackoffEndFor(txop);
      if (tmp > Simulator::Now()) {
        accessTimeoutNeeded = true;
        expectedBackoffEnd = std::min(expectedBackoffEnd, tmp);
      }
    }
  }
  NS_LOG_DEBUG("Access timeout needed: " << accessTimeoutNeeded);
  if (accessTimeoutNeeded) {
    NS_LOG_DEBUG("expected backoff end=" << expectedBackoffEnd);
    Time expectedBackoffDelay = expectedBackoffEnd - Simulator::Now();
    if (m_accessTimeout.IsRunning() &&
        Simulator::GetDelayLeft(m_accessTimeout) > expectedBackoffDelay) {
      m_accessTimeout.Cancel();
    }
    if (m_accessTimeout.IsExpired()) {
      m_accessTimeout = Simulator::Schedule(
          expectedBackoffDelay, &ChannelAccessManager::AccessTimeout, this);
    }
  }
}

uint16_t ChannelAccessManager::GetLargestIdlePrimaryChannel(Time interval,
                                                            Time end) {
  NS_LOG_FUNCTION(this << interval.As(Time::US) << end.As(Time::S));

  UpdateLastIdlePeriod();

  uint16_t width = 0;

  for (const auto &lastIdle : m_lastIdle) {
    if (lastIdle.second.start <= end - interval && lastIdle.second.end >= end) {
      width = (width == 0) ? 20 : (2 * width);
    } else {
      break;
    }
  }
  return width;
}

bool ChannelAccessManager::GetPer20MHzBusy(
    const std::set<uint8_t> &indices) const {
  const auto now = Simulator::Now();

  if (m_phy->GetChannelWidth() < 40) {
    NS_ASSERT_MSG(indices.size() == 1 && *indices.cbegin() == 0,
                  "Index 0 only can be specified if the channel width is less "
                  "than 40 MHz");
    return m_lastBusyEnd.at(WIFI_CHANLIST_PRIMARY) > now;
  }

  for (const auto index : indices) {
    NS_ASSERT(index < m_lastPer20MHzBusyEnd.size());
    if (m_lastPer20MHzBusyEnd.at(index) > now) {
      NS_LOG_DEBUG("20 MHz channel with index " << +index << " is busy");
      return true;
    }
  }
  return false;
}

void ChannelAccessManager::DisableEdcaFor(Ptr<Txop> qosTxop, Time duration) {
  NS_LOG_FUNCTION(this << qosTxop << duration);
  NS_ASSERT(qosTxop->IsQosTxop());
  UpdateBackoff();
  Time resume = Simulator::Now() + duration;
  NS_LOG_DEBUG("Backoff will resume at time "
               << resume << " with " << qosTxop->GetBackoffSlots(m_linkId)
               << " remaining slot(s)");
  qosTxop->UpdateBackoffSlotsNow(0, resume, m_linkId);
  DoRestartAccessTimeoutIfNeeded();
}

void ChannelAccessManager::NotifyRxStartNow(Time duration) {
  NS_LOG_FUNCTION(this << duration);
  NS_LOG_DEBUG("rx start for=" << duration);
  UpdateBackoff();
  UpdateLastIdlePeriod();
  m_lastRx.start = Simulator::Now();
  m_lastRx.end = m_lastRx.start + duration;
  m_lastRxReceivedOk = true;
}

void ChannelAccessManager::NotifyRxEndOkNow() {
  NS_LOG_FUNCTION(this);
  NS_LOG_DEBUG("rx end ok");
  m_lastRx.end = Simulator::Now();
  m_lastRxReceivedOk = true;
}

void ChannelAccessManager::NotifyRxEndErrorNow() {
  NS_LOG_FUNCTION(this);
  NS_LOG_DEBUG("rx end error");
  m_lastRx.end = Simulator::Now();
  m_lastRxReceivedOk = false;
}

void ChannelAccessManager::NotifyTxStartNow(Time duration) {
  NS_LOG_FUNCTION(this << duration);
  m_lastRxReceivedOk = true;
  Time now = Simulator::Now();
  if (m_lastRx.end > now) {
    NS_ASSERT(now - m_lastRx.start <= GetSifs());
    m_lastRx.end = now;
  } else {
    UpdateLastIdlePeriod();
  }
  NS_LOG_DEBUG("tx start for " << duration);
  UpdateBackoff();
  m_lastTxEnd = now + duration;
}

void ChannelAccessManager::NotifyCcaBusyStartNow(
    Time duration, WifiChannelListType channelType,
    const std::vector<Time> &per20MhzDurations) {
  NS_LOG_FUNCTION(this << duration << channelType);
  UpdateBackoff();
  UpdateLastIdlePeriod();
  auto lastBusyEndIt = m_lastBusyEnd.find(channelType);
  NS_ASSERT(lastBusyEndIt != m_lastBusyEnd.end());
  Time now = Simulator::Now();
  lastBusyEndIt->second = now + duration;
  NS_ASSERT_MSG(per20MhzDurations.size() == m_lastPer20MHzBusyEnd.size(),
                "Size of received vector ("
                    << per20MhzDurations.size()
                    << ") differs from the expected size ("
                    << m_lastPer20MHzBusyEnd.size() << ")");
  for (std::size_t chIdx = 0; chIdx < per20MhzDurations.size(); ++chIdx) {
    if (per20MhzDurations[chIdx].IsStrictlyPositive()) {
      m_lastPer20MHzBusyEnd[chIdx] = now + per20MhzDurations[chIdx];
    }
  }
}

void ChannelAccessManager::NotifySwitchingStartNow(PhyListener *phyListener,
                                                   Time duration) {
  NS_LOG_FUNCTION(this << phyListener << duration);

  Time now = Simulator::Now();
  NS_ASSERT(m_lastTxEnd <= now);
  NS_ASSERT(m_lastSwitchingEnd <= now);

  if (phyListener) {

    for (const auto &[phyRef, listener] : m_phyListeners) {
      Ptr<WifiPhy> phy = phyRef;
      auto emlsrInfoIt = m_switchingEmlsrLinks.find(phy);

      if (listener.get() == phyListener &&
          emlsrInfoIt != m_switchingEmlsrLinks.cend() &&
          phy->GetOperatingChannel() == emlsrInfoIt->second.channel) {
        RemovePhyListener(phy);
        auto ehtFem = DynamicCast<EhtFrameExchangeManager>(m_feManager);
        NS_ASSERT(ehtFem);
        ehtFem->NotifySwitchingEmlsrLink(phy, emlsrInfoIt->second.linkId,
                                         duration);
        m_switchingEmlsrLinks.erase(emlsrInfoIt);
        return;
      }
    }
  }

  ResetState();

  for (const auto &txop : m_txops) {
    ResetBackoff(txop);
  }

  m_feManager->NotifySwitchingStartNow(duration);

  NS_LOG_DEBUG("switching start for " << duration);
  m_lastSwitchingEnd = now + duration;
}

void ChannelAccessManager::ResetState() {
  NS_LOG_FUNCTION(this);

  Time now = Simulator::Now();
  m_lastRxReceivedOk = true;
  UpdateLastIdlePeriod();
  m_lastRx.end = std::min(m_lastRx.end, now);
  m_lastNavEnd = std::min(m_lastNavEnd, now);
  m_lastAckTimeoutEnd = std::min(m_lastAckTimeoutEnd, now);
  m_lastCtsTimeoutEnd = std::min(m_lastCtsTimeoutEnd, now);

  InitLastBusyStructs();

  if (m_accessTimeout.IsRunning()) {
    m_accessTimeout.Cancel();
  }
}

void ChannelAccessManager::ResetBackoff(Ptr<Txop> txop) {
  NS_LOG_FUNCTION(this << txop);

  uint32_t remainingSlots = txop->GetBackoffSlots(m_linkId);
  if (remainingSlots > 0) {
    txop->UpdateBackoffSlotsNow(remainingSlots, Simulator::Now(), m_linkId);
    NS_ASSERT(txop->GetBackoffSlots(m_linkId) == 0);
  }
  txop->ResetCw(m_linkId);
  txop->GetLink(m_linkId).access = Txop::NOT_REQUESTED;
}

void ChannelAccessManager::NotifySleepNow() {
  NS_LOG_FUNCTION(this);
  m_sleeping = true;
  if (m_accessTimeout.IsRunning()) {
    m_accessTimeout.Cancel();
  }

  for (auto txop : m_txops) {
    txop->NotifySleep(m_linkId);
  }
}

void ChannelAccessManager::NotifyOffNow() {
  NS_LOG_FUNCTION(this);
  m_off = true;
  if (m_accessTimeout.IsRunning()) {
    m_accessTimeout.Cancel();
  }

  for (auto txop : m_txops) {
    txop->NotifyOff();
  }
}

void ChannelAccessManager::NotifyWakeupNow() {
  NS_LOG_FUNCTION(this);
  m_sleeping = false;
  for (auto txop : m_txops) {
    ResetBackoff(txop);
    txop->NotifyWakeUp(m_linkId);
  }
}

void ChannelAccessManager::NotifyOnNow() {
  NS_LOG_FUNCTION(this);
  m_off = false;
  for (auto txop : m_txops) {
    ResetBackoff(txop);
    txop->NotifyOn();
  }
}

void ChannelAccessManager::NotifyNavResetNow(Time duration) {
  NS_LOG_FUNCTION(this << duration);
  NS_LOG_DEBUG("nav reset for=" << duration);
  UpdateBackoff();
  m_lastNavEnd = Simulator::Now() + duration;
  DoRestartAccessTimeoutIfNeeded();
}

void ChannelAccessManager::NotifyNavStartNow(Time duration) {
  NS_LOG_FUNCTION(this << duration);
  NS_LOG_DEBUG("nav start for=" << duration);
  UpdateBackoff();
  m_lastNavEnd = std::max(m_lastNavEnd, Simulator::Now() + duration);
}

void ChannelAccessManager::NotifyAckTimeoutStartNow(Time duration) {
  NS_LOG_FUNCTION(this << duration);
  NS_ASSERT(m_lastAckTimeoutEnd < Simulator::Now());
  m_lastAckTimeoutEnd = Simulator::Now() + duration;
}

void ChannelAccessManager::NotifyAckTimeoutResetNow() {
  NS_LOG_FUNCTION(this);
  m_lastAckTimeoutEnd = Simulator::Now();
  DoRestartAccessTimeoutIfNeeded();
}

void ChannelAccessManager::NotifyCtsTimeoutStartNow(Time duration) {
  NS_LOG_FUNCTION(this << duration);
  m_lastCtsTimeoutEnd = Simulator::Now() + duration;
}

void ChannelAccessManager::NotifyCtsTimeoutResetNow() {
  NS_LOG_FUNCTION(this);
  m_lastCtsTimeoutEnd = Simulator::Now();
  DoRestartAccessTimeoutIfNeeded();
}

void ChannelAccessManager::NotifyStartUsingOtherEmlsrLink() {
  NS_LOG_FUNCTION(this);
  if (m_phy) {
    UpdateBackoff();
  }
  if (m_accessTimeout.IsRunning()) {
    m_accessTimeout.Cancel();
  }
  m_usingOtherEmlsrLink = true;
  UpdateLastIdlePeriod();
}

void ChannelAccessManager::NotifyStopUsingOtherEmlsrLink() {
  NS_LOG_FUNCTION(this);
  m_usingOtherEmlsrLink = false;
  m_lastUsingOtherEmlsrLinkEnd = Simulator::Now();
  DoRestartAccessTimeoutIfNeeded();
}

void ChannelAccessManager::UpdateLastIdlePeriod() {
  NS_LOG_FUNCTION(this);
  Time idleStart = std::max({m_lastTxEnd, m_lastRx.end, m_lastSwitchingEnd,
                             m_lastUsingOtherEmlsrLinkEnd});
  Time now = Simulator::Now();

  if (idleStart >= now) {
    return;
  }

  for (const auto &busyEnd : m_lastBusyEnd) {
    if (busyEnd.second < now) {
      auto lastIdleIt = m_lastIdle.find(busyEnd.first);
      NS_ASSERT(lastIdleIt != m_lastIdle.end());
      lastIdleIt->second = {std::max(idleStart, busyEnd.second), now};
      NS_LOG_DEBUG("New idle period ("
                   << lastIdleIt->second.start.As(Time::S) << ", "
                   << lastIdleIt->second.end.As(Time::S) << ") on channel "
                   << lastIdleIt->first);
    }
  }
}

} // namespace ns3
