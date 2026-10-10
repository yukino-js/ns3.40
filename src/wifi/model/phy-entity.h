
#ifndef PHY_ENTITY_H
#define PHY_ENTITY_H

#include "wifi-mpdu-type.h"
#include "wifi-phy-band.h"
#include "wifi-ppdu.h"
#include "wifi-tx-vector.h"

#include "ns3/event-id.h"
#include "ns3/nstime.h"
#include "ns3/simple-ref-count.h"

#include <list>
#include <map>
#include <optional>
#include <tuple>
#include <utility>

namespace ns3 {

struct SignalNoiseDbm {
  double signal;
  double noise;
};

struct MpduInfo {
  MpduType type;
  uint32_t mpduRefNumber;
};

struct RxSignalInfo {
  double snr;
  double rssi;
};

using RxPowerWattPerChannelBand = std::map<WifiSpectrumBandInfo, double>;

class WifiPsdu;
class WifiPhy;
class InterferenceHelper;
class Event;
class WifiPhyStateHelper;
class WifiPsdu;
class WifiPpdu;

class PhyEntity : public SimpleRefCount<PhyEntity> {
public:
  enum PhyRxFailureAction { DROP = 0, ABORT, IGNORE };

  struct PhyFieldRxStatus {
    bool isSuccess{true};
    WifiPhyRxfailureReason reason{UNKNOWN};
    PhyRxFailureAction actionIfFailure{DROP};

    PhyFieldRxStatus(bool s) : isSuccess(s) {}

    PhyFieldRxStatus(bool s, WifiPhyRxfailureReason r, PhyRxFailureAction a)
        : isSuccess(s), reason(r), actionIfFailure(a) {}
  };

  struct SnrPer {
    double snr{0.0};
    double per{1.0};

    SnrPer() {}

    SnrPer(double s, double p) : snr(s), per(p) {}
  };

  virtual ~PhyEntity();

  void SetOwner(Ptr<WifiPhy> wifiPhy);

  virtual bool IsModeSupported(WifiMode mode) const;
  virtual uint8_t GetNumModes() const;

  virtual WifiMode GetMcs(uint8_t index) const;
  virtual bool IsMcsSupported(uint8_t index) const;
  virtual bool HandlesMcsModes() const;

  virtual WifiMode GetSigMode(WifiPpduField field,
                              const WifiTxVector &txVector) const;

  std::list<WifiMode>::const_iterator begin() const;
  std::list<WifiMode>::const_iterator end() const;

  WifiPpduField GetNextField(WifiPpduField currentField,
                             WifiPreamble preamble) const;

  virtual Time GetDuration(WifiPpduField field,
                           const WifiTxVector &txVector) const;
  Time
  CalculatePhyPreambleAndHeaderDuration(const WifiTxVector &txVector) const;

  virtual Time GetPayloadDuration(uint32_t size, const WifiTxVector &txVector,
                                  WifiPhyBand band, MpduType mpdutype,
                                  bool incFlag, uint32_t &totalAmpduSize,
                                  double &totalAmpduNumSymbols,
                                  uint16_t staId) const = 0;

  virtual WifiConstPsduMap
  GetWifiConstPsduMap(Ptr<const WifiPsdu> psdu,
                      const WifiTxVector &txVector) const;

  virtual uint32_t GetMaxPsduSize() const = 0;

  typedef std::pair<std::pair<Time, Time>, WifiMode> PhyHeaderChunkInfo;
  typedef std::map<WifiPpduField, PhyHeaderChunkInfo> PhyHeaderSections;
  PhyHeaderSections GetPhyHeaderSections(const WifiTxVector &txVector,
                                         Time ppduStart) const;

  virtual Ptr<WifiPpdu> BuildPpdu(const WifiConstPsduMap &psdus,
                                  const WifiTxVector &txVector,
                                  Time ppduDuration);

  Time GetDurationUpToField(WifiPpduField field,
                            const WifiTxVector &txVector) const;
  Time GetRemainingDurationAfterField(Ptr<const WifiPpdu> ppdu,
                                      WifiPpduField field) const;

