
#ifndef FLAME_PROTOCOL_H
#define FLAME_PROTOCOL_H

#include "ns3/mesh-l2-routing-protocol.h"
#include "ns3/nstime.h"
#include "ns3/tag.h"

#include <map>

namespace ns3 {
namespace flame {
class FlameProtocolMac;
class FlameHeader;
class FlameRtable;

class FlameTag : public Tag {
public:
  Mac48Address transmitter;
  Mac48Address receiver;

  FlameTag(Mac48Address a = Mac48Address()) : receiver(a) {}

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(TagBuffer i) const override;
  void Deserialize(TagBuffer i) override;
  void Print(std::ostream &os) const override;
};

class FlameProtocol : public MeshL2RoutingProtocol {
public:
  static TypeId GetTypeId();

  FlameProtocol();
  ~FlameProtocol() override;

  FlameProtocol(const FlameProtocol &) = delete;
  FlameProtocol &operator=(const FlameProtocol &) = delete;

  void DoDispose() override;

  bool RequestRoute(uint32_t sourceIface, const Mac48Address source,
                    const Mac48Address destination, Ptr<const Packet> packet,
                    uint16_t protocolType,
                    RouteReplyCallback routeReply) override;
  bool RemoveRoutingStuff(uint32_t fromIface, const Mac48Address source,
                          const Mac48Address destination, Ptr<Packet> packet,
                          uint16_t &protocolType) override;
  bool Install(Ptr<MeshPointDevice> mp);
  Mac48Address GetAddress();
  void Report(std::ostream &os) const;
  void ResetStats();

private:
  static const uint16_t FLAME_PROTOCOL = 0x4040;
  bool HandleDataFrame(uint16_t seqno, Mac48Address source,
                       const FlameHeader flameHdr, Mac48Address receiver,
                       uint32_t fromIface);
  typedef std::map<uint32_t, Ptr<FlameProtocolMac>> FlamePluginMap;
  FlamePluginMap m_interfaces;
  Mac48Address m_address;
  Time m_broadcastInterval;
  Time m_lastBroadcast;
  uint8_t m_maxCost;
  uint16_t m_myLastSeqno;
  Ptr<FlameRtable> m_rtable;

  struct Statistics {
    uint16_t txUnicast;
    uint16_t txBroadcast;
    uint32_t txBytes;
    uint16_t droppedTtl;
    uint16_t totalDropped;
    void Print(std::ostream &os) const;
    Statistics();
  };

  Statistics m_stats;
};
} // namespace flame
} // namespace ns3
#endif
