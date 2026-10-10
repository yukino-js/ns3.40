
#ifndef RADVD_INTERFACE_H
#define RADVD_INTERFACE_H

#include "radvd-prefix.h"

#include "ns3/nstime.h"
#include "ns3/simple-ref-count.h"

#include <list>

namespace ns3 {

class RadvdInterface : public SimpleRefCount<RadvdInterface> {
public:
  typedef std::list<Ptr<RadvdPrefix>> RadvdPrefixList;
  typedef std::list<Ptr<RadvdPrefix>>::iterator RadvdPrefixListI;
  typedef std::list<Ptr<RadvdPrefix>>::const_iterator RadvdPrefixListCI;

  RadvdInterface(uint32_t interface);

  RadvdInterface(uint32_t interface, uint32_t maxRtrAdvInterval,
                 uint32_t minRtrAdvInterval);

  ~RadvdInterface();

  uint32_t GetInterface() const;

  RadvdPrefixList GetPrefixes() const;

  void AddPrefix(Ptr<RadvdPrefix> routerPrefix);

  bool IsSendAdvert() const;

  void SetSendAdvert(bool sendAdvert);

  uint32_t GetMaxRtrAdvInterval() const;

  void SetMaxRtrAdvInterval(uint32_t maxRtrAdvInterval);

  uint32_t GetMinRtrAdvInterval() const;

  void SetMinRtrAdvInterval(uint32_t minRtrAdvInterval);

  uint32_t GetMinDelayBetweenRAs() const;

  void SetMinDelayBetweenRAs(uint32_t minDelayBetweenRAs);

  bool IsManagedFlag() const;

  void SetManagedFlag(bool managedFlag);

  bool IsOtherConfigFlag() const;

  void SetOtherConfigFlag(bool otherConfigFlag);

  uint32_t GetLinkMtu() const;

  void SetLinkMtu(uint32_t linkMtu);

  uint32_t GetReachableTime() const;

  void SetReachableTime(uint32_t reachableTime);

  uint32_t GetDefaultLifeTime() const;

  void SetDefaultLifeTime(uint32_t defaultLifeTime);

  uint32_t GetRetransTimer() const;

  void SetRetransTimer(uint32_t retransTimer);

  uint8_t GetCurHopLimit() const;

  void SetCurHopLimit(uint8_t curHopLimit);

  uint8_t GetDefaultPreference() const;

  void SetDefaultPreference(uint8_t defaultPreference);

  bool IsSourceLLAddress() const;

  void SetSourceLLAddress(bool sourceLLAddress);

  bool IsHomeAgentFlag() const;

  void SetHomeAgentFlag(bool homeAgentFlag);

  bool IsHomeAgentInfo() const;

  void SetHomeAgentInfo(bool homeAgentFlag);

  uint32_t GetHomeAgentLifeTime() const;

  void SetHomeAgentLifeTime(uint32_t homeAgentLifeTime);

  uint32_t GetHomeAgentPreference() const;

  void SetHomeAgentPreference(uint32_t homeAgentPreference);

  bool IsMobRtrSupportFlag() const;

  void SetMobRtrSupportFlag(bool mobRtrSupportFlag);

  bool IsIntervalOpt() const;

  void SetIntervalOpt(bool intervalOpt);

  Time GetLastRaTxTime();

  void SetLastRaTxTime(Time now);

  bool IsInitialRtrAdv() const;

private:
  uint32_t m_interface;

  RadvdPrefixList m_prefixes;

  bool m_sendAdvert;

  uint32_t m_maxRtrAdvInterval;

  uint32_t m_minRtrAdvInterval;

  uint32_t m_minDelayBetweenRAs;

  bool m_managedFlag;

  bool m_otherConfigFlag;

  uint32_t m_linkMtu;

  uint32_t m_reachableTime;

  uint32_t m_retransTimer;

  uint32_t m_curHopLimit;

  uint32_t m_defaultLifeTime;

  uint8_t m_defaultPreference;

  bool m_sourceLLAddress;

  bool m_homeAgentFlag;

  bool m_homeAgentInfo;

  uint32_t m_homeAgentLifeTime;

  uint32_t m_homeAgentPreference;

  bool m_mobRtrSupportFlag;

  bool m_intervalOpt;

  Time m_lastSendTime;

  uint8_t m_initialRtrAdvertisementsLeft;
};

} // namespace ns3

#endif
