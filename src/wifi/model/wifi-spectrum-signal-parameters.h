
#ifndef WIFI_SPECTRUM_SIGNAL_PARAMETERS_H
#define WIFI_SPECTRUM_SIGNAL_PARAMETERS_H

#include "ns3/spectrum-signal-parameters.h"

namespace ns3 {

class WifiPpdu;

struct WifiSpectrumSignalParameters : public SpectrumSignalParameters {
  Ptr<SpectrumSignalParameters> Copy() const override;

  WifiSpectrumSignalParameters();

  WifiSpectrumSignalParameters(const WifiSpectrumSignalParameters &p);

  Ptr<const WifiPpdu> ppdu;
};

} // namespace ns3

#endif
