
#ifndef WIFI_MAC_H
#define WIFI_MAC_H

#include "qos-utils.h"
#include "ssid.h"
#include "wifi-mac-queue-scheduler.h"
#include "wifi-remote-station-manager.h"
#include "wifi-standards.h"

#include <functional>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <unordered_map>
#include <vector>

namespace ns3 {

class Txop;
class WifiNetDevice;
class QosTxop;
class WifiPsdu;
class MacRxMiddle;
class MacTxMiddle;
class WifiMacQueue;
class WifiMpdu;
class HtConfiguration;
class VhtConfiguration;
class HeConfiguration;
class EhtConfiguration;
class FrameExchangeManager;
class ChannelAccessManager;
class ExtendedCapabilities;
class OriginatorBlockAckAgreement;
class RecipientBlockAckAgreement;

enum TypeOfStation { STA, AP, ADHOC_STA, MESH, OCB };

enum WifiMacDropReason : uint8_t {
  WIFI_MAC_DROP_FAILED_ENQUEUE = 0,
  WIFI_MAC_DROP_EXPIRED_LIFETIME,
  WIFI_MAC_DROP_REACHED_RETRY_LIMIT,
  WIFI_MAC_DROP_QOS_OLD_PACKET
};

typedef std::unordered_map<uint16_t, Ptr<WifiPsdu>> WifiPsduMap;

class WifiMac : public Object {
public:
  static TypeId GetTypeId();

  WifiMac();
  ~WifiMac() override;

  WifiMac(const WifiMac &) = delete;
  WifiMac &operator=(const WifiMac &) = delete;

  void SetDevice(const Ptr<WifiNetDevice> device);
  Ptr<WifiNetDevice> GetDevice() const;

  Ptr<FrameExchangeManager>
  GetFrameExchangeManager(uint8_t linkId = SINGLE_LINK_OP_ID) const;

  Ptr<ChannelAccessManager>
  GetChannelAccessManager(uint8_t linkId = SINGLE_LINK_OP_ID) const;

  uint8_t GetNLinks() const;

  const std::set<uint8_t> &GetLinkIds() const;

  virtual std::optional<uint8_t>
  GetLinkIdByAddress(const Mac48Address &address) const;

  std::optional<uint8_t> GetLinkForPhy(Ptr<const WifiPhy> phy) const;

  std::optional<uint8_t> GetLinkForPhy(std::size_t phyId) const;

  std::optional<Mac48Address>
  GetMldAddress(const Mac48Address &remoteAddr) const;

  Mac48Address GetLocalAddress(const Mac48Address &remoteAddr) const;

  Ptr<Txop> GetTxop() const;
  Ptr<QosTxop> GetQosTxop(AcIndex ac) const;
  Ptr<QosTxop> GetQosTxop(uint8_t tid) const;
  virtual Ptr<WifiMacQueue> GetTxopQueue(AcIndex ac) const;

  virtual bool HasFramesToTransmit(uint8_t linkId);

  virtual void SetMacQueueScheduler(Ptr<WifiMacQueueScheduler> scheduler);
  Ptr<WifiMacQueueScheduler> GetMacQueueScheduler() const;

  void SetTypeOfStation(TypeOfStation type);
  TypeOfStation GetTypeOfStation() const;

  void SetSsid(Ssid ssid);
  void SetPromisc();
  void SetCtsToSelfSupported(bool enable);

  Mac48Address GetAddress() const;
  Ssid GetSsid() const;
  virtual void SetAddress(Mac48Address address);
  Mac48Address GetBssid(uint8_t linkId) const;
  void SetBssid(Mac48Address bssid, uint8_t linkId);

  void BlockUnicastTxOnLinks(WifiQueueBlockedReason reason,
                             const Mac48Address &address,
                             const std::set<uint8_t> &linkIds);

  void UnblockUnicastTxOnLinks(WifiQueueBlockedReason reason,
                               const Mac48Address &address,
                               const std::set<uint8_t> &linkIds);

  virtual bool CanForwardPacketsTo(Mac48Address to) const = 0;
  virtual void Enqueue(Ptr<Packet> packet, Mac48Address to, Mac48Address from);
  virtual void Enqueue(Ptr<Packet> packet, Mac48Address to) = 0;
  virtual bool SupportsSendFrom() const;

  virtual void SetWifiPhys(const std::vector<Ptr<WifiPhy>> &phys);
  Ptr<WifiPhy> GetWifiPhy(uint8_t linkId = SINGLE_LINK_OP_ID) const;
  void ResetWifiPhys();

  void
  SetWifiRemoteStationManager(Ptr<WifiRemoteStationManager> stationManager);
  void SetWifiRemoteStationManagers(
      const std::vector<Ptr<WifiRemoteStationManager>> &stationManagers);
  Ptr<WifiRemoteStationManager>
  GetWifiRemoteStationManager(uint8_t linkId = 0) const;

  typedef Callback<void, Ptr<const Packet>, Mac48Address, Mac48Address>
      ForwardUpCallback;

  void SetForwardUpCallback(ForwardUpCallback upCallback);
  virtual void SetLinkUpCallback(Callback<void> linkUp);
  void SetLinkDownCallback(Callback<void> linkDown);

