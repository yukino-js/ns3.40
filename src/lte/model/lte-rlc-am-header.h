
#ifndef LTE_RLC_AM_HEADER_H
#define LTE_RLC_AM_HEADER_H

#include "lte-rlc-sequence-number.h"

#include "ns3/header.h"

#include <list>

namespace ns3 {

class LteRlcAmHeader : public Header {
public:
  LteRlcAmHeader();
  ~LteRlcAmHeader() override;

  void SetDataPdu();
  void SetControlPdu(uint8_t controlPduType);
  bool IsDataPdu() const;
  bool IsControlPdu() const;

  enum DataControlPdu_t { CONTROL_PDU = 0, DATA_PDU = 1 };

  static constexpr uint8_t STATUS_PDU{0};

  void SetSequenceNumber(SequenceNumber10 sequenceNumber);
  SequenceNumber10 GetSequenceNumber() const;

  void SetFramingInfo(uint8_t framingInfo);
  uint8_t GetFramingInfo() const;

  enum FramingInfoFirstByte_t { FIRST_BYTE = 0x00, NO_FIRST_BYTE = 0x02 };

  enum FramingInfoLastByte_t { LAST_BYTE = 0x00, NO_LAST_BYTE = 0x01 };

  void PushExtensionBit(uint8_t extensionBit);
  void PushLengthIndicator(uint16_t lengthIndicator);

  uint8_t PopExtensionBit();
  uint16_t PopLengthIndicator();

  enum ExtensionBit_t { DATA_FIELD_FOLLOWS = 0, E_LI_FIELDS_FOLLOWS = 1 };

  void SetResegmentationFlag(uint8_t resegFlag);
  uint8_t GetResegmentationFlag() const;

  enum ResegmentationFlag_t { PDU = 0, SEGMENT = 1 };

  void SetPollingBit(uint8_t pollingBit);
  uint8_t GetPollingBit() const;

  enum PollingBit_t {
    STATUS_REPORT_NOT_REQUESTED = 0,
    STATUS_REPORT_IS_REQUESTED = 1
  };

  void SetLastSegmentFlag(uint8_t lsf);
  uint8_t GetLastSegmentFlag() const;

  enum LastSegmentFlag_t { NO_LAST_PDU_SEGMENT = 0, LAST_PDU_SEGMENT = 1 };

  void SetSegmentOffset(uint16_t segmentOffset);
  uint16_t GetSegmentOffset() const;
  uint16_t GetLastOffset() const;

  void SetAckSn(SequenceNumber10 ackSn);
  SequenceNumber10 GetAckSn() const;

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

  bool OneMoreNackWouldFitIn(uint16_t bytes);

  void PushNack(int nack);

  bool IsNackPresent(SequenceNumber10 nack);

  int PopNack();

private:
  uint16_t m_headerLength;
  uint8_t m_dataControlBit;

  uint8_t m_resegmentationFlag;
  uint8_t m_pollingBit;
  uint8_t m_framingInfo;
  SequenceNumber10 m_sequenceNumber;
  uint8_t m_lastSegmentFlag;
  uint16_t m_segmentOffset;
  uint16_t m_lastOffset;

  std::list<uint8_t> m_extensionBits;
  std::list<uint16_t> m_lengthIndicators;

  uint8_t m_controlPduType;

  SequenceNumber10 m_ackSn;
  std::list<int> m_nackSnList;

  std::list<uint8_t> m_extensionBits1;
  std::list<uint8_t> m_extensionBits2;
};

}; // namespace ns3

#endif
