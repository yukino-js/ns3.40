
#ifndef IPV6_FLOW_CLASSIFIER_H
#define IPV6_FLOW_CLASSIFIER_H

#include "flow-classifier.h"

#include "ns3/ipv6-header.h"

#include <atomic>
#include <map>
#include <stdint.h>

namespace ns3 {

class Packet;

class Ipv6FlowClassifier : public FlowClassifier {
public:
  struct FiveTuple {
    Ipv6Address sourceAddress;
    Ipv6Address destinationAddress;
    uint8_t protocol;
    uint16_t sourcePort;
    uint16_t destinationPort;
  };

  Ipv6FlowClassifier();

  bool Classify(const Ipv6Header &ipHeader, Ptr<const Packet> ipPayload,
                uint32_t *out_flowId, uint32_t *out_packetId);

  FiveTuple FindFlow(FlowId flowId) const;

  class SortByCount {
  public:
    bool operator()(std::pair<Ipv6Header::DscpType, uint32_t> left,
                    std::pair<Ipv6Header::DscpType, uint32_t> right);
  };

  std::vector<std::pair<Ipv6Header::DscpType, uint32_t>>
  GetDscpCounts(FlowId flowId) const;

  void SerializeToXmlStream(std::ostream &os, uint16_t indent) const override;

private:
  std::map<FiveTuple, FlowId> m_flowMap;
  std::map<FlowId, FlowPacketId> m_flowPktIdMap;
  std::map<FlowId, std::map<Ipv6Header::DscpType, uint32_t>> m_flowDscpMap;

#ifdef NS3_MTP
  std::atomic<bool> m_lock;
#endif
};

bool operator<(const Ipv6FlowClassifier::FiveTuple &t1,
               const Ipv6FlowClassifier::FiveTuple &t2);

bool operator==(const Ipv6FlowClassifier::FiveTuple &t1,
                const Ipv6FlowClassifier::FiveTuple &t2);

} // namespace ns3

#endif
