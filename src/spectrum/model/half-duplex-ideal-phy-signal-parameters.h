
#ifndef HALF_DUPLEX_IDEAL_PHY_SPECTRUM_PARAMETERS_H
#define HALF_DUPLEX_IDEAL_PHY_SPECTRUM_PARAMETERS_H

#include "spectrum-signal-parameters.h"

namespace ns3 {

class Packet;

struct HalfDuplexIdealPhySignalParameters : public SpectrumSignalParameters {
  Ptr<SpectrumSignalParameters> Copy() const override;

  HalfDuplexIdealPhySignalParameters();

  HalfDuplexIdealPhySignalParameters(
      const HalfDuplexIdealPhySignalParameters &p);

  Ptr<Packet> data;
};

} // namespace ns3

#endif
