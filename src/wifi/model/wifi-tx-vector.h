
#ifndef WIFI_TX_VECTOR_H
#define WIFI_TX_VECTOR_H

#include "wifi-mode.h"
#include "wifi-phy-common.h"

#include "ns3/he-ru.h"

#include <list>
#include <optional>
#include <set>
#include <vector>

namespace ns3 {

static constexpr uint16_t NO_USER_STA_ID = 2046;

struct HeMuUserInfo {
  HeRu::RuSpec ru;
  uint8_t mcs;
  uint8_t nss;

  bool operator==(const HeMuUserInfo &other) const;
  bool operator!=(const HeMuUserInfo &other) const;
};

using RuAllocation = std::vector<uint8_t>;

enum Center26ToneRuIndication : uint8_t {
  CENTER_26_TONE_RU_UNALLOCATED = 0,
  CENTER_26_TONE_RU_LOW_80_MHZ_ALLOCATED,
  CENTER_26_TONE_RU_HIGH_80_MHZ_ALLOCATED,
  CENTER_26_TONE_RU_LOW_AND_HIGH_80_MHZ_ALLOCATED,
  CENTER_26_TONE_RU_INDICATION_MAX
};

class WifiTxVector {
public:
  typedef std::map<uint16_t, HeMuUserInfo> HeMuUserInfoMap;

  WifiTxVector();
  ~WifiTxVector();
  WifiTxVector(WifiMode mode, uint8_t powerLevel, WifiPreamble preamble,
               uint16_t guardInterval, uint8_t nTx, uint8_t nss, uint8_t ness,
               uint16_t channelWidth, bool aggregation, bool stbc = false,
               bool ldpc = false, uint8_t bssColor = 0, uint16_t length = 0,
               bool triggerResponding = false);
  WifiTxVector(const WifiTxVector &txVector);

  bool GetModeInitialized() const;
  WifiMode GetMode(uint16_t staId = SU_STA_ID) const;
  void SetMode(WifiMode mode);
  void SetMode(WifiMode mode, uint16_t staId);

  WifiModulationClass GetModulationClass() const;

  uint8_t GetTxPowerLevel() const;
  void SetTxPowerLevel(uint8_t powerlevel);
  WifiPreamble GetPreambleType() const;
  void SetPreambleType(WifiPreamble preamble);
  uint16_t GetChannelWidth() const;
  void SetChannelWidth(uint16_t channelWidth);
  uint16_t GetGuardInterval() const;
  void SetGuardInterval(uint16_t guardInterval);
  uint8_t GetNTx() const;
  void SetNTx(uint8_t nTx);
  uint8_t GetNss(uint16_t staId = SU_STA_ID) const;
  uint8_t GetNssMax() const;
  uint8_t GetNssTotal() const;
  void SetNss(uint8_t nss);
  void SetNss(uint8_t nss, uint16_t staId);
  uint8_t GetNess() const;
  void SetNess(uint8_t ness);
  bool IsAggregation() const;
  void SetAggregation(bool aggregation);
  bool IsStbc() const;
  void SetStbc(bool stbc);
  bool IsLdpc() const;
  void SetLdpc(bool ldpc);
  bool IsNonHtDuplicate() const;
  void SetBssColor(uint8_t color);
  uint8_t GetBssColor() const;
  void SetLength(uint16_t length);
  uint16_t GetLength() const;
  bool IsTriggerResponding() const;
  void SetTriggerResponding(bool triggerResponding);
  bool IsValid() const;
  bool IsMu() const;
  bool IsDlMu() const;
  bool IsUlMu() const;
  bool IsDlOfdma() const;
  bool IsDlMuMimo() const;
  bool IsAllocated(uint16_t staId) const;
  HeRu::RuSpec GetRu(uint16_t staId) const;
  void SetRu(HeRu::RuSpec ru, uint16_t staId);
  HeMuUserInfo GetHeMuUserInfo(uint16_t staId) const;
  void SetHeMuUserInfo(uint16_t staId, HeMuUserInfo userInfo);
  const HeMuUserInfoMap &GetHeMuUserInfoMap() const;
  HeMuUserInfoMap &GetHeMuUserInfoMap();

  using UserInfoMapOrderedByRus =
      std::map<HeRu::RuSpec, std::set<uint16_t>, HeRu::RuSpecCompare>;

  UserInfoMapOrderedByRus GetUserInfoMapOrderedByRus(uint8_t p20Index) const;

  bool IsSigBCompression() const;

  void SetInactiveSubchannels(const std::vector<bool> &inactiveSubchannels);
  const std::vector<bool> &GetInactiveSubchannels() const;

  void SetSigBMode(const WifiMode &mode);

  WifiMode GetSigBMode() const;

  void SetRuAllocation(const RuAllocation &ruAlloc, uint8_t p20Index);

  const RuAllocation &GetRuAllocation(uint8_t p20Index) const;

  void SetCenter26ToneRuIndication(
      Center26ToneRuIndication center26ToneRuIndication);

  std::optional<Center26ToneRuIndication> GetCenter26ToneRuIndication() const;

  void SetEhtPpduType(uint8_t type);
  uint8_t GetEhtPpduType() const;

private:
  RuAllocation DeriveRuAllocation(uint8_t p20Index) const;

  Center26ToneRuIndication DeriveCenter26ToneRuIndication() const;

  uint8_t GetNumStasInRu(const HeRu::RuSpec &ru) const;

  WifiMode m_mode;
  uint8_t m_txPowerLevel;
  WifiPreamble m_preamble;
  uint16_t m_channelWidth;
  uint16_t m_guardInterval;
  uint8_t m_nTx;
  uint8_t m_nss;
  uint8_t m_ness;
  bool m_aggregation;
  bool m_stbc;
  bool m_ldpc;
  uint8_t m_bssColor;
  uint16_t m_length;
  bool m_triggerResponding;

  bool m_modeInitialized;

  HeMuUserInfoMap m_muUserInfos;
  std::vector<bool> m_inactiveSubchannels;

  WifiMode m_sigBMcs;

  mutable RuAllocation m_ruAllocation;

  mutable std::optional<Center26ToneRuIndication> m_center26ToneRuIndication;

  uint8_t m_ehtPpduType;
};

std::ostream &operator<<(std::ostream &os, const WifiTxVector &v);

} // namespace ns3

#endif
