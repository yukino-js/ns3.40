

#ifndef ANIMATION_INTERFACE__H
#define ANIMATION_INTERFACE__H

#include "ns3/config.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/ipv4.h"
#include "ns3/log.h"
#include "ns3/lte-enb-net-device.h"
#include "ns3/lte-ue-net-device.h"
#include "ns3/mac48-address.h"
#include "ns3/net-device.h"
#include "ns3/node-container.h"
#include "ns3/node-list.h"
#include "ns3/nstime.h"
#include "ns3/ptr.h"
#include "ns3/random-variable-stream.h"
#include "ns3/rectangle.h"
#include "ns3/simulator.h"
#include "ns3/uan-phy-gen.h"
#include "ns3/wifi-phy.h"

#include <cstdio>
#include <map>
#include <string>

namespace ns3 {

#define MAX_PKTS_PER_TRACE_FILE 100000
#define PURGE_INTERVAL 5
#define NETANIM_VERSION "netanim-3.109"
#define CHECK_STARTED_INTIMEWINDOW                                             \
  {                                                                            \
    if (!m_started || !IsInTimeWindow()) {                                     \
      return;                                                                  \
    }                                                                          \
  }
#define CHECK_STARTED_INTIMEWINDOW_TRACKPACKETS                                \
  {                                                                            \
    if (!m_started || !IsInTimeWindow() || !m_trackPackets) {                  \
      return;                                                                  \
    }                                                                          \
  }

struct NodeSize;
class WifiPsdu;

class AnimationInterface {
public:
  AnimationInterface(const std::string filename);

  enum CounterType { UINT32_COUNTER, DOUBLE_COUNTER };

  typedef void (*AnimWriteCallback)(const char *str);

  ~AnimationInterface();

  void EnableIpv4L3ProtocolCounters(Time startTime, Time stopTime,
                                    Time pollInterval = Seconds(1));

  void EnableQueueCounters(Time startTime, Time stopTime,
                           Time pollInterval = Seconds(1));

  void EnableWifiMacCounters(Time startTime, Time stopTime,
                             Time pollInterval = Seconds(1));

  void EnableWifiPhyCounters(Time startTime, Time stopTime,
                             Time pollInterval = Seconds(1));

  AnimationInterface &EnableIpv4RouteTracking(std::string fileName,
                                              Time startTime, Time stopTime,
                                              Time pollInterval = Seconds(5));

  AnimationInterface &EnableIpv4RouteTracking(std::string fileName,
                                              Time startTime, Time stopTime,
                                              NodeContainer nc,
                                              Time pollInterval = Seconds(5));

  static bool IsInitialized();

  void SetStartTime(Time t);

  void SetStopTime(Time t);

  void SetMaxPktsPerTraceFile(uint64_t maxPktsPerFile);

  void SetMobilityPollInterval(Time t);

  void SetAnimWriteCallback(AnimWriteCallback cb);

  void ResetAnimWriteCallback();

  static void SetConstantPosition(Ptr<Node> n, double x, double y,
                                  double z = 0);

  void UpdateNodeDescription(Ptr<Node> n, std::string descr);

  void UpdateNodeDescription(uint32_t nodeId, std::string descr);

  void UpdateNodeImage(uint32_t nodeId, uint32_t resourceId);

  void UpdateNodeSize(Ptr<Node> n, double width, double height);

  void UpdateNodeSize(uint32_t nodeId, double width, double height);

  void UpdateNodeColor(Ptr<Node> n, uint8_t r, uint8_t g, uint8_t b);

  void UpdateNodeColor(uint32_t nodeId, uint8_t r, uint8_t g, uint8_t b);

  void UpdateNodeCounter(uint32_t nodeCounterId, uint32_t nodeId,
                         double counter);

  void SetBackgroundImage(std::string fileName, double x, double y,
                          double scaleX, double scaleY, double opacity);

  void UpdateLinkDescription(uint32_t fromNode, uint32_t toNode,
                             std::string linkDescription);

  void UpdateLinkDescription(Ptr<Node> fromNode, Ptr<Node> toNode,
                             std::string linkDescription);

  AnimationInterface &AddSourceDestination(uint32_t fromNodeId,
                                           std::string destinationIpv4Address);

  bool IsStarted() const;

  void SkipPacketTracing();

  void EnablePacketMetadata(bool enable = true);

  uint64_t GetTracePktCount() const;

  uint32_t AddNodeCounter(std::string counterName, CounterType counterType);

  uint32_t AddResource(std::string resourcePath);

  double GetNodeEnergyFraction(Ptr<const Node> node) const;

private:
  class AnimPacketInfo

