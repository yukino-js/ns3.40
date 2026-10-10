
#ifndef UAN_TX_MODE_H
#define UAN_TX_MODE_H

#include "ns3/object.h"

#include <map>

namespace ns3 {

class UanTxModeFactory;
class UanTxMode;

class UanTxMode {
public:
  UanTxMode();
  ~UanTxMode();

  enum ModulationType { PSK, QAM, FSK, OTHER };

  ModulationType GetModType() const;
  uint32_t GetDataRateBps() const;
  uint32_t GetPhyRateSps() const;
  uint32_t GetCenterFreqHz() const;
  uint32_t GetBandwidthHz() const;
  uint32_t GetConstellationSize() const;
  std::string GetName() const;
  uint32_t GetUid() const;

private:
  friend class UanTxModeFactory;
  friend std::ostream &operator<<(std::ostream &os, const UanTxMode &mode);
  friend std::istream &operator>>(std::istream &is, UanTxMode &mode);

  uint32_t m_uid;
};

std::ostream &operator<<(std::ostream &os, const UanTxMode &mode);
std::istream &operator>>(std::istream &is, UanTxMode &mode);

class UanTxModeFactory {
public:
  UanTxModeFactory();
  ~UanTxModeFactory();

  static UanTxMode CreateMode(UanTxMode::ModulationType type,
                              uint32_t dataRateBps, uint32_t phyRateSps,
                              uint32_t cfHz, uint32_t bwHz, uint32_t constSize,
                              std::string name);

  static UanTxMode GetMode(std::string name);
  static UanTxMode GetMode(uint32_t uid);

private:
  friend class UanTxMode;
  uint32_t m_nextUid;

  struct UanTxModeItem {
    UanTxMode::ModulationType m_type;
    uint32_t m_cfHz;
    uint32_t m_bwHz;
    uint32_t m_dataRateBps;
    uint32_t m_phyRateSps;
    uint32_t m_constSize;
    uint32_t m_uid;
    std::string m_name;
  };

  std::map<uint32_t, UanTxModeItem> m_modes;

  bool NameUsed(std::string name);

  static UanTxModeFactory &GetFactory();

  UanTxModeItem &GetModeItem(uint32_t uid);

  UanTxModeItem &GetModeItem(std::string name);

  UanTxMode MakeModeFromItem(const UanTxModeItem &item);
};

class UanModesList {
public:
  UanModesList();
  virtual ~UanModesList();

  void AppendMode(UanTxMode mode);
  void DeleteMode(uint32_t num);
  UanTxMode operator[](uint32_t index) const;
  uint32_t GetNModes() const;

private:
  std::vector<UanTxMode> m_modes;

  friend std::ostream &operator<<(std::ostream &os, const UanModesList &ml);
  friend std::istream &operator>>(std::istream &is, UanModesList &ml);
};

std::ostream &operator<<(std::ostream &os, const UanModesList &ml);
std::istream &operator>>(std::istream &is, UanModesList &ml);

ATTRIBUTE_HELPER_HEADER(UanModesList);

} // namespace ns3

#endif
