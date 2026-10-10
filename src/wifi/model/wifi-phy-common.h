
#ifndef WIFI_PHY_COMMON_H
#define WIFI_PHY_COMMON_H

#include "wifi-standards.h"

#include "ns3/fatal-error.h"
#include "ns3/ptr.h"
#include "ns3/wifi-spectrum-value-helper.h"

#include <ostream>

namespace ns3 {

class WifiNetDevice;
class WifiMode;
class Time;

using WifiSpectrumBandFrequencies = std::pair<uint64_t, uint64_t>;

struct WifiSpectrumBandInfo {
  WifiSpectrumBandIndices indices;
  WifiSpectrumBandFrequencies frequencies;
};

using WifiSpectrumBands = std::vector<WifiSpectrumBandInfo>;

inline bool operator<(const WifiSpectrumBandInfo &lhs,
                      const WifiSpectrumBandInfo &rhs) {
  return lhs.frequencies < rhs.frequencies;
}

inline std::ostream &operator<<(std::ostream &os,
                                const WifiSpectrumBandInfo &band) {
  os << "indices: [" << band.indices.first << "-" << band.indices.second
     << "], frequencies: [" << band.frequencies.first << "Hz-"
     << band.frequencies.second << "Hz]";
  return os;
}

enum WifiCodeRate : uint16_t {
  WIFI_CODE_RATE_UNDEFINED,
  WIFI_CODE_RATE_1_2,
  WIFI_CODE_RATE_2_3,
  WIFI_CODE_RATE_3_4,
  WIFI_CODE_RATE_5_6,
  WIFI_CODE_RATE_5_8,
  WIFI_CODE_RATE_13_16,
  WIFI_CODE_RATE_1_4,
  WIFI_CODE_RATE_13_28,
  WIFI_CODE_RATE_13_21,
  WIFI_CODE_RATE_52_63,
  WIFI_CODE_RATE_13_14,
  WIFI_CODE_RATE_7_8,
};

inline std::ostream &operator<<(std::ostream &os,
                                const WifiCodeRate &codeRate) {
  switch (codeRate) {
  case WIFI_CODE_RATE_UNDEFINED:
    return (os << "Code rate undefined");
  case WIFI_CODE_RATE_1_2:
    return (os << "Code rate 1/2");
  case WIFI_CODE_RATE_2_3:
    return (os << "Code rate 2/3");
  case WIFI_CODE_RATE_3_4:
    return (os << "Code rate 3/4");
  case WIFI_CODE_RATE_5_6:
    return (os << "Code rate 5/6");
  case WIFI_CODE_RATE_5_8:
    return (os << "Code rate 5/8");
  case WIFI_CODE_RATE_13_16:
    return (os << "Code rate 13/16");
  case WIFI_CODE_RATE_1_4:
    return (os << "Code rate 1/4");
  case WIFI_CODE_RATE_13_28:
    return (os << "Code rate 13/28");
  case WIFI_CODE_RATE_13_21:
    return (os << "Code rate 13/21");
  case WIFI_CODE_RATE_52_63:
    return (os << "Code rate 52/63");
  case WIFI_CODE_RATE_13_14:
    return (os << "Code rate 13/14");
  case WIFI_CODE_RATE_7_8:
    return (os << "Code rate 7/8");
  default:
    NS_FATAL_ERROR("Unknown code rate");
    return (os << "Unknown");
  }
}

enum WifiPreamble {
  WIFI_PREAMBLE_LONG,
  WIFI_PREAMBLE_SHORT,
  WIFI_PREAMBLE_HT_MF,
  WIFI_PREAMBLE_VHT_SU,
  WIFI_PREAMBLE_VHT_MU,
  WIFI_PREAMBLE_DMG_CTRL,
  WIFI_PREAMBLE_DMG_SC,
  WIFI_PREAMBLE_DMG_OFDM,
  WIFI_PREAMBLE_HE_SU,
  WIFI_PREAMBLE_HE_ER_SU,
  WIFI_PREAMBLE_HE_MU,
  WIFI_PREAMBLE_HE_TB,
  WIFI_PREAMBLE_EHT_MU,
  WIFI_PREAMBLE_EHT_TB
};

inline std::ostream &operator<<(std::ostream &os,
                                const WifiPreamble &preamble) {
  switch (preamble) {
  case WIFI_PREAMBLE_LONG:
    return (os << "LONG");
  case WIFI_PREAMBLE_SHORT:
    return (os << "SHORT");
  case WIFI_PREAMBLE_HT_MF:
    return (os << "HT_MF");
  case WIFI_PREAMBLE_VHT_SU:
    return (os << "VHT_SU");
  case WIFI_PREAMBLE_VHT_MU:
    return (os << "VHT_MU");
  case WIFI_PREAMBLE_DMG_CTRL:
    return (os << "DMG_CTRL");
  case WIFI_PREAMBLE_DMG_SC:
    return (os << "DMG_SC");
  case WIFI_PREAMBLE_DMG_OFDM:
    return (os << "DMG_OFDM");
  case WIFI_PREAMBLE_HE_SU:
    return (os << "HE_SU");
  case WIFI_PREAMBLE_HE_ER_SU:
    return (os << "HE_ER_SU");
  case WIFI_PREAMBLE_HE_MU:
    return (os << "HE_MU");
  case WIFI_PREAMBLE_HE_TB:
    return (os << "HE_TB");
  case WIFI_PREAMBLE_EHT_MU:
    return (os << "EHT_MU");
  case WIFI_PREAMBLE_EHT_TB:
    return (os << "EHT_TB");
  default:
    NS_FATAL_ERROR("Invalid preamble");
    return (os << "INVALID");
  }
}

enum WifiModulationClass {
  WIFI_MOD_CLASS_UNKNOWN = 0,
  WIFI_MOD_CLASS_DSSS,
  WIFI_MOD_CLASS_HR_DSSS,
  WIFI_MOD_CLASS_ERP_OFDM,
  WIFI_MOD_CLASS_OFDM,
  WIFI_MOD_CLASS_HT,
  WIFI_MOD_CLASS_VHT,
  WIFI_MOD_CLASS_DMG_CTRL,
  WIFI_MOD_CLASS_DMG_OFDM,
  WIFI_MOD_CLASS_DMG_SC,
  WIFI_MOD_CLASS_DMG_LP_SC,
  WIFI_MOD_CLASS_HE,
  WIFI_MOD_CLASS_EHT
};

inline std::ostream &operator<<(std::ostream &os,
                                const WifiModulationClass &modulation) {
  switch (modulation) {
  case WIFI_MOD_CLASS_DSSS:
    return (os << "DSSS");
  case WIFI_MOD_CLASS_HR_DSSS:
    return (os << "HR/DSSS");
  case WIFI_MOD_CLASS_ERP_OFDM:
    return (os << "ERP-OFDM");
  case WIFI_MOD_CLASS_OFDM:
    return (os << "OFDM");
  case WIFI_MOD_CLASS_HT:
    return (os << "HT");
  case WIFI_MOD_CLASS_VHT:
    return (os << "VHT");
  case WIFI_MOD_CLASS_DMG_CTRL:
    return (os << "DMG_CTRL");
  case WIFI_MOD_CLASS_DMG_OFDM:
    return (os << "DMG_OFDM");
  case WIFI_MOD_CLASS_DMG_SC:
    return (os << "DMG_SC");
  case WIFI_MOD_CLASS_DMG_LP_SC:
    return (os << "DMG_LP_SC");
  case WIFI_MOD_CLASS_HE:
    return (os << "HE");
  case WIFI_MOD_CLASS_EHT:
    return (os << "EHT");
  default:
    NS_FATAL_ERROR("Unknown modulation");
    return (os << "unknown");
  }
}

enum WifiPpduField {
  WIFI_PPDU_FIELD_PREAMBLE = 0,
  WIFI_PPDU_FIELD_NON_HT_HEADER,
  WIFI_PPDU_FIELD_HT_SIG,
  WIFI_PPDU_FIELD_TRAINING,
  WIFI_PPDU_FIELD_SIG_A,
  WIFI_PPDU_FIELD_SIG_B,
  WIFI_PPDU_FIELD_U_SIG,
  WIFI_PPDU_FIELD_EHT_SIG,
  WIFI_PPDU_FIELD_DATA
};

inline std::ostream &operator<<(std::ostream &os, const WifiPpduField &field) {
  switch (field) {
  case WIFI_PPDU_FIELD_PREAMBLE:
    return (os << "preamble");
  case WIFI_PPDU_FIELD_NON_HT_HEADER:
    return (os << "non-HT header");
  case WIFI_PPDU_FIELD_HT_SIG:
    return (os << "HT-SIG");
  case WIFI_PPDU_FIELD_TRAINING:
    return (os << "training");
  case WIFI_PPDU_FIELD_SIG_A:
    return (os << "SIG-A");
  case WIFI_PPDU_FIELD_SIG_B:
    return (os << "SIG-B");
  case WIFI_PPDU_FIELD_U_SIG:
    return (os << "U-SIG");
  case WIFI_PPDU_FIELD_EHT_SIG:
    return (os << "EHT-SIG");
  case WIFI_PPDU_FIELD_DATA:
    return (os << "data");
  default:
    NS_FATAL_ERROR("Unknown field");
    return (os << "unknown");
  }
}

enum WifiPpduType {
  WIFI_PPDU_TYPE_SU = 0,
  WIFI_PPDU_TYPE_DL_MU,
  WIFI_PPDU_TYPE_UL_MU
};

inline std::ostream &operator<<(std::ostream &os, const WifiPpduType &type) {
  switch (type) {
  case WIFI_PPDU_TYPE_SU:
    return (os << "SU");
  case WIFI_PPDU_TYPE_DL_MU:
    return (os << "DL MU");
  case WIFI_PPDU_TYPE_UL_MU:
    return (os << "UL MU");
  default:
    NS_FATAL_ERROR("Unknown type");
    return (os << "unknown");
  }
}

enum WifiPhyRxfailureReason {
  UNKNOWN = 0,
  UNSUPPORTED_SETTINGS,
  CHANNEL_SWITCHING,
  RXING,
  TXING,
  SLEEPING,
  POWERED_OFF,
  TRUNCATED_TX,
  BUSY_DECODING_PREAMBLE,
  PREAMBLE_DETECT_FAILURE,
  RECEPTION_ABORTED_BY_TX,
  L_SIG_FAILURE,
  HT_SIG_FAILURE,
  SIG_A_FAILURE,
  SIG_B_FAILURE,
  U_SIG_FAILURE,
  EHT_SIG_FAILURE,
  PREAMBLE_DETECTION_PACKET_SWITCH,
  FRAME_CAPTURE_PACKET_SWITCH,
  OBSS_PD_CCA_RESET,
  PPDU_TOO_LATE,
  FILTERED,
  DMG_HEADER_FAILURE,
  DMG_ALLOCATION_ENDED
};

inline std::ostream &operator<<(std::ostream &os,
                                const WifiPhyRxfailureReason &reason) {
  switch (reason) {
  case UNSUPPORTED_SETTINGS:
    return (os << "UNSUPPORTED_SETTINGS");
  case CHANNEL_SWITCHING:
    return (os << "CHANNEL_SWITCHING");
  case RXING:
    return (os << "RXING");
  case TXING:
    return (os << "TXING");
  case SLEEPING:
    return (os << "SLEEPING");
  case POWERED_OFF:
    return (os << "OFF");
  case TRUNCATED_TX:
    return (os << "TRUNCATED_TX");
  case BUSY_DECODING_PREAMBLE:
    return (os << "BUSY_DECODING_PREAMBLE");
  case PREAMBLE_DETECT_FAILURE:
    return (os << "PREAMBLE_DETECT_FAILURE");
  case RECEPTION_ABORTED_BY_TX:
    return (os << "RECEPTION_ABORTED_BY_TX");
  case L_SIG_FAILURE:
    return (os << "L_SIG_FAILURE");
  case HT_SIG_FAILURE:
    return (os << "HT_SIG_FAILURE");
  case SIG_A_FAILURE:
    return (os << "SIG_A_FAILURE");
  case SIG_B_FAILURE:
    return (os << "SIG_B_FAILURE");
  case U_SIG_FAILURE:
    return (os << "U_SIG_FAILURE");
  case EHT_SIG_FAILURE:
    return (os << "EHT_SIG_FAILURE");
  case PREAMBLE_DETECTION_PACKET_SWITCH:
    return (os << "PREAMBLE_DETECTION_PACKET_SWITCH");
  case FRAME_CAPTURE_PACKET_SWITCH:
    return (os << "FRAME_CAPTURE_PACKET_SWITCH");
  case OBSS_PD_CCA_RESET:
    return (os << "OBSS_PD_CCA_RESET");
  case PPDU_TOO_LATE:
    return (os << "PPDU_TOO_LATE");
  case FILTERED:
    return (os << "FILTERED");
  case DMG_HEADER_FAILURE:
    return (os << "DMG_HEADER_FAILURE");
  case DMG_ALLOCATION_ENDED:
    return (os << "DMG_ALLOCATION_ENDED");
  case UNKNOWN:
  default:
    NS_FATAL_ERROR("Unknown reason");
    return (os << "UNKNOWN");
  }
}

enum WifiChannelListType : uint8_t {
  WIFI_CHANLIST_PRIMARY = 0,
  WIFI_CHANLIST_SECONDARY,
  WIFI_CHANLIST_SECONDARY40,
  WIFI_CHANLIST_SECONDARY80
};

inline std::ostream &operator<<(std::ostream &os, WifiChannelListType type) {
  switch (type) {
  case WIFI_CHANLIST_PRIMARY:
    return (os << "PRIMARY");
  case WIFI_CHANLIST_SECONDARY:
    return (os << "SECONDARY");
  case WIFI_CHANLIST_SECONDARY40:
    return (os << "SECONDARY40");
  case WIFI_CHANLIST_SECONDARY80:
    return (os << "SECONDARY80");
  default:
    NS_FATAL_ERROR("Unknown wifi channel type");
    return (os << "UNKNOWN");
  }
}

uint16_t ConvertGuardIntervalToNanoSeconds(WifiMode mode,
                                           const Ptr<WifiNetDevice> device);

uint16_t ConvertGuardIntervalToNanoSeconds(WifiMode mode,
                                           bool htShortGuardInterval,
                                           Time heGuardInterval);

WifiPreamble GetPreambleForTransmission(WifiModulationClass modulation,
                                        bool useShortPreamble);

WifiModulationClass GetModulationClassForPreamble(WifiPreamble preamble);

bool IsAllowedControlAnswerModulationClass(WifiModulationClass modClassReq,
                                           WifiModulationClass modClassAnswer);

Time GetPpduMaxTime(WifiPreamble preamble);

bool IsMu(WifiPreamble preamble);

bool IsDlMu(WifiPreamble preamble);

bool IsUlMu(WifiPreamble preamble);

WifiModulationClass GetModulationClassForStandard(WifiStandard standard);

uint16_t GetMaximumChannelWidth(WifiModulationClass modulation);

bool IsEht(WifiPreamble preamble);

} // namespace ns3

#endif
