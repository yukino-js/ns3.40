
#include "mesh-stack-installer.h"

namespace ns3 {
NS_OBJECT_ENSURE_REGISTERED(MeshStack);

TypeId MeshStack::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::MeshStack").SetParent<Object>().SetGroupName("Mesh");
  return tid;
}

} // namespace ns3
