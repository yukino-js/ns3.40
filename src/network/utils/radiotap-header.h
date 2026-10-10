
#ifndef RADIOTAP_HEADER_H
#define RADIOTAP_HEADER_H

#include <ns3/header.h>

namespace ns3 {

class RadiotapHeader : public Header {
public:
  RadiotapHeader();
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;

  uint32_t GetSerializedSize() const override;

  void Serialize(Buffer::Iterator start) const override;

  uint32_t Deserialize(Buffer::Iterator start) override;

  void Print(std::ostream &os) const override;

  void SetTsft(uint64_t tsft);

  enum FrameFlag {
    FRAME_FLAG_NONE = 0x00,
    FRAME_FLAG_CFP = 0x01,
    FRAME_FLAG_SHORT_PREAMBLE = 0x02,
    FRAME_FLAG_WEP = 0x04,
    FRAME_FLAG_FRAGMENTED = 0x08,
    FRAME_FLAG_FCS_INCLUDED = 0x10,
    FRAME_FLAG_DATA_PADDING = 0x20,
    FRAME_FLAG_BAD_FCS = 0x40,
    FRAME_FLAG_SHORT_GUARD = 0x80
  };

  void SetFrameFlags(uint8_t flags);

  void SetRate(uint8_t rate);

  enum ChannelFlags {
    CHANNEL_FLAG_NONE = 0x0000,
    CHANNEL_FLAG_TURBO = 0x0010,
    CHANNEL_FLAG_CCK = 0x0020,
    CHANNEL_FLAG_OFDM = 0x0040,
    CHANNEL_FLAG_SPECTRUM_2GHZ = 0x0080,
    CHANNEL_FLAG_SPECTRUM_5GHZ = 0x0100,
    CHANNEL_FLAG_PASSIVE = 0x0200,
    CHANNEL_FLAG_DYNAMIC = 0x0400,
    CHANNEL_FLAG_GFSK = 0x0800
  };

  void SetChannelFrequencyAndFlags(uint16_t frequency, uint16_t flags);

  void SetAntennaSignalPower(double signal);

  void SetAntennaNoisePower(double noise);

  enum McsKnown {
    MCS_KNOWN_NONE = 0x00,
    MCS_KNOWN_BANDWIDTH = 0x01,
    MCS_KNOWN_INDEX = 0x02,
    MCS_KNOWN_GUARD_INTERVAL = 0x04,
    MCS_KNOWN_HT_FORMAT = 0x08,
    MCS_KNOWN_FEC_TYPE = 0x10,
    MCS_KNOWN_STBC = 0x20,
    MCS_KNOWN_NESS = 0x40,
    MCS_KNOWN_NESS_BIT_1 = 0x80,
  };

  enum McsFlags {
    MCS_FLAGS_NONE = 0x00,
    MCS_FLAGS_BANDWIDTH_40 = 0x01,
    MCS_FLAGS_BANDWIDTH_20L = 0x02,
    MCS_FLAGS_BANDWIDTH_20U = 0x03,
    MCS_FLAGS_GUARD_INTERVAL = 0x04,
    MCS_FLAGS_HT_GREENFIELD = 0x08,
    MCS_FLAGS_FEC_TYPE = 0x10,
    MCS_FLAGS_STBC_STREAMS = 0x60,
    MCS_FLAGS_NESS_BIT_0 = 0x80,
  };

  void SetMcsFields(uint8_t known, uint8_t flags, uint8_t mcs);

  enum AmpduFlags {
    A_MPDU_STATUS_NONE = 0x00,
    A_MPDU_STATUS_REPORT_ZERO_LENGTH = 0x01,
    A_MPDU_STATUS_IS_ZERO_LENGTH = 0x02,
    A_MPDU_STATUS_LAST_KNOWN = 0x04,
    A_MPDU_STATUS_LAST = 0x08,
    A_MPDU_STATUS_DELIMITER_CRC_ERROR = 0x10,
    A_MPDU_STATUS_DELIMITER_CRC_KNOWN = 0x20
  };

