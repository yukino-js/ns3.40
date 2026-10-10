
#ifndef HE_PHY_H
#define HE_PHY_H

#include "he-ppdu.h"

#include "ns3/callback.h"
#include "ns3/vht-phy.h"
#include "ns3/wifi-phy-band.h"

#include <optional>

namespace ns3 {

class ObssPdAlgorithm;

#define HE_PHY 122

struct HeSigAParameters {
  double rssiW;
  uint8_t bssColor;
};

class HePhy : public VhtPhy {
public:
  typedef Callback<void, HeSigAParameters> EndOfHeSigACallback;

  HePhy(bool buildModeList = true);
  ~HePhy() override;

  WifiMode GetSigMode(WifiPpduField field,
                      const WifiTxVector &txVector) const override;
  WifiMode GetSigAMode() const override;
  WifiMode GetSigBMode(const WifiTxVector &txVector) const override;
  const PpduFormats &GetPpduFormats() const override;
  Time GetLSigDuration(WifiPreamble preamble) const override;
  Time GetTrainingDuration(const WifiTxVector &txVector, uint8_t nDataLtf,
                           uint8_t nExtensionLtf = 0) const override;
  Time GetSigADuration(WifiPreamble preamble) const override;
  Time GetSigBDuration(const WifiTxVector &txVector) const override;
  Ptr<WifiPpdu> BuildPpdu(const WifiConstPsduMap &psdus,
                          const WifiTxVector &txVector,
                          Time ppduDuration) override;
  Ptr<const WifiPsdu>
  GetAddressedPsduInPpdu(Ptr<const WifiPpdu> ppdu) const override;
  void StartReceivePreamble(Ptr<const WifiPpdu> ppdu,
                            RxPowerWattPerChannelBand &rxPowersW,
                            Time rxDuration) override;
  void CancelAllEvents() override;
  uint16_t GetStaId(const Ptr<const WifiPpdu> ppdu) const override;
  uint16_t
  GetMeasurementChannelWidth(const Ptr<const WifiPpdu> ppdu) const override;
  void StartTx(Ptr<const WifiPpdu> ppdu) override;
  Time CalculateTxDuration(WifiConstPsduMap psduMap,
                           const WifiTxVector &txVector,
                           WifiPhyBand band) const override;
  void SwitchMaybeToCcaBusy(const Ptr<const WifiPpdu> ppdu) override;
  double GetCcaThreshold(const Ptr<const WifiPpdu> ppdu,
                         WifiChannelListType channelType) const override;
  void NotifyCcaBusy(const Ptr<const WifiPpdu> ppdu, Time duration,
                     WifiChannelListType channelType) override;
  bool CanStartRx(Ptr<const WifiPpdu> ppdu) const override;
  Ptr<const WifiPpdu> GetRxPpduFromTxPpdu(Ptr<const WifiPpdu> ppdu) override;

  uint8_t GetBssColor() const;

  static std::pair<uint16_t, Time> ConvertHeTbPpduDurationToLSigLength(
      Time ppduDuration, const WifiTxVector &txVector, WifiPhyBand band);
  static Time ConvertLSigLengthToHeTbPpduDuration(uint16_t length,
                                                  const WifiTxVector &txVector,
                                                  WifiPhyBand band);
  virtual Time
  CalculateNonHeDurationForHeTb(const WifiTxVector &txVector) const;

  virtual Time
  CalculateNonHeDurationForHeMu(const WifiTxVector &txVector) const;

  WifiSpectrumBandInfo GetRuBandForTx(const WifiTxVector &txVector,
                                      uint16_t staId) const;
  WifiSpectrumBandInfo GetRuBandForRx(const WifiTxVector &txVector,
                                      uint16_t staId) const;
  WifiSpectrumBandInfo GetNonOfdmaBand(const WifiTxVector &txVector,
                                       uint16_t staId) const;
  uint16_t GetNonOfdmaWidth(HeRu::RuSpec ru) const;

  uint64_t GetCurrentHeTbPpduUid() const;

  void SetTrigVector(const WifiTxVector &trigVector, Time validity);

  uint16_t GetCenterFrequencyForNonHePart(const WifiTxVector &txVector,
                                          uint16_t staId) const;

  void SetObssPdAlgorithm(const Ptr<ObssPdAlgorithm> algorithm);

  void SetEndOfHeSigACallback(EndOfHeSigACallback callback);

  void NotifyEndOfHeSigA(HeSigAParameters params);

  static void InitializeModes();
  static WifiMode GetHeMcs(uint8_t index);

  static WifiMode GetHeMcs0();
  static WifiMode GetHeMcs1();
  static WifiMode GetHeMcs2();
  static WifiMode GetHeMcs3();
  static WifiMode GetHeMcs4();
  static WifiMode GetHeMcs5();
  static WifiMode GetHeMcs6();
  static WifiMode GetHeMcs7();
  static WifiMode GetHeMcs8();
  static WifiMode GetHeMcs9();
  static WifiMode GetHeMcs10();
  static WifiMode GetHeMcs11();