  {
  public:
    AnimPacketInfo();
    AnimPacketInfo(const AnimPacketInfo &pInfo);
    AnimPacketInfo(Ptr<const NetDevice> tx_nd, const Time fbTx,
                   uint32_t txNodeId = 0);
    Ptr<const NetDevice> m_txnd;
    uint32_t m_txNodeId;
    double m_fbTx;
    double m_lbTx;
    double m_fbRx;
    double m_lbRx;
    Ptr<const NetDevice> m_rxnd;
    void ProcessRxBegin(Ptr<const NetDevice> nd, const double fbRx);
  };

  struct Rgb {
    uint8_t r;
    uint8_t g;
    uint8_t b;
  };

  struct P2pLinkNodeIdPair {
    uint32_t fromNode;
    uint32_t toNode;
  };

  struct LinkProperties {
    std::string fromNodeDescription;
    std::string toNodeDescription;
    std::string linkDescription;
  };

  struct LinkPairCompare {
    bool operator()(P2pLinkNodeIdPair first, P2pLinkNodeIdPair second) const {
      if (((first.fromNode == second.fromNode) &&
           (first.toNode == second.toNode)) ||
          ((first.fromNode == second.toNode) &&
           (first.toNode == second.fromNode))) {
        return false;
      }
      std::ostringstream oss1;
      oss1 << first.fromNode << first.toNode;
      std::ostringstream oss2;
      oss2 << second.fromNode << second.toNode;
      return oss1.str() < oss2.str();
    }
  };

  struct Ipv4RouteTrackElement {
    std::string destination;
    uint32_t fromNodeId;
  };

  struct Ipv4RoutePathElement {
    uint32_t nodeId;
    std::string nextHop;
  };

  enum ProtocolType { UAN, LTE, WIFI, WIMAX, CSMA, LRWPAN };

  struct NodeSize {
    double width;
    double height;
  };

  typedef std::map<P2pLinkNodeIdPair, LinkProperties, LinkPairCompare>
      LinkPropertiesMap;
  typedef std::map<uint32_t, std::string> NodeDescriptionsMap;
  typedef std::map<uint32_t, Rgb> NodeColorsMap;
  typedef std::map<uint64_t, AnimPacketInfo> AnimUidPacketInfoMap;
  typedef std::map<uint32_t, double> EnergyFractionMap;
  typedef std::vector<Ipv4RoutePathElement> Ipv4RoutePathElements;
  typedef std::multimap<uint32_t, std::string> NodeIdIpv4Map;
  typedef std::multimap<uint32_t, std::string> NodeIdIpv6Map;
  typedef std::pair<uint32_t, std::string> NodeIdIpv4Pair;
  typedef std::pair<uint32_t, std::string> NodeIdIpv6Pair;

  typedef std::map<uint32_t, uint64_t> NodeCounterMap64;

  class AnimXmlElement {
  public:
    AnimXmlElement(std::string tagName, bool emptyElement = true);
    template <typename T>
    void AddAttribute(std::string attribute, T value, bool xmlEscape = false);
    void SetText(std::string text);
    void AppendChild(AnimXmlElement e);
    std::string ToString(bool autoClose = true);

  private:
    std::string m_tagName;
    std::string m_text;
    std::vector<std::string> m_attributes;
    std::vector<std::string> m_children;
  };

  FILE *m_f;
  FILE *m_routingF;
  Time m_mobilityPollInterval;
  std::string m_outputFileName;
  uint64_t gAnimUid;
  AnimWriteCallback m_writeCallback;
  bool m_started;
  bool m_enablePacketMetadata;
  Time m_startTime;
  Time m_stopTime;
  uint64_t m_maxPktsPerFile;
  std::string m_originalFileName;
  Time m_routingStopTime;
  std::string m_routingFileName;
  Time m_routingPollInterval;
  NodeContainer m_routingNc;
  Time m_ipv4L3ProtocolCountersStopTime;
  Time m_ipv4L3ProtocolCountersPollInterval;
  Time m_queueCountersStopTime;
  Time m_queueCountersPollInterval;
  Time m_wifiMacCountersStopTime;
  Time m_wifiMacCountersPollInterval;
  Time m_wifiPhyCountersStopTime;
  Time m_wifiPhyCountersPollInterval;
  static Rectangle *userBoundary;
  bool m_trackPackets;

  uint32_t m_remainingEnergyCounterId;

  uint32_t m_ipv4L3ProtocolTxCounterId;
  uint32_t m_ipv4L3ProtocolRxCounterId;
  uint32_t m_ipv4L3ProtocolDropCounterId;