  void SetAmpduStatus(uint32_t referenceNumber, uint16_t flags, uint8_t crc);

  enum VhtKnown {
    VHT_KNOWN_NONE = 0x0000,
    VHT_KNOWN_STBC = 0x0001,
    VHT_KNOWN_TXOP_PS_NOT_ALLOWED = 0x0002,
    VHT_KNOWN_GUARD_INTERVAL = 0x0004,
    VHT_KNOWN_SHORT_GI_NSYM_DISAMBIGUATION = 0x0008,
    VHT_KNOWN_LDPC_EXTRA_OFDM_SYMBOL = 0x0010,
    VHT_KNOWN_BEAMFORMED = 0x0020,
    VHT_KNOWN_BANDWIDTH = 0x0040,
    VHT_KNOWN_GROUP_ID = 0x0080,
    VHT_KNOWN_PARTIAL_AID = 0x0100,
  };

  enum VhtFlags {
    VHT_FLAGS_NONE = 0x00,
    VHT_FLAGS_STBC = 0x01,
    VHT_FLAGS_TXOP_PS_NOT_ALLOWED = 0x02,
    VHT_FLAGS_GUARD_INTERVAL = 0x04,
    VHT_FLAGS_SHORT_GI_NSYM_DISAMBIGUATION = 0x08,
    VHT_FLAGS_LDPC_EXTRA_OFDM_SYMBOL = 0x10,
    VHT_FLAGS_BEAMFORMED = 0x20,
  };

  void SetVhtFields(uint16_t known, uint8_t flags, uint8_t bandwidth,
                    uint8_t mcs_nss[4], uint8_t coding, uint8_t group_id,
                    uint16_t partial_aid);

  enum HeData1 {
    HE_DATA1_FORMAT_EXT_SU = 0x0001,
    HE_DATA1_FORMAT_MU = 0x0002,
    HE_DATA1_FORMAT_TRIG = 0x0003,
    HE_DATA1_BSS_COLOR_KNOWN = 0x0004,
    HE_DATA1_BEAM_CHANGE_KNOWN = 0x0008,
    HE_DATA1_UL_DL_KNOWN = 0x0010,
    HE_DATA1_DATA_MCS_KNOWN = 0x0020,
    HE_DATA1_DATA_DCM_KNOWN = 0x0040,
    HE_DATA1_CODING_KNOWN = 0x0080,
    HE_DATA1_LDPC_XSYMSEG_KNOWN = 0x0100,
    HE_DATA1_STBC_KNOWN = 0x0200,
    HE_DATA1_SPTL_REUSE_KNOWN = 0x0400,
    HE_DATA1_SPTL_REUSE2_KNOWN = 0x0800,
    HE_DATA1_SPTL_REUSE3_KNOWN = 0x1000,
    HE_DATA1_SPTL_REUSE4_KNOWN = 0x2000,
    HE_DATA1_BW_RU_ALLOC_KNOWN = 0x4000,
    HE_DATA1_DOPPLER_KNOWN = 0x8000,
  };

  enum HeData2 {
    HE_DATA2_PRISEC_80_KNOWN = 0x0001,
    HE_DATA2_GI_KNOWN = 0x0002,
    HE_DATA2_NUM_LTF_SYMS_KNOWN = 0x0004,
    HE_DATA2_PRE_FEC_PAD_KNOWN = 0x0008,
    HE_DATA2_TXBF_KNOWN = 0x0010,
    HE_DATA2_PE_DISAMBIG_KNOWN = 0x0020,
    HE_DATA2_TXOP_KNOWN = 0x0040,
    HE_DATA2_MIDAMBLE_KNOWN = 0x0080,
    HE_DATA2_RU_OFFSET = 0x3f00,
    HE_DATA2_RU_OFFSET_KNOWN = 0x4000,
    HE_DATA2_PRISEC_80_SEC = 0x8000,
  };

