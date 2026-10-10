
#ifndef CONSTANT_SPECTRUM_PROPAGATION_LOSS_H
#define CONSTANT_SPECTRUM_PROPAGATION_LOSS_H

#include "spectrum-propagation-loss-model.h"

namespace ns3 {

class ConstantSpectrumPropagationLossModel
    : public SpectrumPropagationLossModel {
public:
  ConstantSpectrumPropagationLossModel();
  ~ConstantSpectrumPropagationLossModel() override;

  static TypeId GetTypeId();

  Ptr<SpectrumValue>
  DoCalcRxPowerSpectralDensity(Ptr<const SpectrumSignalParameters> params,
                               Ptr<const MobilityModel> a,
                               Ptr<const MobilityModel> b) const override;
  void SetLossDb(double lossDb);
  double GetLossDb() const;

protected:
  double m_lossDb;
  double m_lossLinear;

private:
};

} // namespace ns3

#endif
