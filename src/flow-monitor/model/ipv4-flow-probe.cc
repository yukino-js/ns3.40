
#include "ipv4-flow-probe.h"

#include "flow-monitor.h"
#include "ipv4-flow-classifier.h"

#include "ns3/config.h"
#include "ns3/flow-id-tag.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/pointer.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("Ipv4FlowProbe");

class Ipv4FlowProbeTag : public Tag {
public:
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(TagBuffer buf) const override;
  void Deserialize(TagBuffer buf) override;
  void Print(std::ostream &os) const override;
  Ipv4FlowProbeTag();
  Ipv4FlowProbeTag(uint32_t flowId, uint32_t packetId, uint32_t packetSize,
                   Ipv4Address src, Ipv4Address dst);
  void SetFlowId(uint32_t flowId);
  void SetPacketId(uint32_t packetId);
  void SetPacketSize(uint32_t packetSize);
  uint32_t GetFlowId() const;
  uint32_t GetPacketId() const;
  uint32_t GetPacketSize() const;
  bool IsSrcDstValid(Ipv4Address src, Ipv4Address dst) const;

private:
  uint32_t m_flowId;
  uint32_t m_packetId;
  uint32_t m_packetSize;
  Ipv4Address m_src;
  Ipv4Address m_dst;
};

NS_OBJECT_ENSURE_REGISTERED(Ipv4FlowProbeTag);

TypeId Ipv4FlowProbeTag::GetTypeId() {
  static TypeId tid = TypeId("ns3::Ipv4FlowProbeTag")
                          .SetParent<Tag>()
                          .SetGroupName("FlowMonitor")
                          .AddConstructor<Ipv4FlowProbeTag>();
  return tid;
}

TypeId Ipv4FlowProbeTag::GetInstanceTypeId() const { return GetTypeId(); }

uint32_t Ipv4FlowProbeTag::GetSerializedSize() const { return 4 + 4 + 4 + 8; }

void Ipv4FlowProbeTag::Serialize(TagBuffer buf) const {
  buf.WriteU32(m_flowId);
  buf.WriteU32(m_packetId);
  buf.WriteU32(m_packetSize);

  uint8_t tBuf[4];
  m_src.Serialize(tBuf);
  buf.Write(tBuf, 4);
  m_dst.Serialize(tBuf);
  buf.Write(tBuf, 4);
}

void Ipv4FlowProbeTag::Deserialize(TagBuffer buf) {
  m_flowId = buf.ReadU32();
  m_packetId = buf.ReadU32();
  m_packetSize = buf.ReadU32();

  uint8_t tBuf[4];
  buf.Read(tBuf, 4);
  m_src = Ipv4Address::Deserialize(tBuf);
  buf.Read(tBuf, 4);
  m_dst = Ipv4Address::Deserialize(tBuf);
}

void Ipv4FlowProbeTag::Print(std::ostream &os) const {
  os << "FlowId=" << m_flowId;
  os << " PacketId=" << m_packetId;
  os << " PacketSize=" << m_packetSize;
}

Ipv4FlowProbeTag::Ipv4FlowProbeTag() : Tag() {}

Ipv4FlowProbeTag::Ipv4FlowProbeTag(uint32_t flowId, uint32_t packetId,
                                   uint32_t packetSize, Ipv4Address src,
                                   Ipv4Address dst)
    : Tag(), m_flowId(flowId), m_packetId(packetId), m_packetSize(packetSize),
      m_src(src), m_dst(dst) {}

void Ipv4FlowProbeTag::SetFlowId(uint32_t id) { m_flowId = id; }

void Ipv4FlowProbeTag::SetPacketId(uint32_t id) { m_packetId = id; }

void Ipv4FlowProbeTag::SetPacketSize(uint32_t size) { m_packetSize = size; }

uint32_t Ipv4FlowProbeTag::GetFlowId() const { return m_flowId; }

uint32_t Ipv4FlowProbeTag::GetPacketId() const { return m_packetId; }

uint32_t Ipv4FlowProbeTag::GetPacketSize() const { return m_packetSize; }

bool Ipv4FlowProbeTag::IsSrcDstValid(Ipv4Address src, Ipv4Address dst) const {
  return ((m_src == src) && (m_dst == dst));
}

