
#ifndef IPCS_CLASSIFIER_H
#define IPCS_CLASSIFIER_H

#include "ss-service-flow-manager.h"

#include "ns3/packet.h"
#include "ns3/ptr.h"

#include <stdint.h>
#include <vector>

namespace ns3 {
class SsServiceFlowManager;

class IpcsClassifier : public Object {
public:
  static TypeId GetTypeId();
  IpcsClassifier();
  ~IpcsClassifier() override;
  ServiceFlow *Classify(Ptr<const Packet> packet, Ptr<ServiceFlowManager> sfm,
                        ServiceFlow::Direction dir);
};
} // namespace ns3

#endif
