
#ifndef CHANNEL_ACCESS_MANAGER_H
#define CHANNEL_ACCESS_MANAGER_H

#include "wifi-phy-common.h"
#include "wifi-phy-operating-channel.h"

#include "ns3/event-id.h"
#include "ns3/nstime.h"
#include "ns3/object.h"

#include <algorithm>
#include <map>
#include <memory>
#include <unordered_map>
#include <vector>

namespace ns3 {

class WifiPhy;
class PhyListener;
class Txop;
class FrameExchangeManager;

class ChannelAccessManager : public Object {
public:
  ChannelAccessManager();
  ~ChannelAccessManager() override;

  void SetupPhyListener(Ptr<WifiPhy> phy);
  void RemovePhyListener(Ptr<WifiPhy> phy);
  void DeactivatePhyListener(Ptr<WifiPhy> phy);
  void SetLinkId(uint8_t linkId);
  void SetupFrameExchangeManager(Ptr<FrameExchangeManager> feManager);

  void Add(Ptr<Txop> txop);

  bool NeedBackoffUponAccess(Ptr<Txop> txop);

  void RequestAccess(Ptr<Txop> txop);

  Time GetAccessGrantStart(bool ignoreNav = false) const;

  void DisableEdcaFor(Ptr<Txop> qosTxop, Time duration);

  uint16_t GetLargestIdlePrimaryChannel(Time interval, Time end);

  bool GetPer20MHzBusy(const std::set<uint8_t> &indices) const;

  void NotifyRxStartNow(Time duration);
  void NotifyRxEndOkNow();
  void NotifyRxEndErrorNow();
  void NotifyTxStartNow(Time duration);
  void NotifyCcaBusyStartNow(Time duration, WifiChannelListType channelType,
                             const std::vector<Time> &per20MhzDurations);
  void NotifySwitchingStartNow(PhyListener *phyListener, Time duration);
  void NotifySleepNow();
  void NotifyOffNow();
  void NotifyWakeupNow();
  void NotifyOnNow();
  void NotifyNavResetNow(Time duration);
  void NotifyNavStartNow(Time duration);
  void NotifyAckTimeoutStartNow(Time duration);
  void NotifyAckTimeoutResetNow();
  void NotifyCtsTimeoutStartNow(Time duration);
  void NotifyCtsTimeoutResetNow();

  void NotifyStartUsingOtherEmlsrLink();
  void NotifyStopUsingOtherEmlsrLink();

  bool IsBusy() const;

  void ResetState();
  void ResetBackoff(Ptr<Txop> txop);

  void NotifySwitchingEmlsrLink(Ptr<WifiPhy> phy,
                                const WifiPhyOperatingChannel &channel,
                                uint8_t linkId);

protected:
  void DoInitialize() override;
  void DoDispose() override;

private:
  PhyListener *GetPhyListener(Ptr<WifiPhy> phy) const;
  void InitLastBusyStructs();
  void UpdateBackoff();
  Time GetBackoffStartFor(Ptr<Txop> txop);
  Time GetBackoffEndFor(Ptr<Txop> txop);
  void UpdateLastIdlePeriod();

  void DoRestartAccessTimeoutIfNeeded();

  void AccessTimeout();
  void DoGrantDcfAccess();

  virtual Time GetSifs() const;
  virtual Time GetSlot() const;
  virtual Time GetEifsNoDifs() const;

  struct Timespan {
    Time start{0};
    Time end{0};
  };

  typedef std::vector<Ptr<Txop>> Txops;

  Txops m_txops;
  Time m_lastAckTimeoutEnd;
  Time m_lastCtsTimeoutEnd;
  Time m_lastNavEnd;
  Timespan m_lastRx;
  bool m_lastRxReceivedOk;
  Time m_lastTxEnd;
  std::map<WifiChannelListType, Time> m_lastBusyEnd;
  std::vector<Time> m_lastPer20MHzBusyEnd;
  std::map<WifiChannelListType, Timespan> m_lastIdle;
  Time m_lastSwitchingEnd;
  bool m_usingOtherEmlsrLink;
  Time m_lastUsingOtherEmlsrLinkEnd;
  bool m_sleeping;
  bool m_off;
  Time m_eifsNoDifs;
  EventId m_accessTimeout;

  struct EmlsrLinkSwitchInfo {
    WifiPhyOperatingChannel channel;
    uint8_t linkId;
  };

  std::unordered_map<Ptr<WifiPhy>, EmlsrLinkSwitchInfo> m_switchingEmlsrLinks;

  using PhyListenerMap =
      std::unordered_map<Ptr<WifiPhy>, std::unique_ptr<PhyListener>>;

  PhyListenerMap m_phyListeners;
  Ptr<WifiPhy> m_phy;
  Ptr<FrameExchangeManager> m_feManager;
  uint8_t m_linkId;
};

} // namespace ns3

#endif
