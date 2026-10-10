
#ifndef WIFI_REMOTE_STATION_MANAGER_H
#define WIFI_REMOTE_STATION_MANAGER_H

#include "qos-utils.h"
#include "wifi-mode.h"
#include "wifi-remote-station-info.h"
#include "wifi-utils.h"

#include "ns3/data-rate.h"
#include "ns3/eht-capabilities.h"
#include "ns3/he-capabilities.h"
#include "ns3/ht-capabilities.h"
#include "ns3/mac48-address.h"
#include "ns3/multi-link-element.h"
#include "ns3/object.h"
#include "ns3/traced-callback.h"
#include "ns3/vht-capabilities.h"

#include <array>
#include <memory>
#include <optional>
#include <unordered_map>

namespace ns3 {

class WifiPhy;
class WifiMac;
class WifiMacHeader;
class Packet;
class WifiMpdu;
class WifiTxVector;

struct WifiRemoteStationState;
struct RxSignalInfo;

struct WifiRemoteStation {
  virtual ~WifiRemoteStation() {};
  WifiRemoteStationState *m_state;
  std::pair<double, Time> m_rssiAndUpdateTimePair;
};

struct WifiRemoteStationState {
  enum {
    BRAND_NEW,
    DISASSOC,
    WAIT_ASSOC_TX_OK,
    GOT_ASSOC_TX_OK,
    ASSOC_REFUSED
  } m_state;

  WifiModeList m_operationalRateSet;
  WifiModeList m_operationalMcsSet;
  Mac48Address m_address;
  uint16_t m_aid;
  WifiRemoteStationInfo m_info;
  bool m_dsssSupported;
  bool m_erpOfdmSupported;
  bool m_ofdmSupported;
  Ptr<const HtCapabilities> m_htCapabilities;
  Ptr<const VhtCapabilities> m_vhtCapabilities;
  Ptr<const HeCapabilities> m_heCapabilities;
  Ptr<const EhtCapabilities> m_ehtCapabilities;
  std::shared_ptr<CommonInfoBasicMle> m_mleCommonInfo;
  bool m_emlsrEnabled;

  uint16_t m_channelWidth;
  uint16_t m_guardInterval;
  uint8_t m_ness;
  bool m_aggregation;
  bool m_shortPreamble;
  bool m_shortSlotTime;
  bool m_qosSupported;
  bool m_isInPsMode;
};

class WifiRemoteStationManager : public Object {
public:
  static TypeId GetTypeId();

  WifiRemoteStationManager();
  ~WifiRemoteStationManager() override;

  enum ProtectionMode { RTS_CTS, CTS_TO_SELF };

  using Stations =
      std::unordered_map<Mac48Address, WifiRemoteStation *, WifiAddressHash>;
  using StationStates =
      std::unordered_map<Mac48Address, std::shared_ptr<WifiRemoteStationState>,
                         WifiAddressHash>;

  virtual void SetupPhy(const Ptr<WifiPhy> phy);
  virtual void SetupMac(const Ptr<WifiMac> mac);

  virtual int64_t AssignStreams(int64_t stream);

  void SetMaxSsrc(uint32_t maxSsrc);
  void SetMaxSlrc(uint32_t maxSlrc);
  void SetRtsCtsThreshold(uint32_t threshold);

  uint32_t GetFragmentationThreshold() const;
  void SetFragmentationThreshold(uint32_t threshold);

