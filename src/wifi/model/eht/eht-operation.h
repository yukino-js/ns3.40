
#ifndef EHT_OPERATION_H
#define EHT_OPERATION_H

#include <ns3/wifi-information-element.h>

#include <optional>
#include <vector>

namespace ns3 {

constexpr uint8_t WIFI_EHT_MAX_MCS_INDEX = 13;
constexpr uint16_t WIFI_EHT_OP_PARAMS_SIZE_B = 1;
constexpr uint16_t WIFI_EHT_OP_INFO_BASIC_SIZE_B = 3;
constexpr uint16_t WIFI_EHT_DISABLED_SUBCH_BM_SIZE_B = 2;
constexpr uint16_t WIFI_EHT_BASIC_MCS_NSS_SET_SIZE_B = 4;
constexpr uint8_t WIFI_DEFAULT_EHT_MAX_NSS = 1;
constexpr uint8_t WIFI_EHT_MAX_NSS_CONFIGURABLE = 8;
constexpr uint8_t WIFI_DEFAULT_EHT_OP_INFO_PRESENT = 0;
constexpr uint8_t WIFI_DEFAULT_EHT_OP_DIS_SUBCH_BM_PRESENT = 0;
constexpr uint8_t WIFI_DEFAULT_EHT_OP_PE_DUR = 0;
constexpr uint8_t WIFI_DEFAULT_GRP_BU_IND_LIMIT = 0;
constexpr uint8_t WIFI_DEFAULT_GRP_BU_EXP = 0;

class EhtOperation : public WifiInformationElement {
public:
  struct EhtOpParams {
    uint8_t opInfoPresent{WIFI_DEFAULT_EHT_OP_INFO_PRESENT};
    uint8_t disabledSubchBmPresent{WIFI_DEFAULT_EHT_OP_DIS_SUBCH_BM_PRESENT};
    uint8_t defaultPeDur{WIFI_DEFAULT_EHT_OP_PE_DUR};
    uint8_t grpBuIndLimit{WIFI_DEFAULT_GRP_BU_IND_LIMIT};
    uint8_t grpBuExp{WIFI_DEFAULT_GRP_BU_EXP};

    void Serialize(Buffer::Iterator &start) const;
    uint16_t Deserialize(Buffer::Iterator start);
  };

  struct EhtOpControl {
    uint8_t channelWidth : 3;
  };

  struct EhtOpInfo {
    EhtOpControl control;
    uint8_t ccfs0;
    uint8_t ccfs1;
    std::optional<uint16_t> disabledSubchBm;

    void Serialize(Buffer::Iterator &start) const;
    uint16_t Deserialize(Buffer::Iterator start, bool disabledSubchBmPresent);
  };

  struct EhtBasicMcsNssSet {
    std::vector<uint8_t> maxRxNss{};
    std::vector<uint8_t> maxTxNss{};

    void Serialize(Buffer::Iterator &start) const;
    uint16_t Deserialize(Buffer::Iterator start);
  };

  EhtOperation();
  WifiInformationElementId ElementId() const override;
  WifiInformationElementId ElementIdExt() const override;
  void Print(std::ostream &os) const override;

  void SetMaxRxNss(uint8_t maxNss, uint8_t mcsStart, uint8_t mcsEnd);
  void SetMaxTxNss(uint8_t maxNss, uint8_t mcsStart, uint8_t mcsEnd);

  EhtOpParams m_params;
  EhtBasicMcsNssSet m_mcsNssSet;
  std::optional<EhtOpInfo> m_opInfo;

private:
  uint16_t GetInformationFieldSize() const override;
  void SerializeInformationField(Buffer::Iterator start) const override;
  uint16_t DeserializeInformationField(Buffer::Iterator start,
                                       uint16_t length) override;
};

} // namespace ns3

#endif
