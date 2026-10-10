
#include "uniform-planar-array.h"

#include <ns3/boolean.h>
#include <ns3/double.h>
#include <ns3/log.h>
#include <ns3/uinteger.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("UniformPlanarArray");

NS_OBJECT_ENSURE_REGISTERED(UniformPlanarArray);

UniformPlanarArray::UniformPlanarArray() : PhasedArrayModel() {}

UniformPlanarArray::~UniformPlanarArray() {}

TypeId UniformPlanarArray::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::UniformPlanarArray")
          .SetParent<PhasedArrayModel>()
          .AddConstructor<UniformPlanarArray>()
          .SetGroupName("Antenna")
          .AddAttribute("AntennaHorizontalSpacing",
                        "Horizontal spacing between antenna elements, in "
                        "multiples of wave length",
                        DoubleValue(0.5),
                        MakeDoubleAccessor(
                            &UniformPlanarArray::SetAntennaHorizontalSpacing,
                            &UniformPlanarArray::GetAntennaHorizontalSpacing),
                        MakeDoubleChecker<double>(0.0))
          .AddAttribute("AntennaVerticalSpacing",
                        "Vertical spacing between antenna elements, in "
                        "multiples of wave length",
                        DoubleValue(0.5),
                        MakeDoubleAccessor(
                            &UniformPlanarArray::SetAntennaVerticalSpacing,
                            &UniformPlanarArray::GetAntennaVerticalSpacing),
                        MakeDoubleChecker<double>(0.0))
          .AddAttribute(
              "NumColumns", "Horizontal size of the array", UintegerValue(4),
              MakeUintegerAccessor(&UniformPlanarArray::SetNumColumns,
                                   &UniformPlanarArray::GetNumColumns),
              MakeUintegerChecker<uint32_t>(1))
          .AddAttribute("NumRows", "Vertical size of the array",
                        UintegerValue(4),
                        MakeUintegerAccessor(&UniformPlanarArray::SetNumRows,
                                             &UniformPlanarArray::GetNumRows),
                        MakeUintegerChecker<uint32_t>(1))
          .AddAttribute("BearingAngle", "The bearing angle in radians",
                        DoubleValue(0.0),
                        MakeDoubleAccessor(&UniformPlanarArray::SetAlpha),
                        MakeDoubleChecker<double>(-M_PI, M_PI))
          .AddAttribute("DowntiltAngle", "The downtilt angle in radians",
                        DoubleValue(0.0),
                        MakeDoubleAccessor(&UniformPlanarArray::SetBeta),
                        MakeDoubleChecker<double>(-M_PI, M_PI))
          .AddAttribute("PolSlantAngle",
                        "The polarization slant angle in radians",
                        DoubleValue(0.0),
                        MakeDoubleAccessor(&UniformPlanarArray::SetPolSlant),
                        MakeDoubleChecker<double>(-M_PI, M_PI));
  return tid;
}

void UniformPlanarArray::SetNumColumns(uint32_t n) {
  NS_LOG_FUNCTION(this << n);
  if (n != m_numColumns) {
    m_isBfVectorValid = false;
  }
  m_numColumns = n;
}

uint32_t UniformPlanarArray::GetNumColumns() const { return m_numColumns; }

void UniformPlanarArray::SetNumRows(uint32_t n) {
  NS_LOG_FUNCTION(this << n);
  if (n != m_numRows) {
    m_isBfVectorValid = false;
  }
  m_numRows = n;
}

uint32_t UniformPlanarArray::GetNumRows() const { return m_numRows; }

void UniformPlanarArray::SetAlpha(double alpha) {
  m_alpha = alpha;
  m_cosAlpha = cos(m_alpha);
  m_sinAlpha = sin(m_alpha);
}

void UniformPlanarArray::SetBeta(double beta) {
  m_beta = beta;
  m_cosBeta = cos(m_beta);
  m_sinBeta = sin(m_beta);
}

void UniformPlanarArray::SetPolSlant(double polSlant) {
  m_polSlant = polSlant;
  m_cosPolSlant = cos(m_polSlant);
  m_sinPolSlant = sin(m_polSlant);
}

void UniformPlanarArray::SetAntennaHorizontalSpacing(double s) {
  NS_LOG_FUNCTION(this << s);
  NS_ABORT_MSG_IF(s <= 0, "Trying to set an invalid spacing: " << s);

  if (s != m_disH) {
    m_isBfVectorValid = false;
  }
  m_disH = s;
}

double UniformPlanarArray::GetAntennaHorizontalSpacing() const {
  return m_disH;
}

void UniformPlanarArray::SetAntennaVerticalSpacing(double s) {
  NS_LOG_FUNCTION(this << s);
  NS_ABORT_MSG_IF(s <= 0, "Trying to set an invalid spacing: " << s);

  if (s != m_disV) {
    m_isBfVectorValid = false;
  }
  m_disV = s;
}

double UniformPlanarArray::GetAntennaVerticalSpacing() const { return m_disV; }

std::pair<double, double>
UniformPlanarArray::GetElementFieldPattern(Angles a) const {
  NS_LOG_FUNCTION(this << a);

  double cosIncl = cos(a.GetInclination());
  double sinIncl = sin(a.GetInclination());
  double cosAzim = cos(a.GetAzimuth() - m_alpha);
  double sinAzim = sin(a.GetAzimuth() - m_alpha);
  double thetaPrime =
      std::acos(m_cosBeta * cosIncl + m_sinBeta * cosAzim * sinIncl);
  double phiPrime = std::arg(std::complex<double>(
      m_cosBeta * sinIncl * cosAzim - m_sinBeta * cosIncl, sinAzim * sinIncl));
  Angles aPrime(phiPrime, thetaPrime);
  NS_LOG_DEBUG(a << " -> " << aPrime);

  double aPrimeDb = m_antennaElement->GetGainDb(aPrime);
  double fieldThetaPrime = pow(10, aPrimeDb / 20) * m_cosPolSlant;
  double fieldPhiPrime = pow(10, aPrimeDb / 20) * m_sinPolSlant;

  double psi = std::arg(
      std::complex<double>(m_cosBeta * sinIncl - m_sinBeta * cosIncl * cosAzim,
                           m_sinBeta * sinAzim));
  NS_LOG_DEBUG("psi " << psi);

  double fieldTheta = cos(psi) * fieldThetaPrime - sin(psi) * fieldPhiPrime;
  double fieldPhi = sin(psi) * fieldThetaPrime + cos(psi) * fieldPhiPrime;
  NS_LOG_DEBUG(RadiansToDegrees(a.GetAzimuth())
               << " " << RadiansToDegrees(a.GetInclination()) << " "
               << fieldTheta * fieldTheta + fieldPhi * fieldPhi);

  return std::make_pair(fieldPhi, fieldTheta);
}

Vector UniformPlanarArray::GetElementLocation(uint64_t index) const {
  NS_LOG_FUNCTION(this << index);

  double xPrime = 0;
  double yPrime = m_disH * (index % m_numColumns);
  double zPrime = m_disV * floor(index / m_numColumns);

  Vector loc;
  loc.x = m_cosAlpha * m_cosBeta * xPrime - m_sinAlpha * yPrime +
          m_cosAlpha * m_sinBeta * zPrime;
  loc.y = m_sinAlpha * m_cosBeta * xPrime + m_cosAlpha * yPrime +
          m_sinAlpha * m_sinBeta * zPrime;
  loc.z = -m_sinBeta * xPrime + m_cosBeta * zPrime;
  return loc;
}

size_t UniformPlanarArray::GetNumberOfElements() const {
  return m_numRows * m_numColumns;
}

} // namespace ns3
