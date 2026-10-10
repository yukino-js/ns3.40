#ifndef CONSTANT_VELOCITY_MOBILITY_MODEL_H
#define CONSTANT_VELOCITY_MOBILITY_MODEL_H

#include "constant-velocity-helper.h"
#include "mobility-model.h"

#include "ns3/nstime.h"

#include <stdint.h>

namespace ns3 {

class ConstantVelocityMobilityModel : public MobilityModel {
public:
  static TypeId GetTypeId();
  ConstantVelocityMobilityModel();
  ~ConstantVelocityMobilityModel() override;

  void SetVelocity(const Vector &speed);

private:
  Vector DoGetPosition() const override;
  void DoSetPosition(const Vector &position) override;
  Vector DoGetVelocity() const override;
  ConstantVelocityHelper m_helper;
};

} // namespace ns3

#endif
