
#ifndef EHT_PPDU_H
#define EHT_PPDU_H

#include "ns3/he-ppdu.h"

#include <optional>

namespace ns3 {

class EhtPpdu : public HePpdu {
public:
  struct EhtTbPhyHeader {
    uint8_t m_phyVersionId{0};
    uint8_t m_bandwidth{0};
    uint8_t m_bssColor{0};
    uint8_t m_ppduType{0};
  };

  struct EhtMuPhyHeader {
    uint8_t m_phyVersionId{0};
    uint8_t m_bandwidth{0};
    uint8_t m_bssColor{0};
    uint8_t m_ppduType{0};
    uint8_t m_ehtSigMcs{0};

    uint8_t m_giLtfSize{0};

    std::optional<RuAllocation> m_ruAllocationA;
    std::optional<RuAllocation> m_ruAllocationB;

    HeSigBContentChannels m_contentChannels;
  };

  using EhtPhyHeader =
      std::variant<std::monostate, EhtTbPhyHeader, EhtMuPhyHeader>;

  EhtPpdu(const WifiConstPsduMap &psdus, const WifiTxVector &txVector,
          const WifiPhyOperatingChannel &channel, Time ppduDuration,
          uint64_t uid, TxPsdFlag flag);

  WifiPpduType GetType() const override;
  Ptr<WifiPpdu> Copy() const override;

  static std::pair<std::size_t, std::size_t>
  GetNumRusPerEhtSigBContentChannel(uint16_t channelWidth, uint8_t ehtPpduType,
                                    const std::vector<uint8_t> &ruAllocation,
                                    bool compression,
                                    std::size_t numMuMimoUsers);

  static HeSigBContentChannels
  GetEhtSigContentChannels(const WifiTxVector &txVector, uint8_t p20Index);

  static uint32_t GetEhtSigFieldSize(uint16_t channelWidth,
                                     const std::vector<uint8_t> &ruAllocation,
                                     uint8_t ehtPpduType, bool compression,
                                     std::size_t numMuMimoUsers);

private:
  bool IsDlMu() const override;
  bool IsUlMu() const override;
  void SetTxVectorFromPhyHeaders(WifiTxVector &txVector) const override;

  void SetPhyHeaders(const WifiTxVector &txVector, Time ppduDuration);

  void SetEhtPhyHeader(const WifiTxVector &txVector);

  EhtPhyHeader m_ehtPhyHeader;
};

} // namespace ns3

#endif