  enum HeData5 {
    HE_DATA5_DATA_BW_RU_ALLOC_40MHZ = 0x0001,
    HE_DATA5_DATA_BW_RU_ALLOC_80MHZ = 0x0002,
    HE_DATA5_DATA_BW_RU_ALLOC_160MHZ = 0x0003,
    HE_DATA5_DATA_BW_RU_ALLOC_26T = 0x0004,
    HE_DATA5_DATA_BW_RU_ALLOC_52T = 0x0005,
    HE_DATA5_DATA_BW_RU_ALLOC_106T = 0x0006,
    HE_DATA5_DATA_BW_RU_ALLOC_242T = 0x0007,
    HE_DATA5_DATA_BW_RU_ALLOC_484T = 0x0008,
    HE_DATA5_DATA_BW_RU_ALLOC_996T = 0x0009,
    HE_DATA5_DATA_BW_RU_ALLOC_2x996T = 0x000a,
    HE_DATA5_GI_1_6 = 0x0010,
    HE_DATA5_GI_3_2 = 0x0020,
    HE_DATA5_LTF_SYM_SIZE = 0x00c0,
    HE_DATA5_NUM_LTF_SYMS = 0x0700,
    HE_DATA5_PRE_FEC_PAD = 0x3000,
    HE_DATA5_TXBF = 0x4000,
    HE_DATA5_PE_DISAMBIG = 0x8000,
  };

  void SetHeFields(uint16_t data1, uint16_t data2, uint16_t data3,
                   uint16_t data4, uint16_t data5, uint16_t data6);

  enum HeMuFlags1 {
    HE_MU_FLAGS1_SIGB_MCS = 0x000f,
    HE_MU_FLAGS1_SIGB_MCS_KNOWN = 0x0010,
    HE_MU_FLAGS1_SIGB_DCM = 0x0020,
    HE_MU_FLAGS1_SIGB_DCM_KNOWN = 0x0040,
    HE_MU_FLAGS1_CH2_CENTER_26T_RU_KNOWN = 0x0080,
    HE_MU_FLAGS1_CH1_RUS_KNOWN = 0x0100,
    HE_MU_FLAGS1_CH2_RUS_KNOWN = 0x0200,
    HE_MU_FLAGS1_CH1_CENTER_26T_RU_KNOWN = 0x1000,
    HE_MU_FLAGS1_CH1_CENTER_26T_RU = 0x2000,
    HE_MU_FLAGS1_SIGB_COMPRESSION_KNOWN = 0x4000,
    HE_MU_FLAGS1_NUM_SIGB_SYMBOLS_KNOWN = 0x8000,
  };

  enum HeMuFlags2 {
    HE_MU_FLAGS2_BW_FROM_SIGA = 0x0003,
    HE_MU_FLAGS2_BW_FROM_SIGA_KNOWN = 0x0004,
    HE_MU_FLAGS2_SIGB_COMPRESSION_FROM_SIGA = 0x0008,
    HE_MU_FLAGS2_NUM_SIGB_SYMBOLS_FROM_SIGA = 0x00f0,
    HE_MU_FLAGS2_PREAMBLE_PUNCTURING_FROM_SIGA_BW_FIELD = 0x0300,
    HE_MU_FLAGS2_PREAMBLE_PUNCTURING_FROM_SIGA_BW_FIELD_KNOWN = 0x0400,
    HE_MU_FLAGS2_CH2_CENTER_26T_RU = 0x0800,
  };

  void SetHeMuFields(uint16_t flags1, uint16_t flags2,
                     const std::array<uint8_t, 4> &ruChannel1,
                     const std::array<uint8_t, 4> &ruChannel2);

