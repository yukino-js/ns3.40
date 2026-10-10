
#ifndef HT_PHY_H
#define HT_PHY_H

#include "ns3/ofdm-phy.h"

namespace ns3 {

#define HT_PHY 127

constexpr uint8_t HT_MAX_NSS = 4;

class HtPhy : public OfdmPhy {
public:
  HtPhy(uint8_t maxNss = 1, bool buildModeList = true);
  ~HtPhy() override;

  WifiMode GetMcs(uint8_t index) const override;
  bool IsMcsSupported(uint8_t index) const override;
  bool HandlesMcsModes() const override;
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

  static WifiMode GetLSigMode();
  virtual WifiMode GetHtSigMode() const;

  uint8_t GetBssMembershipSelector() const;

  uint8_t GetMaxSupportedMcsIndexPerSs() const;
  void SetMaxSupportedMcsIndexPerSs(uint8_t maxIndex);
  void SetMaxSupportedNss(uint8_t maxNss);

  virtual Time GetLSigDuration(WifiPreamble preamble) const;
  virtual Time GetTrainingDuration(const WifiTxVector &txVector,
                                   uint8_t nDataLtf,
                                   uint8_t nExtensionLtf = 0) const;
  virtual Time GetHtSigDuration() const;

  static void InitializeModes();
  static WifiMode GetHtMcs(uint8_t index);

  static WifiMode GetHtMcs0();
  static WifiMode GetHtMcs1();
  static WifiMode GetHtMcs2();
  static WifiMode GetHtMcs3();
  static WifiMode GetHtMcs4();
  static WifiMode GetHtMcs5();
  static WifiMode GetHtMcs6();
  static WifiMode GetHtMcs7();
  static WifiMode GetHtMcs8();
  static WifiMode GetHtMcs9();
  static WifiMode GetHtMcs10();
  static WifiMode GetHtMcs11();
  static WifiMode GetHtMcs12();
  static WifiMode GetHtMcs13();
  static WifiMode GetHtMcs14();
  static WifiMode GetHtMcs15();
  static WifiMode GetHtMcs16();
  static WifiMode GetHtMcs17();
  static WifiMode GetHtMcs18();
  static WifiMode GetHtMcs19();
  static WifiMode GetHtMcs20();
  static WifiMode GetHtMcs21();
  static WifiMode GetHtMcs22();
  static WifiMode GetHtMcs23();
  static WifiMode GetHtMcs24();
  static WifiMode GetHtMcs25();
  static WifiMode GetHtMcs26();
  static WifiMode GetHtMcs27();
  static WifiMode GetHtMcs28();
  static WifiMode GetHtMcs29();
  static WifiMode GetHtMcs30();
  static WifiMode GetHtMcs31();

  static WifiCodeRate GetHtCodeRate(uint8_t mcsValue);
  static WifiCodeRate GetCodeRate(uint8_t mcsValue);
  static uint16_t GetHtConstellationSize(uint8_t mcsValue);
  static uint16_t GetConstellationSize(uint8_t mcsValue);
  static uint64_t GetPhyRate(uint8_t mcsValue, uint16_t channelWidth,
                             uint16_t guardInterval, uint8_t nss);
  static uint64_t GetPhyRateFromTxVector(const WifiTxVector &txVector,
                                         uint16_t staId);
  static uint64_t GetDataRateFromTxVector(const WifiTxVector &txVector,
                                          uint16_t staId);
  static uint64_t GetDataRate(uint8_t mcsValue, uint16_t channelWidth,
                              uint16_t guardInterval, uint8_t nss);
  static uint64_t GetNonHtReferenceRate(uint8_t mcsValue);
  static bool IsAllowed(const WifiTxVector &txVector);

protected:
  PhyFieldRxStatus DoEndReceiveField(WifiPpduField field,
                                     Ptr<Event> event) override;
  bool IsAllConfigSupported(WifiPpduField field,
                            Ptr<const WifiPpdu> ppdu) const override;
  bool IsConfigSupported(Ptr<const WifiPpdu> ppdu) const override;
  Ptr<SpectrumValue>
  GetTxPowerSpectralDensity(double txPowerW,
                            Ptr<const WifiPpdu> ppdu) const override;
  uint32_t GetMaxPsduSize() const override;
  CcaIndication GetCcaIndication(const Ptr<const WifiPpdu> ppdu) override;

  virtual void BuildModeList();

  virtual uint8_t GetNumberBccEncoders(const WifiTxVector &txVector) const;
  virtual Time GetSymbolDuration(const WifiTxVector &txVector) const;

  static uint64_t CalculatePhyRate(WifiCodeRate codeRate, uint64_t dataRate);
  static uint64_t CalculateNonHtReferenceRate(WifiCodeRate codeRate,
                                              uint16_t constellationSize);
  static double GetCodeRatio(WifiCodeRate codeRate);
  static uint64_t CalculateDataRate(Time symbolDuration,
                                    uint16_t usableSubCarriers,
                                    uint16_t numberOfBitsPerSubcarrier,
                                    double codingRate, uint8_t nss);

  static Time GetSymbolDuration(uint16_t channelWidth);

  static uint16_t GetUsableSubcarriers(uint16_t channelWidth);

  static Time GetSymbolDuration(Time guardInterval);

  uint8_t m_maxMcsIndexPerSs;
  uint8_t m_maxSupportedMcsIndexPerSs;
  uint8_t m_bssMembershipSelector;

private:
  PhyFieldRxStatus EndReceiveHtSig(Ptr<Event> event);

  static WifiMode CreateHtMcs(uint8_t index);

  uint8_t m_maxSupportedNss;

  static const PpduFormats m_htPpduFormats;
};

} // namespace ns3

#endif
