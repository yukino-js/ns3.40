
#ifndef OFDM_PHY_H
#define OFDM_PHY_H

#include "ns3/phy-entity.h"

namespace ns3 {

enum OfdmPhyVariant { OFDM_PHY_DEFAULT, OFDM_PHY_10_MHZ, OFDM_PHY_5_MHZ };

class OfdmPhy : public PhyEntity {
public:
  OfdmPhy(OfdmPhyVariant variant = OFDM_PHY_DEFAULT, bool buildModeList = true);
  ~OfdmPhy() override;

  WifiMode GetSigMode(WifiPpduField field,
                      const WifiTxVector &txVector) const override;
  const PpduFormats &GetPpduFormats() const override;
  Time GetDuration(WifiPpduField field,
                   const WifiTxVector &txVector) const override;
  Time GetPayloadDuration(uint32_t size, const WifiTxVector &txVector,
                          WifiPhyBand band, MpduType mpdutype, bool incFlag,
                          uint32_t &totalAmpduSize,
                          double &totalAmpduNumSymbols,
                          uint16_t staId) const override;
  Ptr<WifiPpdu> BuildPpdu(const WifiConstPsduMap &psdus,
                          const WifiTxVector &txVector,
                          Time ppduDuration) override;
  double GetCcaThreshold(const Ptr<const WifiPpdu> ppdu,
                         WifiChannelListType channelType) const override;
  Ptr<const WifiPpdu> GetRxPpduFromTxPpdu(Ptr<const WifiPpdu> ppdu) override;

  static void InitializeModes();
  static WifiMode GetOfdmRate(uint64_t rate, uint16_t bw = 20);
  static WifiMode GetOfdmRate6Mbps();
  static WifiMode GetOfdmRate9Mbps();
  static WifiMode GetOfdmRate12Mbps();
  static WifiMode GetOfdmRate18Mbps();
  static WifiMode GetOfdmRate24Mbps();
  static WifiMode GetOfdmRate36Mbps();
  static WifiMode GetOfdmRate48Mbps();
  static WifiMode GetOfdmRate54Mbps();
  static WifiMode GetOfdmRate3MbpsBW10MHz();
  static WifiMode GetOfdmRate4_5MbpsBW10MHz();
  static WifiMode GetOfdmRate6MbpsBW10MHz();
  static WifiMode GetOfdmRate9MbpsBW10MHz();
  static WifiMode GetOfdmRate12MbpsBW10MHz();
  static WifiMode GetOfdmRate18MbpsBW10MHz();
  static WifiMode GetOfdmRate24MbpsBW10MHz();
  static WifiMode GetOfdmRate27MbpsBW10MHz();
  static WifiMode GetOfdmRate1_5MbpsBW5MHz();
  static WifiMode GetOfdmRate2_25MbpsBW5MHz();
  static WifiMode GetOfdmRate3MbpsBW5MHz();
  static WifiMode GetOfdmRate4_5MbpsBW5MHz();
  static WifiMode GetOfdmRate6MbpsBW5MHz();
  static WifiMode GetOfdmRate9MbpsBW5MHz();
  static WifiMode GetOfdmRate12MbpsBW5MHz();
  static WifiMode GetOfdmRate13_5MbpsBW5MHz();

  static WifiCodeRate GetCodeRate(const std::string &name);
  static uint16_t GetConstellationSize(const std::string &name);
  static uint64_t GetPhyRate(const std::string &name, uint16_t channelWidth);

  static uint64_t GetPhyRateFromTxVector(const WifiTxVector &txVector,
                                         uint16_t staId);
  static uint64_t GetDataRateFromTxVector(const WifiTxVector &txVector,
                                          uint16_t staId);
  static uint64_t GetDataRate(const std::string &name, uint16_t channelWidth);
  static bool IsAllowed(const WifiTxVector &txVector);

protected:
  PhyFieldRxStatus DoEndReceiveField(WifiPpduField field,
                                     Ptr<Event> event) override;
  Ptr<SpectrumValue>
  GetTxPowerSpectralDensity(double txPowerW,
                            Ptr<const WifiPpdu> ppdu) const override;
  uint32_t GetMaxPsduSize() const override;
  uint16_t
  GetMeasurementChannelWidth(const Ptr<const WifiPpdu> ppdu) const override;

  virtual WifiMode GetHeaderMode(const WifiTxVector &txVector) const;

  virtual Time GetPreambleDuration(const WifiTxVector &txVector) const;
  virtual Time GetHeaderDuration(const WifiTxVector &txVector) const;

  uint8_t GetNumberServiceBits() const;
  Time GetSignalExtension(WifiPhyBand band) const;

  PhyFieldRxStatus EndReceiveHeader(Ptr<Event> event);

  virtual bool IsChannelWidthSupported(Ptr<const WifiPpdu> ppdu) const;
  virtual bool IsAllConfigSupported(WifiPpduField field,
                                    Ptr<const WifiPpdu> ppdu) const;

  static uint64_t CalculatePhyRate(WifiCodeRate codeRate, uint64_t dataRate);
  static double GetCodeRatio(WifiCodeRate codeRate);
  static uint64_t CalculateDataRate(WifiCodeRate codeRate,
                                    uint16_t constellationSize,
                                    uint16_t channelWidth);
  static uint64_t CalculateDataRate(Time symbolDuration,
                                    uint16_t usableSubCarriers,
                                    uint16_t numberOfBitsPerSubcarrier,
                                    double codingRate);

  static uint16_t GetUsableSubcarriers();

  static Time GetSymbolDuration(uint16_t channelWidth);

private:
  static WifiMode CreateOfdmMode(std::string uniqueName, bool isMandatory);

  static const PpduFormats m_ofdmPpduFormats;

  static const ModulationLookupTable m_ofdmModulationLookupTable;
};

} // namespace ns3

#endif
