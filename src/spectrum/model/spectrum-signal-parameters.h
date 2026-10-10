
#ifndef SPECTRUM_SIGNAL_PARAMETERS_H
#define SPECTRUM_SIGNAL_PARAMETERS_H

#include <ns3/nstime.h>
#include <ns3/ptr.h>
#include <ns3/simple-ref-count.h>

namespace ns3 {

class SpectrumPhy;
class SpectrumValue;
class AntennaModel;

struct SpectrumSignalParameters
    : public SimpleRefCount<SpectrumSignalParameters> {
  SpectrumSignalParameters();

  virtual ~SpectrumSignalParameters();

  SpectrumSignalParameters(const SpectrumSignalParameters &p);

  virtual Ptr<SpectrumSignalParameters> Copy() const;

  Ptr<SpectrumValue> psd;

  Time duration;

  Ptr<SpectrumPhy> txPhy;

  Ptr<AntennaModel> txAntenna;
};

} // namespace ns3

#endif
