
#ifndef EHT_PHY_H
#define EHT_PHY_H

#include "ns3/he-phy.h"

namespace ns3 {

#define EHT_PHY 121

class EhtPhy : public HePhy {
public:
  EhtPhy(bool buildModeList = true);
  ~EhtPhy() override;

  const PpduFormats &GetPpduFormats() const override;
  Time GetDuration(WifiPpduField field,
                   const WifiTxVector &txVector) const override;
  Ptr<WifiPpdu> BuildPpdu(const WifiConstPsduMap &psdus,
                          const WifiTxVector &txVector,
                          Time ppduDuration) override;
  WifiMode GetSigBMode(const WifiTxVector &txVector) const override;

  static void InitializeModes();

  static WifiMode GetEhtMcs(uint8_t index);

  static WifiMode GetEhtMcs0();
  static WifiMode GetEhtMcs1();
  static WifiMode GetEhtMcs2();
  static WifiMode GetEhtMcs3();
  static WifiMode GetEhtMcs4();
  static WifiMode GetEhtMcs5();
  static WifiMode GetEhtMcs6();
  static WifiMode GetEhtMcs7();
  static WifiMode GetEhtMcs8();
  static WifiMode GetEhtMcs9();
  static WifiMode GetEhtMcs10();
  static WifiMode GetEhtMcs11();
  static WifiMode GetEhtMcs12();
  static WifiMode GetEhtMcs13();

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

protected:
  void BuildModeList() override;
  WifiMode GetSigMode(WifiPpduField field,
                      const WifiTxVector &txVector) const override;
  PhyFieldRxStatus DoEndReceiveField(WifiPpduField field,
                                     Ptr<Event> event) override;
  PhyFieldRxStatus ProcessSig(Ptr<Event> event, PhyFieldRxStatus status,
                              WifiPpduField field) override;
  WifiPhyRxfailureReason GetFailureReason(WifiPpduField field) const override;
  Time
  CalculateNonHeDurationForHeTb(const WifiTxVector &txVector) const override;
  Time
  CalculateNonHeDurationForHeMu(const WifiTxVector &txVector) const override;
  uint32_t GetSigBSize(const WifiTxVector &txVector) const override;

  static WifiMode CreateEhtMcs(uint8_t index);

  static uint64_t CalculateNonHtReferenceRate(WifiCodeRate codeRate,
                                              uint16_t constellationSize);

  static const PpduFormats m_ehtPpduFormats;
};

} // namespace ns3

#endif
