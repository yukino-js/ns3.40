
#ifndef WIFI_MODE_H
#define WIFI_MODE_H

#include "wifi-phy-common.h"

#include "ns3/attribute-helper.h"
#include "ns3/callback.h"

#include <vector>

namespace ns3 {

static constexpr uint16_t SU_STA_ID = 65535;

class WifiTxVector;

class WifiMode {
public:
  bool IsAllowed(uint16_t channelWidth, uint8_t nss) const;
  bool IsAllowed(const WifiTxVector &txVector) const;
  uint64_t GetPhyRate(uint16_t channelWidth, uint16_t guardInterval,
                      uint8_t nss) const;
  uint64_t GetPhyRate(const WifiTxVector &txVector,
                      uint16_t staId = SU_STA_ID) const;
  uint64_t GetPhyRate(uint16_t channelWidth) const;
  uint64_t GetDataRate(uint16_t channelWidth, uint16_t guardInterval,
                       uint8_t nss) const;
  uint64_t GetDataRate(const WifiTxVector &txVector,
                       uint16_t staId = SU_STA_ID) const;
  uint64_t GetDataRate(uint16_t channelWidth) const;

  WifiCodeRate GetCodeRate() const;
  uint16_t GetConstellationSize() const;
  uint8_t GetMcsValue() const;
  std::string GetUniqueName() const;
  bool IsMandatory() const;
  uint32_t GetUid() const;
  WifiModulationClass GetModulationClass() const;
  uint64_t GetNonHtReferenceRate() const;
  bool IsHigherCodeRate(WifiMode mode) const;
  bool IsHigherDataRate(WifiMode mode) const;

  WifiMode();
  WifiMode(std::string name);

private:
  friend class WifiModeFactory;
  WifiMode(uint32_t uid);
  uint32_t m_uid;
};

bool operator==(const WifiMode &a, const WifiMode &b);

bool operator!=(const WifiMode &a, const WifiMode &b);

bool operator<(const WifiMode &a, const WifiMode &b);

std::ostream &operator<<(std::ostream &os, const WifiMode &mode);
std::istream &operator>>(std::istream &is, WifiMode &mode);

ATTRIBUTE_HELPER_HEADER(WifiMode);

typedef std::vector<WifiMode> WifiModeList;
typedef WifiModeList::const_iterator WifiModeListIterator;

class WifiModeFactory {
public:
  typedef Callback<WifiCodeRate> CodeRateCallback;
  typedef Callback<uint16_t> ConstellationSizeCallback;
  typedef Callback<uint64_t, const WifiTxVector &, uint16_t> PhyRateCallback;
  typedef Callback<uint64_t, const WifiTxVector &, uint16_t> DataRateCallback;
  typedef Callback<uint64_t> NonHtReferenceRateCallback;
  typedef Callback<bool, const WifiTxVector &> AllowedCallback;

  static WifiMode
  CreateWifiMode(std::string uniqueName, WifiModulationClass modClass,
                 bool isMandatory, CodeRateCallback codeRateCallback,
                 ConstellationSizeCallback constellationSizeCallback,
                 PhyRateCallback phyRateCallback,
                 DataRateCallback dataRateCallback,
                 AllowedCallback isAllowedCallback);

  static WifiMode CreateWifiMcs(
      std::string uniqueName, uint8_t mcsValue, WifiModulationClass modClass,
      bool isMandatory, CodeRateCallback codeRateCallback,
      ConstellationSizeCallback constellationSizeCallback,
      PhyRateCallback phyRateCallback, DataRateCallback dataRateCallback,
      NonHtReferenceRateCallback nonHtReferenceRateCallback,
      AllowedCallback isAllowedCallback);

private:
  friend class WifiMode;
  friend std::istream &operator>>(std::istream &is, WifiMode &mode);

  static WifiModeFactory *GetFactory();
  WifiModeFactory();

  struct WifiModeItem {
    std::string uniqueUid;
    WifiModulationClass modClass;
    bool isMandatory;
    uint8_t mcsValue;
    CodeRateCallback GetCodeRateCallback;
    ConstellationSizeCallback GetConstellationSizeCallback;
    PhyRateCallback GetPhyRateCallback;
    DataRateCallback GetDataRateCallback;
    NonHtReferenceRateCallback GetNonHtReferenceRateCallback;
    AllowedCallback IsAllowedCallback;
  };

  WifiMode Search(std::string name) const;
  uint32_t AllocateUid(std::string uniqueUid);
  WifiModeItem *Get(uint32_t uid);

  typedef std::vector<WifiModeItem> WifiModeItemList;
  WifiModeItemList m_itemList;
};

} // namespace ns3

#endif
