
#ifndef THREE_GPP_ANTENNA_MODEL_H
#define THREE_GPP_ANTENNA_MODEL_H

#include "antenna-model.h"

#include <ns3/object.h>

namespace ns3 {

class ThreeGppAntennaModel : public AntennaModel {
public:
  ThreeGppAntennaModel();
  ~ThreeGppAntennaModel() override;

  static TypeId GetTypeId();

  double GetGainDb(Angles a) override;

  double GetVerticalBeamwidth() const;

  double GetHorizontalBeamwidth() const;

  double GetSlaV() const;

  double GetMaxAttenuation() const;

  double GetAntennaElementGain() const;

private:
  double m_verticalBeamwidthDegrees;
  double m_horizontalBeamwidthDegrees;
  double m_aMax;
  double m_slaV;
  double m_geMax;
};

} // namespace ns3

#endif
