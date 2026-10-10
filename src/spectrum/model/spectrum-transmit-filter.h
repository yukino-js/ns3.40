
#ifndef SPECTRUM_TRANSMIT_FILTER_H
#define SPECTRUM_TRANSMIT_FILTER_H

#include <ns3/object.h>

namespace ns3 {

struct SpectrumSignalParameters;
class SpectrumPhy;

class SpectrumTransmitFilter : public Object {
public:
  SpectrumTransmitFilter();

  static TypeId GetTypeId();

  void SetNext(Ptr<SpectrumTransmitFilter> next);

  Ptr<const SpectrumTransmitFilter> GetNext() const;

  bool Filter(Ptr<const SpectrumSignalParameters> params,
              Ptr<const SpectrumPhy> receiverPhy);

protected:
  void DoDispose() override;

private:
  virtual bool DoFilter(Ptr<const SpectrumSignalParameters> params,
                        Ptr<const SpectrumPhy> receiverPhy) = 0;

  Ptr<SpectrumTransmitFilter> m_next{nullptr};
};

} // namespace ns3

#endif
