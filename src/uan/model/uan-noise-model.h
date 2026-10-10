
#ifndef UAN_NOISE_MODEL_H
#define UAN_NOISE_MODEL_H

#include "ns3/object.h"

namespace ns3 {

class UanNoiseModel : public Object {
public:
  static TypeId GetTypeId();

  virtual double GetNoiseDbHz(double fKhz) const = 0;

  virtual void Clear();

  void DoDispose() override;
};

} // namespace ns3

#endif
