#ifndef CONSTANT_POSITION_MOBILITY_MODEL_H
#define CONSTANT_POSITION_MOBILITY_MODEL_H

#include "mobility-model.h"

namespace ns3 {

class ConstantPositionMobilityModel : public MobilityModel {
public:
  static TypeId GetTypeId();
  ConstantPositionMobilityModel();
  ~ConstantPositionMobilityModel() override;

private:
  Vector DoGetPosition() const override;
  void DoSetPosition(const Vector &position) override;
  Vector DoGetVelocity() const override;

  Vector m_position;
};

} // namespace ns3

#endif
