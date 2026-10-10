
#ifndef PARABOLIC_ANTENNA_MODEL_H
#define PARABOLIC_ANTENNA_MODEL_H

#include "antenna-model.h"

#include <ns3/object.h>

namespace ns3 {

class ParabolicAntennaModel : public AntennaModel {
public:
  static TypeId GetTypeId();

  double GetGainDb(Angles a) override;

  void SetBeamwidth(double beamwidthDegrees);
  double GetBeamwidth() const;
  void SetOrientation(double orientationDegrees);
  double GetOrientation() const;

private:
  double m_beamwidthRadians;
  double m_orientationRadians;
  double m_maxAttenuation;
};

} // namespace ns3

#endif
