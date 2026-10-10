
#ifndef ISOTROPIC_ANTENNA_MODEL_H
#define ISOTROPIC_ANTENNA_MODEL_H

#include "antenna-model.h"

#include <ns3/object.h>

namespace ns3 {

class IsotropicAntennaModel : public AntennaModel {
public:
  IsotropicAntennaModel();

  static TypeId GetTypeId();

  double GetGainDb(Angles a) override;

protected:
  double m_gainDb;
};

} // namespace ns3

#endif
