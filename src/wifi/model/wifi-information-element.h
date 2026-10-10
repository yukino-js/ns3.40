
#ifndef WIFI_INFORMATION_ELEMENT_H
#define WIFI_INFORMATION_ELEMENT_H

#include "ns3/header.h"

#include <optional>

namespace ns3 {

constexpr uint8_t WIFI_IE_ELEMENT_ID_EXT_SIZE = 1;

typedef uint8_t WifiInformationElementId;

#define IE_SSID ((WifiInformationElementId)0)
#define IE_SUPPORTED_RATES ((WifiInformationElementId)1)
#define IE_FH_PARAMETER_SET ((WifiInformationElementId)2)
#define IE_DSSS_PARAMETER_SET ((WifiInformationElementId)3)
#define IE_CF_PARAMETER_SET ((WifiInformationElementId)4)
#define IE_TIM ((WifiInformationElementId)5)
#define IE_IBSS_PARAMETER_SET ((WifiInformationElementId)6)
#define IE_COUNTRY ((WifiInformationElementId)7)
#define IE_HOPPING_PATTERN_PARAMETERS ((WifiInformationElementId)8)
#define IE_HOPPING_PATTERN_TABLE ((WifiInformationElementId)9)
#define IE_REQUEST ((WifiInformationElementId)10)
#define IE_BSS_LOAD ((WifiInformationElementId)11)
#define IE_EDCA_PARAMETER_SET ((WifiInformationElementId)12)
#define IE_TSPEC ((WifiInformationElementId)13)
#define IE_TCLAS ((WifiInformationElementId)14)
#define IE_SCHEDULE ((WifiInformationElementId)15)
#define IE_CHALLENGE_TEXT ((WifiInformationElementId)16)
#define IE_POWER_CONSTRAINT ((WifiInformationElementId)32)
#define IE_POWER_CAPABILITY ((WifiInformationElementId)33)
#define IE_TPC_REQUEST ((WifiInformationElementId)34)
#define IE_TPC_REPORT ((WifiInformationElementId)35)
#define IE_SUPPORTED_CHANNELS ((WifiInformationElementId)36)
#define IE_CHANNEL_SWITCH_ANNOUNCEMENT ((WifiInformationElementId)37)
#define IE_MEASUREMENT_REQUEST ((WifiInformationElementId)38)
#define IE_MEASUREMENT_REPORT ((WifiInformationElementId)39)
#define IE_QUIET ((WifiInformationElementId)40)
#define IE_IBSS_DFS ((WifiInformationElementId)41)
#define IE_ERP_INFORMATION ((WifiInformationElementId)42)
#define IE_TS_DELAY ((WifiInformationElementId)43)
#define IE_TCLAS_PROCESSING ((WifiInformationElementId)44)
#define IE_HT_CAPABILITIES ((WifiInformationElementId)45)
#define IE_QOS_CAPABILITY ((WifiInformationElementId)46)
#define IE_RSN ((WifiInformationElementId)48)
#define IE_EXTENDED_SUPPORTED_RATES ((WifiInformationElementId)50)
#define IE_AP_CHANNEL_REPORT ((WifiInformationElementId)51)
#define IE_NEIGHBOR_REPORT ((WifiInformationElementId)52)
#define IE_RCPI ((WifiInformationElementId)53)
#define IE_MOBILITY_DOMAIN ((WifiInformationElementId)54)
#define IE_FAST_BSS_TRANSITION ((WifiInformationElementId)55)
#define IE_TIMEOUT_INTERVAL ((WifiInformationElementId)56)
#define IE_RIC_DATA ((WifiInformationElementId)57)
#define IE_DSE_REGISTERED_LOCATION ((WifiInformationElementId)58)
#define IE_SUPPORTED_OPERATING_CLASSES ((WifiInformationElementId)59)
#define IE_EXTENDED_CHANNEL_SWITCH_ANNOUNCEMENT ((WifiInformationElementId)60)
#define IE_HT_OPERATION ((WifiInformationElementId)61)
#define IE_SECONDARY_CHANNEL_OFFSET ((WifiInformationElementId)62)
#define IE_BSS_AVERAGE_ACCESS_DELAY ((WifiInformationElementId)63)
#define IE_ANTENNA ((WifiInformationElementId)64)
#define IE_RSNI ((WifiInformationElementId)65)
#define IE_MEASUREMENT_PILOT_TRANSMISSION ((WifiInformationElementId)66)
#define IE_BSS_AVAILABLE_ADMISSION_CAPACITY ((WifiInformationElementId)67)
#define IE_BSS_AC_ACCESS_DELAY ((WifiInformationElementId)68)
#define IE_TIME_ADVERTISEMENT ((WifiInformationElementId)69)
#define IE_RM_ENABLED_CAPACITIES ((WifiInformationElementId)70)
#define IE_MULTIPLE_BSSID ((WifiInformationElementId)71)
#define IE_20_40_BSS_COEXISTENCE ((WifiInformationElementId)72)
#define IE_20_40_BSS_INTOLERANT_CHANNEL_REPORT ((WifiInformationElementId)73)
#define IE_OVERLAPPING_BSS_SCAN_PARAMETERS ((WifiInformationElementId)74)
#define IE_RIC_DESCRIPTOR ((WifiInformationElementId)75)
#define IE_MANAGEMENT_MIC ((WifiInformationElementId)76)
#define IE_EVENT_REQUEST ((WifiInformationElementId)78)
#define IE_EVENT_REPORT ((WifiInformationElementId)79)
#define IE_DIAGNOSTIC_REQUEST ((WifiInformationElementId)80)
#define IE_DIAGNOSTIC_REPORT ((WifiInformationElementId)81)
#define IE_LOCATION_PARAMETERS ((WifiInformationElementId)82)
#define IE_NONTRANSMITTED_BSSID_CAPABILITY ((WifiInformationElementId)83)
#define IE_SSID_LIST ((WifiInformationElementId)84)
#define IE_MULTIPLE_BSSID_INDEX ((WifiInformationElementId)85)
#define IE_FMS_DESCRIPTOR ((WifiInformationElementId)86)
#define IE_FMS_REQUEST ((WifiInformationElementId)87)
#define IE_FMS_RESPONSE ((WifiInformationElementId)88)
#define IE_QOS_TRAFFIC_CAPABILITY ((WifiInformationElementId)89)
#define IE_BSS_MAX_IDLE_PERIOD ((WifiInformationElementId)90)
#define IE_TFS_REQUEST ((WifiInformationElementId)91)
#define IE_TFS_RESPONSE ((WifiInformationElementId)92)
#define IE_WNM_SLEEP_MODE ((WifiInformationElementId)93)
#define IE_TIM_BROADCAST_REQUEST ((WifiInformationElementId)94)
#define IE_TIM_BROADCAST_RESPONSE ((WifiInformationElementId)95)
#define IE_COLLOCATED_INTERFERENCE_REPORT ((WifiInformationElementId)96)
#define IE_CHANNEL_USAGE ((WifiInformationElementId)97)
#define IE_TIME_ZONE ((WifiInformationElementId)98)
#define IE_DMS_REQUEST ((WifiInformationElementId)99)
#define IE_DMS_RESPONSE ((WifiInformationElementId)100)
#define IE_LINK_IDENTIFIER ((WifiInformationElementId)101)
#define IE_WAKEUP_SCHEDULE ((WifiInformationElementId)102)
#define IE_CHANNEL_SWITCH_TIMING ((WifiInformationElementId)104)
#define IE_PTI_CONTROL ((WifiInformationElementId)105)
#define IE_TPU_BUFFER_STATUS ((WifiInformationElementId)106)
#define IE_INTERWORKING ((WifiInformationElementId)107)
#define IE_ADVERTISEMENT_PROTOCOL ((WifiInformationElementId)108)
#define IE_EXPEDITED_BANDWIDTH_REQUEST ((WifiInformationElementId)109)
#define IE_QOS_MAP_SET ((WifiInformationElementId)110)
#define IE_ROAMING_CONSORTIUM ((WifiInformationElementId)111)
#define IE_EMERGENCY_ALART_IDENTIFIER ((WifiInformationElementId)112)
#define IE_MESH_CONFIGURATION ((WifiInformationElementId)113)
#define IE_MESH_ID ((WifiInformationElementId)114)
#define IE_MESH_LINK_METRIC_REPORT ((WifiInformationElementId)115)
#define IE_CONGESTION_NOTIFICATION ((WifiInformationElementId)116)
#define IE_MESH_PEERING_MANAGEMENT ((WifiInformationElementId)117)
#define IE_MESH_CHANNEL_SWITCH_PARAMETERS ((WifiInformationElementId)118)
#define IE_MESH_AWAKE_WINDOW ((WifiInformationElementId)119)
#define IE_BEACON_TIMING ((WifiInformationElementId)120)
#define IE_MCCAOP_SETUP_REQUEST ((WifiInformationElementId)121)
#define IE_MCCAOP_SETUP_REPLY ((WifiInformationElementId)122)
#define IE_MCCAOP_ADVERTISEMENT ((WifiInformationElementId)123)
#define IE_MCCAOP_TEARDOWN ((WifiInformationElementId)124)
#define IE_GANN ((WifiInformationElementId)125)
#define IE_RANN ((WifiInformationElementId)126)
#define IE_EXTENDED_CAPABILITIES ((WifiInformationElementId)127)
#define IE_PREQ ((WifiInformationElementId)130)
#define IE_PREP ((WifiInformationElementId)131)
#define IE_PERR ((WifiInformationElementId)132)
#define IE_PROXY_UPDATE ((WifiInformationElementId)137)
#define IE_PROXY_UPDATE_CONFIRMATION ((WifiInformationElementId)138)
#define IE_AUTHENTICATED_MESH_PEERING_EXCHANGE ((WifiInformationElementId)139)
#define IE_MIC ((WifiInformationElementId)140)
#define IE_DESTINATION_URI ((WifiInformationElementId)141)
#define IE_UAPSD_COEXISTENCE ((WifiInformationElementId)142)
#define IE_DMG_WAKEUP_SCHEDULE ((WifiInformationElementId)143)
#define IE_EXTENDED_SCHEDULE ((WifiInformationElementId)144)
#define IE_STA_AVAILABILITY ((WifiInformationElementId)145)
#define IE_DMG_TSPEC ((WifiInformationElementId)146)
#define IE_NEXT_DMG_ATI ((WifiInformationElementId)147)
#define IE_DMG_CAPABILITIES ((WifiInformationElementId)148)
#define IE_DMG_OPERATION ((WifiInformationElementId)151)
#define IE_DMG_BSS_PARAMETER_CHANGE ((WifiInformationElementId)152)
#define IE_DMG_BEAM_REFINEMENT ((WifiInformationElementId)153)
#define IE_CHANNEL_MEASUREMENT_FEEDBACK ((WifiInformationElementId)154)
#define IE_AWAKE_WINDOW ((WifiInformationElementId)157)
#define IE_MULTI_BAND ((WifiInformationElementId)158)
#define IE_ADDBA_EXTENSION ((WifiInformationElementId)159)
#define IE_NEXT_PCP_LIST ((WifiInformationElementId)160)
#define IE_PCP_HANDOVER ((WifiInformationElementId)161)
#define IE_DMG_LINK_MARGIN ((WifiInformationElementId)162)
#define IE_SWITCHING_STREAM ((WifiInformationElementId)163)
#define IE_SESSION_TRANSITION ((WifiInformationElementId)164)
#define IE_DYNAMIC_TONE_PAIRING_REPORT ((WifiInformationElementId)165)
#define IE_CLUSTER_REPORT ((WifiInformationElementId)166)
#define IE_RELAY_CAPABILITIES ((WifiInformationElementId)167)
#define IE_RELAY_TRANSFER_PARAMETER_SET ((WifiInformationElementId)168)
#define IE_BEAMLINK_MAINTENANCE ((WifiInformationElementId)169)
#define IE_DMG_LINK_ADAPTATION_ACKNOWLEDGMENT ((WifiInformationElementId)172)
#define IE_MCCAOP_ADVERTISEMENT_OVERVIEW ((WifiInformationElementId)174)
#define IE_QUIET_PERIOD_REQUEST ((WifiInformationElementId)175)
#define IE_QUIET_PERIOD_RESPONSE ((WifiInformationElementId)177)
#define IE_ECPAC_POLICY ((WifiInformationElementId)182)
#define IE_VHT_CAPABILITIES ((WifiInformationElementId)191)
#define IE_VHT_OPERATION ((WifiInformationElementId)192)
#define IE_EXTENDED_BSS_LOAD ((WifiInformationElementId)193)
#define IE_WIDE_BANDWIDTH_CHANNEL_SWITCH ((WifiInformationElementId)194)
#define IE_VHT_TRANSMIT_POWER_ENVELOPE ((WifiInformationElementId)195)
#define IE_CHANNEL_SWITCH_WRAPPER ((WifiInformationElementId)196)
#define IE_AID ((WifiInformationElementId)197)
#define IE_QUIET_CHANNEL ((WifiInformationElementId)198)
#define IE_OPERATING_MODE_NOTIFICATION ((WifiInformationElementId)199)
#define IE_UPSIM ((WifiInformationElementId)200)
#define IE_REDUCED_NEIGHBOR_REPORT ((WifiInformationElementId)201)
#define IE_VENDOR_SPECIFIC ((WifiInformationElementId)221)
#define IE_FRAGMENT ((WifiInformationElementId)242)
#define IE_EXTENSION ((WifiInformationElementId)255)

#define IE_EXT_HE_CAPABILITIES ((WifiInformationElementId)35)
#define IE_EXT_HE_OPERATION ((WifiInformationElementId)36)
#define IE_EXT_UORA_PARAMETER_SET ((WifiInformationElementId)37)
#define IE_EXT_MU_EDCA_PARAMETER_SET ((WifiInformationElementId)38)

#define IE_EXT_NON_INHERITANCE ((WifiInformationElementId)56)

#define IE_EXT_EHT_OPERATION ((WifiInformationElementId)106)
#define IE_EXT_MULTI_LINK_ELEMENT ((WifiInformationElementId)107)
#define IE_EXT_EHT_CAPABILITIES ((WifiInformationElementId)108)
#define IE_EXT_TID_TO_LINK_MAPPING_ELEMENT ((WifiInformationElementId)109)

class WifiInformationElement : public SimpleRefCount<WifiInformationElement> {
public:
  virtual ~WifiInformationElement();
  Buffer::Iterator Serialize(Buffer::Iterator i) const;
  Buffer::Iterator Deserialize(Buffer::Iterator i);
  Buffer::Iterator DeserializeIfPresent(Buffer::Iterator i);
  uint16_t GetSerializedSize() const;

  virtual WifiInformationElementId ElementId() const = 0;

  virtual WifiInformationElementId ElementIdExt() const;

  virtual void Print(std::ostream &os) const;
  virtual bool operator==(const WifiInformationElement &a) const;

private:
  Buffer::Iterator SerializeFragments(Buffer::Iterator i, uint16_t size) const;
  Buffer::Iterator DoDeserialize(Buffer::Iterator i, uint16_t length);
  virtual uint16_t GetInformationFieldSize() const = 0;
  virtual void SerializeInformationField(Buffer::Iterator start) const = 0;
  virtual uint16_t DeserializeInformationField(Buffer::Iterator start,
                                               uint16_t length) = 0;
};

std::ostream &operator<<(std::ostream &os,
                         const WifiInformationElement &element);

} // namespace ns3

#endif
