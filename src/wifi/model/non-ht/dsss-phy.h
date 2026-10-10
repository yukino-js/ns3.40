
#ifndef DSSS_PHY_H
#define DSSS_PHY_H

#include "ns3/phy-entity.h"

#include <vector>

namespace ns3 {

class DsssPhy : public PhyEntity {
public:
  DsssPhy();
  ~DsssPhy() override;

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
  uint32_t GetMaxPsduSize() const override;

  static void InitializeModes();
  static WifiMode GetDsssRate(uint64_t rate);
  static WifiMode GetDsssRate1Mbps();
  static WifiMode GetDsssRate2Mbps();
  static WifiMode GetDsssRate5_5Mbps();
  static WifiMode GetDsssRate11Mbps();

  static WifiCodeRate GetCodeRate(const std::string &name);
  static uint16_t GetConstellationSize(const std::string &name);
  static uint64_t GetDataRateFromTxVector(const WifiTxVector &txVector,
                                          uint16_t staId);
  static uint64_t GetDataRate(const std::string &name,
                              WifiModulationClass modClass);
  static bool IsAllowed(const WifiTxVector &txVector);

private:
  PhyFieldRxStatus DoEndReceiveField(WifiPpduField field,
                                     Ptr<Event> event) override;
  Ptr<SpectrumValue>
  GetTxPowerSpectralDensity(double txPowerW,
                            Ptr<const WifiPpdu> ppdu) const override;
  uint16_t GetRxChannelWidth(const WifiTxVector &txVector) const override;
  uint16_t
  GetMeasurementChannelWidth(const Ptr<const WifiPpdu> ppdu) const override;

  WifiMode GetHeaderMode(const WifiTxVector &txVector) const;

  Time GetPreambleDuration(const WifiTxVector &txVector) const;
  Time GetHeaderDuration(const WifiTxVector &txVector) const;

  PhyFieldRxStatus EndReceiveHeader(Ptr<Event> event);

  static WifiMode CreateDsssMode(std::string uniqueName,
                                 WifiModulationClass modClass);

  static const PpduFormats m_dsssPpduFormats;

  static const ModulationLookupTable m_dsssModulationLookupTable;
};

} // namespace ns3

#endif
