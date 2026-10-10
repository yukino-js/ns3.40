
#ifndef EDCA_PARAMETER_SET_H
#define EDCA_PARAMETER_SET_H

#include "wifi-information-element.h"

namespace ns3 {

class EdcaParameterSet : public WifiInformationElement {
public:
  EdcaParameterSet();

  WifiInformationElementId ElementId() const override;

  void SetQosInfo(uint8_t qosInfo);
  void SetBeAifsn(uint8_t aifsn);
  void SetBeAci(uint8_t aci);
  void SetBeCWmin(uint32_t cwMin);
  void SetBeCWmax(uint32_t cwMax);
  void SetBeTxopLimit(uint16_t txop);
  void SetBkAifsn(uint8_t aifsn);
  void SetBkAci(uint8_t aci);
  void SetBkCWmin(uint32_t cwMin);
  void SetBkCWmax(uint32_t cwMax);
  void SetBkTxopLimit(uint16_t txop);
  void SetViAifsn(uint8_t aifsn);
  void SetViAci(uint8_t aci);
  void SetViCWmin(uint32_t cwMin);
  void SetViCWmax(uint32_t cwMax);
  void SetViTxopLimit(uint16_t txop);
  void SetVoAifsn(uint8_t aifsn);
  void SetVoAci(uint8_t aci);
  void SetVoCWmin(uint32_t cwMin);
  void SetVoCWmax(uint32_t cwMax);
  void SetVoTxopLimit(uint16_t txop);

  uint8_t GetQosInfo() const;
  uint8_t GetBeAifsn() const;
  uint32_t GetBeCWmin() const;
  uint32_t GetBeCWmax() const;
  uint16_t GetBeTxopLimit() const;
  uint8_t GetBkAifsn() const;
  uint32_t GetBkCWmin() const;
  uint32_t GetBkCWmax() const;
  uint16_t GetBkTxopLimit() const;
  uint8_t GetViAifsn() const;
  uint32_t GetViCWmin() const;
  uint32_t GetViCWmax() const;
  uint16_t GetViTxopLimit() const;
  uint8_t GetVoAifsn() const;
  uint32_t GetVoCWmin() const;
  uint32_t GetVoCWmax() const;
  uint16_t GetVoTxopLimit() const;

private:
  uint16_t GetInformationFieldSize() const override;
  void SerializeInformationField(Buffer::Iterator start) const override;
  uint16_t DeserializeInformationField(Buffer::Iterator start,
                                       uint16_t length) override;

  uint8_t m_qosInfo;
  uint8_t m_reserved;
  uint32_t m_acBE;
  uint32_t m_acBK;
  uint32_t m_acVI;
  uint32_t m_acVO;
};

} // namespace ns3

#endif
