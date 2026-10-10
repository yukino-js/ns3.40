
#ifndef HT_OPERATION_H
#define HT_OPERATION_H

#include "ns3/wifi-information-element.h"

#define MAX_SUPPORTED_MCS (77)

namespace ns3 {

enum HtProtectionType {
  NO_PROTECTION,
  NON_MEMBER_PROTECTION,
  TWENTY_MHZ_PROTECTION,
  MIXED_MODE_PROTECTION
};

class HtOperation : public WifiInformationElement {
public:
  HtOperation();

  WifiInformationElementId ElementId() const override;
  void Print(std::ostream &os) const override;

  void SetPrimaryChannel(uint8_t ctrl);
  void SetInformationSubset1(uint8_t ctrl);
  void SetInformationSubset2(uint16_t ctrl);
  void SetInformationSubset3(uint16_t ctrl);
  void SetBasicMcsSet(uint64_t ctrl1, uint64_t ctrl2);

  void SetSecondaryChannelOffset(uint8_t secondaryChannelOffset);
  void SetStaChannelWidth(uint8_t staChannelWidth);
  void SetRifsMode(uint8_t rifsMode);

  void SetHtProtection(uint8_t htProtection);
  void SetNonGfHtStasPresent(uint8_t nonGfHtStasPresent);
  void SetObssNonHtStasPresent(uint8_t obssNonHtStasPresent);

  void SetDualBeacon(uint8_t dualBeacon);
  void SetDualCtsProtection(uint8_t dualCtsProtection);
  void SetStbcBeacon(uint8_t stbcBeacon);
  void SetLSigTxopProtectionFullSupport(uint8_t lSigTxopProtectionFullSupport);
  void SetPcoActive(uint8_t pcoActive);
  void SetPhase(uint8_t pcoPhase);

  void SetRxMcsBitmask(uint8_t index);
  void SetRxHighestSupportedDataRate(uint16_t maxSupportedRate);
  void SetTxMcsSetDefined(uint8_t txMcsSetDefined);
  void SetTxRxMcsSetUnequal(uint8_t txRxMcsSetUnequal);
  void SetTxMaxNSpatialStreams(uint8_t maxTxSpatialStreams);
  void SetTxUnequalModulation(uint8_t txUnequalModulation);

  uint8_t GetPrimaryChannel() const;
  uint8_t GetInformationSubset1() const;
  uint16_t GetInformationSubset2() const;
  uint16_t GetInformationSubset3() const;
  uint64_t GetBasicMcsSet1() const;
  uint64_t GetBasicMcsSet2() const;

  uint8_t GetSecondaryChannelOffset() const;
  uint8_t GetStaChannelWidth() const;
  uint8_t GetRifsMode() const;

  uint8_t GetHtProtection() const;
  uint8_t GetNonGfHtStasPresent() const;
  uint8_t GetObssNonHtStasPresent() const;

  uint8_t GetDualBeacon() const;
  uint8_t GetDualCtsProtection() const;
  uint8_t GetStbcBeacon() const;
  uint8_t GetLSigTxopProtectionFullSupport() const;
  uint8_t GetPcoActive() const;
  uint8_t GetPhase() const;

  bool IsSupportedMcs(uint8_t mcs) const;
  uint16_t GetRxHighestSupportedDataRate() const;
  uint8_t GetTxMcsSetDefined() const;
  uint8_t GetTxRxMcsSetUnequal() const;
  uint8_t GetTxMaxNSpatialStreams() const;
  uint8_t GetTxUnequalModulation() const;

private:
  uint16_t GetInformationFieldSize() const override;
  void SerializeInformationField(Buffer::Iterator start) const override;
  uint16_t DeserializeInformationField(Buffer::Iterator start,
                                       uint16_t length) override;

  uint8_t m_primaryChannel;

  uint8_t m_secondaryChannelOffset;
  uint8_t m_staChannelWidth;
  uint8_t m_rifsMode;
  uint8_t m_reservedInformationSubset1;

  uint8_t m_htProtection;
  uint8_t m_nonGfHtStasPresent;
  uint8_t m_reservedInformationSubset2_1;
  uint8_t m_obssNonHtStasPresent;
  uint8_t m_reservedInformationSubset2_2;

  uint8_t m_reservedInformationSubset3_1;
  uint8_t m_dualBeacon;
  uint8_t m_dualCtsProtection;
  uint8_t m_stbcBeacon;
  uint8_t m_lSigTxopProtectionFullSupport;
  uint8_t m_pcoActive;
  uint8_t m_pcoPhase;
  uint8_t m_reservedInformationSubset3_2;

  uint8_t m_reservedMcsSet1;
  uint16_t m_rxHighestSupportedDataRate;
  uint8_t m_reservedMcsSet2;
  uint8_t m_txMcsSetDefined;
  uint8_t m_txRxMcsSetUnequal;
  uint8_t m_txMaxNSpatialStreams;
  uint8_t m_txUnequalModulation;
  uint32_t m_reservedMcsSet3;
  uint8_t m_rxMcsBitmask[MAX_SUPPORTED_MCS];
};

} // namespace ns3

#endif
