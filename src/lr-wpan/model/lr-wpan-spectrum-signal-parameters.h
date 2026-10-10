
#ifndef LR_WPAN_SPECTRUM_SIGNAL_PARAMETERS_H
#define LR_WPAN_SPECTRUM_SIGNAL_PARAMETERS_H

#include <ns3/spectrum-signal-parameters.h>

namespace ns3 {

class PacketBurst;

struct LrWpanSpectrumSignalParameters : public SpectrumSignalParameters {
  Ptr<SpectrumSignalParameters> Copy() const override;

  LrWpanSpectrumSignalParameters();

  LrWpanSpectrumSignalParameters(const LrWpanSpectrumSignalParameters &p);

  Ptr<PacketBurst> packetBurst;
};

} // namespace ns3

#endif