Ipv4FlowProbe::Ipv4FlowProbe(Ptr<FlowMonitor> monitor,
                             Ptr<Ipv4FlowClassifier> classifier, Ptr<Node> node)
    : FlowProbe(monitor), m_classifier(classifier) {
  NS_LOG_FUNCTION(this << node->GetId());

  m_ipv4 = node->GetObject<Ipv4L3Protocol>();

  if (!m_ipv4->TraceConnectWithoutContext(
          "SendOutgoing", MakeCallback(&Ipv4FlowProbe::SendOutgoingLogger,
                                       Ptr<Ipv4FlowProbe>(this)))) {
    NS_FATAL_ERROR("trace fail");
  }
  if (!m_ipv4->TraceConnectWithoutContext(
          "UnicastForward", MakeCallback(&Ipv4FlowProbe::ForwardLogger,
                                         Ptr<Ipv4FlowProbe>(this)))) {
    NS_FATAL_ERROR("trace fail");
  }
  if (!m_ipv4->TraceConnectWithoutContext(
          "LocalDeliver", MakeCallback(&Ipv4FlowProbe::ForwardUpLogger,
                                       Ptr<Ipv4FlowProbe>(this)))) {
    NS_FATAL_ERROR("trace fail");
  }

  if (!m_ipv4->TraceConnectWithoutContext(
          "Drop",
          MakeCallback(&Ipv4FlowProbe::DropLogger, Ptr<Ipv4FlowProbe>(this)))) {
    NS_FATAL_ERROR("trace fail");
  }

  std::ostringstream qd;
  qd << "/NodeList/" << node->GetId()
     << "/$ns3::TrafficControlLayer/RootQueueDiscList/*/Drop";
  Config::ConnectWithoutContextFailSafe(
      qd.str(), MakeCallback(&Ipv4FlowProbe::QueueDiscDropLogger,
                             Ptr<Ipv4FlowProbe>(this)));

  std::ostringstream oss;
  oss << "/NodeList/" << node->GetId() << "/DeviceList/*/TxQueue/Drop";
  Config::ConnectWithoutContextFailSafe(
      oss.str(),
      MakeCallback(&Ipv4FlowProbe::QueueDropLogger, Ptr<Ipv4FlowProbe>(this)));
}

Ipv4FlowProbe::~Ipv4FlowProbe() {}

TypeId Ipv4FlowProbe::GetTypeId() {
  static TypeId tid = TypeId("ns3::Ipv4FlowProbe")
                          .SetParent<FlowProbe>()
                          .SetGroupName("FlowMonitor");

  return tid;
}

void Ipv4FlowProbe::DoDispose() {
  m_ipv4 = nullptr;
  m_classifier = nullptr;
  FlowProbe::DoDispose();
}

void Ipv4FlowProbe::SendOutgoingLogger(const Ipv4Header &ipHeader,
                                       Ptr<const Packet> ipPayload,
                                       uint32_t interface) {
  FlowId flowId;
  FlowPacketId packetId;

  if (!m_ipv4->IsUnicast(ipHeader.GetDestination())) {
    return;
  }

  Ipv4FlowProbeTag fTag;
  bool found = ipPayload->FindFirstMatchingByteTag(fTag);
  if (found) {
    return;
  }

  if (m_classifier->Classify(ipHeader, ipPayload, &flowId, &packetId)) {
    uint32_t size = (ipPayload->GetSize() + ipHeader.GetSerializedSize());
    NS_LOG_DEBUG("ReportFirstTx (" << this << ", " << flowId << ", " << packetId
                                   << ", " << size << "); " << ipHeader
                                   << *ipPayload);
    m_flowMonitor->ReportFirstTx(this, flowId, packetId, size);

    Ipv4FlowProbeTag fTag(flowId, packetId, size, ipHeader.GetSource(),
                          ipHeader.GetDestination());
    ipPayload->AddByteTag(fTag);
  }
}

void Ipv4FlowProbe::ForwardLogger(const Ipv4Header &ipHeader,
                                  Ptr<const Packet> ipPayload,
                                  uint32_t interface) {
  Ipv4FlowProbeTag fTag;
  bool found = ipPayload->FindFirstMatchingByteTag(fTag);

  if (found) {
    if (!ipHeader.IsLastFragment() || ipHeader.GetFragmentOffset() != 0) {
      NS_LOG_WARN("Not counting fragmented packets");
      return;
    }
    if (!fTag.IsSrcDstValid(ipHeader.GetSource(), ipHeader.GetDestination())) {
      NS_LOG_LOGIC("Not reporting encapsulated packet");
      return;
    }

    FlowId flowId = fTag.GetFlowId();
    FlowPacketId packetId = fTag.GetPacketId();

    uint32_t size = (ipPayload->GetSize() + ipHeader.GetSerializedSize());
    NS_LOG_DEBUG("ReportForwarding (" << this << ", " << flowId << ", "
                                      << packetId << ", " << size << ");");
    m_flowMonitor->ReportForwarding(this, flowId, packetId, size);
  }
}

void Ipv4FlowProbe::ForwardUpLogger(const Ipv4Header &ipHeader,
                                    Ptr<const Packet> ipPayload,
                                    uint32_t interface) {
  Ipv4FlowProbeTag fTag;
  bool found = ipPayload->FindFirstMatchingByteTag(fTag);

  if (found) {
    if (!fTag.IsSrcDstValid(ipHeader.GetSource(), ipHeader.GetDestination())) {
      NS_LOG_LOGIC("Not reporting encapsulated packet");
      return;
    }

    FlowId flowId = fTag.GetFlowId();
    FlowPacketId packetId = fTag.GetPacketId();

    uint32_t size = (ipPayload->GetSize() + ipHeader.GetSerializedSize());
    NS_LOG_DEBUG("ReportLastRx (" << this << ", " << flowId << ", " << packetId
                                  << ", " << size << "); " << ipHeader
                                  << *ipPayload);
    m_flowMonitor->ReportLastRx(this, flowId, packetId, size);
  }
}

