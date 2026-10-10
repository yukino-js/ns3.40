
#ifndef CONSTANT_OBSS_PD_ALGORITHM_H
#define CONSTANT_OBSS_PD_ALGORITHM_H

#include "obss-pd-algorithm.h"

namespace ns3 {

class ConstantObssPdAlgorithm : public ObssPdAlgorithm {
public:
  ConstantObssPdAlgorithm();

  static TypeId GetTypeId();

  void ConnectWifiNetDevice(const Ptr<WifiNetDevice> device) override;
  void ReceiveHeSigA(HeSigAParameters params) override;
};

} // namespace ns3

#endif
