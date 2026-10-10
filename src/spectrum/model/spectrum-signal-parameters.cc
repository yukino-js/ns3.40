
#include "spectrum-signal-parameters.h"

#include "spectrum-phy.h"
#include "spectrum-value.h"

#include <ns3/antenna-model.h>
#include <ns3/log.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("SpectrumSignalParameters");

SpectrumSignalParameters::SpectrumSignalParameters() { NS_LOG_FUNCTION(this); }

SpectrumSignalParameters::~SpectrumSignalParameters() { NS_LOG_FUNCTION(this); }

SpectrumSignalParameters::SpectrumSignalParameters(
    const SpectrumSignalParameters &p) {
  NS_LOG_FUNCTION(this << &p);
  psd = p.psd->Copy();
  duration = p.duration;
  txPhy = p.txPhy;
  txAntenna = p.txAntenna;
}

Ptr<SpectrumSignalParameters> SpectrumSignalParameters::Copy() const {
  NS_LOG_FUNCTION(this);
  return Create<SpectrumSignalParameters>(*this);
}

} // namespace ns3
