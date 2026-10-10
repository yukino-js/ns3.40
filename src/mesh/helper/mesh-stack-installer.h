
#ifndef MESH_STACK_INSTALLER_H
#define MESH_STACK_INSTALLER_H
#include "ns3/mesh-point-device.h"

namespace ns3 {
class MeshStack : public Object {
public:
  static TypeId GetTypeId();

  virtual bool InstallStack(Ptr<MeshPointDevice> mp) = 0;
  virtual void Report(const Ptr<MeshPointDevice> mp, std::ostream &os) = 0;
  virtual void ResetStats(const Ptr<MeshPointDevice> mp) = 0;
};
} // namespace ns3
#endif
