
#ifndef FRIIS_SPECTRUM_PROPAGATION_LOSS_H
#define FRIIS_SPECTRUM_PROPAGATION_LOSS_H

#include "spectrum-propagation-loss-model.h"

namespace ns3 {

class MobilityModel;

class FriisSpectrumPropagationLossModel : public SpectrumPropagationLossModel {
public:
  FriisSpectrumPropagationLossModel();
  ~FriisSpectrumPropagationLossModel() override;

  static TypeId GetTypeId();

  Ptr<SpectrumValue>
  DoCalcRxPowerSpectralDensity(Ptr<const SpectrumSignalParameters> params,
                               Ptr<const MobilityModel> a,
                               Ptr<const MobilityModel> b) const override;

  double CalculateLoss(double f, double d) const;
};

} // namespace ns3

#endif
