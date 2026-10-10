
#include "wifi-spectrum-signal-parameters.h"

#include "wifi-ppdu.h"

#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("WifiSpectrumSignalParameters");

WifiSpectrumSignalParameters::WifiSpectrumSignalParameters()
    : SpectrumSignalParameters(), ppdu(nullptr) {
  NS_LOG_FUNCTION(this);
}

WifiSpectrumSignalParameters::WifiSpectrumSignalParameters(
    const WifiSpectrumSignalParameters &p)
    : SpectrumSignalParameters(p), ppdu(p.ppdu) {
  NS_LOG_FUNCTION(this << &p);
}

Ptr<SpectrumSignalParameters> WifiSpectrumSignalParameters::Copy() const {
  NS_LOG_FUNCTION(this);
  Ptr<WifiSpectrumSignalParameters> wssp(
      new WifiSpectrumSignalParameters(*this), false);
  return wssp;
}

} // namespace ns3