  enum HeMuPerUserKnown {
    HE_MU_PER_USER_POSITION_KNOWN = 0x01,
    HE_MU_PER_USER_STA_ID_KNOWN = 0x02,
    HE_MU_PER_USER_NSTS_KNOWN = 0x04,
    HE_MU_PER_USER_TX_BF_KNOWN = 0x08,
    HE_MU_PER_USER_SPATIAL_CONFIGURATION_KNOWN = 0x10,
    HE_MU_PER_USER_MCS_KNOWN = 0x20,
    HE_MU_PER_USER_DCM_KNOWN = 0x40,
    HE_MU_PER_USER_CODING_KNOWN = 0x80,
  };

  void SetHeMuPerUserFields(uint16_t perUser1, uint16_t perUser2,
                            uint8_t perUserPosition, uint8_t perUserKnown);

private:
  enum RadiotapFlags {
    RADIOTAP_TSFT = 0x00000001,
    RADIOTAP_FLAGS = 0x00000002,
    RADIOTAP_RATE = 0x00000004,
    RADIOTAP_CHANNEL = 0x00000008,
    RADIOTAP_FHSS = 0x00000010,
    RADIOTAP_DBM_ANTSIGNAL = 0x00000020,
    RADIOTAP_DBM_ANTNOISE = 0x00000040,
    RADIOTAP_LOCK_QUALITY = 0x00000080,
    RADIOTAP_TX_ATTENUATION = 0x00000100,
    RADIOTAP_DB_TX_ATTENUATION = 0x00000200,
    RADIOTAP_DBM_TX_POWER = 0x00000400,
    RADIOTAP_ANTENNA = 0x00000800,
    RADIOTAP_DB_ANTSIGNAL = 0x00001000,
    RADIOTAP_DB_ANTNOISE = 0x00002000,
    RADIOTAP_RX_FLAGS = 0x00004000,
    RADIOTAP_MCS = 0x00080000,
    RADIOTAP_AMPDU_STATUS = 0x00100000,
    RADIOTAP_VHT = 0x00200000,
    RADIOTAP_HE = 0x00800000,
    RADIOTAP_HE_MU = 0x01000000,
    RADIOTAP_HE_MU_OTHER_USER = 0x02000000,
    RADIOTAP_ZERO_LEN_PSDU = 0x04000000,
    RADIOTAP_LSIG = 0x08000000,
    RADIOTAP_EXT = 0x80000000
  };

  uint16_t m_length;
  uint32_t m_present;

  uint64_t m_tsft;
  uint8_t m_flags;
  uint8_t m_rate;
  uint8_t m_channelPad;
  uint16_t m_channelFreq;
  uint16_t m_channelFlags;
  int8_t m_antennaSignal;
  int8_t m_antennaNoise;

  uint8_t m_mcsKnown;
  uint8_t m_mcsFlags;
  uint8_t m_mcsRate;

  uint8_t m_ampduStatusPad;
  uint32_t m_ampduStatusRef;
  uint16_t m_ampduStatusFlags;
  uint8_t m_ampduStatusCRC;

  uint8_t m_vhtPad;
  uint16_t m_vhtKnown;
  uint8_t m_vhtFlags;
  uint8_t m_vhtBandwidth;
  uint8_t m_vhtMcsNss[4];
  uint8_t m_vhtCoding;
  uint8_t m_vhtGroupId;
  uint16_t m_vhtPartialAid;

  uint8_t m_hePad;
  uint16_t m_heData1;
  uint16_t m_heData2;
  uint16_t m_heData3;
  uint16_t m_heData4;
  uint16_t m_heData5;
  uint16_t m_heData6;

  uint8_t m_heMuPad;
  uint16_t m_heMuFlags1;
  uint16_t m_heMuFlags2;

  uint8_t m_heMuOtherUserPad;
  uint16_t m_heMuPerUser1;
  uint16_t m_heMuPerUser2;
  uint8_t m_heMuPerUserPosition;
  uint8_t m_heMuPerUserKnown;
};

} // namespace ns3

#endif
