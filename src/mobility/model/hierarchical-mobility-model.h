#ifndef HIERARCHICAL_MOBILITY_MODEL_H
#define HIERARCHICAL_MOBILITY_MODEL_H

#include "mobility-model.h"

namespace ns3 {

class HierarchicalMobilityModel : public MobilityModel {
public:
  static TypeId GetTypeId();

  HierarchicalMobilityModel();

  Ptr<MobilityModel> GetChild() const;
  Ptr<MobilityModel> GetParent() const;
  void SetChild(Ptr<MobilityModel> model);
  void SetParent(Ptr<MobilityModel> model);

private:
  Vector DoGetPosition() const override;
  void DoSetPosition(const Vector &position) override;
  Vector DoGetVelocity() const override;
  void DoInitialize() override;
  int64_t DoAssignStreams(int64_t) override;

  void ParentChanged(Ptr<const MobilityModel> model);
  void ChildChanged(Ptr<const MobilityModel> model);

  Ptr<MobilityModel> m_child;
  Ptr<MobilityModel> m_parent;
};

} // namespace ns3

#endif
