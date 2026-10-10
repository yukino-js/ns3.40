
#ifndef AMCMODULE_H
#define AMCMODULE_H

#include <ns3/object.h>
#include <ns3/ptr.h>

#include <vector>

namespace ns3 {

class SpectrumValue;

class LteAmc : public Object {
public:
  static TypeId GetTypeId();

  LteAmc();
  ~LteAmc() override;

  enum AmcModel { PiroEW2010, MiErrorModel };

  int GetMcsFromCqi(int cqi);

  int GetDlTbSizeFromMcs(int mcs, int nprb);

  int GetUlTbSizeFromMcs(int mcs, int nprb);

  double GetSpectralEfficiencyFromCqi(int cqi);

  std::vector<int> CreateCqiFeedbacks(const SpectrumValue &sinr,
                                      uint8_t rbgSize = 0);

  int GetCqiFromSpectralEfficiency(double s);

private:
  double m_ber;

  AmcModel m_amcModel;
};

} // namespace ns3

#endif
