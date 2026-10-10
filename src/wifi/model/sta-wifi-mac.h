
#ifndef STA_WIFI_MAC_H
#define STA_WIFI_MAC_H

#include "mgt-headers.h"
#include "wifi-mac.h"

#include <set>
#include <variant>

class TwoLevelAggregationTest;
class AmpduAggregationTest;
class HeAggregationTest;
class MultiLinkOperationsTestBase;

namespace ns3 {

class SupportedRates;
class CapabilityInformation;
class RandomVariableStream;
class WifiAssocManager;
class EmlsrManager;

enum class WifiScanType : uint8_t { ACTIVE = 0, PASSIVE };

struct WifiScanParams {
  struct Channel {
    uint16_t number{0};
    WifiPhyBand band{WIFI_PHY_BAND_UNSPECIFIED};
  };

  using ChannelList = std::list<Channel>;

  WifiScanType type;
  Ssid ssid;
  std::vector<ChannelList> channelList;
  Time probeDelay;
  Time minChannelTime;
  Time maxChannelTime;
};

enum WifiPowerManagementMode : uint8_t {
  WIFI_PM_ACTIVE = 0,
  WIFI_PM_SWITCHING_TO_PS,
  WIFI_PM_POWERSAVE,
  WIFI_PM_SWITCHING_TO_ACTIVE
};

class StaWifiMac : public WifiMac {
public:
  friend class ::TwoLevelAggregationTest;
  friend class ::AmpduAggregationTest;
  friend class ::HeAggregationTest;
  friend class ::MultiLinkOperationsTestBase;

  using MgtFrameType = std::variant<MgtBeaconHeader, MgtProbeResponseHeader,
                                    MgtAssocResponseHeader>;

  struct ApInfo {
    struct SetupLinksInfo {
      uint8_t localLinkId;
      uint8_t apLinkId;
      Mac48Address bssid;
    };

    Mac48Address m_bssid;
    Mac48Address m_apAddr;
    double m_snr;
    MgtFrameType m_frame;
    WifiScanParams::Channel m_channel;
    uint8_t m_linkId;
    std::list<SetupLinksInfo> m_setupLinks;
  };

  static TypeId GetTypeId();

  StaWifiMac();
  ~StaWifiMac() override;

  void Enqueue(Ptr<Packet> packet, Mac48Address to) override;
  bool CanForwardPacketsTo(Mac48Address to) const override;

  void SetWifiPhys(const std::vector<Ptr<WifiPhy>> &phys) override;

  void SetAssocManager(Ptr<WifiAssocManager> assocManager);

  void SetEmlsrManager(Ptr<EmlsrManager> emlsrManager);

  Ptr<EmlsrManager> GetEmlsrManager() const;

  void SendProbeRequest(uint8_t linkId);

  void ScanningTimeout(const std::optional<ApInfo> &bestAp);

  bool IsAssociated() const;

  std::set<uint8_t> GetSetupLinkIds() const;

  uint16_t GetAssociationId() const;

  void SetPowerSaveMode(const std::pair<bool, uint8_t> &enableLinkIdPair);

  WifiPowerManagementMode GetPmMode(uint8_t linkId) const;

  void SetPmModeAfterAssociation(uint8_t linkId);

  void TxOk(Ptr<const WifiMpdu> mpdu);

  void NotifyChannelSwitching(uint8_t linkId) override;

  void NotifyEmlsrModeChanged(const std::set<uint8_t> &linkIds);

  bool IsEmlsrLink(uint8_t linkId) const;

  void NotifySwitchingEmlsrLink(Ptr<WifiPhy> phy, uint8_t linkId);

  void BlockTxOnLink(uint8_t linkId, WifiQueueBlockedReason reason);

  void UnblockTxOnLink(uint8_t linkId, WifiQueueBlockedReason reason);

  int64_t AssignStreams(int64_t stream);

protected:
  struct StaLinkEntity : public WifiMac::LinkEntity {
    ~StaLinkEntity() override;

    bool sendAssocReq;
    std::optional<Mac48Address> bssid;
    EventId beaconWatchdog;
    Time beaconWatchdogEnd{0};
    WifiPowerManagementMode pmMode{WIFI_PM_ACTIVE};
    bool emlsrEnabled{false};
  };