  virtual Ptr<const WifiPsdu>
  GetAddressedPsduInPpdu(Ptr<const WifiPpdu> ppdu) const;

  virtual void StartReceivePreamble(Ptr<const WifiPpdu> ppdu,
                                    RxPowerWattPerChannelBand &rxPowersW,
                                    Time rxDuration);
  void StartReceiveField(WifiPpduField field, Ptr<Event> event);
  void EndReceiveField(WifiPpduField field, Ptr<Event> event);

  void EndReceivePayload(Ptr<Event> event);

  void ResetReceive(Ptr<Event> event);

  virtual void CancelAllEvents();
  bool NoEndPreambleDetectionEvents() const;
  void CancelRunningEndPreambleDetectionEvents(bool clear = false);

  virtual uint16_t GetStaId(const Ptr<const WifiPpdu> ppdu) const;

  virtual bool CanStartRx(Ptr<const WifiPpdu> ppdu) const;

  virtual void SwitchMaybeToCcaBusy(const Ptr<const WifiPpdu> ppdu);
  virtual void NotifyCcaBusy(const Ptr<const WifiPpdu> ppdu, Time duration,
                             WifiChannelListType channelType);
  virtual void StartTx(Ptr<const WifiPpdu> ppdu);

  void Transmit(Time txDuration, Ptr<const WifiPpdu> ppdu, double txPowerDbm,
                Ptr<SpectrumValue> txPowerSpectrum, const std::string &type);

  virtual Time CalculateTxDuration(WifiConstPsduMap psduMap,
                                   const WifiTxVector &txVector,
                                   WifiPhyBand band) const;
  virtual double GetCcaThreshold(const Ptr<const WifiPpdu> ppdu,
                                 WifiChannelListType channelType) const;

  virtual Ptr<const WifiPpdu> GetRxPpduFromTxPpdu(Ptr<const WifiPpdu> ppdu);

  virtual uint64_t ObtainNextUid(const WifiTxVector &txVector);

  virtual Time GetMaxDelayPpduSameUid(const WifiTxVector &txVector);

protected:
  typedef std::map<WifiPreamble, std::vector<WifiPpduField>> PpduFormats;

  typedef std::pair<WifiCodeRate, uint16_t> CodeRateConstellationSizePair;

  typedef std::map<std::string, CodeRateConstellationSizePair>
      ModulationLookupTable;

  virtual const PpduFormats &GetPpduFormats() const = 0;

  virtual bool DoStartReceiveField(WifiPpduField field, Ptr<Event> event);
  virtual PhyFieldRxStatus DoEndReceiveField(WifiPpduField field,
                                             Ptr<Event> event);

  virtual Ptr<Event> DoGetEvent(Ptr<const WifiPpdu> ppdu,
                                RxPowerWattPerChannelBand &rxPowersW);
  virtual PhyFieldRxStatus DoEndReceivePreamble(Ptr<Event> event);
  void StartPreambleDetectionPeriod(Ptr<Event> event);
  void EndPreambleDetectionPeriod(Ptr<Event> event);

  void StartReceivePayload(Ptr<Event> event);

  virtual Time DoStartReceivePayload(Ptr<Event> event);

  virtual void DoResetReceive(Ptr<Event> event);

  virtual void DoAbortCurrentReception(WifiPhyRxfailureReason reason);

  virtual bool IsConfigSupported(Ptr<const WifiPpdu> ppdu) const;

  void DropPreambleEvent(Ptr<const WifiPpdu> ppdu,
                         WifiPhyRxfailureReason reason, Time endRx);

  void ErasePreambleEvent(Ptr<const WifiPpdu> ppdu, Time rxDuration);

  std::pair<bool, SignalNoiseDbm>
  GetReceptionStatus(Ptr<const WifiPsdu> psdu, Ptr<Event> event, uint16_t staId,
                     Time relativeMpduStart, Time mpduDuration);
  void EndOfMpdu(Ptr<Event> event, Ptr<const WifiPsdu> psdu, size_t mpduIndex,
                 Time relativeStart, Time mpduDuration);