  void SetAssociationId(Mac48Address remoteAddress, uint16_t aid);
  void SetQosSupport(Mac48Address from, bool qosSupported);
  void SetEmlsrEnabled(const Mac48Address &from, bool emlsrEnabled);
  void AddStationHtCapabilities(Mac48Address from,
                                HtCapabilities htCapabilities);
  void AddStationVhtCapabilities(Mac48Address from,
                                 VhtCapabilities vhtCapabilities);
  void AddStationHeCapabilities(Mac48Address from,
                                HeCapabilities heCapabilities);
  void AddStationEhtCapabilities(Mac48Address from,
                                 EhtCapabilities ehtCapabilities);
  void AddStationMleCommonInfo(
      Mac48Address from,
      const std::shared_ptr<CommonInfoBasicMle> &mleCommonInfo);
  Ptr<const HtCapabilities> GetStationHtCapabilities(Mac48Address from);
  Ptr<const VhtCapabilities> GetStationVhtCapabilities(Mac48Address from);
  Ptr<const HeCapabilities> GetStationHeCapabilities(Mac48Address from);
  Ptr<const EhtCapabilities> GetStationEhtCapabilities(Mac48Address from);
  std::optional<std::reference_wrapper<CommonInfoBasicMle::EmlCapabilities>>
  GetStationEmlCapabilities(const Mac48Address &from);
  std::optional<std::reference_wrapper<CommonInfoBasicMle::MldCapabilities>>
  GetStationMldCapabilities(const Mac48Address &from);
  bool GetHtSupported() const;
  bool GetVhtSupported() const;
  bool GetHeSupported() const;
  bool GetEhtSupported() const;
  bool GetLdpcSupported() const;
  bool GetShortGuardIntervalSupported() const;
  uint16_t GetGuardInterval() const;
  void SetUseNonErpProtection(bool enable);
  bool GetUseNonErpProtection() const;
  void SetUseNonHtProtection(bool enable);
  bool GetUseNonHtProtection() const;
  void SetShortPreambleEnabled(bool enable);
  bool GetShortPreambleEnabled() const;
  void SetShortSlotTimeEnabled(bool enable);
  bool GetShortSlotTimeEnabled() const;

  void Reset();

  void AddBasicMode(WifiMode mode);
  WifiMode GetDefaultMode() const;
  uint8_t GetNBasicModes() const;
  WifiMode GetBasicMode(uint8_t i) const;
  uint32_t GetNNonErpBasicModes() const;
  WifiMode GetNonErpBasicMode(uint8_t i) const;
  bool GetLdpcSupported(Mac48Address address) const;
  bool GetShortPreambleSupported(Mac48Address address) const;
  bool GetShortSlotTimeSupported(Mac48Address address) const;
  bool GetQosSupported(Mac48Address address) const;
  uint16_t GetAssociationId(Mac48Address remoteAddress) const;
  void AddBasicMcs(WifiMode mcs);
  WifiMode GetDefaultMcs() const;
  WifiMode GetDefaultModeForSta(const WifiRemoteStation *st) const;
  uint8_t GetNBasicMcs() const;
  WifiMode GetBasicMcs(uint8_t i) const;
  void AddSupportedMcs(Mac48Address address, WifiMode mcs);
  uint16_t GetChannelWidthSupported(Mac48Address address) const;
  bool GetShortGuardIntervalSupported(Mac48Address address) const;
  uint8_t GetNumberOfSupportedStreams(Mac48Address address) const;
  uint8_t GetNMcsSupported(Mac48Address address) const;
  bool GetDsssSupported(const Mac48Address &address) const;
  bool GetErpOfdmSupported(const Mac48Address &address) const;
  bool GetOfdmSupported(const Mac48Address &address) const;
  bool GetHtSupported(Mac48Address address) const;
  bool GetVhtSupported(Mac48Address address) const;
  bool GetHeSupported(Mac48Address address) const;
  bool GetEhtSupported(Mac48Address address) const;
  bool GetEmlsrSupported(const Mac48Address &address) const;
  bool GetEmlsrEnabled(const Mac48Address &address) const;

  WifiMode GetNonUnicastMode() const;

  void AddSupportedMode(Mac48Address address, WifiMode mode);
  void AddAllSupportedModes(Mac48Address address);
  void AddAllSupportedMcs(Mac48Address address);
  void RemoveAllSupportedMcs(Mac48Address address);
  void AddSupportedPhyPreamble(Mac48Address address,
                               bool isShortPreambleSupported);
  void AddSupportedErpSlotTime(Mac48Address address,
                               bool isShortSlotTimeSupported);
  bool IsBrandNew(Mac48Address address) const;
  bool IsAssociated(Mac48Address address) const;
  bool IsWaitAssocTxOk(Mac48Address address) const;
  void RecordWaitAssocTxOk(Mac48Address address);
  void RecordGotAssocTxOk(Mac48Address address);
  void RecordGotAssocTxFailed(Mac48Address address);
  void RecordDisassociated(Mac48Address address);
  bool IsAssocRefused(Mac48Address address) const;
  void RecordAssocRefused(Mac48Address address);

  bool IsInPsMode(const Mac48Address &address) const;
  void SetPsMode(const Mac48Address &address, bool isInPsMode);