  uint32_t m_queueEnqueueCounterId;
  uint32_t m_queueDequeueCounterId;
  uint32_t m_queueDropCounterId;

  uint32_t m_wifiMacTxCounterId;
  uint32_t m_wifiMacTxDropCounterId;
  uint32_t m_wifiMacRxCounterId;
  uint32_t m_wifiMacRxDropCounterId;

  uint32_t m_wifiPhyTxDropCounterId;
  uint32_t m_wifiPhyRxDropCounterId;

  AnimUidPacketInfoMap m_pendingWifiPackets;
  AnimUidPacketInfoMap m_pendingWimaxPackets;
  AnimUidPacketInfoMap m_pendingLrWpanPackets;
  AnimUidPacketInfoMap m_pendingLtePackets;
  AnimUidPacketInfoMap m_pendingCsmaPackets;
  AnimUidPacketInfoMap m_pendingUanPackets;

  std::map<uint32_t, Vector> m_nodeLocation;
  std::map<std::string, uint32_t> m_macToNodeIdMap;
  std::map<std::string, uint32_t> m_ipv4ToNodeIdMap;
  std::map<std::string, uint32_t> m_ipv6ToNodeIdMap;
  NodeIdIpv4Map m_nodeIdIpv4Map;
  NodeIdIpv6Map m_nodeIdIpv6Map;

  NodeColorsMap m_nodeColors;
  NodeDescriptionsMap m_nodeDescriptions;
  LinkPropertiesMap m_linkProperties;
  EnergyFractionMap m_nodeEnergyFraction;
  uint64_t m_currentPktCount;
  std::vector<Ipv4RouteTrackElement> m_ipv4RouteTrackElements;
  std::map<uint32_t, NodeSize> m_nodeSizes;
  std::vector<std::string> m_resources;
  std::vector<std::string> m_nodeCounters;

  NodeCounterMap64 m_nodeIpv4Drop;
  NodeCounterMap64 m_nodeIpv4Tx;
  NodeCounterMap64 m_nodeIpv4Rx;
  NodeCounterMap64 m_nodeQueueEnqueue;
  NodeCounterMap64 m_nodeQueueDequeue;
  NodeCounterMap64 m_nodeQueueDrop;
  NodeCounterMap64 m_nodeWifiMacTx;
  NodeCounterMap64 m_nodeWifiMacTxDrop;
  NodeCounterMap64 m_nodeWifiMacRx;
  NodeCounterMap64 m_nodeWifiMacRxDrop;
  NodeCounterMap64 m_nodeWifiPhyTxDrop;
  NodeCounterMap64 m_nodeWifiPhyRxDrop;
  NodeCounterMap64 m_nodeLrWpanMacTx;
  NodeCounterMap64 m_nodeLrWpanMacTxDrop;
  NodeCounterMap64 m_nodeLrWpanMacRx;
  NodeCounterMap64 m_nodeLrWpanMacRxDrop;

  const std::vector<std::string>
  GetElementsFromContext(const std::string &context) const;
  Ptr<Node> GetNodeFromContext(const std::string &context) const;
  Ptr<NetDevice> GetNetDeviceFromContext(std::string context);

  void StartAnimation(bool restart = false);
  void SetOutputFile(const std::string &fn, bool routing = false);
  void StopAnimation(bool onlyAnimation = false);
  std::string CounterTypeToString(CounterType counterType);
  std::string GetPacketMetadata(Ptr<const Packet> p);
  void AddByteTag(uint64_t animUid, Ptr<const Packet> p);
  int WriteN(const char *data, uint32_t count, FILE *f);
  int WriteN(const std::string &st, FILE *f);
  std::string GetMacAddress(Ptr<NetDevice> nd);
  std::string GetIpv4Address(Ptr<NetDevice> nd);
  std::string GetIpv6Address(Ptr<NetDevice> nd);
  std::vector<std::string> GetIpv4Addresses(Ptr<NetDevice> nd);
  std::vector<std::string> GetIpv6Addresses(Ptr<NetDevice> nd);

  std::string GetNetAnimVersion();
  void MobilityAutoCheck();
  bool IsPacketPending(uint64_t animUid, ProtocolType protocolType);
  void PurgePendingPackets(ProtocolType protocolType);
  AnimUidPacketInfoMap *ProtocolTypeToPendingPackets(ProtocolType protocolType);
  std::string ProtocolTypeToString(ProtocolType protocolType);
  void AddPendingPacket(ProtocolType protocolType, uint64_t animUid,
                        AnimPacketInfo pktInfo);
  uint64_t GetAnimUidFromPacket(Ptr<const Packet>);
  void AddToIpv4AddressNodeIdTable(std::string ipv4Address, uint32_t nodeId);
  void AddToIpv4AddressNodeIdTable(std::vector<std::string> ipv4Addresses,
                                   uint32_t nodeId);
  void AddToIpv6AddressNodeIdTable(std::string ipv6Address, uint32_t nodeId);
  void AddToIpv6AddressNodeIdTable(std::vector<std::string> ipv6Addresses,
                                   uint32_t nodeId);
  bool IsInTimeWindow();
  void CheckMaxPktsPerTraceFile();