  virtual void NotifyChannelSwitching(uint8_t linkId);

  void NotifyTx(Ptr<const Packet> packet);
  void NotifyTxDrop(Ptr<const Packet> packet);
  void NotifyRx(Ptr<const Packet> packet);
  void NotifyPromiscRx(Ptr<const Packet> packet);
  void NotifyRxDrop(Ptr<const Packet> packet);

  virtual void ConfigureStandard(WifiStandard standard);

  Ptr<HtConfiguration> GetHtConfiguration() const;
  Ptr<VhtConfiguration> GetVhtConfiguration() const;
  Ptr<HeConfiguration> GetHeConfiguration() const;
  Ptr<EhtConfiguration> GetEhtConfiguration() const;

  ExtendedCapabilities GetExtendedCapabilities() const;
  HtCapabilities GetHtCapabilities(uint8_t linkId) const;
  VhtCapabilities GetVhtCapabilities(uint8_t linkId) const;
  HeCapabilities GetHeCapabilities(uint8_t linkId) const;
  EhtCapabilities GetEhtCapabilities(uint8_t linkId) const;

  bool GetQosSupported() const;
  bool GetErpSupported(uint8_t linkId) const;
  bool GetDsssSupported(uint8_t linkId) const;
  bool GetHtSupported() const;
  bool GetVhtSupported(uint8_t linkId) const;
  bool GetHeSupported() const;
  bool GetEhtSupported() const;

  bool GetHtSupported(const Mac48Address &address) const;
  bool GetVhtSupported(const Mac48Address &address) const;
  bool GetHeSupported(const Mac48Address &address) const;
  bool GetEhtSupported(const Mac48Address &address) const;

  uint32_t GetMaxAmpduSize(AcIndex ac) const;
  uint16_t GetMaxAmsduSize(AcIndex ac) const;

  using OriginatorAgreementOptConstRef =
      std::optional<std::reference_wrapper<const OriginatorBlockAckAgreement>>;
  using RecipientAgreementOptConstRef =
      std::optional<std::reference_wrapper<const RecipientBlockAckAgreement>>;

  OriginatorAgreementOptConstRef
  GetBaAgreementEstablishedAsOriginator(Mac48Address recipient,
                                        uint8_t tid) const;
  RecipientAgreementOptConstRef
  GetBaAgreementEstablishedAsRecipient(Mac48Address originator,
                                       uint8_t tid) const;

  BlockAckType GetBaTypeAsOriginator(const Mac48Address &recipient,
                                     uint8_t tid) const;
  BlockAckReqType GetBarTypeAsOriginator(const Mac48Address &recipient,
                                         uint8_t tid) const;
  BlockAckType GetBaTypeAsRecipient(Mac48Address originator, uint8_t tid) const;
  BlockAckReqType GetBarTypeAsRecipient(Mac48Address originator,
                                        uint8_t tid) const;

  std::optional<std::reference_wrapper<const WifiTidLinkMapping>>
  GetTidToLinkMapping(Mac48Address mldAddr, WifiDirection dir) const;

  bool TidMappedOnLink(Mac48Address mldAddr, WifiDirection dir, uint8_t tid,
                       uint8_t linkId) const;

protected:
  void DoInitialize() override;
  void DoDispose() override;

  virtual void ConfigureContentionWindow(uint32_t cwMin, uint32_t cwMax);

  void SetQosSupported(bool enable);

  void SetShortSlotTimeSupported(bool enable);
  bool GetShortSlotTimeSupported() const;

  Ptr<QosTxop> GetVOQueue() const;
  Ptr<QosTxop> GetVIQueue() const;
  Ptr<QosTxop> GetBEQueue() const;
  Ptr<QosTxop> GetBKQueue() const;

  virtual void Receive(Ptr<const WifiMpdu> mpdu, uint8_t linkId);
  void ForwardUp(Ptr<const Packet> packet, Mac48Address from, Mac48Address to);

  virtual void DeaggregateAmsduAndForward(Ptr<const WifiMpdu> mpdu);

  void ApplyTidLinkMapping(const Mac48Address &mldAddr, WifiDirection dir);

  void SwapLinks(std::map<uint8_t, uint8_t> links);

  struct LinkEntity {
    virtual ~LinkEntity();

    Ptr<WifiPhy> phy;
    Ptr<ChannelAccessManager> channelAccessManager;
    Ptr<FrameExchangeManager> feManager;
    Ptr<WifiRemoteStationManager> stationManager;
    bool erpSupported{false};
    bool dsssSupported{false};
  };

  const std::map<uint8_t, std::unique_ptr<LinkEntity>> &GetLinks() const;

  LinkEntity &GetLink(uint8_t linkId) const;

  void UpdateTidToLinkMapping(const Mac48Address &mldAddr, WifiDirection dir,
                              const WifiTidLinkMapping &mapping);

  Ptr<MacRxMiddle> m_rxMiddle;
  Ptr<MacTxMiddle> m_txMiddle;
  Ptr<Txop> m_txop;
  Ptr<WifiMacQueueScheduler> m_scheduler;

