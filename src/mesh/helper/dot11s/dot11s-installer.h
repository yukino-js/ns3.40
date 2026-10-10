
#ifndef DOT11S_STACK_INSTALLER_H
#define DOT11S_STACK_INSTALLER_H

#include "ns3/mesh-stack-installer.h"

namespace ns3 {

class Dot11sStack : public MeshStack {
public:
  static TypeId GetTypeId();

  Dot11sStack();

  ~Dot11sStack() override;

  void DoDispose() override;

  bool InstallStack(Ptr<MeshPointDevice> mp) override;

  void Report(const Ptr<MeshPointDevice> mp, std::ostream &) override;

  void ResetStats(const Ptr<MeshPointDevice> mp) override;

private:
  Mac48Address m_root;
};

} // namespace ns3

#endif
