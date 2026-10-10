
#include "phased-array-spectrum-propagation-loss-model.h"

#include "spectrum-signal-parameters.h"

#include <ns3/log.h>
#include <ns3/phased-array-model.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("PhasedArraySpectrumPropagationLossModel");

NS_OBJECT_ENSURE_REGISTERED(PhasedArraySpectrumPropagationLossModel);

PhasedArraySpectrumPropagationLossModel::
    PhasedArraySpectrumPropagationLossModel()
    : m_next(nullptr) {}

PhasedArraySpectrumPropagationLossModel::
    ~PhasedArraySpectrumPropagationLossModel() {}

void PhasedArraySpectrumPropagationLossModel::DoDispose() { m_next = nullptr; }

TypeId PhasedArraySpectrumPropagationLossModel::GetTypeId() {
  static TypeId tid = TypeId("ns3::PhasedArraySpectrumPropagationLossModel")
                          .SetParent<Object>()
                          .SetGroupName("Spectrum");
  return tid;
}

void PhasedArraySpectrumPropagationLossModel::SetNext(
    Ptr<PhasedArraySpectrumPropagationLossModel> next) {
  m_next = next;
}

Ptr<SpectrumValue>
PhasedArraySpectrumPropagationLossModel::CalcRxPowerSpectralDensity(
    Ptr<const SpectrumSignalParameters> params, Ptr<const MobilityModel> a,
    Ptr<const MobilityModel> b, Ptr<const PhasedArrayModel> aPhasedArrayModel,
    Ptr<const PhasedArrayModel> bPhasedArrayModel) const {
  Ptr<SpectrumValue> rxPsd = DoCalcRxPowerSpectralDensity(
      params, a, b, aPhasedArrayModel, bPhasedArrayModel);
  if (m_next) {
    rxPsd = m_next->CalcRxPowerSpectralDensity(params, a, b, aPhasedArrayModel,
                                               bPhasedArrayModel);
  }
  return rxPsd;
}

} // namespace ns3
