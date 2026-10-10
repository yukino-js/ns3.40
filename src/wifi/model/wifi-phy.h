
#ifndef WIFI_PHY_H
#define WIFI_PHY_H

#include "phy-entity.h"
#include "wifi-phy-operating-channel.h"
#include "wifi-phy-state-helper.h"
#include "wifi-standards.h"

#include "ns3/error-model.h"

#include <limits>

namespace ns3 {

class Channel;
class WifiNetDevice;
class MobilityModel;
class WifiPhyStateHelper;
class FrameCaptureModel;
class PreambleDetectionModel;
class WifiRadioEnergyModel;
class UniformRandomVariable;
class InterferenceHelper;
class ErrorRateModel;

class WifiPhy : public Object {
public:
  friend class PhyEntity;
  static TypeId GetTypeId();

  WifiPhy();
  ~WifiPhy() override;

  Ptr<WifiPhyStateHelper> GetState() const;

  void SetReceiveOkCallback(RxOkCallback callback);
  void SetReceiveErrorCallback(RxErrorCallback callback);

  void RegisterListener(WifiPhyListener *listener);
  void UnregisterListener(WifiPhyListener *listener);

  void SetCapabilitiesChangedCallback(Callback<void> callback);

  void StartReceivePreamble(Ptr<const WifiPpdu> ppdu,
                            RxPowerWattPerChannelBand &rxPowersW,
                            Time rxDuration);

  void EndReceiveInterBss();

  static WifiConstPsduMap GetWifiConstPsduMap(Ptr<const WifiPsdu> psdu,
                                              const WifiTxVector &txVector);

  void Send(Ptr<const WifiPsdu> psdu, const WifiTxVector &txVector);
  void Send(WifiConstPsduMap psdus, const WifiTxVector &txVector);

  virtual void StartTx(Ptr<const WifiPpdu> ppdu) = 0;

  void SetSleepMode();
  void ResumeFromSleep();
  void SetOffMode();
  void ResumeFromOff();

  bool IsStateIdle() const;
  bool IsStateCcaBusy() const;
  bool IsStateRx() const;
  bool IsStateTx() const;
  bool IsStateSwitching() const;
  bool IsStateSleep() const;
  bool IsStateOff() const;

  Time GetDelayUntilIdle();

  Time GetLastRxStartTime() const;
  Time GetLastRxEndTime() const;

  static Time CalculateTxDuration(uint32_t size, const WifiTxVector &txVector,
                                  WifiPhyBand band, uint16_t staId = SU_STA_ID);
  static Time CalculateTxDuration(Ptr<const WifiPsdu> psdu,
                                  const WifiTxVector &txVector,
                                  WifiPhyBand band);
  static Time CalculateTxDuration(WifiConstPsduMap psduMap,
                                  const WifiTxVector &txVector,
                                  WifiPhyBand band);

  static Time
  CalculatePhyPreambleAndHeaderDuration(const WifiTxVector &txVector);
  static Time GetPreambleDetectionDuration();
  static Time GetPayloadDuration(uint32_t size, const WifiTxVector &txVector,
                                 WifiPhyBand band,
                                 MpduType mpdutype = NORMAL_MPDU,
                                 uint16_t staId = SU_STA_ID);
  static Time GetPayloadDuration(uint32_t size, const WifiTxVector &txVector,
                                 WifiPhyBand band, MpduType mpdutype,
                                 bool incFlag, uint32_t &totalAmpduSize,
                                 double &totalAmpduNumSymbols, uint16_t staId);
  static Time GetStartOfPacketDuration(const WifiTxVector &txVector);

  std::list<WifiMode> GetModeList() const;
  std::list<WifiMode> GetModeList(WifiModulationClass modulation) const;
  bool IsModeSupported(WifiMode mode) const;
  WifiMode GetDefaultMode() const;
  bool IsMcsSupported(WifiModulationClass modulation, uint8_t mcs) const;

  double CalculateSnr(const WifiTxVector &txVector, double ber) const;

