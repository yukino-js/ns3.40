#ifndef NS3_PYVIZ_H
#define NS3_PYVIZ_H

#include "ns3/channel.h"
#include "ns3/event-id.h"
#include "ns3/ipv4-header.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/mac48-address.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/packet.h"

#include <map>
#include <set>

namespace ns3 {

class PyViz {
public:
  PyViz();
  ~PyViz();

  void RegisterDropTracePath(const std::string &tracePath);

  void RegisterCsmaLikeDevice(const std::string &deviceTypeName);
  void RegisterWifiLikeDevice(const std::string &deviceTypeName);
  void RegisterPointToPointLikeDevice(const std::string &deviceTypeName);

  void SimulatorRunUntil(Time time);

  static void Pause(const std::string &message);
  std::vector<std::string> GetPauseMessages() const;

  struct TransmissionSample {
    Ptr<Node> transmitter;
    Ptr<Node> receiver;
    Ptr<Channel> channel;
    uint32_t bytes;
  };

  typedef std::vector<TransmissionSample> TransmissionSampleList;
  TransmissionSampleList GetTransmissionSamples() const;

  struct PacketDropSample {
    Ptr<Node> transmitter;
    uint32_t bytes;
  };

  typedef std::vector<PacketDropSample> PacketDropSampleList;
  PacketDropSampleList GetPacketDropSamples() const;

  struct PacketSample {
    Time time;
    Ptr<Packet> packet;
    Ptr<NetDevice> device;
  };

  struct TxPacketSample : PacketSample {
    Mac48Address to;
  };

  struct RxPacketSample : PacketSample {
    Mac48Address from;
  };

  struct LastPacketsSample {
    std::vector<RxPacketSample> lastReceivedPackets;
    std::vector<TxPacketSample> lastTransmittedPackets;
    std::vector<PacketSample> lastDroppedPackets;
  };

  LastPacketsSample GetLastPackets(uint32_t nodeId) const;

  void SetNodesOfInterest(std::set<uint32_t> nodes);

  struct NetDeviceStatistics {
    NetDeviceStatistics()
        : transmittedBytes(0), receivedBytes(0), transmittedPackets(0),
          receivedPackets(0) {}

    uint64_t transmittedBytes;
    uint64_t receivedBytes;
    uint32_t transmittedPackets;
    uint32_t receivedPackets;
  };

  struct NodeStatistics {
    uint32_t nodeId;
    std::vector<NetDeviceStatistics> statistics;
  };

  std::vector<NodeStatistics> GetNodesStatistics() const;

  enum PacketCaptureMode {
    PACKET_CAPTURE_DISABLED = 1,
    PACKET_CAPTURE_FILTER_HEADERS_OR,
    PACKET_CAPTURE_FILTER_HEADERS_AND,
  };

  struct PacketCaptureOptions {
    std::set<TypeId> headers;
    uint32_t numLastPackets;
    PacketCaptureMode mode;
  };

  void SetPacketCaptureOptions(uint32_t nodeId, PacketCaptureOptions options);

  static void LineClipping(double boundsX1, double boundsY1, double boundsX2,
                           double boundsY2, double &lineX1, double &lineY1,
                           double &lineX2, double &lineY2);

private:
  bool GetPacketCaptureOptions(uint32_t nodeId,
                               const PacketCaptureOptions **outOptions) const;
  static bool FilterPacket(Ptr<const Packet> packet,
                           const PacketCaptureOptions &options);

  typedef std::pair<Ptr<Channel>, uint32_t> TxRecordKey;

  struct TxRecordValue {
    Time time;
    Ptr<Node> srcNode;
    bool isBroadcast;
  };

  struct TransmissionSampleKey {
    bool operator<(const TransmissionSampleKey &other) const;
    bool operator==(const TransmissionSampleKey &other) const;
    Ptr<Node> transmitter;
    Ptr<Node> receiver;
    Ptr<Channel> channel;
  };

  struct TransmissionSampleValue {
    uint32_t bytes;
  };

  std::map<uint32_t, PacketCaptureOptions> m_packetCaptureOptions;
  std::vector<std::string> m_pauseMessages;
  std::map<TxRecordKey, TxRecordValue> m_txRecords;
  std::map<TransmissionSampleKey, TransmissionSampleValue>
      m_transmissionSamples;
  std::map<Ptr<Node>, uint32_t> m_packetDrops;
  std::set<uint32_t> m_nodesOfInterest;
  std::map<uint32_t, Time> m_packetsOfInterest;
  std::map<uint32_t, LastPacketsSample> m_lastPackets;
  std::map<uint32_t, std::vector<NetDeviceStatistics>> m_nodesStatistics;

  void TraceNetDevTxCommon(const std::string &context, Ptr<const Packet> packet,
                           const Mac48Address &destination);
  void TraceNetDevRxCommon(const std::string &context, Ptr<const Packet> packet,
                           const Mac48Address &source);

  void TraceNetDevTxWifi(std::string context, Ptr<const Packet> packet);
  void TraceNetDevRxWifi(std::string context, Ptr<const Packet> packet);

  void TraceDevQueueDrop(std::string context, Ptr<const Packet> packet);
  void TraceIpv4Drop(std::string context, const ns3::Ipv4Header &hdr,
                     Ptr<const Packet> packet,
                     ns3::Ipv4L3Protocol::DropReason reason,
                     Ptr<Ipv4> dummy_ipv4, uint32_t interface);

  void TraceNetDevTxCsma(std::string context, Ptr<const Packet> packet);
  void TraceNetDevRxCsma(std::string context, Ptr<const Packet> packet);
  void TraceNetDevPromiscRxCsma(std::string context, Ptr<const Packet> packet);

  void TraceNetDevTxPointToPoint(std::string context, Ptr<const Packet> packet);
  void TraceNetDevRxPointToPoint(std::string context, Ptr<const Packet> packet);

  void TraceNetDevTxWimax(std::string context, Ptr<const Packet> packet,
                          const Mac48Address &destination);
  void TraceNetDevRxWimax(std::string context, Ptr<const Packet> packet,
                          const Mac48Address &source);

  void TraceNetDevTxLte(std::string context, Ptr<const Packet> packet,
                        const Mac48Address &destination);
  void TraceNetDevRxLte(std::string context, Ptr<const Packet> packet,
                        const Mac48Address &source);

  inline NetDeviceStatistics &FindNetDeviceStatistics(int node, int interface);

  void DoPause(const std::string &message);

  bool m_stop;
  Time m_runUntil;

  void CallbackStopSimulation();
};

} // namespace ns3

#endif
