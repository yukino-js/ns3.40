
#ifndef UNIFORM_PLANAR_ARRAY_H
#define UNIFORM_PLANAR_ARRAY_H

#include "phased-array-model.h"

#include <ns3/object.h>

namespace ns3 {

class UniformPlanarArray : public PhasedArrayModel {
public:
  UniformPlanarArray();

  ~UniformPlanarArray() override;

  static TypeId GetTypeId();

  std::pair<double, double> GetElementFieldPattern(Angles a) const override;

  Vector GetElementLocation(uint64_t index) const override;

  size_t GetNumberOfElements() const override;

private:
  void SetNumColumns(uint32_t n);

  uint32_t GetNumColumns() const;

  void SetNumRows(uint32_t n);

  uint32_t GetNumRows() const;

  void SetAlpha(double alpha);

  void SetBeta(double beta);

  void SetPolSlant(double polSlant);

  void SetAntennaHorizontalSpacing(double s);

  double GetAntennaHorizontalSpacing() const;

  void SetAntennaVerticalSpacing(double s);

  double GetAntennaVerticalSpacing() const;

  uint32_t m_numColumns{1};
  uint32_t m_numRows{1};
  double m_disV{0.5};
  double m_disH{0.5};
  double m_alpha{0.0};
  double m_cosAlpha{1.0};
  double m_sinAlpha{0.0};
  double m_beta{0.0};
  double m_cosBeta{1.0};
  double m_sinBeta{0.0};
  double m_polSlant{0.0};
  double m_cosPolSlant{1.0};
  double m_sinPolSlant{0.0};
};

} // namespace ns3

#endif
