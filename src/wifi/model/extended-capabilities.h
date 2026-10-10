
#ifndef EXTENDED_CAPABILITIES_H
#define EXTENDED_CAPABILITIES_H

#include "wifi-information-element.h"

namespace ns3 {

class ExtendedCapabilities : public WifiInformationElement {
public:
  ExtendedCapabilities();

  WifiInformationElementId ElementId() const override;
  uint16_t GetInformationFieldSize() const override;
  void SerializeInformationField(Buffer::Iterator start) const override;
  uint16_t DeserializeInformationField(Buffer::Iterator start,
                                       uint16_t length) override;
  void Print(std::ostream &os) const override;
  void SetHtSupported(uint8_t htSupported);
  void SetVhtSupported(uint8_t vhtSupported);

  void SetExtendedCapabilitiesByte1(uint8_t ctrl);
  void SetExtendedCapabilitiesByte2(uint8_t ctrl);
  void SetExtendedCapabilitiesByte3(uint8_t ctrl);
  void SetExtendedCapabilitiesByte4(uint8_t ctrl);
  void SetExtendedCapabilitiesByte5(uint8_t ctrl);
  void SetExtendedCapabilitiesByte6(uint8_t ctrl);
  void SetExtendedCapabilitiesByte7(uint8_t ctrl);
  void SetExtendedCapabilitiesByte8(uint8_t ctrl);

  uint8_t GetExtendedCapabilitiesByte1() const;
  uint8_t GetExtendedCapabilitiesByte2() const;
  uint8_t GetExtendedCapabilitiesByte3() const;
  uint8_t GetExtendedCapabilitiesByte4() const;
  uint8_t GetExtendedCapabilitiesByte5() const;
  uint8_t GetExtendedCapabilitiesByte6() const;
  uint8_t GetExtendedCapabilitiesByte7() const;
  uint8_t GetExtendedCapabilitiesByte8() const;

private:
  uint8_t m_20_40_bssCoexistenceManagementSupport;
  uint8_t m_extendedChannelSwitching;
  uint8_t m_psmpCapability;
  uint8_t m_spsmpSupport;

  uint8_t m_event;
  uint8_t m_diagnostics;
  uint8_t m_multicastDiagnostics;
  uint8_t m_locationTracking;
  uint8_t m_fms;
  uint8_t m_proxyArpService;
  uint8_t m_collocatedInterferenceReporting;
  uint8_t m_civicLocation;
  uint8_t m_geospatialLocation;

  uint8_t m_tfs;
  uint8_t m_wnmSleepMode;
  uint8_t m_timBroadcast;
  uint8_t m_bssTransition;
  uint8_t m_qosTrafficCapability;
  uint8_t m_acStationCount;
  uint8_t m_multipleBssid;
  uint8_t m_timingMeasurement;

  uint8_t m_channelUsage;
  uint8_t m_ssidList;
  uint8_t m_dms;
  uint8_t m_utcTsfOffset;
  uint8_t m_tpuBufferStaSupport;
  uint8_t m_tdlsPeerPsmSupport;
  uint8_t m_tdlsChannelSwitching;
  uint8_t m_interworking;

  uint8_t m_qosMap;
  uint8_t m_ebr;
  uint8_t m_sspnInterface;
  uint8_t m_msgcfCapability;
  uint8_t m_tdlsSupport;
  uint8_t m_tdlsProhibited;
  uint8_t m_tdlsChannelSwitchingProhibited;

  uint8_t m_rejectUnadmittedFrame;
  uint8_t m_serviceIntervalGranularity;
  uint8_t m_identifierLocation;
  uint8_t m_uapsdCoexistence;
  uint8_t m_wnmNotification;
  uint8_t m_qabCapability;

  uint8_t m_utf8Ssid;
  uint8_t m_qmfActivated;
  uint8_t m_qmfReconfigurationActivated;
  uint8_t m_robustAvStreaming;
  uint8_t m_advancedGcr;
  uint8_t m_meshGcr;
  uint8_t m_scs;
  uint8_t m_qloadReport;

  uint8_t m_alternateEdca;
  uint8_t m_unprotectedTxopNegotiation;
  uint8_t m_protectedTxopNegotiation;
  uint8_t m_protectedQloadReport;
  uint8_t m_tdlsWiderBandwidth;
  uint8_t m_operatingModeNotification;
  uint8_t m_maxNumberOfMsdusInAmsdu;

  uint8_t m_htSupported;
  uint8_t m_vhtSupported;
};

} // namespace ns3

#endif