  void SetSifs(Time sifs);
  Time GetSifs() const;
  void SetSlot(Time slot);
  Time GetSlot() const;
  void SetPifs(Time pifs);
  Time GetPifs() const;
  Time GetAckTxTime() const;
  Time GetBlockAckTxTime() const;

  static uint32_t GetMaxPsduSize(WifiModulationClass modulation);

  std::list<uint8_t> GetBssMembershipSelectorList() const;
  uint16_t GetNMcs() const;
  std::list<WifiMode> GetMcsList() const;
  std::list<WifiMode> GetMcsList(WifiModulationClass modulation) const;
  WifiMode GetMcs(WifiModulationClass modulation, uint8_t mcs) const;

  uint8_t GetChannelNumber() const;
  Time GetChannelSwitchDelay() const;

  virtual void ConfigureStandard(WifiStandard standard);

  WifiStandard GetStandard() const;

  WifiPhyBand GetPhyBand() const;

  const WifiPhyOperatingChannel &GetOperatingChannel() const;

  virtual Ptr<Channel> GetChannel() const = 0;

  void NotifyTxBegin(WifiConstPsduMap psdus, double txPowerW);
  void NotifyTxEnd(WifiConstPsduMap psdus);
  void NotifyTxDrop(Ptr<const WifiPsdu> psdu);
  void NotifyRxBegin(Ptr<const WifiPsdu> psdu,
                     const RxPowerWattPerChannelBand &rxPowersW);
  void NotifyRxEnd(Ptr<const WifiPsdu> psdu);
  void NotifyRxDrop(Ptr<const WifiPsdu> psdu, WifiPhyRxfailureReason reason);

  void NotifyMonitorSniffRx(Ptr<const WifiPsdu> psdu, uint16_t channelFreqMhz,
                            WifiTxVector txVector, SignalNoiseDbm signalNoise,
                            std::vector<bool> statusPerMpdu,
                            uint16_t staId = SU_STA_ID);

  typedef void (*MonitorSnifferRxCallback)(
      Ptr<const Packet> packet, uint16_t channelFreqMhz, WifiTxVector txVector,
      MpduInfo aMpdu, SignalNoiseDbm signalNoise, uint16_t staId);

  void NotifyMonitorSniffTx(Ptr<const WifiPsdu> psdu, uint16_t channelFreqMhz,
                            WifiTxVector txVector, uint16_t staId = SU_STA_ID);

  typedef void (*MonitorSnifferTxCallback)(const Ptr<const Packet> packet,
                                           uint16_t channelFreqMhz,
                                           WifiTxVector txVector,
                                           MpduInfo aMpdu, uint16_t staId);

  typedef void (*PhyTxBeginTracedCallback)(Ptr<const Packet> packet,
                                           double txPowerW);

  typedef void (*PsduTxBeginCallback)(WifiConstPsduMap psduMap,
                                      WifiTxVector txVector, double txPowerW);

  typedef void (*PhyRxBeginTracedCallback)(Ptr<const Packet> packet,
                                           RxPowerWattPerChannelBand rxPowersW);

  typedef void (*PhyRxPayloadBeginTracedCallback)(WifiTxVector txVector,
                                                  Time psduDuration);

  virtual int64_t AssignStreams(int64_t stream);

  void SetRxSensitivity(double threshold);
  double GetRxSensitivity() const;
  void SetCcaEdThreshold(double threshold);
  double GetCcaEdThreshold() const;
  void SetCcaSensitivityThreshold(double threshold);
  double GetCcaSensitivityThreshold() const;
  void SetRxNoiseFigure(double noiseFigureDb);
  void SetTxPowerStart(double start);
  double GetTxPowerStart() const;
  void SetTxPowerEnd(double end);
  double GetTxPowerEnd() const;
  void SetNTxPower(uint8_t n);
  uint8_t GetNTxPower() const;
  void SetTxGain(double gain);
  double GetTxGain() const;
  void SetRxGain(double gain);
  double GetRxGain() const;

  virtual void SetDevice(const Ptr<WifiNetDevice> device);
  Ptr<WifiNetDevice> GetDevice() const;
  void SetMobility(const Ptr<MobilityModel> mobility);
  Ptr<MobilityModel> GetMobility() const;

