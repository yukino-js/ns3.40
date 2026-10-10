
#ifndef VHT_CAPABILITIES_H
#define VHT_CAPABILITIES_H

#include "ns3/wifi-information-element.h"

namespace ns3 {

class VhtCapabilities : public WifiInformationElement {
public:
  VhtCapabilities();

  WifiInformationElementId ElementId() const override;
  void Print(std::ostream &os) const override;

  void SetVhtCapabilitiesInfo(uint32_t ctrl);
  void SetSupportedMcsAndNssSet(uint64_t ctrl);

  uint32_t GetVhtCapabilitiesInfo() const;
  uint64_t GetSupportedMcsAndNssSet() const;

  void SetMaxMpduLength(uint16_t length);
  void SetSupportedChannelWidthSet(uint8_t channelWidthSet);
  void SetRxLdpc(uint8_t rxLdpc);
  void SetShortGuardIntervalFor80Mhz(uint8_t shortGuardInterval);
  void SetShortGuardIntervalFor160Mhz(uint8_t shortGuardInterval);
  void SetRxStbc(uint8_t rxStbc);
  void SetTxStbc(uint8_t txStbc);
  void SetMaxAmpduLength(uint32_t maxAmpduLength);

  uint16_t GetMaxMpduLength() const;
  uint8_t GetSupportedChannelWidthSet() const;
  uint8_t GetRxLdpc() const;
  uint8_t GetRxStbc() const;
  uint8_t GetTxStbc() const;

  void SetRxMcsMap(uint8_t mcs, uint8_t nss);
  void SetTxMcsMap(uint8_t mcs, uint8_t nss);
  void SetRxHighestSupportedLgiDataRate(uint16_t supportedDatarate);
  void SetTxHighestSupportedLgiDataRate(uint16_t supportedDatarate);
  bool IsSupportedMcs(uint8_t mcs, uint8_t nss) const;

  uint16_t GetRxHighestSupportedLgiDataRate() const;

  bool IsSupportedTxMcs(uint8_t mcs) const;
  bool IsSupportedRxMcs(uint8_t mcs) const;

  uint32_t GetMaxAmpduLength() const;

private:
  uint16_t GetInformationFieldSize() const override;
  void SerializeInformationField(Buffer::Iterator start) const override;
  uint16_t DeserializeInformationField(Buffer::Iterator start,
                                       uint16_t length) override;

  uint8_t m_maxMpduLength;
  uint8_t m_supportedChannelWidthSet;
  uint8_t m_rxLdpc;
  uint8_t m_shortGuardIntervalFor80Mhz;
  uint8_t m_shortGuardIntervalFor160Mhz;
  uint8_t m_txStbc;
  uint8_t m_rxStbc;
  uint8_t m_suBeamformerCapable;
  uint8_t m_suBeamformeeCapable;
  uint8_t m_beamformeeStsCapable;
  uint8_t m_numberOfSoundingDimensions;
  uint8_t m_muBeamformerCapable;
  uint8_t m_muBeamformeeCapable;
  uint8_t m_vhtTxopPs;
  uint8_t m_htcVhtCapable;
  uint8_t m_maxAmpduLengthExponent;
  uint8_t m_vhtLinkAdaptationCapable;
  uint8_t m_rxAntennaPatternConsistency;
  uint8_t m_txAntennaPatternConsistency;

  std::vector<uint8_t> m_rxMcsMap;
  uint16_t m_rxHighestSupportedLongGuardIntervalDataRate;
  std::vector<uint8_t> m_txMcsMap;
  uint16_t m_txHighestSupportedLongGuardIntervalDataRate;
};

} // namespace ns3

#endif
