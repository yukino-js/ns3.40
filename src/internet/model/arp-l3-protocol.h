#ifndef ARP_L3_PROTOCOL_H
#define ARP_L3_PROTOCOL_H

#include "ipv4-header.h"

#include "ns3/address.h"
#include "ns3/net-device.h"
#include "ns3/ptr.h"
#include "ns3/random-variable-stream.h"
#include "ns3/traced-callback.h"

#include <list>

namespace ns3 {

class ArpCache;
class NetDevice;
class Node;
class Packet;
class Ipv4Interface;
class TrafficControlLayer;

class ArpL3Protocol : public Object {
public:
  static TypeId GetTypeId();
  static const uint16_t PROT_NUMBER;

  ArpL3Protocol();
  ~ArpL3Protocol() override;

  ArpL3Protocol(const ArpL3Protocol &) = delete;
  ArpL3Protocol &operator=(const ArpL3Protocol &) = delete;

  void SetNode(Ptr<Node> node);

  void SetTrafficControl(Ptr<TrafficControlLayer> tc);

  Ptr<ArpCache> CreateCache(Ptr<NetDevice> device,
                            Ptr<Ipv4Interface> interface);

  void Receive(Ptr<NetDevice> device, Ptr<const Packet> p, uint16_t protocol,
               const Address &from, const Address &to,
               NetDevice::PacketType packetType);
  bool Lookup(Ptr<Packet> p, const Ipv4Header &ipHeader,
              Ipv4Address destination, Ptr<NetDevice> device,
              Ptr<ArpCache> cache, Address *hardwareDestination);

  int64_t AssignStreams(int64_t stream);

protected:
  void DoDispose() override;
  void NotifyNewAggregate() override;

private:
  typedef std::list<Ptr<ArpCache>> CacheList;

  Ptr<ArpCache> FindCache(Ptr<NetDevice> device);

  void SendArpRequest(Ptr<const ArpCache> cache, Ipv4Address to);
  void SendArpReply(Ptr<const ArpCache> cache, Ipv4Address myIp,
                    Ipv4Address toIp, Address toMac);

  CacheList m_cacheList;
  Ptr<Node> m_node;
  TracedCallback<Ptr<const Packet>> m_dropTrace;
  Ptr<RandomVariableStream> m_requestJitter;
  Ptr<TrafficControlLayer> m_tc;
};

} // namespace ns3

#endif