  Callback<void> m_linkUp;
  Callback<void> m_linkDown;

private:
  void ConfigureDcf(Ptr<Txop> dcf, uint32_t cwmin, uint32_t cwmax,
                    std::list<bool> isDsss, AcIndex ac);

  void ConfigurePhyDependentParameters(uint8_t linkId);

  void SetupEdcaQueue(AcIndex ac);

  Ptr<FrameExchangeManager> SetupFrameExchangeManager(WifiStandard standard);

  virtual std::unique_ptr<LinkEntity> CreateLinkEntity() const;

  void UpdateLinkId(uint8_t id);

  virtual Mac48Address DoGetLocalAddress(const Mac48Address &remoteAddr) const;

  void SetErpSupported(bool enable, uint8_t linkId);
  void SetDsssSupported(bool enable, uint8_t linkId);

  void SetVoBlockAckThreshold(uint8_t threshold);
  void SetViBlockAckThreshold(uint8_t threshold);
  void SetBeBlockAckThreshold(uint8_t threshold);
  void SetBkBlockAckThreshold(uint8_t threshold);

  void SetVoBlockAckInactivityTimeout(uint16_t timeout);
  void SetViBlockAckInactivityTimeout(uint16_t timeout);
  void SetBeBlockAckInactivityTimeout(uint16_t timeout);
  void SetBkBlockAckInactivityTimeout(uint16_t timeout);

  bool m_qosSupported;

  bool m_shortSlotTimeSupported;
  bool m_ctsToSelfSupported;

  TypeOfStation m_typeOfStation;

  Ptr<WifiNetDevice> m_device;
  std::map<uint8_t, std::unique_ptr<LinkEntity>> m_links;
  std::set<uint8_t> m_linkIds;

  Mac48Address m_address;
  Ssid m_ssid;

  typedef std::map<AcIndex, Ptr<QosTxop>, std::greater<>> EdcaQueues;

  EdcaQueues m_edca;

  uint16_t m_voMaxAmsduSize;
  uint16_t m_viMaxAmsduSize;
  uint16_t m_beMaxAmsduSize;
  uint16_t m_bkMaxAmsduSize;

  uint32_t m_voMaxAmpduSize;
  uint32_t m_viMaxAmpduSize;
  uint32_t m_beMaxAmpduSize;
  uint32_t m_bkMaxAmpduSize;

  std::unordered_map<Mac48Address, WifiTidLinkMapping, WifiAddressHash>
      m_dlTidLinkMappings;
  std::unordered_map<Mac48Address, WifiTidLinkMapping, WifiAddressHash>
      m_ulTidLinkMappings;

  ForwardUpCallback m_forwardUp;

  TracedCallback<Ptr<const Packet>> m_macTxTrace;
  TracedCallback<Ptr<const Packet>> m_macTxDropTrace;
  TracedCallback<Ptr<const Packet>> m_macPromiscRxTrace;
  TracedCallback<Ptr<const Packet>> m_macRxTrace;
  TracedCallback<Ptr<const Packet>> m_macRxDropTrace;

  TracedCallback<const WifiMacHeader &> m_txOkCallback;
  TracedCallback<const WifiMacHeader &> m_txErrCallback;

  typedef void (*DroppedMpduCallback)(WifiMacDropReason reason,
                                      Ptr<const WifiMpdu> mpdu);

  typedef TracedCallback<WifiMacDropReason, Ptr<const WifiMpdu>>
      DroppedMpduTracedCallback;

  DroppedMpduTracedCallback m_droppedMpduCallback;

  typedef TracedCallback<Ptr<const WifiMpdu>> MpduTracedCallback;

  MpduTracedCallback m_ackedMpduCallback;
  MpduTracedCallback m_nackedMpduCallback;

  typedef void (*MpduResponseTimeoutCallback)(uint8_t reason,
                                              Ptr<const WifiMpdu> mpdu,
                                              const WifiTxVector &txVector);

  typedef TracedCallback<uint8_t, Ptr<const WifiMpdu>, const WifiTxVector &>
      MpduResponseTimeoutTracedCallback;

  MpduResponseTimeoutTracedCallback m_mpduResponseTimeoutCallback;

  typedef void (*PsduResponseTimeoutCallback)(uint8_t reason,
                                              Ptr<const WifiPsdu> psdu,
                                              const WifiTxVector &txVector);

  typedef TracedCallback<uint8_t, Ptr<const WifiPsdu>, const WifiTxVector &>
      PsduResponseTimeoutTracedCallback;

  PsduResponseTimeoutTracedCallback m_psduResponseTimeoutCallback;

  typedef void (*PsduMapResponseTimeoutCallback)(
      uint8_t reason, WifiPsduMap *psduMap,
      const std::set<Mac48Address> *missingStations,
      std::size_t nTotalStations);

  typedef TracedCallback<uint8_t, WifiPsduMap *, const std::set<Mac48Address> *,
                         std::size_t>
      PsduMapResponseTimeoutTracedCallback;

  PsduMapResponseTimeoutTracedCallback m_psduMapResponseTimeoutCallback;
};

} // namespace ns3

#endif
