
#ifndef SPECTRUM_PROPAGATION_LOSS_MODEL_H
#define SPECTRUM_PROPAGATION_LOSS_MODEL_H

#include "spectrum-value.h"

#include <ns3/mobility-model.h>
#include <ns3/object.h>

namespace ns3 {

struct SpectrumSignalParameters;

class SpectrumPropagationLossModel : public Object {
public:
  SpectrumPropagationLossModel();
  ~SpectrumPropagationLossModel() override;

  static TypeId GetTypeId();

  void SetNext(Ptr<SpectrumPropagationLossModel> next);

  Ptr<SpectrumValue>
  CalcRxPowerSpectralDensity(Ptr<const SpectrumSignalParameters> params,
                             Ptr<const MobilityModel> a,
                             Ptr<const MobilityModel> b) const;

protected:
  void DoDispose() override;

private:
  virtual Ptr<SpectrumValue>
  DoCalcRxPowerSpectralDensity(Ptr<const SpectrumSignalParameters> params,
                               Ptr<const MobilityModel> a,
                               Ptr<const MobilityModel> b) const = 0;

  Ptr<SpectrumPropagationLossModel> m_next;
};

} // namespace ns3

#endif