  std::optional<Mac48Address> GetMldAddress(const Mac48Address &address) const;
  std::optional<Mac48Address>
  GetAffiliatedStaAddress(const Mac48Address &mldAddress) const;

  WifiTxVector GetDataTxVector(const WifiMacHeader &header,
                               uint16_t allowedWidth);
  WifiTxVector GetRtsTxVector(Mac48Address address);
  WifiTxVector GetCtsTxVector(Mac48Address to, WifiMode rtsTxMode) const;
  WifiTxVector GetCtsToSelfTxVector();
  void AdjustTxVectorForIcf(WifiTxVector &txVector) const;
  WifiTxVector GetAckTxVector(Mac48Address to,
                              const WifiTxVector &dataTxVector) const;
  WifiTxVector GetBlockAckTxVector(Mac48Address to,
                                   const WifiTxVector &dataTxVector) const;
  WifiMode GetControlAnswerMode(WifiMode reqMode) const;

  void ReportRtsFailed(const WifiMacHeader &header);
  void ReportDataFailed(Ptr<const WifiMpdu> mpdu);
  void ReportRtsOk(const WifiMacHeader &header, double ctsSnr, WifiMode ctsMode,
                   double rtsSnr);
  void ReportDataOk(Ptr<const WifiMpdu> mpdu, double ackSnr, WifiMode ackMode,
                    double dataSnr, WifiTxVector dataTxVector);
  void ReportFinalRtsFailed(const WifiMacHeader &header);
  void ReportFinalDataFailed(Ptr<const WifiMpdu> mpdu);
  void ReportAmpduTxStatus(Mac48Address address, uint16_t nSuccessfulMpdus,
                           uint16_t nFailedMpdus, double rxSnr, double dataSnr,
                           WifiTxVector dataTxVector);

  void ReportRxOk(Mac48Address address, RxSignalInfo rxSignalInfo,
                  WifiTxVector txVector);

  bool NeedRts(const WifiMacHeader &header, uint32_t size);
  bool NeedCtsToSelf(WifiTxVector txVector);

  bool NeedRetransmission(Ptr<const WifiMpdu> mpdu);
  bool NeedFragmentation(Ptr<const WifiMpdu> mpdu);
  uint32_t GetFragmentSize(Ptr<const WifiMpdu> mpdu, uint32_t fragmentNumber);
  uint32_t GetFragmentOffset(Ptr<const WifiMpdu> mpdu, uint32_t fragmentNumber);
  bool IsLastFragment(Ptr<const WifiMpdu> mpdu, uint32_t fragmentNumber);

  uint8_t GetDefaultTxPowerLevel() const;
  WifiRemoteStationInfo GetInfo(Mac48Address address);
  std::optional<double> GetMostRecentRssi(Mac48Address address) const;
  void SetDefaultTxPowerLevel(uint8_t txPower);
  uint8_t GetNumberOfAntennas() const;
  uint8_t GetMaxNumberOfTransmitStreams() const;
  bool UseLdpcForDestination(Mac48Address dest) const;

  typedef void (*PowerChangeTracedCallback)(double oldPower, double newPower,
                                            Mac48Address remoteAddress);

  typedef void (*RateChangeTracedCallback)(DataRate oldRate, DataRate newRate,
                                           Mac48Address remoteAddress);

  Ptr<WifiPhy> GetPhy() const;
  Ptr<WifiMac> GetMac() const;

protected:
  void DoDispose() override;
  WifiMode GetSupported(const WifiRemoteStation *station, uint8_t i) const;
  uint8_t GetNSupported(const WifiRemoteStation *station) const;
  bool GetQosSupported(const WifiRemoteStation *station) const;
  bool GetHtSupported(const WifiRemoteStation *station) const;
  bool GetVhtSupported(const WifiRemoteStation *station) const;
  bool GetHeSupported(const WifiRemoteStation *station) const;
  bool GetEhtSupported(const WifiRemoteStation *station) const;
  bool GetEmlsrSupported(const WifiRemoteStation *station) const;
  bool GetEmlsrEnabled(const WifiRemoteStation *station) const;

