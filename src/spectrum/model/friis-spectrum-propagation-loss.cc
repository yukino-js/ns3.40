
#include "friis-spectrum-propagation-loss.h"

#include "spectrum-signal-parameters.h"

#include <ns3/mobility-model.h>

#include <cmath>

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(FriisSpectrumPropagationLossModel);

FriisSpectrumPropagationLossModel::FriisSpectrumPropagationLossModel() {}

FriisSpectrumPropagationLossModel::~FriisSpectrumPropagationLossModel() {}

TypeId FriisSpectrumPropagationLossModel::GetTypeId() {
  static TypeId tid = TypeId("ns3::FriisSpectrumPropagationLossModel")
                          .SetParent<SpectrumPropagationLossModel>()
                          .SetGroupName("Spectrum")
                          .AddConstructor<FriisSpectrumPropagationLossModel>();
  return tid;
}

Ptr<SpectrumValue>
FriisSpectrumPropagationLossModel::DoCalcRxPowerSpectralDensity(
    Ptr<const SpectrumSignalParameters> params, Ptr<const MobilityModel> a,
    Ptr<const MobilityModel> b) const {
  Ptr<SpectrumValue> rxPsd = Copy<SpectrumValue>(params->psd);
  auto vit = rxPsd->ValuesBegin();
  auto fit = rxPsd->ConstBandsBegin();

  NS_ASSERT(a);
  NS_ASSERT(b);

  double d = a->GetDistanceFrom(b);

  while (vit != rxPsd->ValuesEnd()) {
    NS_ASSERT(fit != rxPsd->ConstBandsEnd());
    *vit /= CalculateLoss(fit->fc, d);
    ++vit;
    ++fit;
  }
  return rxPsd;
}

double FriisSpectrumPropagationLossModel::CalculateLoss(double f,
                                                        double d) const {
  NS_ASSERT(d >= 0);

  if (d == 0) {
    return 1;
  }

  NS_ASSERT(f > 0);
  double loss_sqrt = (4 * M_PI * f * d) / 3e8;
  double loss = loss_sqrt * loss_sqrt;

  if (loss < 1) {
    loss = 1;
  }
  return loss;
}

} // namespace ns3
