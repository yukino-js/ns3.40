
#ifndef HWMP_PROTOCOL_H
#define HWMP_PROTOCOL_H

#include "ns3/event-id.h"
#include "ns3/mesh-l2-routing-protocol.h"
#include "ns3/nstime.h"
#include "ns3/traced-value.h"

#include <map>
#include <vector>

namespace ns3 {
class MeshPointDevice;
class Packet;
class Mac48Address;
class UniformRandomVariable;
class RandomVariableStream;

namespace dot11s {
class HwmpProtocolMac;
class HwmpRtable;
class IePerr;
class IePreq;
class IePrep;

struct RouteChange {
  std::string type;
  Mac48Address destination;
  Mac48Address retransmitter;
  uint32_t interface;
  uint32_t metric;
  Time lifetime;
  uint32_t seqnum;
};

class HwmpProtocol : public MeshL2RoutingProtocol {
public:
  static TypeId GetTypeId();

  HwmpProtocol();
  ~HwmpProtocol() override;

  HwmpProtocol(const HwmpProtocol &) = delete;
  HwmpProtocol &operator=(const HwmpProtocol &) = delete;

  void DoDispose() override;

  struct FailedDestination {
    Mac48Address destination;
    uint32_t seqnum;
  };

  bool RequestRoute(uint32_t sourceIface, const Mac48Address source,
                    const Mac48Address destination, Ptr<const Packet> packet,
                    uint16_t protocolType,
                    RouteReplyCallback routeReply) override;
  bool RemoveRoutingStuff(uint32_t fromIface, const Mac48Address source,
                          const Mac48Address destination, Ptr<Packet> packet,
                          uint16_t &protocolType) override;
  bool Install(Ptr<MeshPointDevice> mp);
  void PeerLinkStatus(Mac48Address meshPointAddress, Mac48Address peerAddress,
                      uint32_t interface, bool status);
  void SetNeighboursCallback(Callback<std::vector<Mac48Address>, uint32_t> cb);
  void SetRoot();
  void UnsetRoot();

  void Report(std::ostream &os) const;
  void ResetStats();

  int64_t AssignStreams(int64_t stream);

  Ptr<HwmpRtable> GetRoutingTable() const;

private:
  friend class HwmpProtocolMac;

  void DoInitialize() override;

  struct PathError {
    std::vector<FailedDestination> destinations;
    std::vector<std::pair<uint32_t, Mac48Address>> receivers;
  };

  struct QueuedPacket {
    Ptr<Packet> pkt;
    Mac48Address src;
    Mac48Address dst;
    uint16_t protocol;
    uint32_t inInterface;
    RouteReplyCallback reply;

    QueuedPacket();
  };

  typedef std::map<uint32_t, Ptr<HwmpProtocolMac>> HwmpProtocolMacMap;
  bool ForwardUnicast(uint32_t sourceIface, const Mac48Address source,
                      const Mac48Address destination, Ptr<Packet> packet,
                      uint16_t protocolType, RouteReplyCallback routeReply,
                      uint32_t ttl);

  void ReceivePreq(IePreq preq, Mac48Address from, uint32_t interface,
                   Mac48Address fromMp, uint32_t metric);
  void ReceivePrep(IePrep prep, Mac48Address from, uint32_t interface,
                   Mac48Address fromMp, uint32_t metric);
  void ReceivePerr(std::vector<FailedDestination> destinations,
                   Mac48Address from, uint32_t interface, Mac48Address fromMp);
  void SendPrep(Mac48Address src, Mac48Address dst, Mac48Address retransmitter,
                uint32_t initMetric, uint32_t originatorDsn,
                uint32_t destinationSN, uint32_t lifetime, uint32_t interface);
  PathError MakePathError(std::vector<FailedDestination> destinations);
  void ForwardPathError(PathError perr);
  void InitiatePathError(PathError perr);
  std::vector<std::pair<uint32_t, Mac48Address>>
  GetPerrReceivers(std::vector<FailedDestination> failedDest);

  std::vector<Mac48Address> GetPreqReceivers(uint32_t interface);
  std::vector<Mac48Address> GetBroadcastReceivers(uint32_t interface);
  bool DropDataFrame(uint32_t seqno, Mac48Address source);

  TracedCallback<Time> m_routeDiscoveryTimeCallback;
  typedef TracedCallback<RouteChange> RouteChangeTracedCallback;
  TracedCallback<RouteChange> m_routeChangeTraceSource;

  bool QueuePacket(QueuedPacket packet);
  QueuedPacket DequeueFirstPacketByDst(Mac48Address dst);
  QueuedPacket DequeueFirstPacket();
  void ReactivePathResolved(Mac48Address dst);
  void ProactivePathResolved();

  bool ShouldSendPreq(Mac48Address dst);

  void RetryPathDiscovery(Mac48Address dst, uint8_t numOfRetry);
  void SendProactivePreq();

  Mac48Address GetAddress();
  bool GetDoFlag() const;
  bool GetRfFlag() const;
  Time GetPreqMinInterval();
  Time GetPerrMinInterval();
  uint8_t GetMaxTtl() const;
  uint32_t GetNextPreqId();
  uint32_t GetNextHwmpSeqno();
  uint32_t GetActivePathLifetime();
  uint8_t GetUnicastPerrThreshold() const;

private:
  struct Statistics {
    uint16_t txUnicast;
    uint16_t txBroadcast;
    uint32_t txBytes;
    uint16_t droppedTtl;
    uint16_t totalQueued;
    uint16_t totalDropped;
    uint16_t initiatedPreq;
    uint16_t initiatedPrep;
    uint16_t initiatedPerr;

    void Print(std::ostream &os) const;
    Statistics();
  };

  Statistics m_stats;

  HwmpProtocolMacMap m_interfaces;
  Mac48Address m_address;
  uint32_t m_dataSeqno;
  uint32_t m_hwmpSeqno;
  uint32_t m_preqId;
  std::map<Mac48Address, uint32_t> m_lastDataSeqno;
  std::map<Mac48Address, std::pair<uint32_t, uint32_t>>
      m_hwmpSeqnoMetricDatabase;

  Ptr<HwmpRtable> m_rtable;

  struct PreqEvent {
    EventId preqTimeout;
    Time whenScheduled;
  };

  std::map<Mac48Address, PreqEvent> m_preqTimeouts;
  EventId m_proactivePreqTimer;
  Time m_randomStart;
  std::vector<QueuedPacket> m_rqueue;

  uint16_t m_maxQueueSize;
  uint8_t m_dot11MeshHWMPmaxPREQretries;
  Time m_dot11MeshHWMPnetDiameterTraversalTime;
  Time m_dot11MeshHWMPpreqMinInterval;
  Time m_dot11MeshHWMPperrMinInterval;
  Time m_dot11MeshHWMPactiveRootTimeout;
  Time m_dot11MeshHWMPactivePathTimeout;
  Time m_dot11MeshHWMPpathToRootInterval;
  Time m_dot11MeshHWMPrannInterval;
  bool m_isRoot;
  uint8_t m_maxTtl;
  uint8_t m_unicastPerrThreshold;
  uint8_t m_unicastPreqThreshold;
  uint8_t m_unicastDataThreshold;
  bool m_doFlag;
  bool m_rfFlag;

  Ptr<UniformRandomVariable> m_coefficient;
  Callback<std::vector<Mac48Address>, uint32_t> m_neighboursCallback;
};
} // namespace dot11s
} // namespace ns3
#endif
