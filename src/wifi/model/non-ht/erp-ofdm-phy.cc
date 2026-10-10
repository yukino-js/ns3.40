
#include "erp-ofdm-phy.h"

#include "erp-ofdm-ppdu.h"

#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/wifi-phy.h"
#include "ns3/wifi-psdu.h"

#include <array>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("ErpOfdmPhy");

// clang-format off

const PhyEntity::ModulationLookupTable ErpOfdmPhy::m_erpOfdmModulationLookupTable {
    { "ErpOfdmRate6Mbps",  { WIFI_CODE_RATE_1_2, 2 } },
    { "ErpOfdmRate9Mbps",  { WIFI_CODE_RATE_3_4, 2 } },
    { "ErpOfdmRate12Mbps", { WIFI_CODE_RATE_1_2, 4 } },
    { "ErpOfdmRate18Mbps", { WIFI_CODE_RATE_3_4, 4 } },
    { "ErpOfdmRate24Mbps", { WIFI_CODE_RATE_1_2, 16 } },
    { "ErpOfdmRate36Mbps", { WIFI_CODE_RATE_3_4, 16 } },
    { "ErpOfdmRate48Mbps", { WIFI_CODE_RATE_2_3, 64 } },
    { "ErpOfdmRate54Mbps", { WIFI_CODE_RATE_3_4, 64 } }
};

static const std::array<uint64_t, 8> s_erpOfdmRatesBpsList =
    {  6000000,  9000000, 12000000, 18000000,
      24000000, 36000000, 48000000, 54000000};

// clang-format on

const std::array<uint64_t, 8> &GetErpOfdmRatesBpsList() {
  return s_erpOfdmRatesBpsList;
};

ErpOfdmPhy::ErpOfdmPhy() : OfdmPhy(OFDM_PHY_DEFAULT, false) {
  NS_LOG_FUNCTION(this);
  for (const auto &rate : GetErpOfdmRatesBpsList()) {
    WifiMode mode = GetErpOfdmRate(rate);
    NS_LOG_LOGIC("Add " << mode << " to list");
    m_modeList.emplace_back(mode);
  }
}

ErpOfdmPhy::~ErpOfdmPhy() { NS_LOG_FUNCTION(this); }

WifiMode ErpOfdmPhy::GetHeaderMode(const WifiTxVector &txVector) const {
  NS_ASSERT(txVector.GetMode().GetModulationClass() == WIFI_MOD_CLASS_ERP_OFDM);
  return GetErpOfdmRate6Mbps();
}

Time ErpOfdmPhy::GetPreambleDuration(const WifiTxVector &) const {
  return MicroSeconds(16);
}

Time ErpOfdmPhy::GetHeaderDuration(const WifiTxVector &) const {
  return MicroSeconds(4);
}

Ptr<WifiPpdu> ErpOfdmPhy::BuildPpdu(const WifiConstPsduMap &psdus,
                                    const WifiTxVector &txVector, Time) {
  NS_LOG_FUNCTION(this << psdus << txVector);
  return Create<ErpOfdmPpdu>(
      psdus.begin()->second, txVector, m_wifiPhy->GetOperatingChannel(),
      m_wifiPhy->GetLatestPhyEntity()->ObtainNextUid(txVector));
}

void ErpOfdmPhy::InitializeModes() {
  for (const auto &rate : GetErpOfdmRatesBpsList()) {
    GetErpOfdmRate(rate);
  }
}

WifiMode ErpOfdmPhy::GetErpOfdmRate(uint64_t rate) {
  switch (rate) {
  case 6000000:
    return GetErpOfdmRate6Mbps();
  case 9000000:
    return GetErpOfdmRate9Mbps();
  case 12000000:
    return GetErpOfdmRate12Mbps();
  case 18000000:
    return GetErpOfdmRate18Mbps();
  case 24000000:
    return GetErpOfdmRate24Mbps();
  case 36000000:
    return GetErpOfdmRate36Mbps();
  case 48000000:
    return GetErpOfdmRate48Mbps();
  case 54000000:
    return GetErpOfdmRate54Mbps();
  default:
    NS_ABORT_MSG("Inexistent rate (" << rate << " bps) requested for ERP-OFDM");
    return WifiMode();
  }
}

