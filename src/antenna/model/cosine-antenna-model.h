
#ifndef COSINE_ANTENNA_MODEL_H
#define COSINE_ANTENNA_MODEL_H

#include "antenna-model.h"

#include <ns3/object.h>

namespace ns3 {

class CosineAntennaModel : public AntennaModel {
public:
  static TypeId GetTypeId();

  double GetGainDb(Angles a) override;

  double GetVerticalBeamwidth() const;

  double GetHorizontalBeamwidth() const;

  double GetOrientation() const;

private:
  void SetVerticalBeamwidth(double verticalBeamwidthDegrees);

  void SetHorizontalBeamwidth(double horizontalBeamwidthDegrees);

  void SetOrientation(double orientationDegrees);

  static double GetExponentFromBeamwidth(double beamwidthDegrees);

  static double GetBeamwidthFromExponent(double exponent);

  double m_verticalExponent;
  double m_horizontalExponent;
  double m_orientationRadians;
  double m_maxGain;
};

} // namespace ns3

#endif
