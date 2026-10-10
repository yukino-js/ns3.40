
#ifndef VHT_PHY_H
#define VHT_PHY_H

#include "ns3/ht-phy.h"

namespace ns3 {

#define VHT_PHY 126

class VhtPhy : public HtPhy {
public:
  VhtPhy(bool buildModeList = true);
  ~VhtPhy() override;

  WifiMode GetSigMode(WifiPpduField field,
                      const WifiTxVector &txVector) const override;
  const PpduFormats &GetPpduFormats() const override;
  Time GetDuration(WifiPpduField field,
                   const WifiTxVector &txVector) const override;
  Time GetLSigDuration(WifiPreamble preamble) const override;
  Time GetTrainingDuration(const WifiTxVector &txVector, uint8_t nDataLtf,
                           uint8_t nExtensionLtf = 0) const override;
  Ptr<WifiPpdu> BuildPpdu(const WifiConstPsduMap &psdus,
                          const WifiTxVector &txVector,
                          Time ppduDuration) override;
  double GetCcaThreshold(const Ptr<const WifiPpdu> ppdu,
                         WifiChannelListType channelType) const override;

  virtual WifiMode GetSigAMode() const;
  virtual WifiMode GetSigBMode(const WifiTxVector &txVector) const;

  virtual Time GetSigADuration(WifiPreamble preamble) const;
  virtual Time GetSigBDuration(const WifiTxVector &txVector) const;

  static void InitializeModes();
  static WifiMode GetVhtMcs(uint8_t index);

  static WifiMode GetVhtMcs0();
  static WifiMode GetVhtMcs1();
  static WifiMode GetVhtMcs2();
  static WifiMode GetVhtMcs3();
  static WifiMode GetVhtMcs4();
  static WifiMode GetVhtMcs5();
  static WifiMode GetVhtMcs6();
  static WifiMode GetVhtMcs7();
  static WifiMode GetVhtMcs8();
  static WifiMode GetVhtMcs9();

  static WifiCodeRate GetCodeRate(uint8_t mcsValue);
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
  static bool IsCombinationAllowed(uint8_t mcsValue, uint16_t channelWidth,
                                   uint8_t nss);
  static bool IsAllowed(const WifiTxVector &txVector);

protected:
  WifiMode GetHtSigMode() const override;
  Time GetHtSigDuration() const override;
  uint8_t GetNumberBccEncoders(const WifiTxVector &txVector) const override;
  PhyFieldRxStatus DoEndReceiveField(WifiPpduField field,
                                     Ptr<Event> event) override;
  bool IsAllConfigSupported(WifiPpduField field,
                            Ptr<const WifiPpdu> ppdu) const override;
  uint32_t GetMaxPsduSize() const override;
  CcaIndication GetCcaIndication(const Ptr<const WifiPpdu> ppdu) override;

  PhyFieldRxStatus EndReceiveSig(Ptr<Event> event, WifiPpduField field);

  virtual WifiPhyRxfailureReason GetFailureReason(WifiPpduField field) const;

  virtual PhyFieldRxStatus ProcessSig(Ptr<Event> event, PhyFieldRxStatus status,
                                      WifiPpduField field);

  static uint64_t CalculateNonHtReferenceRate(WifiCodeRate codeRate,
                                              uint16_t constellationSize);
  static uint16_t GetUsableSubcarriers(uint16_t channelWidth);

private:
  void BuildModeList() override;

  static WifiMode CreateVhtMcs(uint8_t index);

  typedef std::map<std::tuple<uint16_t, uint8_t, uint8_t>, uint8_t>
      NesExceptionMap;
  static const NesExceptionMap m_exceptionsMap;
  static const PpduFormats m_vhtPpduFormats;
};

} // namespace ns3

#endif