  using ChannelTuple = std::tuple<uint8_t, uint16_t, int, uint8_t>;

  void SetOperatingChannel(const ChannelTuple &channelTuple);
  void SetOperatingChannel(const WifiPhyOperatingChannel &channel);
  void SetFixedPhyBand(bool enable);
  bool HasFixedPhyBand() const;
  uint16_t GetFrequency() const;
  uint8_t GetPrimary20Index() const;
  uint16_t GetTxBandwidth(WifiMode mode,
                          uint16_t maxAllowedBandWidth =
                              std::numeric_limits<uint16_t>::max()) const;
  void SetNumberOfAntennas(uint8_t antennas);
  uint8_t GetNumberOfAntennas() const;
  void SetMaxSupportedTxSpatialStreams(uint8_t streams);
  uint8_t GetMaxSupportedTxSpatialStreams() const;
  void SetMaxSupportedRxSpatialStreams(uint8_t streams);
  uint8_t GetMaxSupportedRxSpatialStreams() const;
  void SetShortPhyPreambleSupported(bool preamble);
  bool GetShortPhyPreambleSupported() const;

  virtual void SetInterferenceHelper(const Ptr<InterferenceHelper> helper);

  void SetErrorRateModel(const Ptr<ErrorRateModel> model);
  void SetPostReceptionErrorModel(const Ptr<ErrorModel> em);
  void SetFrameCaptureModel(const Ptr<FrameCaptureModel> frameCaptureModel);
  void SetPreambleDetectionModel(
      const Ptr<PreambleDetectionModel> preambleDetectionModel);
  void
  SetWifiRadioEnergyModel(const Ptr<WifiRadioEnergyModel> wifiRadioEnergyModel);

  uint16_t GetChannelWidth() const;

  double GetPowerDbm(uint8_t power) const;

  void ResetCca(bool powerRestricted, double txPowerMaxSiso = 0,
                double txPowerMaxMimo = 0);
  double GetTxPowerForTransmission(Ptr<const WifiPpdu> ppdu) const;
  void NotifyChannelAccessRequested();

  virtual WifiSpectrumBandFrequencies
  ConvertIndicesToFrequencies(const WifiSpectrumBandIndices &indices) const = 0;

  static void AddStaticPhyEntity(WifiModulationClass modulation,
                                 Ptr<PhyEntity> phyEntity);

  static const Ptr<const PhyEntity>
  GetStaticPhyEntity(WifiModulationClass modulation);

  Ptr<PhyEntity> GetPhyEntityForPpdu(const Ptr<const WifiPpdu> ppdu) const;

  Ptr<PhyEntity> GetPhyEntity(WifiModulationClass modulation) const;
  Ptr<PhyEntity> GetPhyEntity(WifiStandard standard) const;
  Ptr<PhyEntity> GetLatestPhyEntity() const;

  uint64_t GetPreviouslyRxPpduUid() const;

  void SetPreviouslyRxPpduUid(uint64_t uid);

  virtual uint16_t GetGuardBandwidth(uint16_t currentChannelWidth) const = 0;
  virtual std::tuple<double, double, double>
  GetTxMaskRejectionParams() const = 0;

  uint8_t GetPrimaryChannelNumber(uint16_t primaryChannelWidth) const;

  virtual WifiSpectrumBandInfo GetBand(uint16_t bandWidth,
                                       uint8_t bandIndex = 0) = 0;

  virtual FrequencyRange GetCurrentFrequencyRange() const = 0;

  uint32_t GetSubcarrierSpacing() const;

protected:
  void DoInitialize() override;
  void DoDispose() override;

  void Reset();

  Time GetDelayUntilChannelSwitch();
  virtual void DoChannelSwitch();

  void SwitchMaybeToCcaBusy(const Ptr<const WifiPpdu> ppdu);
  void NotifyCcaBusy(const Ptr<const WifiPpdu> ppdu, Time duration);

  void AddPhyEntity(WifiModulationClass modulation, Ptr<PhyEntity> phyEntity);

  Ptr<InterferenceHelper> m_interference;