  void TrackWifiPhyCounters();
  void TrackWifiMacCounters();
  void TrackIpv4L3ProtocolCounters();
  void TrackQueueCounters();
  void TrackIpv4Route();
  void TrackIpv4RoutePaths();
  std::string GetIpv4RoutingTable(Ptr<Node> n);
  void RecursiveIpv4RoutePathSearch(std::string from, std::string to,
                                    Ipv4RoutePathElements &rpElements);
  void WriteRoutePath(uint32_t nodeId, std::string destination,
                      Ipv4RoutePathElements rpElements);

  void EnqueueTrace(std::string context, Ptr<const Packet>);
  void DequeueTrace(std::string context, Ptr<const Packet>);
  void QueueDropTrace(std::string context, Ptr<const Packet>);
  void Ipv4TxTrace(std::string context, Ptr<const Packet> p, Ptr<Ipv4> ipv4,
                   uint32_t interfaceIndex);
  void Ipv4RxTrace(std::string context, Ptr<const Packet> p, Ptr<Ipv4> ipv4,
                   uint32_t interfaceIndex);
  void Ipv4DropTrace(std::string context, const Ipv4Header &ipv4Header,
                     Ptr<const Packet> p, Ipv4L3Protocol::DropReason dropReason,
                     Ptr<Ipv4> ipv4, uint32_t interfaceIndex);

  void WifiMacTxTrace(std::string context, Ptr<const Packet> p);
  void WifiMacTxDropTrace(std::string context, Ptr<const Packet> p);
  void WifiMacRxTrace(std::string context, Ptr<const Packet> p);
  void WifiMacRxDropTrace(std::string context, Ptr<const Packet> p);
  void WifiPhyTxDropTrace(std::string context, Ptr<const Packet> p);
  void WifiPhyRxDropTrace(std::string context, Ptr<const Packet> p,
                          WifiPhyRxfailureReason reason);
  void LrWpanMacTxTrace(std::string context, Ptr<const Packet> p);
  void LrWpanMacTxDropTrace(std::string context, Ptr<const Packet> p);
  void LrWpanMacRxTrace(std::string context, Ptr<const Packet> p);
  void LrWpanMacRxDropTrace(std::string context, Ptr<const Packet> p);
  void DevTxTrace(std::string context, Ptr<const Packet> p, Ptr<NetDevice> tx,
                  Ptr<NetDevice> rx, Time txTime, Time rxTime);
  void WifiPhyTxBeginTrace(std::string context, WifiConstPsduMap psduMap,
                           WifiTxVector txVector, double txPowerW);
  void WifiPhyRxBeginTrace(std::string context, Ptr<const Packet> p,
                           RxPowerWattPerChannelBand rxPowersW);
  void LrWpanPhyTxBeginTrace(std::string context, Ptr<const Packet> p);
  void LrWpanPhyRxBeginTrace(std::string context, Ptr<const Packet> p);
  void WimaxTxTrace(std::string context, Ptr<const Packet> p,
                    const Mac48Address &m);
  void WimaxRxTrace(std::string context, Ptr<const Packet> p,
                    const Mac48Address &m);
  void CsmaPhyTxBeginTrace(std::string context, Ptr<const Packet> p);
  void CsmaPhyTxEndTrace(std::string context, Ptr<const Packet> p);
  void CsmaPhyRxEndTrace(std::string context, Ptr<const Packet> p);
  void CsmaMacRxTrace(std::string context, Ptr<const Packet> p);
  void LteTxTrace(std::string context, Ptr<const Packet> p,
                  const Mac48Address &m);
  void LteRxTrace(std::string context, Ptr<const Packet> p,
                  const Mac48Address &m);
  void LteSpectrumPhyTxStart(std::string context, Ptr<const PacketBurst> pb);
  void LteSpectrumPhyRxStart(std::string context, Ptr<const PacketBurst> pb);
  void UanPhyGenTxTrace(std::string context, Ptr<const Packet>);
  void UanPhyGenRxTrace(std::string context, Ptr<const Packet>);
  void RemainingEnergyTrace(std::string context, double previousEnergy,
                            double currentEnergy);
  void GenericWirelessTxTrace(std::string context, Ptr<const Packet> p,
                              ProtocolType protocolType);
  void GenericWirelessRxTrace(std::string context, Ptr<const Packet> p,
                              ProtocolType protocolType);

