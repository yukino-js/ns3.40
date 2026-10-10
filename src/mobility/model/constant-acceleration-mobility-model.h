
#ifndef CONSTANT_ACCELERATION_MOBILITY_MODEL_H
#define CONSTANT_ACCELERATION_MOBILITY_MODEL_H

#include "mobility-model.h"

#include "ns3/nstime.h"

namespace ns3 {

class ConstantAccelerationMobilityModel : public MobilityModel {
public:
  static TypeId GetTypeId();
  ConstantAccelerationMobilityModel();
  ~ConstantAccelerationMobilityModel() override;
  void SetVelocityAndAcceleration(const Vector &velocity,
                                  const Vector &acceleration);

private:
  Vector DoGetPosition() const override;
  void DoSetPosition(const Vector &position) override;
  Vector DoGetVelocity() const override;

  Time m_baseTime;
  Vector m_basePosition;
  Vector m_baseVelocity;
  Vector m_acceleration;
};

} // namespace ns3

#endif
