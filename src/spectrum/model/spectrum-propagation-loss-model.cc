
#include "spectrum-propagation-loss-model.h"

#include "spectrum-signal-parameters.h"

#include <ns3/log.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("SpectrumPropagationLossModel");

NS_OBJECT_ENSURE_REGISTERED(SpectrumPropagationLossModel);

SpectrumPropagationLossModel::SpectrumPropagationLossModel()
    : m_next(nullptr) {}

SpectrumPropagationLossModel::~SpectrumPropagationLossModel() {}

void SpectrumPropagationLossModel::DoDispose() { m_next = nullptr; }

TypeId SpectrumPropagationLossModel::GetTypeId() {
  static TypeId tid = TypeId("ns3::SpectrumPropagationLossModel")
                          .SetParent<Object>()
                          .SetGroupName("Spectrum");
  return tid;
}

void SpectrumPropagationLossModel::SetNext(
    Ptr<SpectrumPropagationLossModel> next) {
  m_next = next;
}

Ptr<SpectrumValue> SpectrumPropagationLossModel::CalcRxPowerSpectralDensity(
    Ptr<const SpectrumSignalParameters> params, Ptr<const MobilityModel> a,
    Ptr<const MobilityModel> b) const {
  Ptr<SpectrumValue> rxPsd = DoCalcRxPowerSpectralDensity(params, a, b);
  if (m_next) {
    rxPsd = m_next->CalcRxPowerSpectralDensity(params, a, b);
  }
  return rxPsd;
}

} // namespace ns3
