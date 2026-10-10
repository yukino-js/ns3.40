
#ifndef IPV4_FLOW_PROBE_H
#define IPV4_FLOW_PROBE_H

#include "flow-probe.h"
#include "ipv4-flow-classifier.h"

#include "ns3/ipv4-l3-protocol.h"
#include "ns3/queue-item.h"

namespace ns3 {

class FlowMonitor;
class Node;

class Ipv4FlowProbe : public FlowProbe {
public:
  Ipv4FlowProbe(Ptr<FlowMonitor> monitor, Ptr<Ipv4FlowClassifier> classifier,
                Ptr<Node> node);
  ~Ipv4FlowProbe() override;

  static TypeId GetTypeId();

  enum DropReason {
    DROP_NO_ROUTE = 0,

    DROP_TTL_EXPIRE,

    DROP_BAD_CHECKSUM,

    DROP_QUEUE,

    DROP_QUEUE_DISC,

    DROP_INTERFACE_DOWN,
    DROP_ROUTE_ERROR,
    DROP_FRAGMENT_TIMEOUT,

    DROP_INVALID_REASON,
  };

protected:
  void DoDispose() override;

private:
  void SendOutgoingLogger(const Ipv4Header &ipHeader,
                          Ptr<const Packet> ipPayload, uint32_t interface);
  void ForwardLogger(const Ipv4Header &ipHeader, Ptr<const Packet> ipPayload,
                     uint32_t interface);
  void ForwardUpLogger(const Ipv4Header &ipHeader, Ptr<const Packet> ipPayload,
                       uint32_t interface);
  void DropLogger(const Ipv4Header &ipHeader, Ptr<const Packet> ipPayload,
                  Ipv4L3Protocol::DropReason reason, Ptr<Ipv4> ipv4,
                  uint32_t ifIndex);
  void QueueDropLogger(Ptr<const Packet> ipPayload);
  void QueueDiscDropLogger(Ptr<const QueueDiscItem> item);

  Ptr<Ipv4FlowClassifier> m_classifier;
  Ptr<Ipv4L3Protocol> m_ipv4;
};

} // namespace ns3

#endif
