
#include "cosine-antenna-model.h"

#include "antenna-model.h"

#include <ns3/double.h>
#include <ns3/log.h>

#include <cmath>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("CosineAntennaModel");

NS_OBJECT_ENSURE_REGISTERED(CosineAntennaModel);

TypeId CosineAntennaModel::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::CosineAntennaModel")
          .SetParent<AntennaModel>()
          .SetGroupName("Antenna")
          .AddConstructor<CosineAntennaModel>()
          .AddAttribute(
              "VerticalBeamwidth",
              "The 3 dB vertical beamwidth (degrees). A beamwidth of 360 deg "
              "corresponds to constant gain",
              DoubleValue(360),
              MakeDoubleAccessor(&CosineAntennaModel::SetVerticalBeamwidth,
                                 &CosineAntennaModel::GetVerticalBeamwidth),
              MakeDoubleChecker<double>(0, 360))
          .AddAttribute(
              "HorizontalBeamwidth",
              "The 3 dB horizontal beamwidth (degrees). A beamwidth of 360 deg "
              "corresponds to constant gain",
              DoubleValue(120),
              MakeDoubleAccessor(&CosineAntennaModel::SetHorizontalBeamwidth,
                                 &CosineAntennaModel::GetHorizontalBeamwidth),
              MakeDoubleChecker<double>(0, 360))
          .AddAttribute("Orientation",
                        "The angle (degrees) that expresses the orientation of "
                        "the antenna on "
                        "the x-y plane relative to the x axis",
                        DoubleValue(0.0),
                        MakeDoubleAccessor(&CosineAntennaModel::SetOrientation,
                                           &CosineAntennaModel::GetOrientation),
                        MakeDoubleChecker<double>(-360, 360))
          .AddAttribute("MaxGain",
                        "The gain (dB) at the antenna boresight (the direction "
                        "of maximum gain)",
                        DoubleValue(0.0),
                        MakeDoubleAccessor(&CosineAntennaModel::m_maxGain),
                        MakeDoubleChecker<double>());
  return tid;
}

double CosineAntennaModel::GetExponentFromBeamwidth(double beamwidthDegrees) {
  NS_LOG_FUNCTION(beamwidthDegrees);

  double exponent;
  if (beamwidthDegrees == 360.0) {
    exponent = 0.0;
  } else {
    exponent =
        -3.0 /
        (20 * std::log10(std::cos(DegreesToRadians(beamwidthDegrees / 4.0))));
  }

  return exponent;
}

double CosineAntennaModel::GetBeamwidthFromExponent(double exponent) {
  NS_LOG_FUNCTION(exponent);

  double beamwidthRadians = 4 * std::acos(std::pow(0.5, 1 / (2 * exponent)));
  return RadiansToDegrees(beamwidthRadians);
}

void CosineAntennaModel::SetVerticalBeamwidth(double verticalBeamwidthDegrees) {
  NS_LOG_FUNCTION(this << verticalBeamwidthDegrees);
  m_verticalExponent = GetExponentFromBeamwidth(verticalBeamwidthDegrees);
}

void CosineAntennaModel::SetHorizontalBeamwidth(
    double horizontalBeamwidthDegrees) {
  NS_LOG_FUNCTION(this << horizontalBeamwidthDegrees);
  m_horizontalExponent = GetExponentFromBeamwidth(horizontalBeamwidthDegrees);
}

double CosineAntennaModel::GetVerticalBeamwidth() const {
  return GetBeamwidthFromExponent(m_verticalExponent);
}

double CosineAntennaModel::GetHorizontalBeamwidth() const {
  return GetBeamwidthFromExponent(m_horizontalExponent);
}

void CosineAntennaModel::SetOrientation(double orientationDegrees) {
  NS_LOG_FUNCTION(this << orientationDegrees);
  m_orientationRadians = DegreesToRadians(orientationDegrees);
}

double CosineAntennaModel::GetOrientation() const {
  return RadiansToDegrees(m_orientationRadians);
}

double CosineAntennaModel::GetGainDb(Angles a) {
  NS_LOG_FUNCTION(this << a);

  a.SetAzimuth(a.GetAzimuth() - m_orientationRadians);

  NS_LOG_LOGIC(a);

  double gain =
      (std::pow(std::cos(a.GetAzimuth() / 2), 2 * m_horizontalExponent)) *
      (std::pow(std::cos((M_PI / 2 - a.GetInclination()) / 2),
                2 * m_verticalExponent));
  double gainDb = 10 * std::log10(gain);

  NS_LOG_LOGIC("gain = " << gainDb << " + " << m_maxGain << " dB");
  return gainDb + m_maxGain;
}

} // namespace ns3
