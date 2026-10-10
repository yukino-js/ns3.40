
#ifndef DSSS_PARAMETER_SET_H
#define DSSS_PARAMETER_SET_H

#include "ns3/wifi-information-element.h"

namespace ns3 {

class DsssParameterSet : public WifiInformationElement {
public:
  DsssParameterSet();

  WifiInformationElementId ElementId() const override;

  void SetCurrentChannel(uint8_t currentChannel);

private:
  uint16_t GetInformationFieldSize() const override;
  void SerializeInformationField(Buffer::Iterator start) const override;
  uint16_t DeserializeInformationField(Buffer::Iterator start,
                                       uint16_t length) override;

  uint8_t m_currentChannel;
};

} // namespace ns3

#endif
