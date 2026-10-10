#include "constant-velocity-mobility-model.h"

#include "ns3/simulator.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(ConstantVelocityMobilityModel);

TypeId ConstantVelocityMobilityModel::GetTypeId() {
  static TypeId tid = TypeId("ns3::ConstantVelocityMobilityModel")
                          .SetParent<MobilityModel>()
                          .SetGroupName("Mobility")
                          .AddConstructor<ConstantVelocityMobilityModel>();
  return tid;
}

ConstantVelocityMobilityModel::ConstantVelocityMobilityModel() {}

ConstantVelocityMobilityModel::~ConstantVelocityMobilityModel() {}

void ConstantVelocityMobilityModel::SetVelocity(const Vector &speed) {
  m_helper.Update();
  m_helper.SetVelocity(speed);
  m_helper.Unpause();
  NotifyCourseChange();
}

Vector ConstantVelocityMobilityModel::DoGetPosition() const {
  m_helper.Update();
  return m_helper.GetCurrentPosition();
}

void ConstantVelocityMobilityModel::DoSetPosition(const Vector &position) {
  m_helper.SetPosition(position);
  NotifyCourseChange();
}

Vector ConstantVelocityMobilityModel::DoGetVelocity() const {
  return m_helper.GetVelocity();
}

} // namespace ns3
