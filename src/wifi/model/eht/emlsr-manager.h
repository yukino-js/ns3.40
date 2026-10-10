
#ifndef EMLSR_MANAGER_H
#define EMLSR_MANAGER_H

#include "ns3/ctrl-headers.h"
#include "ns3/mac48-address.h"
#include "ns3/mgt-headers.h"
#include "ns3/object.h"
#include "ns3/sta-wifi-mac.h"
#include "ns3/wifi-phy-operating-channel.h"

#include <map>
#include <optional>
#include <set>

namespace ns3 {

class EhtFrameExchangeManager;
class WifiMpdu;

class EmlsrManager : public Object {
public:
  static TypeId GetTypeId();
  EmlsrManager();
  ~EmlsrManager() override;

  void SetWifiMac(Ptr<StaWifiMac> mac);

  void SetTransitionTimeout(Time timeout);

  std::optional<Time> GetTransitionTimeout() const;

  void SetMainPhyId(uint8_t mainPhyId);

  uint8_t GetMainPhyId() const;

  void SetEmlsrLinks(const std::set<uint8_t> &linkIds);

  const std::set<uint8_t> &GetEmlsrLinks() const;

  void SetCamStateReset(bool enable);

  bool GetCamStateReset() const;

  void NotifyMgtFrameReceived(Ptr<const WifiMpdu> mpdu, uint8_t linkId);

  void NotifyIcfReceived(uint8_t linkId);

  void NotifyUlTxopStart(uint8_t linkId);

  void NotifyTxopEnd(uint8_t linkId);

protected:
  void DoDispose() override;

  Ptr<StaWifiMac> GetStaMac() const;

  Ptr<EhtFrameExchangeManager> GetEhtFem(uint8_t linkId) const;

  virtual uint8_t GetLinkToSendEmlOmn() = 0;

  virtual std::optional<uint8_t>
  ResendNotification(Ptr<const WifiMpdu> mpdu) = 0;

  const WifiPhyOperatingChannel &GetChannelForMainPhy(uint8_t linkId) const;

  const WifiPhyOperatingChannel &GetChannelForAuxPhy(uint8_t linkId) const;

  MgtEmlOmn GetEmlOmn();

  Time m_emlsrPaddingDelay;
  Time m_emlsrTransitionDelay;
  uint8_t m_mainPhyId;
  uint16_t m_auxPhyMaxWidth;

private:
  void ComputeOperatingChannels();

  void SendEmlOmn();

  virtual void DoNotifyMgtFrameReceived(Ptr<const WifiMpdu> mpdu,
                                        uint8_t linkId) = 0;

  void TxOk(Ptr<const WifiMpdu> mpdu);

  void TxDropped(WifiMacDropReason reason, Ptr<const WifiMpdu> mpdu);

  void ChangeEmlsrMode();

  void SwitchMainPhy(uint8_t linkId, bool noSwitchDelay);

  void ApplyMaxChannelWidthOnAuxPhys();

  virtual void NotifyEmlsrModeChanged() = 0;

  virtual void NotifyMainPhySwitch(uint8_t currLinkId, uint8_t nextLinkId) = 0;

  Ptr<StaWifiMac> m_staMac;
  std::optional<Time> m_emlsrTransitionTimeout;
  std::set<uint8_t> m_emlsrLinks;
  std::optional<std::set<uint8_t>> m_nextEmlsrLinks;
  Time m_lastAdvPaddingDelay;
  Time m_lastAdvTransitionDelay;
  EventId m_transitionTimeoutEvent;
  bool m_resetCamState;
  std::map<uint8_t, WifiPhyOperatingChannel> m_mainPhyChannels;
  std::map<uint8_t, WifiPhyOperatingChannel> m_auxPhyChannels;
};

} // namespace ns3

#endif
