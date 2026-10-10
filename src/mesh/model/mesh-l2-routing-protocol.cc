
#include "mesh-l2-routing-protocol.h"

#include "mesh-point-device.h"

#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("MeshL2RoutingProtocol");

NS_OBJECT_ENSURE_REGISTERED(MeshL2RoutingProtocol);

TypeId MeshL2RoutingProtocol::GetTypeId() {
  static TypeId tid = TypeId("ns3::MeshL2RoutingProtocol")
                          .SetParent<Object>()
                          .SetGroupName("Mesh");
  return tid;
}

MeshL2RoutingProtocol::~MeshL2RoutingProtocol() { m_mp = nullptr; }

void MeshL2RoutingProtocol::SetMeshPoint(Ptr<MeshPointDevice> mp) { m_mp = mp; }

Ptr<MeshPointDevice> MeshL2RoutingProtocol::GetMeshPoint() const {
  return m_mp;
}

} // namespace ns3
