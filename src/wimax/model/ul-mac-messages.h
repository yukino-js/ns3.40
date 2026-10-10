
#ifndef UCD_CHANNEL_ENCODINGS_H
#define UCD_CHANNEL_ENCODINGS_H

#include "ns3/buffer.h"

#include <list>
#include <stdint.h>

namespace ns3 {

class UcdChannelEncodings {
public:
  UcdChannelEncodings();
  virtual ~UcdChannelEncodings();

  void SetBwReqOppSize(uint16_t bwReqOppSize);
  void SetRangReqOppSize(uint16_t rangReqOppSize);
  void SetFrequency(uint32_t frequency);

  uint16_t GetBwReqOppSize() const;
  uint16_t GetRangReqOppSize() const;
  uint32_t GetFrequency() const;

  uint16_t GetSize() const;

  Buffer::Iterator Write(Buffer::Iterator start) const;
  Buffer::Iterator Read(Buffer::Iterator start);

private:
  virtual Buffer::Iterator DoWrite(Buffer::Iterator start) const = 0;
  virtual Buffer::Iterator DoRead(Buffer::Iterator start) = 0;

  uint16_t m_bwReqOppSize;
  uint16_t m_rangReqOppSize;
  uint32_t m_frequency;
};

} // namespace ns3

#endif

#ifndef OFDM_UCD_CHANNEL_ENCODINGS_H
#define OFDM_UCD_CHANNEL_ENCODINGS_H

#include <stdint.h>

namespace ns3 {

class OfdmUcdChannelEncodings : public UcdChannelEncodings {
public:
  OfdmUcdChannelEncodings();
  ~OfdmUcdChannelEncodings() override;

  void SetSbchnlReqRegionFullParams(uint8_t sbchnlReqRegionFullParams);
  void SetSbchnlFocContCodes(uint8_t sbchnlFocContCodes);

  uint8_t GetSbchnlReqRegionFullParams() const;
  uint8_t GetSbchnlFocContCodes() const;

  uint16_t GetSize() const;

private:
  Buffer::Iterator DoWrite(Buffer::Iterator start) const override;
  Buffer::Iterator DoRead(Buffer::Iterator start) override;

  uint8_t m_sbchnlReqRegionFullParams;
  uint8_t m_sbchnlFocContCodes;
};

} // namespace ns3

#endif

#ifndef OFDM_UL_BURST_PROFILE_H
#define OFDM_UL_BURST_PROFILE_H

#include "ns3/buffer.h"

#include <stdint.h>

namespace ns3 {

class OfdmUlBurstProfile {
public:
  enum Uiuc {
    UIUC_INITIAL_RANGING = 1,
    UIUC_REQ_REGION_FULL,
    UIUC_REQ_REGION_FOCUSED,
    UIUC_FOCUSED_CONTENTION_IE,
    UIUC_BURST_PROFILE_5,
    UIUC_BURST_PROFILE_6,
    UIUC_BURST_PROFILE_7,
    UIUC_BURST_PROFILE_8,
    UIUC_BURST_PROFILE_9,
    UIUC_BURST_PROFILE_10,
    UIUC_BURST_PROFILE_11,
    UIUC_BURST_PROFILE_12,
    UIUC_SUBCH_NETWORK_ENTRY,
    UIUC_END_OF_MAP
  };

  OfdmUlBurstProfile();
  ~OfdmUlBurstProfile();

  void SetType(uint8_t type);
  void SetLength(uint8_t length);
  void SetUiuc(uint8_t uiuc);
  void SetFecCodeType(uint8_t fecCodeType);

  uint8_t GetType() const;
  uint8_t GetLength() const;
  uint8_t GetUiuc() const;
  uint8_t GetFecCodeType() const;

  uint16_t GetSize() const;

  Buffer::Iterator Write(Buffer::Iterator start) const;
  Buffer::Iterator Read(Buffer::Iterator start);

private:
  uint8_t m_type;
  uint8_t m_length;
  uint8_t m_uiuc;

  uint8_t m_fecCodeType;
};

} // namespace ns3