  StaLinkEntity &GetLink(uint8_t linkId) const;

  StaLinkEntity &
  GetStaLink(const std::unique_ptr<WifiMac::LinkEntity> &link) const;

public:
  enum MacState {
    ASSOCIATED,
    SCANNING,
    WAIT_ASSOC_RESP,
    UNASSOCIATED,
    REFUSED
  };

private:
  void SetActiveProbing(bool enable);
  bool GetActiveProbing() const;

  bool CheckSupportedRates(
      std::variant<MgtBeaconHeader, MgtProbeResponseHeader> frame,
      uint8_t linkId);

  void Receive(Ptr<const WifiMpdu> mpdu, uint8_t linkId) override;
  std::unique_ptr<LinkEntity> CreateLinkEntity() const override;
  Mac48Address DoGetLocalAddress(const Mac48Address &remoteAddr) const override;

  void ReceiveBeacon(Ptr<const WifiMpdu> mpdu, uint8_t linkId);

  void ReceiveProbeResp(Ptr<const WifiMpdu> mpdu, uint8_t linkId);

  void ReceiveAssocResp(Ptr<const WifiMpdu> mpdu, uint8_t linkId);

  void UpdateApInfo(const MgtFrameType &frame, const Mac48Address &apAddr,
                    const Mac48Address &bssid, uint8_t linkId);

  std::variant<MgtAssocRequestHeader, MgtReassocRequestHeader>
  GetAssociationRequest(bool isReassoc, uint8_t linkId) const;

  void SendAssociationRequest(bool isReassoc);
  void TryToEnsureAssociated();
  void AssocRequestTimeout();
  void StartScanning();
  bool IsWaitAssocResp() const;
  void MissedBeacons(uint8_t linkId);
  void RestartBeaconWatchdog(Time delay, uint8_t linkId);
  void Disassociated(uint8_t linkId);
  AllSupportedRates GetSupportedRates(uint8_t linkId) const;
  MultiLinkElement GetMultiLinkElement(bool isReassoc, uint8_t linkId) const;

  std::vector<TidToLinkMapping>
  GetTidToLinkMappingElements(uint8_t apNegSupport);

  void SetState(MacState value);

  struct EdcaParams {
    AcIndex ac;
    uint32_t cwMin;
    uint32_t cwMax;
    uint8_t aifsn;
    Time txopLimit;
  };

  void SetEdcaParameters(const EdcaParams &params, uint8_t linkId);

  struct MuEdcaParams {
    AcIndex ac;
    uint32_t cwMin;
    uint32_t cwMax;
    uint8_t aifsn;
    Time muEdcaTimer;
  };

  void SetMuEdcaParameters(const MuEdcaParams &params, uint8_t linkId);

  CapabilityInformation GetCapabilities(uint8_t linkId) const;

  void PhyCapabilitiesChanged();

  WifiScanParams::Channel GetCurrentChannel(uint8_t linkId) const;

  void DoInitialize() override;
  void DoDispose() override;

  MacState m_state;
  uint16_t m_aid;
  Ptr<WifiAssocManager> m_assocManager;
  Ptr<EmlsrManager> m_emlsrManager;
  Time m_waitBeaconTimeout;
  Time m_probeRequestTimeout;
  Time m_assocRequestTimeout;
  EventId m_assocRequestEvent;
  uint32_t m_maxMissedBeacons;
  bool m_activeProbing;
  Ptr<RandomVariableStream> m_probeDelay;
  Time m_pmModeSwitchTimeout;

  WifiTidLinkMapping m_dlTidLinkMappingInAssocReq;
  WifiTidLinkMapping m_ulTidLinkMappingInAssocReq;

  TracedCallback<Mac48Address> m_assocLogger;
  TracedCallback<uint8_t, Mac48Address> m_setupCompleted;
  TracedCallback<Mac48Address> m_deAssocLogger;
  TracedCallback<uint8_t, Mac48Address> m_setupCanceled;
  TracedCallback<Time> m_beaconArrival;
  TracedCallback<ApInfo> m_beaconInfo;

  using LinkSetupCallback = void (*)(uint8_t, Mac48Address);
};

std::ostream &operator<<(std::ostream &os, const StaWifiMac::ApInfo &apInfo);

} // namespace ns3

#endif
