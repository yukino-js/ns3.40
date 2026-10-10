
#ifndef ERP_OFDM_PHY_H
#define ERP_OFDM_PHY_H

#include "ofdm-phy.h"

namespace ns3 {

class ErpOfdmPhy : public OfdmPhy {
public:
  ErpOfdmPhy();
  ~ErpOfdmPhy() override;

  Ptr<WifiPpdu> BuildPpdu(const WifiConstPsduMap &psdus,
                          const WifiTxVector &txVector,
                          Time ppduDuration) override;
  uint32_t GetMaxPsduSize() const override;

  static void InitializeModes();
  static WifiMode GetErpOfdmRate(uint64_t rate);

  static WifiMode GetErpOfdmRate6Mbps();
  static WifiMode GetErpOfdmRate9Mbps();
  static WifiMode GetErpOfdmRate12Mbps();
  static WifiMode GetErpOfdmRate18Mbps();
  static WifiMode GetErpOfdmRate24Mbps();
  static WifiMode GetErpOfdmRate36Mbps();
  static WifiMode GetErpOfdmRate48Mbps();
  static WifiMode GetErpOfdmRate54Mbps();

  static WifiCodeRate GetCodeRate(const std::string &name);
  static uint16_t GetConstellationSize(const std::string &name);
  static uint64_t GetPhyRate(const std::string &name, uint16_t channelWidth);
  static uint64_t GetPhyRateFromTxVector(const WifiTxVector &txVector,
                                         uint16_t staId);
  static uint64_t GetDataRateFromTxVector(const WifiTxVector &txVector,
                                          uint16_t staId);
  static uint64_t GetDataRate(const std::string &name, uint16_t channelWidth);
  static bool IsAllowed(const WifiTxVector &txVector);

private:
  WifiMode GetHeaderMode(const WifiTxVector &txVector) const override;
  Time GetPreambleDuration(const WifiTxVector &txVector) const override;
  Time GetHeaderDuration(const WifiTxVector &txVector) const override;

  static WifiMode CreateErpOfdmMode(std::string uniqueName, bool isMandatory);

  static const ModulationLookupTable m_erpOfdmModulationLookupTable;
};

} // namespace ns3

#endif
