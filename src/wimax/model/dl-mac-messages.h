
#ifndef DCD_CHANNEL_ENCODINGS_H
#define DCD_CHANNEL_ENCODINGS_H

#include "ns3/buffer.h"

#include <list>
#include <stdint.h>

namespace ns3 {

class DcdChannelEncodings {
public:
  DcdChannelEncodings();
  virtual ~DcdChannelEncodings();

  void SetBsEirp(uint16_t bs_eirp);
  void SetEirxPIrMax(uint16_t rss_ir_max);
  void SetFrequency(uint32_t frequency);

  uint16_t GetBsEirp() const;
  uint16_t GetEirxPIrMax() const;
  uint32_t GetFrequency() const;

  uint16_t GetSize() const;

  Buffer::Iterator Write(Buffer::Iterator start) const;
  Buffer::Iterator Read(Buffer::Iterator start);

private:
  virtual Buffer::Iterator DoWrite(Buffer::Iterator start) const = 0;
  virtual Buffer::Iterator DoRead(Buffer::Iterator start) = 0;

  uint16_t m_bsEirp;
  uint16_t m_eirXPIrMax;
  uint32_t m_frequency;
};

} // namespace ns3

#endif

#ifndef OFDM_DCD_CHANNEL_ENCODINGS_H
#define OFDM_DCD_CHANNEL_ENCODINGS_H

#include "ns3/mac48-address.h"

#include <stdint.h>

namespace ns3 {

class OfdmDcdChannelEncodings : public DcdChannelEncodings {
public:
  OfdmDcdChannelEncodings();
  ~OfdmDcdChannelEncodings() override;

  void SetChannelNr(uint8_t channelNr);
  void SetTtg(uint8_t ttg);
  void SetRtg(uint8_t rtg);

  void SetBaseStationId(Mac48Address baseStationId);
  void SetFrameDurationCode(uint8_t frameDurationCode);
  void SetFrameNumber(uint32_t frameNumber);

  uint8_t GetChannelNr() const;
  uint8_t GetTtg() const;
  uint8_t GetRtg() const;

  Mac48Address GetBaseStationId() const;
  uint8_t GetFrameDurationCode() const;
  uint32_t GetFrameNumber() const;

  uint16_t GetSize() const;

private:
  Buffer::Iterator DoWrite(Buffer::Iterator start) const override;
  Buffer::Iterator DoRead(Buffer::Iterator start) override;

  uint8_t m_channelNr;
  uint8_t m_ttg;
  uint8_t m_rtg;

  Mac48Address m_baseStationId;
  uint8_t m_frameDurationCode;
  uint32_t m_frameNumber;
};

} // namespace ns3

#endif

#ifndef OFDM_DL_BURST_PROFILE_H
#define OFDM_DL_BURST_PROFILE_H

#include "ns3/buffer.h"

#include <stdint.h>

namespace ns3 {

class OfdmDlBurstProfile {
public:
  enum Diuc {
    DIUC_STC_ZONE = 0,
    DIUC_BURST_PROFILE_1,
    DIUC_BURST_PROFILE_2,
    DIUC_BURST_PROFILE_3,
    DIUC_BURST_PROFILE_4,
    DIUC_BURST_PROFILE_5,
    DIUC_BURST_PROFILE_6,
    DIUC_BURST_PROFILE_7,
    DIUC_BURST_PROFILE_8,
    DIUC_BURST_PROFILE_9,
    DIUC_BURST_PROFILE_10,
    DIUC_BURST_PROFILE_11,
    DIUC_GAP = 13,
    DIUC_END_OF_MAP
  };

  OfdmDlBurstProfile();
  ~OfdmDlBurstProfile();

  void SetType(uint8_t type);
  void SetLength(uint8_t length);
  void SetDiuc(uint8_t diuc);

  void SetFecCodeType(uint8_t fecCodeType);

  uint8_t GetType() const;
  uint8_t GetLength() const;
  uint8_t GetDiuc() const;

  uint8_t GetFecCodeType() const;

  uint16_t GetSize() const;

  Buffer::Iterator Write(Buffer::Iterator start) const;
  Buffer::Iterator Read(Buffer::Iterator start);

private:
  uint8_t m_type;
  uint8_t m_length;
  uint8_t m_diuc;

  uint8_t m_fecCodeType;
};

} // namespace ns3

#endif

#ifndef DCD_H
#define DCD_H

#include "ns3/header.h"

#include <stdint.h>
#include <vector>

namespace ns3 {

class Dcd : public Header {
public:
  Dcd();
  ~Dcd() override;

  void SetConfigurationChangeCount(uint8_t configurationChangeCount);
  void SetChannelEncodings(OfdmDcdChannelEncodings channelEncodings);
  void AddDlBurstProfile(OfdmDlBurstProfile dlBurstProfile);
  void SetNrDlBurstProfiles(uint8_t nrDlBurstProfiles);

  uint8_t GetConfigurationChangeCount() const;
  OfdmDcdChannelEncodings GetChannelEncodings() const;
  std::vector<OfdmDlBurstProfile> GetDlBurstProfiles() const;
  uint8_t GetNrDlBurstProfiles() const;

  std::string GetName() const;
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

private:
  uint8_t m_reserved;
  uint8_t m_configurationChangeCount;
  OfdmDcdChannelEncodings m_channelEncodings;
  std::vector<OfdmDlBurstProfile> m_dlBurstProfiles;

  uint8_t m_nrDlBurstProfiles;
};

} // namespace ns3

#endif

#ifndef OFDM_DL_MAP_IE_H
#define OFDM_DL_MAP_IE_H

#include "cid.h"

#include <stdint.h>

namespace ns3 {

class OfdmDlMapIe {
public:
  OfdmDlMapIe();
  ~OfdmDlMapIe();

  void SetCid(Cid cid);
  void SetDiuc(uint8_t diuc);
  void SetPreamblePresent(uint8_t preamblePresent);
  void SetStartTime(uint16_t startTime);

  Cid GetCid() const;
  uint8_t GetDiuc() const;
  uint8_t GetPreamblePresent() const;
  uint16_t GetStartTime() const;

  uint16_t GetSize() const;

  Buffer::Iterator Write(Buffer::Iterator start) const;
  Buffer::Iterator Read(Buffer::Iterator start);

private:
  Cid m_cid;
  uint8_t m_diuc;
  uint8_t m_preamblePresent;
  uint16_t m_startTime;
};

} // namespace ns3

#endif

#ifndef DL_MAP_H
#define DL_MAP_H

#include "ns3/header.h"
#include "ns3/mac48-address.h"

#include <stdint.h>
#include <vector>

namespace ns3 {

class DlMap : public Header {
public:
  DlMap();
  ~DlMap() override;

  void SetDcdCount(uint8_t dcdCount);
  void SetBaseStationId(Mac48Address baseStationID);
  void AddDlMapElement(OfdmDlMapIe dlMapElement);

  uint8_t GetDcdCount() const;
  Mac48Address GetBaseStationId() const;
  std::list<OfdmDlMapIe> GetDlMapElements() const;

  std::string GetName() const;
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

private:
  uint8_t m_dcdCount;
  Mac48Address m_baseStationId;
  std::list<OfdmDlMapIe> m_dlMapElements;
};

} // namespace ns3

#endif
