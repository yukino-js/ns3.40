
#ifndef HE_CAPABILITIES_H
#define HE_CAPABILITIES_H

#include "ns3/wifi-information-element.h"

namespace ns3 {

class HeCapabilities : public WifiInformationElement {
public:
  HeCapabilities();

  WifiInformationElementId ElementId() const override;
  WifiInformationElementId ElementIdExt() const override;
  void Print(std::ostream &os) const override;

  void SetHeMacCapabilitiesInfo(uint32_t ctrl1, uint16_t ctrl2);
  void SetHePhyCapabilitiesInfo(uint64_t ctrl1, uint16_t ctrl2, uint8_t ctrl3);
  void SetSupportedMcsAndNss(uint16_t ctrl);

  uint32_t GetHeMacCapabilitiesInfo1() const;
  uint16_t GetHeMacCapabilitiesInfo2() const;
  uint64_t GetHePhyCapabilitiesInfo1() const;
  uint16_t GetHePhyCapabilitiesInfo2() const;
  uint8_t GetHePhyCapabilitiesInfo3() const;
  uint16_t GetSupportedMcsAndNss() const;

  void SetChannelWidthSet(uint8_t channelWidthSet);
  void SetLdpcCodingInPayload(uint8_t ldpcCodingInPayload);
  void SetHeSuPpdu1xHeLtf800nsGi(bool heSuPpdu1xHeLtf800nsGi);
  void SetHePpdu4xHeLtf800nsGi(bool heSuPpdu4xHeLtf800nsGi);
  uint8_t GetChannelWidthSet() const;
  uint8_t GetLdpcCodingInPayload() const;
  bool GetHeSuPpdu1xHeLtf800nsGi() const;
  bool GetHePpdu4xHeLtf800nsGi() const;
  uint8_t GetHighestMcsSupported() const;
  uint8_t GetHighestNssSupported() const;

  void SetMaxAmpduLength(uint32_t maxAmpduLength);

  void SetHighestMcsSupported(uint8_t mcs);
  void SetHighestNssSupported(uint8_t nss);

  bool IsSupportedTxMcs(uint8_t mcs) const;
  bool IsSupportedRxMcs(uint8_t mcs) const;

  uint32_t GetMaxAmpduLength() const;

private:
  uint16_t GetInformationFieldSize() const override;
  void SerializeInformationField(Buffer::Iterator start) const override;
  uint16_t DeserializeInformationField(Buffer::Iterator start,
                                       uint16_t length) override;

  uint8_t m_plusHtcHeSupport;
  uint8_t m_twtRequesterSupport;
  uint8_t m_twtResponderSupport;
  uint8_t m_fragmentationSupport;
  uint8_t m_maximumNumberOfFragmentedMsdus;
  uint8_t m_minimumFragmentSize;
  uint8_t m_triggerFrameMacPaddingDuration;
  uint8_t m_multiTidAggregationRxSupport;
  uint8_t m_heLinkAdaptation;
  uint8_t m_allAckSupport;
  uint8_t m_trsSupport;
  uint8_t m_bsrSupport;
  uint8_t m_broadcastTwtSupport;
  uint8_t m_32bitBaBitmapSupport;
  uint8_t m_muCascadeSupport;
  uint8_t m_ackEnabledAggregationSupport;
  uint8_t m_omControlSupport;
  uint8_t m_ofdmaRaSupport;
  uint8_t m_maxAmpduLengthExponent;
  uint8_t m_amsduFragmentationSupport;
  uint8_t m_flexibleTwtScheduleSupport;
  uint8_t m_rxControlFrameToMultiBss;
  uint8_t m_bsrpBqrpAmpduAggregation;
  uint8_t m_qtpSupport;
  uint8_t m_bqrSupport;
  uint8_t m_psrResponder;
  uint8_t m_ndpFeedbackReportSupport;
  uint8_t m_opsSupport;
  uint8_t m_amsduNotUnderBaInAmpduSupport;
  uint8_t m_multiTidAggregationTxSupport;
  uint8_t m_heSubchannelSelectiveTxSupport;
  uint8_t m_ul2x996ToneRuSupport;
  uint8_t m_omControlUlMuDataDisableRxSupport;
  uint8_t m_heDynamicSmPowerSave;
  uint8_t m_puncturedSoundingSupport;
  uint8_t m_heVhtTriggerFrameRxSupport;