void Ipv4FlowProbe::DropLogger(const Ipv4Header &ipHeader,
                               Ptr<const Packet> ipPayload,
                               Ipv4L3Protocol::DropReason reason,
                               Ptr<Ipv4> ipv4, uint32_t ifIndex) {
#if 0
  switch (reason)
    {
    case Ipv4L3Protocol::DROP_NO_ROUTE:
      break;

    case Ipv4L3Protocol::DROP_TTL_EXPIRED:
    case Ipv4L3Protocol::DROP_BAD_CHECKSUM:
      Ipv4Address addri = m_ipv4->GetAddress (ifIndex);
      Ipv4Mask maski = m_ipv4->GetNetworkMask (ifIndex);
      Ipv4Address bcast = addri.GetSubnetDirectedBroadcast (maski);
      if (ipHeader.GetDestination () == bcast)
        {
          return;
        }
    }
#endif

  Ipv4FlowProbeTag fTag;
  bool found = ipPayload->FindFirstMatchingByteTag(fTag);

  if (found) {
    FlowId flowId = fTag.GetFlowId();
    FlowPacketId packetId = fTag.GetPacketId();

    uint32_t size = (ipPayload->GetSize() + ipHeader.GetSerializedSize());
    NS_LOG_DEBUG("Drop (" << this << ", " << flowId << ", " << packetId << ", "
                          << size << ", " << reason
                          << ", destIp=" << ipHeader.GetDestination() << "); "
                          << "HDR: " << ipHeader << " PKT: " << *ipPayload);

    DropReason myReason;

    switch (reason) {
    case Ipv4L3Protocol::DROP_TTL_EXPIRED:
      myReason = DROP_TTL_EXPIRE;
      NS_LOG_DEBUG("DROP_TTL_EXPIRE");
      break;
    case Ipv4L3Protocol::DROP_NO_ROUTE:
      myReason = DROP_NO_ROUTE;
      NS_LOG_DEBUG("DROP_NO_ROUTE");
      break;
    case Ipv4L3Protocol::DROP_BAD_CHECKSUM:
      myReason = DROP_BAD_CHECKSUM;
      NS_LOG_DEBUG("DROP_BAD_CHECKSUM");
      break;
    case Ipv4L3Protocol::DROP_INTERFACE_DOWN:
      myReason = DROP_INTERFACE_DOWN;
      NS_LOG_DEBUG("DROP_INTERFACE_DOWN");
      break;
    case Ipv4L3Protocol::DROP_ROUTE_ERROR:
      myReason = DROP_ROUTE_ERROR;
      NS_LOG_DEBUG("DROP_ROUTE_ERROR");
      break;
    case Ipv4L3Protocol::DROP_FRAGMENT_TIMEOUT:
      myReason = DROP_FRAGMENT_TIMEOUT;
      NS_LOG_DEBUG("DROP_FRAGMENT_TIMEOUT");
      break;

    default:
      myReason = DROP_INVALID_REASON;
      NS_FATAL_ERROR("Unexpected drop reason code " << reason);
    }

    m_flowMonitor->ReportDrop(this, flowId, packetId, size, myReason);
  }
}

void Ipv4FlowProbe::QueueDropLogger(Ptr<const Packet> ipPayload) {
  Ipv4FlowProbeTag fTag;
  bool tagFound = ipPayload->FindFirstMatchingByteTag(fTag);

  if (!tagFound) {
    return;
  }

  FlowId flowId = fTag.GetFlowId();
  FlowPacketId packetId = fTag.GetPacketId();
  uint32_t size = fTag.GetPacketSize();

  NS_LOG_DEBUG("Drop (" << this << ", " << flowId << ", " << packetId << ", "
                        << size << ", " << DROP_QUEUE << "); ");

  m_flowMonitor->ReportDrop(this, flowId, packetId, size, DROP_QUEUE);
}

void Ipv4FlowProbe::QueueDiscDropLogger(Ptr<const QueueDiscItem> item) {
  Ipv4FlowProbeTag fTag;
  bool tagFound = item->GetPacket()->FindFirstMatchingByteTag(fTag);

  if (!tagFound) {
    return;
  }

  FlowId flowId = fTag.GetFlowId();
  FlowPacketId packetId = fTag.GetPacketId();
  uint32_t size = fTag.GetPacketSize();

  NS_LOG_DEBUG("Drop (" << this << ", " << flowId << ", " << packetId << ", "
                        << size << ", " << DROP_QUEUE_DISC << "); ");

  m_flowMonitor->ReportDrop(this, flowId, packetId, size, DROP_QUEUE_DISC);
}

} // namespace ns3
