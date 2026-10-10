
#ifndef IPV6_FLOW_PROBE_H
#define IPV6_FLOW_PROBE_H

#include "flow-probe.h"
#include "ipv6-flow-classifier.h"

#include "ns3/ipv6-l3-protocol.h"
#include "ns3/queue-item.h"

namespace ns3 {

class FlowMonitor;
class Node;

class Ipv6FlowProbe : public FlowProbe {
public:
  Ipv6FlowProbe(Ptr<FlowMonitor> monitor, Ptr<Ipv6FlowClassifier> classifier,
                Ptr<Node> node);
  ~Ipv6FlowProbe() override;

  static TypeId GetTypeId();

  enum DropReason {
    DROP_NO_ROUTE = 0,

    DROP_TTL_EXPIRE,

    DROP_BAD_CHECKSUM,

    DROP_QUEUE,

    DROP_QUEUE_DISC,

    DROP_INTERFACE_DOWN,
    DROP_ROUTE_ERROR,

    DROP_UNKNOWN_PROTOCOL,
    DROP_UNKNOWN_OPTION,
    DROP_MALFORMED_HEADER,

    DROP_FRAGMENT_TIMEOUT,

    DROP_INVALID_REASON,
  };

protected:
  void DoDispose() override;

private:
  void SendOutgoingLogger(const Ipv6Header &ipHeader,
                          Ptr<const Packet> ipPayload, uint32_t interface);
  void ForwardLogger(const Ipv6Header &ipHeader, Ptr<const Packet> ipPayload,
                     uint32_t interface);
  void ForwardUpLogger(const Ipv6Header &ipHeader, Ptr<const Packet> ipPayload,
                       uint32_t interface);
  void DropLogger(const Ipv6Header &ipHeader, Ptr<const Packet> ipPayload,
                  Ipv6L3Protocol::DropReason reason, Ptr<Ipv6> ipv6,
                  uint32_t ifIndex);
  void QueueDropLogger(Ptr<const Packet> ipPayload);
  void QueueDiscDropLogger(Ptr<const QueueDiscItem> item);

  Ptr<Ipv6FlowClassifier> m_classifier;
};

} // namespace ns3

#endif
