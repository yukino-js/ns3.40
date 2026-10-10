
#ifndef UAN_NOISE_MODEL_DEFAULT_H
#define UAN_NOISE_MODEL_DEFAULT_H

#include "uan-noise-model.h"

#include "ns3/attribute.h"
#include "ns3/object.h"

namespace ns3 {

class UanNoiseModelDefault : public UanNoiseModel {
public:
  UanNoiseModelDefault();
  ~UanNoiseModelDefault() override;

  static TypeId GetTypeId();

  double GetNoiseDbHz(double fKhz) const override;

private:
  double m_wind;
  double m_shipping;
};

} // namespace ns3

#endif