#define GET_ERP_OFDM_MODE(x, f)                                                \
  WifiMode ErpOfdmPhy::Get##x() {                                              \
    static WifiMode mode = CreateErpOfdmMode(#x, f);                           \
    return mode;                                                               \
  };

GET_ERP_OFDM_MODE(ErpOfdmRate6Mbps, true)
GET_ERP_OFDM_MODE(ErpOfdmRate9Mbps, false)
GET_ERP_OFDM_MODE(ErpOfdmRate12Mbps, true)
GET_ERP_OFDM_MODE(ErpOfdmRate18Mbps, false)
GET_ERP_OFDM_MODE(ErpOfdmRate24Mbps, true)
GET_ERP_OFDM_MODE(ErpOfdmRate36Mbps, false)
GET_ERP_OFDM_MODE(ErpOfdmRate48Mbps, false)
GET_ERP_OFDM_MODE(ErpOfdmRate54Mbps, false)
#undef GET_ERP_OFDM_MODE

WifiMode ErpOfdmPhy::CreateErpOfdmMode(std::string uniqueName,
                                       bool isMandatory) {
  const auto it = m_erpOfdmModulationLookupTable.find(uniqueName);
  NS_ASSERT_MSG(
      it != m_erpOfdmModulationLookupTable.end(),
      "ERP-OFDM mode cannot be created because it is not in the lookup table!");

  return WifiModeFactory::CreateWifiMode(
      uniqueName, WIFI_MOD_CLASS_ERP_OFDM, isMandatory,
      MakeBoundCallback(&GetCodeRate, uniqueName),
      MakeBoundCallback(&GetConstellationSize, uniqueName),
      MakeCallback(&GetPhyRateFromTxVector),
      MakeCallback(&GetDataRateFromTxVector), MakeCallback(&IsAllowed));
}

WifiCodeRate ErpOfdmPhy::GetCodeRate(const std::string &name) {
  return m_erpOfdmModulationLookupTable.at(name).first;
}

uint16_t ErpOfdmPhy::GetConstellationSize(const std::string &name) {
  return m_erpOfdmModulationLookupTable.at(name).second;
}

uint64_t ErpOfdmPhy::GetPhyRate(const std::string &name,
                                uint16_t channelWidth) {
  WifiCodeRate codeRate = GetCodeRate(name);
  uint16_t constellationSize = GetConstellationSize(name);
  uint64_t dataRate =
      OfdmPhy::CalculateDataRate(codeRate, constellationSize, channelWidth);
  return OfdmPhy::CalculatePhyRate(codeRate, dataRate);
}

uint64_t ErpOfdmPhy::GetPhyRateFromTxVector(const WifiTxVector &txVector,
                                            uint16_t) {
  return GetPhyRate(txVector.GetMode().GetUniqueName(),
                    txVector.GetChannelWidth());
}

uint64_t ErpOfdmPhy::GetDataRateFromTxVector(const WifiTxVector &txVector,
                                             uint16_t) {
  return GetDataRate(txVector.GetMode().GetUniqueName(),
                     txVector.GetChannelWidth());
}

uint64_t ErpOfdmPhy::GetDataRate(const std::string &name,
                                 uint16_t channelWidth) {
  WifiCodeRate codeRate = GetCodeRate(name);
  uint16_t constellationSize = GetConstellationSize(name);
  return OfdmPhy::CalculateDataRate(codeRate, constellationSize, channelWidth);
}

bool ErpOfdmPhy::IsAllowed(const WifiTxVector &) { return true; }

uint32_t ErpOfdmPhy::GetMaxPsduSize() const { return 4095; }

} // namespace ns3

namespace {

class ConstructorErpOfdm {
public:
  ConstructorErpOfdm() {
    ns3::ErpOfdmPhy::InitializeModes();
    ns3::WifiPhy::AddStaticPhyEntity(ns3::WIFI_MOD_CLASS_ERP_OFDM,
                                     ns3::Create<ns3::ErpOfdmPhy>());
  }
} g_constructor_erp_ofdm;

} // namespace
