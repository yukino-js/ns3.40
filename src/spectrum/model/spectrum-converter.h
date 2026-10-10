
#ifndef SPECTRUM_CONVERTER_H
#define SPECTRUM_CONVERTER_H

#include "spectrum-value.h"

namespace ns3 {

class SpectrumConverter : public SimpleRefCount<SpectrumConverter> {
public:
  SpectrumConverter(Ptr<const SpectrumModel> fromSpectrumModel,
                    Ptr<const SpectrumModel> toSpectrumModel);

  SpectrumConverter();

  Ptr<SpectrumValue> Convert(Ptr<const SpectrumValue> vvf) const;

private:
  double GetCoefficient(const BandInfo &from, const BandInfo &to) const;

  std::vector<double> m_conversionMatrix;
  std::vector<size_t> m_conversionRowPtr;
  std::vector<size_t> m_conversionColInd;

  Ptr<const SpectrumModel> m_fromSpectrumModel;
  Ptr<const SpectrumModel> m_toSpectrumModel;
};

} // namespace ns3

#endif
