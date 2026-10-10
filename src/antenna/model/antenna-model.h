
#ifndef ANTENNA_MODEL_H
#define ANTENNA_MODEL_H

#include "angles.h"

#include <ns3/object.h>

namespace ns3 {

class AntennaModel : public Object {
public:
  AntennaModel();
  ~AntennaModel() override;

  static TypeId GetTypeId();

  virtual double GetGainDb(Angles a) = 0;
};

} // namespace ns3

#endif
