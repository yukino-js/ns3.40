
#ifndef PHASED_ARRAY_MODEL_H
#define PHASED_ARRAY_MODEL_H

#include "angles.h"
#include "antenna-model.h"

#include <ns3/matrix-array.h>
#include <ns3/object.h>

#include <complex>

namespace ns3 {

class PhasedArrayModel : public Object {
public:
  PhasedArrayModel();

  ~PhasedArrayModel() override;

  static TypeId GetTypeId();

  using ComplexVector = ComplexMatrixArray;

  double norm(const ComplexVector &complexVector) const {
    double norm = 0;
    for (size_t i = 0; i < complexVector.GetSize(); i++) {
      norm += std::norm(complexVector[i]);
    }
    return std::sqrt(norm);
  }

  virtual std::pair<double, double> GetElementFieldPattern(Angles a) const = 0;

  virtual Vector GetElementLocation(uint64_t index) const = 0;

  virtual size_t GetNumberOfElements() const = 0;

  void SetBeamformingVector(const ComplexVector &beamformingVector);

  ComplexVector GetBeamformingVector() const;

  ComplexVector GetBeamformingVector(Angles a) const;

  ComplexVector GetSteeringVector(Angles a) const;

  void SetAntennaElement(Ptr<AntennaModel> antennaElement);

  Ptr<const AntennaModel> GetAntennaElement() const;

  uint32_t GetId() const;

protected:
  ComplexVector m_beamformingVector;
  Ptr<AntennaModel> m_antennaElement;
  bool m_isBfVectorValid;
  static uint32_t m_idCounter;
  uint32_t m_id{0};
};

std::ostream &operator<<(std::ostream &os,
                         const PhasedArrayModel::ComplexVector &cv);

} // namespace ns3

#endif
