
#ifndef INTERFERENCE_HELPER_H
#define INTERFERENCE_HELPER_H

#include "phy-entity.h"

#include "ns3/object.h"

namespace ns3 {

class WifiPpdu;
class WifiPsdu;
class ErrorRateModel;

class Event : public SimpleRefCount<Event> {
public:
  Event(Ptr<const WifiPpdu> ppdu, Time duration,
        RxPowerWattPerChannelBand &&rxPower);
  ~Event();

  Ptr<const WifiPpdu> GetPpdu() const;
  Time GetStartTime() const;
  Time GetEndTime() const;
  Time GetDuration() const;
  double GetRxPowerW() const;
  double GetRxPowerW(const WifiSpectrumBandInfo &band) const;
  const RxPowerWattPerChannelBand &GetRxPowerWPerBand() const;
  void UpdateRxPowerW(const RxPowerWattPerChannelBand &rxPower);
  void UpdatePpdu(Ptr<const WifiPpdu> ppdu);

private:
  Ptr<const WifiPpdu> m_ppdu;
  Time m_startTime;
  Time m_endTime;
  RxPowerWattPerChannelBand m_rxPowerW;
};

std::ostream &operator<<(std::ostream &os, const Event &event);

class InterferenceHelper : public Object {
public:
  InterferenceHelper();
  ~InterferenceHelper() override;

  static TypeId GetTypeId();

  void AddBand(const WifiSpectrumBandInfo &band);

  bool HasBands() const;

  void UpdateBands(const std::vector<WifiSpectrumBandInfo> &bands,
                   const FrequencyRange &freqRange);

  void SetNoiseFigure(double value);
  void SetErrorRateModel(const Ptr<ErrorRateModel> rate);

  Ptr<ErrorRateModel> GetErrorRateModel() const;
  void SetNumberOfReceiveAntennas(uint8_t rx);

  Time GetEnergyDuration(double energyW, const WifiSpectrumBandInfo &band);

  Ptr<Event> Add(Ptr<const WifiPpdu> ppdu, Time duration,
                 RxPowerWattPerChannelBand &rxPower,
                 bool isStartHePortionRxing = false);

  void AddForeignSignal(Time duration, RxPowerWattPerChannelBand &rxPower);
  PhyEntity::SnrPer
  CalculatePayloadSnrPer(Ptr<Event> event, uint16_t channelWidth,
                         const WifiSpectrumBandInfo &band, uint16_t staId,
                         std::pair<Time, Time> relativeMpduStartStop) const;
  double CalculateSnr(Ptr<Event> event, uint16_t channelWidth, uint8_t nss,
                      const WifiSpectrumBandInfo &band) const;
  PhyEntity::SnrPer CalculatePhyHeaderSnrPer(Ptr<Event> event,
                                             uint16_t channelWidth,
                                             const WifiSpectrumBandInfo &band,
                                             WifiPpduField header) const;

  void NotifyRxStart();
  void NotifyRxEnd(Time endTime, const FrequencyRange &freqRange);

  void UpdateEvent(Ptr<Event> event, const RxPowerWattPerChannelBand &rxPower);

protected:
  void DoDispose() override;

  double CalculateSnr(double signal, double noiseInterference,
                      uint16_t channelWidth, uint8_t nss) const;
  double CalculateChunkSuccessRate(double snir, Time duration, WifiMode mode,
                                   const WifiTxVector &txVector,
                                   WifiPpduField field) const;
  double CalculatePayloadChunkSuccessRate(double snir, Time duration,
                                          const WifiTxVector &txVector,
                                          uint16_t staId = SU_STA_ID) const;

private:
  class NiChange {
  public:
    NiChange(double power, Ptr<Event> event);
    ~NiChange();
    double GetPower() const;
    void AddPower(double power);
    Ptr<Event> GetEvent() const;

  private:
    double m_power;
    Ptr<Event> m_event;
  };

  using NiChanges = std::multimap<Time, NiChange>;

  using NiChangesPerBand = std::map<WifiSpectrumBandInfo, NiChanges>;

  using FirstPowerPerBand = std::map<WifiSpectrumBandInfo, double>;

  bool HasBand(const WifiSpectrumBandInfo &band) const;

  bool IsBandInFrequencyRange(const WifiSpectrumBandInfo &band,
                              const FrequencyRange &freqRange) const;

  void AppendEvent(Ptr<Event> event, bool isStartHePortionRxing);

  double CalculateNoiseInterferenceW(Ptr<Event> event, NiChangesPerBand &nis,
                                     const WifiSpectrumBandInfo &band) const;

  double CalculateMuMimoPowerW(Ptr<const Event> event,
                               const WifiSpectrumBandInfo &band) const;

  double CalculatePayloadPer(Ptr<const Event> event, uint16_t channelWidth,
                             NiChangesPerBand *nis,
                             const WifiSpectrumBandInfo &band, uint16_t staId,
                             std::pair<Time, Time> window) const;
  double CalculatePhyHeaderPer(Ptr<const Event> event, NiChangesPerBand *nis,
                               uint16_t channelWidth,
                               const WifiSpectrumBandInfo &band,
                               WifiPpduField header) const;
  double CalculatePhyHeaderSectionPsr(
      Ptr<const Event> event, NiChangesPerBand *nis, uint16_t channelWidth,
      const WifiSpectrumBandInfo &band,
      PhyEntity::PhyHeaderSections phyHeaderSections) const;

  double m_noiseFigure;
  Ptr<ErrorRateModel> m_errorRateModel;
  uint8_t m_numRxAntennas;
  NiChangesPerBand m_niChanges;
  FirstPowerPerBand m_firstPowers;
  bool m_rxing;

  NiChanges::iterator GetNextPosition(Time moment,
                                      NiChangesPerBand::iterator niIt);
  NiChanges::iterator GetPreviousPosition(Time moment,
                                          NiChangesPerBand::iterator niIt);

  NiChanges::iterator AddNiChangeEvent(Time moment, NiChange change,
                                       NiChangesPerBand::iterator niIt);

  bool IsSameMuMimoTransmission(Ptr<const Event> currentEvent,
                                Ptr<const Event> otherEvent) const;
};

} // namespace ns3

#endif