  WifiMode GetMcsSupported(const WifiRemoteStation *station, uint8_t i) const;
  uint8_t GetNMcsSupported(const WifiRemoteStation *station) const;
  WifiMode GetNonErpSupported(const WifiRemoteStation *station,
                              uint8_t i) const;
  uint32_t GetNNonErpSupported(const WifiRemoteStation *station) const;
  Mac48Address GetAddress(const WifiRemoteStation *station) const;
  uint16_t GetChannelWidth(const WifiRemoteStation *station) const;
  bool GetShortGuardIntervalSupported(const WifiRemoteStation *station) const;
  uint16_t GetGuardInterval(const WifiRemoteStation *station) const;
  bool GetAggregation(const WifiRemoteStation *station) const;

  uint8_t GetNumberOfSupportedStreams(const WifiRemoteStation *station) const;
  uint8_t GetNess(const WifiRemoteStation *station) const;

private:
  uint16_t GetStaId(Mac48Address address, const WifiTxVector &txVector) const;

  virtual bool DoNeedRts(WifiRemoteStation *station, uint32_t size,
                         bool normally);
  virtual bool DoNeedRetransmission(WifiRemoteStation *station,
                                    Ptr<const Packet> packet, bool normally);
  virtual bool DoNeedFragmentation(WifiRemoteStation *station,
                                   Ptr<const Packet> packet, bool normally);
  virtual WifiRemoteStation *DoCreateStation() const = 0;
  virtual WifiTxVector DoGetDataTxVector(WifiRemoteStation *station,
                                         uint16_t allowedWidth) = 0;
  virtual WifiTxVector DoGetRtsTxVector(WifiRemoteStation *station) = 0;

  virtual void DoReportRtsFailed(WifiRemoteStation *station) = 0;
  virtual void DoReportDataFailed(WifiRemoteStation *station) = 0;
  virtual void DoReportRtsOk(WifiRemoteStation *station, double ctsSnr,
                             WifiMode ctsMode, double rtsSnr) = 0;
  virtual void DoReportDataOk(WifiRemoteStation *station, double ackSnr,
                              WifiMode ackMode, double dataSnr,
                              uint16_t dataChannelWidth, uint8_t dataNss) = 0;
  virtual void DoReportFinalRtsFailed(WifiRemoteStation *station) = 0;
  virtual void DoReportFinalDataFailed(WifiRemoteStation *station) = 0;
  virtual void DoReportRxOk(WifiRemoteStation *station, double rxSnr,
                            WifiMode txMode) = 0;
  virtual void DoReportAmpduTxStatus(WifiRemoteStation *station,
                                     uint16_t nSuccessfulMpdus,
                                     uint16_t nFailedMpdus, double rxSnr,
                                     double dataSnr, uint16_t dataChannelWidth,
                                     uint8_t dataNss);

  std::shared_ptr<WifiRemoteStationState>
  LookupState(Mac48Address address) const;
  WifiRemoteStation *Lookup(Mac48Address address) const;

  void DoSetFragmentationThreshold(uint32_t threshold);
  uint32_t DoGetFragmentationThreshold() const;
  uint32_t GetNFragments(Ptr<const WifiMpdu> mpdu);

  Ptr<WifiPhy> m_wifiPhy;
  Ptr<WifiMac> m_wifiMac;

  WifiModeList m_bssBasicRateSet;
  WifiModeList m_bssBasicMcsSet;

  StationStates m_states;
  Stations m_stations;

  WifiMode m_defaultTxMode;
  WifiMode m_defaultTxMcs;

  uint32_t m_maxSsrc;
  uint32_t m_maxSlrc;
  uint32_t m_rtsCtsThreshold;
  uint32_t m_fragmentationThreshold;
  uint8_t m_defaultTxPowerLevel;
  WifiMode m_nonUnicastMode;
  bool m_useNonErpProtection;
  bool m_useNonHtProtection;
  bool m_shortPreambleEnabled;
  bool m_shortSlotTimeEnabled;
  ProtectionMode m_erpProtectionMode;
  ProtectionMode m_htProtectionMode;

  std::array<uint32_t, AC_BE_NQOS> m_ssrc;
  std::array<uint32_t, AC_BE_NQOS> m_slrc;

  TracedCallback<Mac48Address> m_macTxRtsFailed;
  TracedCallback<Mac48Address> m_macTxDataFailed;
  TracedCallback<Mac48Address> m_macTxFinalRtsFailed;
  TracedCallback<Mac48Address> m_macTxFinalDataFailed;
};

} // namespace ns3

#endif
