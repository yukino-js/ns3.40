
#ifndef HT_CAPABILITIES_H
#define HT_CAPABILITIES_H

#include "ns3/wifi-information-element.h"

#define MAX_SUPPORTED_MCS (77)

namespace ns3 {

class HtCapabilities : public WifiInformationElement {
public:
  HtCapabilities();

  WifiInformationElementId ElementId() const override;

  void SetHtCapabilitiesInfo(uint16_t ctrl);
  void SetAmpduParameters(uint8_t ctrl);
  void SetSupportedMcsSet(uint64_t ctrl1, uint64_t ctrl2);
  void SetExtendedHtCapabilities(uint16_t ctrl);
  void SetTxBfCapabilities(uint32_t ctrl);
  void SetAntennaSelectionCapabilities(uint8_t ctrl);

  void SetLdpc(uint8_t ldpc);
  void SetSupportedChannelWidth(uint8_t supportedChannelWidth);
  void SetShortGuardInterval20(uint8_t shortGuardInterval);
  void SetShortGuardInterval40(uint8_t shortGuardInterval);
  void SetMaxAmsduLength(uint16_t maxAmsduLength);
  void SetLSigProtectionSupport(uint8_t lSigProtection);

  void SetMaxAmpduLength(uint32_t maxAmpduLength);

  void SetRxMcsBitmask(uint8_t index);
  void SetRxHighestSupportedDataRate(uint16_t maxSupportedRate);
  void SetTxMcsSetDefined(uint8_t txMcsSetDefined);
  void SetTxRxMcsSetUnequal(uint8_t txRxMcsSetUnequal);
  void SetTxMaxNSpatialStreams(uint8_t maxTxSpatialStreams);
  void SetTxUnequalModulation(uint8_t txUnequalModulation);

  uint16_t GetHtCapabilitiesInfo() const;
  uint8_t GetAmpduParameters() const;
  uint64_t GetSupportedMcsSet1() const;
  uint64_t GetSupportedMcsSet2() const;
  uint16_t GetExtendedHtCapabilities() const;
  uint32_t GetTxBfCapabilities() const;
  uint8_t GetAntennaSelectionCapabilities() const;

  uint8_t GetLdpc() const;
  uint8_t GetSupportedChannelWidth() const;
  uint8_t GetShortGuardInterval20() const;
  uint16_t GetMaxAmsduLength() const;
  uint32_t GetMaxAmpduLength() const;
  bool IsSupportedMcs(uint8_t mcs) const;
  uint8_t GetRxHighestSupportedAntennas() const;

private:
  uint16_t GetInformationFieldSize() const override;
  void SerializeInformationField(Buffer::Iterator start) const override;
  uint16_t DeserializeInformationField(Buffer::Iterator start,
                                       uint16_t length) override;
  void Print(std::ostream &os) const override;

  uint8_t m_ldpc;
  uint8_t m_supportedChannelWidth;
  uint8_t m_smPowerSave;
  uint8_t m_greenField;
  uint8_t m_shortGuardInterval20;
  uint8_t m_shortGuardInterval40;
  uint8_t m_txStbc;
  uint8_t m_rxStbc;
  uint8_t m_htDelayedBlockAck;
  uint8_t m_maxAmsduLength;
  uint8_t m_dssMode40;
  uint8_t m_psmpSupport;
  uint8_t m_fortyMhzIntolerant;
  uint8_t m_lsigProtectionSupport;

  uint8_t m_maxAmpduLengthExponent;
  uint8_t m_minMpduStartSpace;
  uint8_t m_ampduReserved;

  uint8_t m_reservedMcsSet1;
  uint16_t m_rxHighestSupportedDataRate;
  uint8_t m_reservedMcsSet2;
  uint8_t m_txMcsSetDefined;
  uint8_t m_txRxMcsSetUnequal;
  uint8_t m_txMaxNSpatialStreams;
  uint8_t m_txUnequalModulation;
  uint32_t m_reservedMcsSet3;
  uint8_t m_rxMcsBitmask[MAX_SUPPORTED_MCS];

  uint8_t m_pco;
  uint8_t m_pcoTransitionTime;
  uint8_t m_reservedExtendedCapabilities;
  uint8_t m_mcsFeedback;
  uint8_t m_htcSupport;
  uint8_t m_reverseDirectionResponder;
  uint8_t m_reservedExtendedCapabilities2;

  uint8_t m_implicitRxBfCapable;
  uint8_t m_rxStaggeredSoundingCapable;
  uint8_t m_txStaggeredSoundingCapable;
  uint8_t m_rxNdpCapable;
  uint8_t m_txNdpCapable;
  uint8_t m_implicitTxBfCapable;
  uint8_t m_calibration;
  uint8_t m_explicitCsiTxBfCapable;
  uint8_t m_explicitNoncompressedSteeringCapable;
  uint8_t m_explicitCompressedSteeringCapable;
  uint8_t m_explicitTxBfCsiFeedback;
  uint8_t m_explicitNoncompressedBfFeedbackCapable;
  uint8_t m_explicitCompressedBfFeedbackCapable;
  uint8_t m_minimalGrouping;
  uint8_t m_csiNBfAntennasSupported;
  uint8_t m_noncompressedSteeringNBfAntennasSupported;
  uint8_t m_compressedSteeringNBfAntennasSupported;
  uint8_t m_csiMaxNRowsBfSupported;
  uint8_t m_channelEstimationCapability;
  uint8_t m_reservedTxBf;

  uint8_t m_antennaSelectionCapability;
  uint8_t m_explicitCsiFeedbackBasedTxASelCapable;
  uint8_t m_antennaIndicesFeedbackBasedTxASelCapable;
  uint8_t m_explicitCsiFeedbackCapable;
  uint8_t m_antennaIndicesFeedbackCapable;
  uint8_t m_rxASelCapable;
  uint8_t m_txSoundingPpdusCapable;
  uint8_t m_reservedASel;
};

} // namespace ns3

#endif
