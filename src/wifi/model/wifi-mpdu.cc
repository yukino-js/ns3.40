
#include "wifi-mpdu.h"

#include "msdu-aggregator.h"
#include "wifi-mac-trailer.h"
#include "wifi-utils.h"

#include "ns3/log.h"
#include "ns3/packet.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("WifiMpdu");

WifiMpdu::WifiMpdu(Ptr<const Packet> p, const WifiMacHeader &header, Time stamp)
    : m_header(header) {
  auto &original = std::get<OriginalInfo>(m_instanceInfo);
  original.m_packet = p;
  original.m_timestamp = stamp;

  if (header.IsQosData() && header.IsQosAmsdu()) {
    original.m_msduList = MsduAggregator::Deaggregate(p->Copy());
  }
}

WifiMpdu::~WifiMpdu() {
  NS_ASSERT(std::holds_alternative<Ptr<WifiMpdu>>(m_instanceInfo) ||
            !IsQueued());
}

bool WifiMpdu::IsOriginal() const {
  return std::holds_alternative<OriginalInfo>(m_instanceInfo);
}

Ptr<const WifiMpdu> WifiMpdu::GetOriginal() const {
  if (std::holds_alternative<OriginalInfo>(m_instanceInfo)) {
    return this;
  }
  return std::get<ALIAS>(m_instanceInfo);
}

Ptr<WifiMpdu> WifiMpdu::CreateAlias(uint8_t linkId) const {
  NS_LOG_FUNCTION(this << +linkId);
  NS_ABORT_MSG_IF(
      !std::holds_alternative<OriginalInfo>(m_instanceInfo),
      "This method can only be called on the original version of the MPDU");
  NS_ABORT_MSG_IF(
      !IsQueued(),
      "This method can only be called if the MPDU is stored in a MAC queue");

  auto alias = Ptr<WifiMpdu>(new WifiMpdu, false);

  alias->m_header = m_header;
  alias->m_instanceInfo = Ptr(const_cast<WifiMpdu *>(this));
  NS_ASSERT(alias->m_instanceInfo.index() == ALIAS);

  return alias;
}

WifiMpdu::OriginalInfo &WifiMpdu::GetOriginalInfo() {
  if (auto original = std::get_if<ORIGINAL>(&m_instanceInfo)) {
    return *original;
  }
  auto &origInstanceInfo =
      std::get<Ptr<WifiMpdu>>(m_instanceInfo)->m_instanceInfo;
  return std::get<OriginalInfo>(origInstanceInfo);
}

const WifiMpdu::OriginalInfo &WifiMpdu::GetOriginalInfo() const {
  if (auto original = std::get_if<ORIGINAL>(&m_instanceInfo)) {
    return *original;
  }
  const auto &origInstanceInfo =
      std::get<Ptr<WifiMpdu>>(m_instanceInfo)->m_instanceInfo;
  return std::get<OriginalInfo>(origInstanceInfo);
}

Ptr<const Packet> WifiMpdu::GetPacket() const {
  return GetOriginalInfo().m_packet;
}

Time WifiMpdu::GetTimestamp() const {
  if (auto original = std::get_if<ORIGINAL>(&m_instanceInfo)) {
    return original->m_timestamp;
  }
  const auto &origInstanceInfo =
      std::get<Ptr<WifiMpdu>>(m_instanceInfo)->m_instanceInfo;
  return std::get<OriginalInfo>(origInstanceInfo).m_timestamp;
}

const WifiMacHeader &WifiMpdu::GetHeader() const { return m_header; }

WifiMacHeader &WifiMpdu::GetHeader() { return m_header; }

Mac48Address WifiMpdu::GetDestinationAddress() const {
  return m_header.GetAddr1();
}

uint32_t WifiMpdu::GetPacketSize() const { return GetPacket()->GetSize(); }

uint32_t WifiMpdu::GetSize() const {
  return GetPacketSize() + m_header.GetSerializedSize() + WIFI_MAC_FCS_LENGTH;
}

bool WifiMpdu::IsFragment() const {
  return m_header.IsMoreFragments() || m_header.GetFragmentNumber() > 0;
}

Ptr<Packet> WifiMpdu::GetProtocolDataUnit() const {
  Ptr<Packet> mpdu = GetPacket()->Copy();
  mpdu->AddHeader(m_header);
  AddWifiMacTrailer(mpdu);
  return mpdu;
}

void WifiMpdu::Aggregate(Ptr<const WifiMpdu> msdu) {
  NS_ASSERT(msdu);
  NS_LOG_FUNCTION(this << *msdu);
  NS_ABORT_MSG_IF(
      !msdu->GetHeader().IsQosData() || msdu->GetHeader().IsQosAmsdu(),
      "Only QoS data frames that do not contain an A-MSDU can be aggregated");
  NS_ABORT_MSG_IF(
      !std::holds_alternative<OriginalInfo>(m_instanceInfo),
      "This method can only be called on the original version of the MPDU");

  auto &original = std::get<OriginalInfo>(m_instanceInfo);

  if (original.m_msduList.empty()) {
    Ptr<const WifiMpdu> firstMsdu = Create<const WifiMpdu>(*this);
    original.m_packet = Create<Packet>();
    DoAggregate(firstMsdu);

    m_header.SetQosAmsdu();
    if (m_header.IsToDs() && !m_header.IsFromDs()) {
      m_header.SetAddr3(m_header.GetAddr1());
    } else if (!m_header.IsToDs() && m_header.IsFromDs()) {
      m_header.SetAddr3(m_header.GetAddr2());
    }
  }
  DoAggregate(msdu);
}