  void ConnectCallbacks();
  void ConnectLte();
  void ConnectLteUe(Ptr<Node> n, Ptr<LteUeNetDevice> nd, uint32_t devIndex);
  void ConnectLteEnb(Ptr<Node> n, Ptr<LteEnbNetDevice> nd, uint32_t devIndex);

  Vector GetPosition(Ptr<Node> n);
  Vector UpdatePosition(Ptr<Node> n);
  Vector UpdatePosition(Ptr<Node> n, Vector v);
  Vector UpdatePosition(Ptr<NetDevice> ndev);
  bool NodeHasMoved(Ptr<Node> n, Vector newLocation);
  std::vector<Ptr<Node>> GetMovedNodes();
  void MobilityCourseChangeTrace(Ptr<const MobilityModel> mob);

  void WriteNonP2pLinkProperties(uint32_t id, std::string ipv4Address,
                                 std::string channelType);
  void WriteNodeUpdate(uint32_t nodeId);
  void OutputWirelessPacketTxInfo(Ptr<const Packet> p, AnimPacketInfo &pktInfo,
                                  uint64_t animUid);
  void OutputWirelessPacketRxInfo(Ptr<const Packet> p, AnimPacketInfo &pktInfo,
                                  uint64_t animUid);
  void OutputCsmaPacket(Ptr<const Packet> p, AnimPacketInfo &pktInfo);
  void WriteLinkProperties();
  void WriteIpv4Addresses();
  void WriteIpv6Addresses();
  void WriteNodes();
  void WriteNodeColors();
  void WriteNodeSizes();
  void WriteNodeEnergies();
  void WriteXmlAnim(bool routing = false);
  void WriteXmlUpdateNodePosition(uint32_t nodeId, double x, double y);
  void WriteXmlUpdateNodeColor(uint32_t nodeId, uint8_t r, uint8_t g,
                               uint8_t b);
  void WriteXmlUpdateNodeDescription(uint32_t nodeId);
  void WriteXmlUpdateNodeSize(uint32_t nodeId, double width, double height);
  void WriteXmlAddResource(uint32_t resourceId, std::string resourcePath);
  void WriteXmlAddNodeCounter(uint32_t counterId, std::string counterName,
                              CounterType counterType);
  void WriteXmlUpdateNodeImage(uint32_t nodeId, uint32_t resourceId);
  void WriteXmlUpdateNodeCounter(uint32_t counterId, uint32_t nodeId,
                                 double value);
  void WriteXmlNode(uint32_t id, uint32_t sysId, double locX, double locY);
  void WriteXmlLink(uint32_t fromId, uint32_t toLp, uint32_t toId);
  void WriteXmlUpdateLink(uint32_t fromId, uint32_t toId,
                          std::string linkDescription);
  void WriteXmlP(std::string pktType, uint32_t fId, double fbTx, double lbTx,
                 uint32_t tId, double fbRx, double lbRx,
                 std::string metaInfo = "");
  void WriteXmlP(uint64_t animUid, std::string pktType, uint32_t fId,
                 double fbTx, double lbTx);
  void WriteXmlPRef(uint64_t animUid, uint32_t fId, double fbTx,
                    std::string metaInfo = "");
  void WriteXmlClose(std::string name, bool routing = false);
  void WriteXmlNonP2pLinkProperties(uint32_t id, std::string ipAddress,
                                    std::string channelType);
  void WriteXmlRouting(uint32_t id, std::string routingInfo);
  void WriteXmlRp(uint32_t nodeId, std::string destination,
                  Ipv4RoutePathElements rpElements);
  void WriteXmlUpdateBackground(std::string fileName, double x, double y,
                                double scaleX, double scaleY, double opacity);
  void WriteXmlIpv4Addresses(uint32_t nodeId,
                             std::vector<std::string> ipv4Addresses);
  void WriteXmlIpv6Addresses(uint32_t nodeId,
                             std::vector<std::string> ipv6Addresses);
};

class AnimByteTag : public Tag {
public:
  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  uint32_t GetSerializedSize() const override;

  void Serialize(TagBuffer i) const override;

  void Deserialize(TagBuffer i) override;

  void Print(std::ostream &os) const override;

  void Set(uint64_t AnimUid);

  uint64_t Get() const;

private:
  uint64_t m_AnimUid;
};

} // namespace ns3
#endif
