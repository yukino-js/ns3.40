
#ifndef SIXLOWPAN_HELPER_H
#define SIXLOWPAN_HELPER_H

#include "ns3/net-device-container.h"
#include "ns3/object-factory.h"

#include <string>

namespace ns3 {

class Node;
class AttributeValue;
class Time;

class SixLowPanHelper {
public:
  SixLowPanHelper();
  void SetDeviceAttribute(std::string n1, const AttributeValue &v1);

  NetDeviceContainer Install(NetDeviceContainer c);

  void AddContext(NetDeviceContainer c, uint8_t contextId, Ipv6Prefix context,
                  Time validity);

  void RenewContext(NetDeviceContainer c, uint8_t contextId, Time validity);

  void InvalidateContext(NetDeviceContainer c, uint8_t contextId);

  void RemoveContext(NetDeviceContainer c, uint8_t contextId);

  int64_t AssignStreams(NetDeviceContainer c, int64_t stream);

private:
  ObjectFactory m_deviceFactory;
};

} // namespace ns3

#endif
