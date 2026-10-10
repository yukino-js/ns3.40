
#ifndef DL_FRAME_PREFIX_IE_H
#define DL_FRAME_PREFIX_IE_H

#include "ns3/header.h"

#include <stdint.h>

namespace ns3 {

class DlFramePrefixIe {
public:
  DlFramePrefixIe();
  ~DlFramePrefixIe();

  void SetRateId(uint8_t rateId);
  void SetDiuc(uint8_t diuc);
  void SetPreamblePresent(uint8_t preamblePresent);
  void SetLength(uint16_t length);
  void SetStartTime(uint16_t startTime);

  uint8_t GetRateId() const;
  uint8_t GetDiuc() const;
  uint8_t GetPreamblePresent() const;
  uint16_t GetLength() const;
  uint16_t GetStartTime() const;

  uint16_t GetSize() const;

  Buffer::Iterator Write(Buffer::Iterator start) const;
  Buffer::Iterator Read(Buffer::Iterator start);

private:
  uint8_t m_rateId;
  uint8_t m_diuc;
  uint8_t m_preamblePresent;
  uint16_t m_length;
  uint16_t m_startTime;
};

} // namespace ns3

#endif

#ifndef OFDM_DOWNLINK_FRAME_PREFIX_H
#define OFDM_DOWNLINK_FRAME_PREFIX_H

#include "ns3/header.h"
#include "ns3/mac48-address.h"

#include <stdint.h>

namespace ns3 {

class OfdmDownlinkFramePrefix : public Header {
public:
  OfdmDownlinkFramePrefix();
  ~OfdmDownlinkFramePrefix() override;

  static TypeId GetTypeId();

  void SetBaseStationId(Mac48Address baseStationId);
  void SetFrameNumber(uint32_t frameNumber);
  void SetConfigurationChangeCount(uint8_t configurationChangeCount);
  void AddDlFramePrefixElement(DlFramePrefixIe dlFramePrefixElement);
  void SetHcs(uint8_t hcs);

  Mac48Address GetBaseStationId() const;
  uint32_t GetFrameNumber() const;
  uint8_t GetConfigurationChangeCount() const;
  std::vector<DlFramePrefixIe> GetDlFramePrefixElements() const;
  uint8_t GetHcs() const;

  std::string GetName() const;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

private:
  Mac48Address m_baseStationId;
  uint32_t m_frameNumber;
  uint8_t m_configurationChangeCount;
  std::vector<DlFramePrefixIe> m_dlFramePrefixElements;
  uint8_t m_hcs;
};

} // namespace ns3

#endif
