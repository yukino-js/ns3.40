
#ifndef VHT_OPERATION_H
#define VHT_OPERATION_H

#include "ns3/wifi-information-element.h"

namespace ns3 {

class VhtOperation : public WifiInformationElement {
public:
  VhtOperation();

  WifiInformationElementId ElementId() const override;
  void Print(std::ostream &os) const override;

  void SetChannelWidth(uint8_t channelWidth);
  void
  SetChannelCenterFrequencySegment0(uint8_t channelCenterFrequencySegment0);
  void
  SetChannelCenterFrequencySegment1(uint8_t channelCenterFrequencySegment1);
  void SetBasicVhtMcsAndNssSet(uint16_t basicVhtMcsAndNssSet);
  void SetMaxVhtMcsPerNss(uint8_t nss, uint8_t maxVhtMcs);

  uint8_t GetChannelWidth() const;
  uint8_t GetChannelCenterFrequencySegment0() const;
  uint8_t GetChannelCenterFrequencySegment1() const;
  uint16_t GetBasicVhtMcsAndNssSet() const;

private:
  uint16_t GetInformationFieldSize() const override;
  void SerializeInformationField(Buffer::Iterator start) const override;
  uint16_t DeserializeInformationField(Buffer::Iterator start,
                                       uint16_t length) override;

  uint8_t m_channelWidth;
  uint8_t m_channelCenterFrequencySegment0;
  uint8_t m_channelCenterFrequencySegment1;

  uint16_t m_basicVhtMcsAndNssSet;
};

} // namespace ns3

#endif