void WifiMpdu::DoAggregate(Ptr<const WifiMpdu> msdu) {
  NS_LOG_FUNCTION(this << *msdu);

  AmsduSubframeHeader hdr;
  hdr.SetDestinationAddr(msdu->GetHeader().IsToDs()
                             ? msdu->GetHeader().GetAddr3()
                             : msdu->GetHeader().GetAddr1());
  hdr.SetSourceAddr(!msdu->GetHeader().IsFromDs()
                        ? msdu->GetHeader().GetAddr2()
                        : (!msdu->GetHeader().IsToDs()
                               ? msdu->GetHeader().GetAddr3()
                               : msdu->GetHeader().GetAddr4()));
  hdr.SetLength(static_cast<uint16_t>(msdu->GetPacket()->GetSize()));

  auto &original = std::get<OriginalInfo>(m_instanceInfo);

  original.m_msduList.emplace_back(msdu->GetPacket(), hdr);

  NS_ASSERT(original.m_packet);
  Ptr<Packet> amsdu = original.m_packet->Copy();

  if (original.m_packet->GetSize() > 0) {
    uint8_t padding =
        MsduAggregator::CalculatePadding(original.m_packet->GetSize());

    if (padding) {
      amsdu->AddAtEnd(Create<Packet>(padding));
    }
  }

  Ptr<Packet> amsduSubframe = msdu->GetPacket()->Copy();
  amsduSubframe->AddHeader(hdr);
  amsdu->AddAtEnd(amsduSubframe);
  original.m_packet = amsdu;
}

bool WifiMpdu::IsQueued() const {
  return GetOriginalInfo().m_queueIt.has_value();
}

void WifiMpdu::SetQueueIt(std::optional<Iterator> queueIt, WmqIteratorTag tag) {
  NS_ABORT_MSG_IF(
      !std::holds_alternative<OriginalInfo>(m_instanceInfo),
      "This method can only be called on the original version of the MPDU");

  auto &original = std::get<OriginalInfo>(m_instanceInfo);
  original.m_queueIt = queueIt;
}

WifiMpdu::Iterator WifiMpdu::GetQueueIt(WmqIteratorTag tag) const {
  return GetQueueIt();
}

WifiMpdu::Iterator WifiMpdu::GetQueueIt() const {
  NS_ASSERT(IsQueued());
  return GetOriginalInfo().m_queueIt.value();
}

AcIndex WifiMpdu::GetQueueAc() const { return GetQueueIt()->ac; }

Time WifiMpdu::GetExpiryTime() const { return GetQueueIt()->expiryTime; }

void WifiMpdu::SetInFlight(uint8_t linkId) const {
  GetQueueIt()->inflights[linkId] = Ptr(const_cast<WifiMpdu *>(this));
}

void WifiMpdu::ResetInFlight(uint8_t linkId) const {
  GetQueueIt()->inflights.erase(linkId);
}

std::set<uint8_t> WifiMpdu::GetInFlightLinkIds() const {
  if (!IsQueued()) {
    return {};
  }
  std::set<uint8_t> linkIds;
  for (const auto &[linkId, mpdu] : GetQueueIt()->inflights) {
    linkIds.insert(linkId);
  }
  return linkIds;
}

bool WifiMpdu::IsInFlight() const {
  return IsQueued() && !GetQueueIt()->inflights.empty();
}

void WifiMpdu::AssignSeqNo(uint16_t seqNo) {
  NS_LOG_FUNCTION(this << seqNo);

  m_header.SetSequenceNumber(seqNo);
  if (auto originalPtr = std::get_if<ALIAS>(&m_instanceInfo)) {
    (*originalPtr)->m_header.SetSequenceNumber(seqNo);
  }
  GetOriginalInfo().m_seqNoAssigned = true;
}

bool WifiMpdu::HasSeqNoAssigned() const {
  return GetOriginalInfo().m_seqNoAssigned;
}

void WifiMpdu::UnassignSeqNo() { GetOriginalInfo().m_seqNoAssigned = false; }

WifiMpdu::DeaggregatedMsdusCI WifiMpdu::begin() const {
  return GetOriginalInfo().m_msduList.cbegin();
}

WifiMpdu::DeaggregatedMsdusCI WifiMpdu::end() const {
  return GetOriginalInfo().m_msduList.cend();
}

void WifiMpdu::Print(std::ostream &os) const {
  os << m_header.GetTypeString() << ", payloadSize=" << GetPacketSize()
     << ", to=" << m_header.GetAddr1()
     << ", seqN=" << m_header.GetSequenceNumber()
     << ", duration/ID=" << m_header.GetDuration();
  if (m_header.IsQosData()) {
    os << ", tid=" << +m_header.GetQosTid();
    if (m_header.IsQosNoAck()) {
      os << ", ack=NoAck";
    } else if (m_header.IsQosAck()) {
      os << ", ack=NormalAck";
    } else if (m_header.IsQosBlockAck()) {
      os << ", ack=BlockAck";
    }
  }
  os << ", queued=" << IsQueued();
  if (IsQueued()) {
    os << ", residualLifetime="
       << (GetExpiryTime() - Simulator::Now()).As(Time::US)
       << ", inflight=" << IsInFlight();
  }
  os << ", packet=" << GetPacket();
}

std::ostream &operator<<(std::ostream &os, const WifiMpdu &item) {
  item.Print(os);
  return os;
}

} // namespace ns3