  void ScheduleEndOfMpdus(Ptr<Event> event);

  virtual void RxPayloadSucceeded(Ptr<const WifiPsdu> psdu,
                                  RxSignalInfo rxSignalInfo,
                                  const WifiTxVector &txVector, uint16_t staId,
                                  const std::vector<bool> &statusPerMpdu);
  virtual void RxPayloadFailed(Ptr<const WifiPsdu> psdu, double snr,
                               const WifiTxVector &txVector);

  virtual void DoEndReceivePayload(Ptr<const WifiPpdu> ppdu);

  virtual std::pair<uint16_t, WifiSpectrumBandInfo>
  GetChannelWidthAndBand(const WifiTxVector &txVector, uint16_t staId) const;

  void AbortCurrentReception(WifiPhyRxfailureReason reason);

  double GetRandomValue() const;
  SnrPer GetPhyHeaderSnrPer(WifiPpduField field, Ptr<Event> event) const;
  double GetRxPowerWForPpdu(Ptr<Event> event) const;
  Ptr<const Event> GetCurrentEvent() const;
  const std::map<std::pair<uint64_t, WifiPreamble>, Ptr<Event>> &
  GetCurrentPreambleEvents() const;
  void AddPreambleEvent(Ptr<Event> event);

  Ptr<Event> CreateInterferenceEvent(Ptr<const WifiPpdu> ppdu, Time duration,
                                     RxPowerWattPerChannelBand &rxPower,
                                     bool isStartHePortionRxing = false);
  virtual void HandleRxPpduWithSameContent(Ptr<Event> event,
                                           Ptr<const WifiPpdu> ppdu,
                                           RxPowerWattPerChannelBand &rxPower);

  void NotifyInterferenceRxEndAndClear(bool reset);

  virtual Ptr<SpectrumValue>
  GetTxPowerSpectralDensity(double txPowerW,
                            Ptr<const WifiPpdu> ppdu) const = 0;

  uint16_t
  GetCenterFrequencyForChannelWidth(const WifiTxVector &txVector) const;

  void NotifyPayloadBegin(const WifiTxVector &txVector,
                          const Time &payloadDuration);

  WifiSpectrumBandInfo GetPrimaryBand(uint16_t bandWidth) const;
  WifiSpectrumBandInfo GetSecondaryBand(uint16_t bandWidth) const;

  virtual uint16_t
  GetMeasurementChannelWidth(const Ptr<const WifiPpdu> ppdu) const = 0;

  virtual uint16_t GetRxChannelWidth(const WifiTxVector &txVector) const;

  Time GetDelayUntilCcaEnd(double thresholdDbm,
                           const WifiSpectrumBandInfo &band);

  uint16_t GetGuardBandwidth(uint16_t currentChannelWidth) const;
  std::tuple<double, double, double> GetTxMaskRejectionParams() const;

  using CcaIndication = std::optional<std::pair<Time, WifiChannelListType>>;

  virtual CcaIndication GetCcaIndication(const Ptr<const WifiPpdu> ppdu);

  Ptr<WifiPhy> m_wifiPhy;
  Ptr<WifiPhyStateHelper> m_state;

  std::list<WifiMode> m_modeList;

  std::vector<EventId> m_endPreambleDetectionEvents;
  std::vector<EventId> m_endOfMpduEvents;

  std::vector<EventId> m_endRxPayloadEvents;

  typedef std::pair<uint64_t, uint16_t> UidStaIdPair;

  std::map<UidStaIdPair, std::vector<bool>> m_statusPerMpduMap;
  std::map<UidStaIdPair, SignalNoiseDbm> m_signalNoiseMap;

  static uint64_t m_globalPpduUid;
};

std::ostream &operator<<(std::ostream &os,
                         const PhyEntity::PhyRxFailureAction &action);
std::ostream &operator<<(std::ostream &os,
                         const PhyEntity::PhyFieldRxStatus &status);

} // namespace ns3

#endif