  uint8_t m_channelWidthSet;
  uint8_t m_puncturedPreambleRx;
  uint8_t m_deviceClass;
  uint8_t m_ldpcCodingInPayload;
  uint8_t m_heSuPpdu1xHeLtf800nsGi;
  uint8_t m_midambleRxMaxNsts;
  uint8_t m_ndp4xHeLtfAnd32msGi;
  uint8_t m_stbcTxLeq80MHz;
  uint8_t m_stbcRxLeq80MHz;
  uint8_t m_dopplerTx;
  uint8_t m_dopplerRx;
  uint8_t m_fullBwUlMuMimo;
  uint8_t m_partialBwUlMuMimo;
  uint8_t m_dcmMaxConstellationTx;
  uint8_t m_dcmMaxNssTx;
  uint8_t m_dcmMaxConstellationRx;
  uint8_t m_dcmMaxNssRx;
  uint8_t m_rxPartialBwSuInHeMu;
  uint8_t m_suBeamformer;
  uint8_t m_suBeamformee;
  uint8_t m_muBeamformer;
  uint8_t m_beamformeeStsForSmallerOrEqualThan80Mhz;
  uint8_t m_beamformeeStsForLargerThan80Mhz;
  uint8_t m_numberOfSoundingDimensionsForSmallerOrEqualThan80Mhz;
  uint8_t m_numberOfSoundingDimensionsForLargerThan80Mhz;
  uint8_t m_ngEqual16ForSuFeedbackSupport;
  uint8_t m_ngEqual16ForMuFeedbackSupport;
  uint8_t m_codebookSize42SuFeedback;
  uint8_t m_codebookSize75MuFeedback;
  uint8_t m_triggeredSuBfFeedback;
  uint8_t m_triggeredMuBfFeedback;
  uint8_t m_triggeredCqiFeedback;
  uint8_t m_erPartialBandwidth;
  uint8_t m_dlMuMimoOnPartialBandwidth;
  uint8_t m_ppeThresholdPresent;
  uint8_t m_psrBasedSrSupport;
  uint8_t m_powerBoostFactorAlphaSupport;
  uint8_t m_hePpdu4xHeLtf800nsGi;
  uint8_t m_maxNc;
  uint8_t m_stbcTxGt80MHz;
  uint8_t m_stbcRxGt80MHz;
  uint8_t m_heErSuPpdu4xHeLtf08sGi;
  uint8_t m_hePpdu20MHzIn40MHz24GHz;
  uint8_t m_hePpdu20MHzIn160MHz;
  uint8_t m_hePpdu80MHzIn160MHz;
  uint8_t m_heErSuPpdu1xHeLtf08Gi;
  uint8_t m_midamble2xAnd1xHeLtf;
  uint8_t m_dcmMaxRu;
  uint8_t m_longerThan16HeSigbOfdm;
  uint8_t m_nonTriggeredCqiFeedback;
  uint8_t m_tx1024QamLt242Ru;
  uint8_t m_rx1024QamLt242Ru;
  uint8_t m_rxFullBwSuInHeMuCompressedSigB;
  uint8_t m_rxFullBwSuInHeMuNonCompressedSigB;
  uint8_t m_nominalPacketPadding;
  uint8_t m_maxHeLtfRxInHeMuMoreThanOneRu;

  uint8_t m_highestNssSupportedM1;
  uint8_t m_highestMcsSupported;
  std::vector<uint8_t> m_txBwMap;
  std::vector<uint8_t> m_rxBwMap;
};

} // namespace ns3

#endif
