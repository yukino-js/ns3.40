
#include "phased-array-model.h"

#include "isotropic-antenna-model.h"

#include <ns3/boolean.h>
#include <ns3/double.h>
#include <ns3/log.h>
#include <ns3/pointer.h>
#include <ns3/uinteger.h>

namespace ns3 {

uint32_t PhasedArrayModel::m_idCounter = 0;

NS_LOG_COMPONENT_DEFINE("PhasedArrayModel");

NS_OBJECT_ENSURE_REGISTERED(PhasedArrayModel);

std::ostream &operator<<(std::ostream &os,
                         const PhasedArrayModel::ComplexVector &cv) {
  size_t N = cv.GetSize();

  if (N == 0) {
    os << "[]";
    return os;
  }

  os << "[";
  for (std::size_t i = 0; i < N - 1; ++i) {
    os << cv[i] << ", ";
  }
  os << cv[N - 1] << "]";
  return os;
}

PhasedArrayModel::PhasedArrayModel() : m_isBfVectorValid{false} {
  m_id = m_idCounter++;
}

PhasedArrayModel::~PhasedArrayModel() {}

TypeId PhasedArrayModel::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::PhasedArrayModel")
          .SetParent<Object>()
          .SetGroupName("Antenna")
          .AddAttribute(
              "AntennaElement",
              "A pointer to the antenna element used by the phased array",
              PointerValue(CreateObject<IsotropicAntennaModel>()),
              MakePointerAccessor(&PhasedArrayModel::m_antennaElement),
              MakePointerChecker<AntennaModel>());
  return tid;
}

void PhasedArrayModel::SetBeamformingVector(
    const ComplexVector &beamformingVector) {
  NS_LOG_FUNCTION(this << beamformingVector);
  NS_ASSERT_MSG(beamformingVector.GetSize() == GetNumberOfElements(),
                beamformingVector.GetSize() << " != " << GetNumberOfElements());
  m_beamformingVector = beamformingVector;
  m_isBfVectorValid = true;
}

PhasedArrayModel::ComplexVector PhasedArrayModel::GetBeamformingVector() const {
  NS_LOG_FUNCTION(this);
  NS_ASSERT_MSG(m_isBfVectorValid, "The beamforming vector should be Set "
                                   "before it's Get, and should refer to the "
                                   "current array configuration");
  return m_beamformingVector;
}

PhasedArrayModel::ComplexVector
PhasedArrayModel::GetBeamformingVector(Angles a) const {
  NS_LOG_FUNCTION(this << a);

  ComplexVector beamformingVector = GetSteeringVector(a);
  double normRes = norm(beamformingVector);

  for (size_t i = 0; i < GetNumberOfElements(); i++) {
    beamformingVector[i] = std::conj(beamformingVector[i]) / normRes;
  }

  return beamformingVector;
}

PhasedArrayModel::ComplexVector
PhasedArrayModel::GetSteeringVector(Angles a) const {
  ComplexVector steeringVector(GetNumberOfElements());
  for (size_t i = 0; i < GetNumberOfElements(); i++) {
    Vector loc = GetElementLocation(i);
    double phase = -2 * M_PI *
                   (sin(a.GetInclination()) * cos(a.GetAzimuth()) * loc.x +
                    sin(a.GetInclination()) * sin(a.GetAzimuth()) * loc.y +
                    cos(a.GetInclination()) * loc.z);
    steeringVector[i] = std::polar<double>(1.0, phase);
  }
  return steeringVector;
}

void PhasedArrayModel::SetAntennaElement(Ptr<AntennaModel> antennaElement) {
  NS_LOG_FUNCTION(this);
  m_antennaElement = antennaElement;
}

Ptr<const AntennaModel> PhasedArrayModel::GetAntennaElement() const {
  NS_LOG_FUNCTION(this);
  return m_antennaElement;
}

uint32_t PhasedArrayModel::GetId() const { return m_id; }

} // namespace ns3
