

#ifndef IP_L4_PROTOCOL_H
#define IP_L4_PROTOCOL_H

#include "ipv4-header.h"
#include "ipv6-header.h"

#include "ns3/callback.h"
#include "ns3/object.h"

namespace ns3 {

class Packet;
class Ipv4Address;
class Ipv4Interface;
class Ipv6Address;
class Ipv6Interface;
class Ipv4Route;
class Ipv6Route;

class IpL4Protocol : public Object {
public:
  enum RxStatus {
    RX_OK,
    RX_CSUM_FAILED,
    RX_ENDPOINT_CLOSED,
    RX_ENDPOINT_UNREACH
  };

  static TypeId GetTypeId();

  ~IpL4Protocol() override;

  virtual int GetProtocolNumber() const = 0;

  virtual RxStatus Receive(Ptr<Packet> p, const Ipv4Header &header,
                           Ptr<Ipv4Interface> incomingInterface) = 0;

  virtual RxStatus Receive(Ptr<Packet> p, const Ipv6Header &header,
                           Ptr<Ipv6Interface> incomingInterface) = 0;

  virtual void ReceiveIcmp(Ipv4Address icmpSource, uint8_t icmpTtl,
                           uint8_t icmpType, uint8_t icmpCode,
                           uint32_t icmpInfo, Ipv4Address payloadSource,
                           Ipv4Address payloadDestination,
                           const uint8_t payload[8]);

  virtual void ReceiveIcmp(Ipv6Address icmpSource, uint8_t icmpTtl,
                           uint8_t icmpType, uint8_t icmpCode,
                           uint32_t icmpInfo, Ipv6Address payloadSource,
                           Ipv6Address payloadDestination,
                           const uint8_t payload[8]);

  typedef Callback<void, Ptr<Packet>, Ipv4Address, Ipv4Address, uint8_t,
                   Ptr<Ipv4Route>>
      DownTargetCallback;
  typedef Callback<void, Ptr<Packet>, Ipv6Address, Ipv6Address, uint8_t,
                   Ptr<Ipv6Route>>
      DownTargetCallback6;

  virtual void SetDownTarget(DownTargetCallback cb) = 0;

  virtual void SetDownTarget6(DownTargetCallback6 cb) = 0;

  virtual DownTargetCallback GetDownTarget() const = 0;

  virtual DownTargetCallback6 GetDownTarget6() const = 0;
};

} // namespace ns3

#endif
