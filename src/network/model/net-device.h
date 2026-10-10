#ifndef NET_DEVICE_H
#define NET_DEVICE_H

#include "address.h"
#include "packet.h"

#include "ns3/callback.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/object.h"
#include "ns3/ptr.h"

#include <stdint.h>

namespace ns3 {

class Node;
class Channel;

class NetDevice : public Object {
public:
  static TypeId GetTypeId();
  ~NetDevice() override;

  virtual void SetIfIndex(const uint32_t index) = 0;
  virtual uint32_t GetIfIndex() const = 0;

  virtual Ptr<Channel> GetChannel() const = 0;

  virtual void SetAddress(Address address) = 0;

  virtual Address GetAddress() const = 0;

  virtual bool SetMtu(const uint16_t mtu) = 0;
  virtual uint16_t GetMtu() const = 0;
  virtual bool IsLinkUp() const = 0;
  typedef void (*LinkChangeTracedCallback)();
  virtual void AddLinkChangeCallback(Callback<void> callback) = 0;
  virtual bool IsBroadcast() const = 0;
  virtual Address GetBroadcast() const = 0;

  virtual bool IsMulticast() const = 0;

  virtual Address GetMulticast(Ipv4Address multicastGroup) const = 0;

  virtual Address GetMulticast(Ipv6Address addr) const = 0;

  virtual bool IsBridge() const = 0;

  virtual bool IsPointToPoint() const = 0;
  virtual bool Send(Ptr<Packet> packet, const Address &dest,
                    uint16_t protocolNumber) = 0;
  virtual bool SendFrom(Ptr<Packet> packet, const Address &source,
                        const Address &dest, uint16_t protocolNumber) = 0;
  virtual Ptr<Node> GetNode() const = 0;

  virtual void SetNode(Ptr<Node> node) = 0;

  virtual bool NeedsArp() const = 0;

  enum PacketType {
    PACKET_HOST = 1,
    NS3_PACKET_HOST = PACKET_HOST,
    PACKET_BROADCAST,
    NS3_PACKET_BROADCAST = PACKET_BROADCAST,
    PACKET_MULTICAST,
    NS3_PACKET_MULTICAST = PACKET_MULTICAST,
    PACKET_OTHERHOST,
    NS3_PACKET_OTHERHOST = PACKET_OTHERHOST,
  };

  typedef Callback<bool, Ptr<NetDevice>, Ptr<const Packet>, uint16_t,
                   const Address &>
      ReceiveCallback;

  virtual void SetReceiveCallback(ReceiveCallback cb) = 0;

  typedef Callback<bool, Ptr<NetDevice>, Ptr<const Packet>, uint16_t,
                   const Address &, const Address &, PacketType>
      PromiscReceiveCallback;

  virtual void SetPromiscReceiveCallback(PromiscReceiveCallback cb) = 0;

  virtual bool SupportsSendFrom() const = 0;
};

} // namespace ns3

#endif
