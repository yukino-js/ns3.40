
#ifndef HE_PPDU_H
#define HE_PPDU_H

#include "ns3/ofdm-ppdu.h"

#include <variant>

namespace ns3 {

constexpr size_t WIFI_MAX_NUM_HE_SIGB_CONTENT_CHANNELS = 2;

class WifiPsdu;

class HePpdu : public OfdmPpdu {
public:
  struct HeSigBUserSpecificField {
    uint16_t staId : 11;
    uint8_t nss : 4;
    uint8_t mcs : 4;
  };

  using HeSigBContentChannels =
      std::vector<std::vector<HeSigBUserSpecificField>>;

  struct HeSuSigHeader {
    uint8_t m_format{1};
    uint8_t m_bssColor{0};
    uint8_t m_mcs{0};
    uint8_t m_bandwidth{0};
    uint8_t m_giLtfSize{0};
    uint8_t m_nsts{0};
  };

  struct HeTbSigHeader {
    uint8_t m_format{0};
    uint8_t m_bssColor{0};
    uint8_t m_bandwidth{0};
  };

  struct HeMuSigHeader {
    uint8_t m_bssColor{0};
    uint8_t m_bandwidth{0};
    uint8_t m_sigBMcs{0};
    uint8_t m_muMimoUsers;
    uint8_t m_sigBCompression{0};
    uint8_t m_giLtfSize{0};

    RuAllocation m_ruAllocation;
    HeSigBContentChannels m_contentChannels;
    std::optional<Center26ToneRuIndication> m_center26ToneRuIndication;
  };

  using HeSigHeader =
      std::variant<std::monostate, HeSuSigHeader, HeTbSigHeader, HeMuSigHeader>;

  enum TxPsdFlag { PSD_NON_HE_PORTION, PSD_HE_PORTION };

  HePpdu(Ptr<const WifiPsdu> psdu, const WifiTxVector &txVector,
         const WifiPhyOperatingChannel &channel, Time ppduDuration,
         uint64_t uid);
  HePpdu(const WifiConstPsduMap &psdus, const WifiTxVector &txVector,
         const WifiPhyOperatingChannel &channel, Time ppduDuration,
         uint64_t uid, TxPsdFlag flag);

  Time GetTxDuration() const override;
  Ptr<WifiPpdu> Copy() const override;
  WifiPpduType GetType() const override;
  uint16_t GetStaId() const override;
  uint16_t GetTxChannelWidth() const override;

  Ptr<const WifiPsdu> GetPsdu(uint8_t bssColor,
                              uint16_t staId = SU_STA_ID) const;

  TxPsdFlag GetTxPsdFlag() const;

  void SetTxPsdFlag(TxPsdFlag flag) const;

  void
  UpdateTxVectorForUlMu(const std::optional<WifiTxVector> &trigVector) const;

  static std::pair<std::size_t, std::size_t> GetNumRusPerHeSigBContentChannel(
      uint16_t channelWidth, const RuAllocation &ruAllocation,
      bool sigBCompression, uint8_t numMuMimoUsers);

  static HeSigBContentChannels
  GetHeSigBContentChannels(const WifiTxVector &txVector, uint8_t p20Index);

  static uint32_t GetSigBFieldSize(uint16_t channelWidth,
                                   const RuAllocation &ruAllocation,
                                   bool sigBCompression,
                                   std::size_t numMuMimoUsers);

protected:
  virtual void SetTxVectorFromPhyHeaders(WifiTxVector &txVector) const;

  void SetHeMuUserInfos(WifiTxVector &txVector,
                        const RuAllocation &ruAllocation,
                        const HeSigBContentChannels &contentChannels,
                        bool sigBCompression, uint8_t numMuMimoUsers) const;

  static uint8_t GetChannelWidthEncodingFromMhz(uint16_t channelWidth);

  static uint8_t GetNstsEncodingFromNss(uint8_t nss);

  static uint8_t GetGuardIntervalAndNltfEncoding(uint16_t gi, uint8_t nltf);

  static uint8_t GetNssFromNstsEncoding(uint8_t nsts);

  static uint16_t GetChannelWidthMhzFromEncoding(uint8_t bandwidth);

  static uint16_t GetGuardIntervalFromEncoding(uint8_t giAndNltfSize);

  static uint8_t GetMuMimoUsersEncoding(uint8_t nUsers);

  static uint8_t GetMuMimoUsersFromEncoding(uint8_t encoding);

  mutable TxPsdFlag m_txPsdFlag;

private:
  std::string PrintPayload() const override;
  WifiTxVector DoGetTxVector() const override;

  void SetPhyHeaders(const WifiTxVector &txVector, Time ppduDuration);

  void SetLSigHeader(Time ppduDuration);

  void SetHeSigHeader(const WifiTxVector &txVector);

  virtual bool IsMu() const;

  virtual bool IsDlMu() const;

  virtual bool IsUlMu() const;

  HeSigHeader m_heSig;
};

std::ostream &operator<<(std::ostream &os, const HePpdu::TxPsdFlag &flag);

} // namespace ns3

#endif