  static WifiCodeRate GetCodeRate(uint8_t mcsValue);
  static uint16_t GetConstellationSize(uint8_t mcsValue);
  static uint64_t GetPhyRate(uint8_t mcsValue, uint16_t channelWidth,
                             uint16_t guardInterval, uint8_t nss);
  static uint64_t GetPhyRateFromTxVector(const WifiTxVector &txVector,
                                         uint16_t staId = SU_STA_ID);
  static uint64_t GetDataRateFromTxVector(const WifiTxVector &txVector,
                                          uint16_t staId = SU_STA_ID);
  static uint64_t GetDataRate(uint8_t mcsValue, uint16_t channelWidth,
                              uint16_t guardInterval, uint8_t nss);
  static uint64_t GetNonHtReferenceRate(uint8_t mcsValue);
  static bool IsAllowed(const WifiTxVector &txVector);

  static WifiMode CreateHeMcs(uint8_t index);

  static WifiSpectrumBandIndices ConvertHeRuSubcarriers(
      uint16_t bandWidth, uint16_t guardBandwidth, uint32_t subcarrierSpacing,
      HeRu::SubcarrierRange subcarrierRange, uint8_t bandIndex = 0);

protected:
  PhyFieldRxStatus ProcessSig(Ptr<Event> event, PhyFieldRxStatus status,
                              WifiPpduField field) override;
  Ptr<Event> DoGetEvent(Ptr<const WifiPpdu> ppdu,
                        RxPowerWattPerChannelBand &rxPowersW) override;
  bool IsConfigSupported(Ptr<const WifiPpdu> ppdu) const override;
  Time DoStartReceivePayload(Ptr<Event> event) override;
  std::pair<uint16_t, WifiSpectrumBandInfo>
  GetChannelWidthAndBand(const WifiTxVector &txVector,
                         uint16_t staId) const override;
  void RxPayloadSucceeded(Ptr<const WifiPsdu> psdu, RxSignalInfo rxSignalInfo,
                          const WifiTxVector &txVector, uint16_t staId,
                          const std::vector<bool> &statusPerMpdu) override;
  void RxPayloadFailed(Ptr<const WifiPsdu> psdu, double snr,
                       const WifiTxVector &txVector) override;
  void DoEndReceivePayload(Ptr<const WifiPpdu> ppdu) override;
  void DoResetReceive(Ptr<Event> event) override;
  void DoAbortCurrentReception(WifiPhyRxfailureReason reason) override;
  uint64_t ObtainNextUid(const WifiTxVector &txVector) override;
  Time GetMaxDelayPpduSameUid(const WifiTxVector &txVector) override;
  Ptr<SpectrumValue>
  GetTxPowerSpectralDensity(double txPowerW,
                            Ptr<const WifiPpdu> ppdu) const override;
  uint32_t GetMaxPsduSize() const override;
  WifiConstPsduMap
  GetWifiConstPsduMap(Ptr<const WifiPsdu> psdu,
                      const WifiTxVector &txVector) const override;
  void HandleRxPpduWithSameContent(Ptr<Event> event, Ptr<const WifiPpdu> ppdu,
                                   RxPowerWattPerChannelBand &rxPower) override;

  virtual PhyFieldRxStatus ProcessSigA(Ptr<Event> event,
                                       PhyFieldRxStatus status);

  virtual PhyFieldRxStatus ProcessSigB(Ptr<Event> event,
                                       PhyFieldRxStatus status);

  virtual uint32_t GetSigBSize(const WifiTxVector &txVector) const;

  void StartReceiveMuPayload(Ptr<Event> event);

  static uint64_t CalculateNonHtReferenceRate(WifiCodeRate codeRate,
                                              uint16_t constellationSize);

  static uint16_t GetUsableSubcarriers(uint16_t channelWidth);

  static Time GetSymbolDuration(Time guardInterval);

  uint64_t m_previouslyTxPpduUid;
  uint64_t m_currentMuPpduUid;

  std::map<uint16_t, EventId> m_beginMuPayloadRxEvents;

  EndOfHeSigACallback m_endOfHeSigACallback;
  std::optional<WifiTxVector> m_trigVector;
  std::optional<Time> m_trigVectorExpirationTime;
  std::optional<WifiTxVector> m_currentTxVector;

private:
  void BuildModeList() override;
  uint8_t GetNumberBccEncoders(const WifiTxVector &txVector) const override;
  Time GetSymbolDuration(const WifiTxVector &txVector) const override;

  Ptr<SpectrumValue> GetTxPowerSpectralDensity(double txPowerW,
                                               Ptr<const WifiPpdu> ppdu,
                                               HePpdu::TxPsdFlag flag) const;

  void StartTxHePortion(Ptr<const WifiPpdu> ppdu, double txPowerDbm,
                        Ptr<SpectrumValue> txPowerSpectrum,
                        Time hePortionDuration);

  void NotifyCcaBusy(Time duration, WifiChannelListType channelType,
                     const std::vector<Time> &per20MHzDurations);

  std::vector<Time> GetPer20MHzDurations(const Ptr<const WifiPpdu> ppdu);

  static Time GetValidPpduDuration(Time ppduDuration,
                                   const WifiTxVector &txVector,
                                   WifiPhyBand band);

  static const PpduFormats m_hePpduFormats;

  std::size_t m_rxHeTbPpdus;
  Ptr<ObssPdAlgorithm> m_obssPdAlgorithm;
  std::vector<Time> m_lastPer20MHzDurations;
};

} // namespace ns3

#endif
