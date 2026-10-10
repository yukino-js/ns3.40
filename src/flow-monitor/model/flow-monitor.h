
#ifndef FLOW_MONITOR_H
#define FLOW_MONITOR_H

#include "flow-classifier.h"
#include "flow-probe.h"

#include "ns3/event-id.h"
#include "ns3/histogram.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/ptr.h"

#include <atomic>
#include <map>
#include <vector>

namespace ns3 {

class FlowMonitor : public Object {
public:
  struct FlowStats {
    Time timeFirstTxPacket;

    Time timeFirstRxPacket;

    Time timeLastTxPacket;

    Time timeLastRxPacket;

    Time delaySum;

    Time jitterSum;

    Time lastDelay;

    uint64_t txBytes;
    uint64_t rxBytes;
    uint32_t txPackets;
    uint32_t rxPackets;

    uint32_t lostPackets;

    uint32_t timesForwarded;

    Histogram delayHistogram;
    Histogram jitterHistogram;
    Histogram packetSizeHistogram;

    std::vector<uint32_t> packetsDropped;

    std::vector<uint64_t> bytesDropped;
    Histogram flowInterruptionsHistogram;
  };

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  FlowMonitor();

  void AddFlowClassifier(Ptr<FlowClassifier> classifier);

  void Start(const Time &time);
  void Stop(const Time &time);
  void StartRightNow();
  void StopRightNow();

  void AddProbe(Ptr<FlowProbe> probe);

  void ReportFirstTx(Ptr<FlowProbe> probe, FlowId flowId, FlowPacketId packetId,
                     uint32_t packetSize);
  void ReportForwarding(Ptr<FlowProbe> probe, FlowId flowId,
                        FlowPacketId packetId, uint32_t packetSize);
  void ReportLastRx(Ptr<FlowProbe> probe, FlowId flowId, FlowPacketId packetId,
                    uint32_t packetSize);
  void ReportDrop(Ptr<FlowProbe> probe, FlowId flowId, FlowPacketId packetId,
                  uint32_t packetSize, uint32_t reasonCode);

  void CheckForLostPackets();

  void CheckForLostPackets(Time maxDelay);

  typedef std::map<FlowId, FlowStats> FlowStatsContainer;
  typedef std::map<FlowId, FlowStats>::iterator FlowStatsContainerI;
  typedef std::map<FlowId, FlowStats>::const_iterator FlowStatsContainerCI;
  typedef std::vector<Ptr<FlowProbe>> FlowProbeContainer;
  typedef std::vector<Ptr<FlowProbe>>::iterator FlowProbeContainerI;
  typedef std::vector<Ptr<FlowProbe>>::const_iterator FlowProbeContainerCI;

  const FlowStatsContainer &GetFlowStats() const;

  const FlowProbeContainer &GetAllProbes() const;

  void SerializeToXmlStream(std::ostream &os, uint16_t indent,
                            bool enableHistograms, bool enableProbes);

  std::string SerializeToXmlString(uint16_t indent, bool enableHistograms,
                                   bool enableProbes);

  void SerializeToXmlFile(std::string fileName, bool enableHistograms,
                          bool enableProbes);

  void ResetAllStats();

protected:
  void NotifyConstructionCompleted() override;
  void DoDispose() override;

private:
  struct TrackedPacket {
    Time firstSeenTime;
    Time lastSeenTime;
    uint32_t timesForwarded;
  };

  FlowStatsContainer m_flowStats;

  typedef std::map<std::pair<FlowId, FlowPacketId>, TrackedPacket>
      TrackedPacketMap;
  TrackedPacketMap m_trackedPackets;
  Time m_maxPerHopDelay;
  FlowProbeContainer m_flowProbes;

  std::list<Ptr<FlowClassifier>> m_classifiers;

  EventId m_startEvent;
  EventId m_stopEvent;
  bool m_enabled;
  double m_delayBinWidth;
  double m_jitterBinWidth;
  double m_packetSizeBinWidth;
  double m_flowInterruptionsBinWidth;
  Time m_flowInterruptionsMinTime;
#ifdef NS3_MTP
  std::atomic<bool> m_lock;
#endif

  FlowStats &GetStatsForFlow(FlowId flowId);

  void PeriodicCheckForLostPackets();
};

} // namespace ns3

#endif
