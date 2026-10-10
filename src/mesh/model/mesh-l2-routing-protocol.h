
#ifndef MESH_L2_ROUTING_PROTOCOL_H
#define MESH_L2_ROUTING_PROTOCOL_H

#include "ns3/mac48-address.h"
#include "ns3/object.h"
#include "ns3/packet.h"

namespace ns3 {

class Packet;
class MeshPointDevice;

class MeshL2RoutingProtocol : public Object {
public:
  static TypeId GetTypeId();
  ~MeshL2RoutingProtocol() override;
  typedef Callback<void, bool, Ptr<Packet>, Mac48Address, Mac48Address,
                   uint16_t, uint32_t>
      RouteReplyCallback;
  virtual bool RequestRoute(uint32_t sourceIface, const Mac48Address source,
                            const Mac48Address destination,
                            Ptr<const Packet> packet, uint16_t protocolType,
                            RouteReplyCallback routeReply) = 0;
  virtual bool RemoveRoutingStuff(uint32_t fromIface, const Mac48Address source,
                                  const Mac48Address destination,
                                  Ptr<Packet> packet,
                                  uint16_t &protocolType) = 0;
  void SetMeshPoint(Ptr<MeshPointDevice> mp);
  Ptr<MeshPointDevice> GetMeshPoint() const;

protected:
  Ptr<MeshPointDevice> m_mp;
};
} // namespace ns3
#endif
