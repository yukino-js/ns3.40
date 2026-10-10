
#ifndef FLAME_INSTALLER_H
#define FLAME_INSTALLER_H

#include "ns3/mesh-stack-installer.h"

namespace ns3 {

class FlameStack : public MeshStack {
public:
  static TypeId GetTypeId();

  FlameStack();

  ~FlameStack() override;

  void DoDispose() override;

  bool InstallStack(Ptr<MeshPointDevice> mp) override;

  void Report(const Ptr<MeshPointDevice> mp, std::ostream &) override;

  void ResetStats(const Ptr<MeshPointDevice> mp) override;
};

} // namespace ns3

#endif
