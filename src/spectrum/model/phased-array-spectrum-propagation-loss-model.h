
#ifndef PHASED_ARRAY_SPECTRUM_PROPAGATION_LOSS_MODEL_H
#define PHASED_ARRAY_SPECTRUM_PROPAGATION_LOSS_MODEL_H

#include "spectrum-value.h"

#include <ns3/mobility-model.h>
#include <ns3/object.h>
#include <ns3/phased-array-model.h>

namespace ns3 {

struct SpectrumSignalParameters;

class PhasedArraySpectrumPropagationLossModel : public Object {
public:
  PhasedArraySpectrumPropagationLossModel();
  ~PhasedArraySpectrumPropagationLossModel() override;

  static TypeId GetTypeId();

  void SetNext(Ptr<PhasedArraySpectrumPropagationLossModel> next);

  Ptr<SpectrumValue> CalcRxPowerSpectralDensity(
      Ptr<const SpectrumSignalParameters> txPsd, Ptr<const MobilityModel> a,
      Ptr<const MobilityModel> b, Ptr<const PhasedArrayModel> aPhasedArrayModel,
      Ptr<const PhasedArrayModel> bPhasedArrayModel) const;

protected:
  void DoDispose() override;

private:
  virtual Ptr<SpectrumValue> DoCalcRxPowerSpectralDensity(
      Ptr<const SpectrumSignalParameters> params, Ptr<const MobilityModel> a,
      Ptr<const MobilityModel> b, Ptr<const PhasedArrayModel> aPhasedArrayModel,
      Ptr<const PhasedArrayModel> bPhasedArrayModel) const = 0;

  Ptr<PhasedArraySpectrumPropagationLossModel> m_next;
};

} // namespace ns3

#endif
