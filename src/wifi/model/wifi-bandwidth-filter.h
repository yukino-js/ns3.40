
#ifndef WIFI_BANDWIDTH_FILTER_H
#define WIFI_BANDWIDTH_FILTER_H

#include <ns3/spectrum-transmit-filter.h>

namespace ns3 {

class WifiBandwidthFilter : public SpectrumTransmitFilter {
public:
  WifiBandwidthFilter();

  static TypeId GetTypeId();

  bool DoFilter(Ptr<const SpectrumSignalParameters> params,
                Ptr<const SpectrumPhy> receiverPhy) override;
};

} // namespace ns3

#endif
