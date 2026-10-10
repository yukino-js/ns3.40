
#ifndef FLOW_PROBE_H
#define FLOW_PROBE_H

#include "flow-classifier.h"

#include "ns3/nstime.h"
#include "ns3/object.h"

#include <map>
#include <vector>

namespace ns3 {

class FlowMonitor;

class FlowProbe : public Object {
protected:
  FlowProbe(Ptr<FlowMonitor> flowMonitor);
  void DoDispose() override;

public:
  ~FlowProbe() override;

  FlowProbe(const FlowProbe &) = delete;
  FlowProbe &operator=(const FlowProbe &) = delete;

  static TypeId GetTypeId();

  struct FlowStats {
    FlowStats() : delayFromFirstProbeSum(Seconds(0)), bytes(0), packets(0) {}

    std::vector<uint32_t> packetsDropped;
    std::vector<uint64_t> bytesDropped;
    Time delayFromFirstProbeSum;
    uint64_t bytes;
    uint32_t packets;
  };

  typedef std::map<FlowId, FlowStats> Stats;

  void AddPacketStats(FlowId flowId, uint32_t packetSize,
                      Time delayFromFirstProbe);
  void AddPacketDropStats(FlowId flowId, uint32_t packetSize,
                          uint32_t reasonCode);

  Stats GetStats() const;

  void SerializeToXmlStream(std::ostream &os, uint16_t indent,
                            uint32_t index) const;

protected:
  Ptr<FlowMonitor> m_flowMonitor;
  Stats m_stats;
};

} // namespace ns3

#endif