#endif

#ifndef UCD_H
#define UCD_H

#include "ns3/header.h"

#include <stdint.h>
#include <vector>

namespace ns3 {

class Ucd : public Header {
public:
  Ucd();
  ~Ucd() override;

  void SetConfigurationChangeCount(uint8_t ucdCount);
  void SetRangingBackoffStart(uint8_t rangingBackoffStart);
  void SetRangingBackoffEnd(uint8_t rangingBackoffEnd);
  void SetRequestBackoffStart(uint8_t requestBackoffStart);
  void SetRequestBackoffEnd(uint8_t requestBackoffEnd);
  void SetChannelEncodings(OfdmUcdChannelEncodings channelEncodings);
  void AddUlBurstProfile(OfdmUlBurstProfile ulBurstProfile);
  void SetNrUlBurstProfiles(uint8_t nrUlBurstProfiles);

  uint8_t GetConfigurationChangeCount() const;
  uint8_t GetRangingBackoffStart() const;
  uint8_t GetRangingBackoffEnd() const;
  uint8_t GetRequestBackoffStart() const;
  uint8_t GetRequestBackoffEnd() const;
  OfdmUcdChannelEncodings GetChannelEncodings() const;
  std::vector<OfdmUlBurstProfile> GetUlBurstProfiles() const;
  uint8_t GetNrUlBurstProfiles() const;

  std::string GetName() const;
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

private:
  uint8_t m_configurationChangeCount;
  uint8_t m_rangingBackoffStart;
  uint8_t m_rangingBackoffEnd;
  uint8_t m_requestBackoffStart;
  uint8_t m_requestBackoffEnd;
  OfdmUcdChannelEncodings m_channelEncodings;
  std::vector<OfdmUlBurstProfile> m_ulBurstProfiles;

  uint8_t m_nrUlBurstProfiles;
};

} // namespace ns3

#endif

#ifndef OFDM_UL_MAP_IE_H
#define OFDM_UL_MAP_IE_H

#include "cid.h"

#include "ns3/header.h"

#include <stdint.h>

namespace ns3 {

class OfdmUlMapIe {
public:
  OfdmUlMapIe();
  ~OfdmUlMapIe();

  void SetCid(const Cid &cid);
  void SetStartTime(uint16_t startTime);
  void SetSubchannelIndex(uint8_t subchannelIndex);
  void SetUiuc(uint8_t uiuc);
  void SetDuration(uint16_t duration);
  void SetMidambleRepetitionInterval(uint8_t midambleRepetitionInterval);

  Cid GetCid() const;
  uint16_t GetStartTime() const;
  uint8_t GetSubchannelIndex() const;
  uint8_t GetUiuc() const;
  uint16_t GetDuration() const;
  uint8_t GetMidambleRepetitionInterval() const;

  uint16_t GetSize() const;

  Buffer::Iterator Write(Buffer::Iterator start) const;
  Buffer::Iterator Read(Buffer::Iterator start);

private:
  Cid m_cid;
  uint16_t m_startTime;
  uint8_t m_subchannelIndex;
  uint8_t m_uiuc;
  uint16_t m_duration;
  uint8_t m_midambleRepetitionInterval;
};

} // namespace ns3

#endif

#ifndef UL_MAP_H
#define UL_MAP_H

#include "ns3/header.h"

#include <stdint.h>
#include <vector>

namespace ns3 {

class UlMap : public Header {
public:
  UlMap();
  ~UlMap() override;

  void SetUcdCount(uint8_t ucdCount);
  void SetAllocationStartTime(uint32_t allocationStartTime);
  void AddUlMapElement(OfdmUlMapIe ulMapElement);

  uint8_t GetUcdCount() const;
  uint32_t GetAllocationStartTime() const;
  std::list<OfdmUlMapIe> GetUlMapElements() const;

  std::string GetName() const;
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

private:
  uint8_t m_reserved;

  uint8_t m_ucdCount;
  uint32_t m_allocationStartTime;
  std::list<OfdmUlMapIe> m_ulMapElements;
};

} // namespace ns3

#endif