  Ptr<UniformRandomVariable> m_random;
  Ptr<WifiPhyStateHelper> m_state;

  uint32_t m_txMpduReferenceNumber;
  uint32_t m_rxMpduReferenceNumber;

  EventId m_endPhyRxEvent;
  EventId m_endTxEvent;

  Ptr<Event> m_currentEvent;
  std::map<std::pair<uint64_t, WifiPreamble>, Ptr<Event>>
      m_currentPreambleEvents;

  uint64_t m_previouslyRxPpduUid;

  std::map<WifiModulationClass, Ptr<PhyEntity>> m_phyEntities;

private:
  void Configure80211a();
  void Configure80211b();
  void Configure80211g();
  void Configure80211p();
  void Configure80211n();
  void Configure80211ac();
  void Configure80211ax();
  void Configure80211be();
  void ConfigureHtDeviceMcsSet();
  void PushMcs(WifiMode mode);
  void RebuildMcsMap();

  void AbortCurrentReception(WifiPhyRxfailureReason reason);

  Ptr<const WifiPsdu> GetAddressedPsduInPpdu(Ptr<const WifiPpdu> ppdu) const;

  TracedCallback<Ptr<const Packet>, double> m_phyTxBeginTrace;
  TracedCallback<WifiConstPsduMap, WifiTxVector, double> m_phyTxPsduBeginTrace;

  TracedCallback<Ptr<const Packet>> m_phyTxEndTrace;

  TracedCallback<Ptr<const Packet>> m_phyTxDropTrace;

  TracedCallback<Ptr<const Packet>, RxPowerWattPerChannelBand>
      m_phyRxBeginTrace;

  TracedCallback<WifiTxVector, Time> m_phyRxPayloadBeginTrace;

  TracedCallback<Ptr<const Packet>> m_phyRxEndTrace;

  TracedCallback<Ptr<const Packet>, WifiPhyRxfailureReason> m_phyRxDropTrace;

  TracedCallback<Ptr<const Packet>, uint16_t, WifiTxVector, MpduInfo,
                 SignalNoiseDbm, uint16_t>
      m_phyMonitorSniffRxTrace;

  TracedCallback<Ptr<const Packet>, uint16_t, WifiTxVector, MpduInfo, uint16_t>
      m_phyMonitorSniffTxTrace;

  static std::map<WifiModulationClass, Ptr<PhyEntity>> &GetStaticPhyEntities();

  WifiStandard m_standard;
  WifiPhyBand m_band;
  ChannelTuple m_channelSettings;
  WifiPhyOperatingChannel m_operatingChannel;
  bool m_fixedPhyBand;

  Time m_sifs;
  Time m_slot;
  Time m_pifs;
  Time m_ackTxTime;
  Time m_blockAckTxTime;

  double m_rxSensitivityW;
  double m_ccaEdThresholdW;
  double m_ccaSensitivityThresholdW;

  double m_txGainDb;
  double m_rxGainDb;
  double m_txPowerBaseDbm;
  double m_txPowerEndDbm;
  uint8_t m_nTxPower;
  double m_powerDensityLimit;

  bool m_powerRestricted;
  double m_txPowerMaxSiso;
  double m_txPowerMaxMimo;
  bool m_channelAccessRequested;

  bool m_shortPreamble;
  uint8_t m_numberOfAntennas;
  uint8_t m_txSpatialStreams;
  uint8_t m_rxSpatialStreams;

  double m_noiseFigureDb;

  Time m_channelSwitchDelay;

  Ptr<WifiNetDevice> m_device;
  Ptr<MobilityModel> m_mobility;

  Ptr<FrameCaptureModel> m_frameCaptureModel;
  Ptr<PreambleDetectionModel> m_preambleDetectionModel;
  Ptr<WifiRadioEnergyModel> m_wifiRadioEnergyModel;
  Ptr<ErrorModel> m_postReceptionErrorModel;
  Time m_timeLastPreambleDetected;

  Callback<void> m_capabilitiesChangedCallback;
};

std::ostream &operator<<(std::ostream &os, RxSignalInfo rxSignalInfo);

} // namespace ns3

#endif
