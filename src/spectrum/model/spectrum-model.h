
#ifndef SPECTRUM_MODEL_H
#define SPECTRUM_MODEL_H

#include <ns3/simple-ref-count.h>

#include <vector>

namespace ns3 {

struct BandInfo {
  double fl;
  double fc;
  double fh;
};

typedef std::vector<BandInfo> Bands;

typedef uint32_t SpectrumModelUid_t;

class SpectrumModel : public SimpleRefCount<SpectrumModel> {
public:
  friend bool operator==(const SpectrumModel &lhs, const SpectrumModel &rhs);

  SpectrumModel(const std::vector<double> &centerFreqs);

  SpectrumModel(const Bands &bands);

  SpectrumModel(Bands &&bands);

  size_t GetNumBands() const;

  SpectrumModelUid_t GetUid() const;

  Bands::const_iterator Begin() const;
  Bands::const_iterator End() const;

  bool IsOrthogonal(const SpectrumModel &other) const;

private:
  Bands m_bands;
  SpectrumModelUid_t m_uid;
  static SpectrumModelUid_t m_uidCount;
};

} // namespace ns3

#endif
