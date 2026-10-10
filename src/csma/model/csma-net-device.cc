
#include "csma-net-device.h"

#include "csma-channel.h"

#include "ns3/boolean.h"
#include "ns3/enum.h"
#include "ns3/error-model.h"
#include "ns3/ethernet-header.h"
#include "ns3/ethernet-trailer.h"
#include "ns3/llc-snap-header.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/queue.h"
#include "ns3/simulator.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/uinteger.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("CsmaNetDevice");

NS_OBJECT_ENSURE_REGISTERED(CsmaNetDevice);

TypeId CsmaNetDevice::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::CsmaNetDevice")
          .SetParent<NetDevice>()
          .SetGroupName("Csma")
          .AddConstructor<CsmaNetDevice>()
          .AddAttribute("Address", "The MAC address of this device.",
                        Mac48AddressValue(Mac48Address("ff:ff:ff:ff:ff:ff")),
                        MakeMac48AddressAccessor(&CsmaNetDevice::m_address),
                        MakeMac48AddressChecker())
          .AddAttribute("Mtu", "The MAC-level Maximum Transmission Unit",
                        UintegerValue(DEFAULT_MTU),
                        MakeUintegerAccessor(&CsmaNetDevice::SetMtu,
                                             &CsmaNetDevice::GetMtu),
                        MakeUintegerChecker<uint16_t>())
          .AddAttribute("EncapsulationMode",
                        "The link-layer encapsulation type to use.",
                        EnumValue(DIX),
                        MakeEnumAccessor(&CsmaNetDevice::SetEncapsulationMode),
                        MakeEnumChecker(DIX, "Dix", LLC, "Llc"))
          .AddAttribute(
              "SendEnable",
              "Enable or disable the transmitter section of the device.",
              BooleanValue(true),
              MakeBooleanAccessor(&CsmaNetDevice::m_sendEnable),
              MakeBooleanChecker())
          .AddAttribute("ReceiveEnable",
                        "Enable or disable the receiver section of the device.",
                        BooleanValue(true),
                        MakeBooleanAccessor(&CsmaNetDevice::m_receiveEnable),
                        MakeBooleanChecker())
          .AddAttribute(
              "ReceiveErrorModel",
              "The receiver error model used to simulate packet loss",
              PointerValue(),
              MakePointerAccessor(&CsmaNetDevice::m_receiveErrorModel),
              MakePointerChecker<ErrorModel>())

          .AddAttribute(
              "TxQueue", "A queue to use as the transmit queue in the device.",
              PointerValue(), MakePointerAccessor(&CsmaNetDevice::m_queue),
              MakePointerChecker<Queue<Packet>>())

          .AddTraceSource("MacTx",
                          "Trace source indicating a packet has "
                          "arrived for transmission by this device",
                          MakeTraceSourceAccessor(&CsmaNetDevice::m_macTxTrace),
                          "ns3::Packet::TracedCallback")
          .AddTraceSource(
              "MacTxDrop",
              "Trace source indicating a packet has been "
              "dropped by the device before transmission",
              MakeTraceSourceAccessor(&CsmaNetDevice::m_macTxDropTrace),
              "ns3::Packet::TracedCallback")
          .AddTraceSource(
              "MacPromiscRx",
              "A packet has been received by this device, "
              "has been passed up from the physical layer "
              "and is being forwarded up the local protocol stack.  "
              "This is a promiscuous trace,",
              MakeTraceSourceAccessor(&CsmaNetDevice::m_macPromiscRxTrace),
              "ns3::Packet::TracedCallback")
          .AddTraceSource(
              "MacRx",
              "A packet has been received by this device, "
              "has been passed up from the physical layer "
              "and is being forwarded up the local protocol stack.  "
              "This is a non-promiscuous trace,",
              MakeTraceSourceAccessor(&CsmaNetDevice::m_macRxTrace),
              "ns3::Packet::TracedCallback")
#if 0
    .AddTraceSource ("MacRxDrop",
                     "Trace source indicating a packet was received, "
                     "but dropped before being forwarded up the stack",
                     MakeTraceSourceAccessor (&CsmaNetDevice::m_macRxDropTrace),
                     "ns3::Packet::TracedCallback")
#endif
          .AddTraceSource(
              "MacTxBackoff",
              "Trace source indicating a packet has been "
              "delayed by the CSMA backoff process",
              MakeTraceSourceAccessor(&CsmaNetDevice::m_macTxBackoffTrace),
              "ns3::Packet::TracedCallback")
          .AddTraceSource(
              "PhyTxBegin",
              "Trace source indicating a packet has "
              "begun transmitting over the channel",
              MakeTraceSourceAccessor(&CsmaNetDevice::m_phyTxBeginTrace),
              "ns3::Packet::TracedCallback")
          .AddTraceSource(
              "PhyTxEnd",
              "Trace source indicating a packet has been "
              "completely transmitted over the channel",
              MakeTraceSourceAccessor(&CsmaNetDevice::m_phyTxEndTrace),
              "ns3::Packet::TracedCallback")
          .AddTraceSource(
              "PhyTxDrop",
              "Trace source indicating a packet has been "
              "dropped by the device during transmission",
              MakeTraceSourceAccessor(&CsmaNetDevice::m_phyTxDropTrace),
              "ns3::Packet::TracedCallback")
#if 0
    .AddTraceSource ("PhyRxBegin",
                     "Trace source indicating a packet has "
                     "begun being received by the device",
                     MakeTraceSourceAccessor (&CsmaNetDevice::m_phyRxBeginTrace),
                     "ns3::Packet::TracedCallback")
#endif
          .AddTraceSource(
              "PhyRxEnd",
              "Trace source indicating a packet has been "
              "completely received by the device",
              MakeTraceSourceAccessor(&CsmaNetDevice::m_phyRxEndTrace),
              "ns3::Packet::TracedCallback")
          .AddTraceSource(
              "PhyRxDrop",
              "Trace source indicating a packet has been "
              "dropped by the device during reception",
              MakeTraceSourceAccessor(&CsmaNetDevice::m_phyRxDropTrace),
              "ns3::Packet::TracedCallback")
          .AddTraceSource(
              "Sniffer",
              "Trace source simulating a non-promiscuous "
              "packet sniffer attached to the device",
              MakeTraceSourceAccessor(&CsmaNetDevice::m_snifferTrace),
              "ns3::Packet::TracedCallback")
          .AddTraceSource(
              "PromiscSniffer",
              "Trace source simulating a promiscuous "
              "packet sniffer attached to the device",
              MakeTraceSourceAccessor(&CsmaNetDevice::m_promiscSnifferTrace),
              "ns3::Packet::TracedCallback");
  return tid;
}

CsmaNetDevice::CsmaNetDevice() : m_linkUp(false) {
  NS_LOG_FUNCTION(this);
  m_txMachineState = READY;
  m_tInterframeGap = Seconds(0);
  m_channel = nullptr;

  m_encapMode = DIX;
}

CsmaNetDevice::~CsmaNetDevice() {
  NS_LOG_FUNCTION_NOARGS();
  m_queue = nullptr;
}

void CsmaNetDevice::DoDispose() {
  NS_LOG_FUNCTION_NOARGS();
  m_channel = nullptr;
  m_node = nullptr;
  m_queue = nullptr;
  NetDevice::DoDispose();
}

void CsmaNetDevice::SetEncapsulationMode(EncapsulationMode mode) {
  NS_LOG_FUNCTION(mode);

  m_encapMode = mode;

  NS_LOG_LOGIC("m_encapMode = " << m_encapMode);
  NS_LOG_LOGIC("m_mtu = " << m_mtu);
}

CsmaNetDevice::EncapsulationMode CsmaNetDevice::GetEncapsulationMode() {
  NS_LOG_FUNCTION_NOARGS();
  return m_encapMode;
}

bool CsmaNetDevice::SetMtu(uint16_t mtu) {
  NS_LOG_FUNCTION(this << mtu);
  m_mtu = mtu;

  NS_LOG_LOGIC("m_encapMode = " << m_encapMode);
  NS_LOG_LOGIC("m_mtu = " << m_mtu);

  return true;
}

uint16_t CsmaNetDevice::GetMtu() const {
  NS_LOG_FUNCTION_NOARGS();
  return m_mtu;
}

void CsmaNetDevice::SetSendEnable(bool sendEnable) {
  NS_LOG_FUNCTION(sendEnable);
  m_sendEnable = sendEnable;
}

void CsmaNetDevice::SetReceiveEnable(bool receiveEnable) {
  NS_LOG_FUNCTION(receiveEnable);
  m_receiveEnable = receiveEnable;
}

bool CsmaNetDevice::IsSendEnabled() const {
  NS_LOG_FUNCTION_NOARGS();
  return m_sendEnable;
}

bool CsmaNetDevice::IsReceiveEnabled() const {
  NS_LOG_FUNCTION_NOARGS();
  return m_receiveEnable;
}

void CsmaNetDevice::SetInterframeGap(Time t) {
  NS_LOG_FUNCTION(t);
  m_tInterframeGap = t;
}

void CsmaNetDevice::SetBackoffParams(Time slotTime, uint32_t minSlots,
                                     uint32_t maxSlots, uint32_t ceiling,
                                     uint32_t maxRetries) {
  NS_LOG_FUNCTION(slotTime << minSlots << maxSlots << ceiling << maxRetries);
  m_backoff.m_slotTime = slotTime;
  m_backoff.m_minSlots = minSlots;
  m_backoff.m_maxSlots = maxSlots;
  m_backoff.m_ceiling = ceiling;
  m_backoff.m_maxRetries = maxRetries;
}

void CsmaNetDevice::AddHeader(Ptr<Packet> p, Mac48Address source,
                              Mac48Address dest, uint16_t protocolNumber) {
  NS_LOG_FUNCTION(p << source << dest << protocolNumber);

  EthernetHeader header(false);
  header.SetSource(source);
  header.SetDestination(dest);

  EthernetTrailer trailer;

  NS_LOG_LOGIC("p->GetSize () = " << p->GetSize());
  NS_LOG_LOGIC("m_encapMode = " << m_encapMode);
  NS_LOG_LOGIC("m_mtu = " << m_mtu);

  uint16_t lengthType = 0;
  switch (m_encapMode) {
  case DIX:
    NS_LOG_LOGIC("Encapsulating packet as DIX (type interpretation)");
    lengthType = protocolNumber;

    if (p->GetSize() < 46) {
      uint8_t buffer[46];
      memset(buffer, 0, 46);
      Ptr<Packet> padd = Create<Packet>(buffer, 46 - p->GetSize());
      p->AddAtEnd(padd);
    }
    break;
  case LLC: {
    NS_LOG_LOGIC("Encapsulating packet as LLC (length interpretation)");

    LlcSnapHeader llc;
    llc.SetType(protocolNumber);
    p->AddHeader(llc);

    lengthType = p->GetSize();

    if (p->GetSize() < 46) {
      uint8_t buffer[46];
      memset(buffer, 0, 46);
      Ptr<Packet> padd = Create<Packet>(buffer, 46 - p->GetSize());
      p->AddAtEnd(padd);
    }

    NS_ASSERT_MSG(
        p->GetSize() <= GetMtu(),
        "CsmaNetDevice::AddHeader(): 802.3 Length/Type field with LLC/SNAP: "
        "length interpretation must not exceed device frame size minus "
        "overhead");
  } break;
  case ILLEGAL:
  default:
    NS_FATAL_ERROR(
        "CsmaNetDevice::AddHeader(): Unknown packet encapsulation mode");
    break;
  }

  NS_LOG_LOGIC("header.SetLengthType (" << lengthType << ")");
  header.SetLengthType(lengthType);
  p->AddHeader(header);

  if (Node::ChecksumEnabled()) {
    trailer.EnableFcs(true);
  }
  trailer.CalcFcs(p);
  p->AddTrailer(trailer);
}

#if 0
bool
CsmaNetDevice::ProcessHeader (Ptr<Packet> p, uint16_t & param)
{
  NS_LOG_FUNCTION (p << param);

  EthernetTrailer trailer;
  p->RemoveTrailer (trailer);

  EthernetHeader header (false);
  p->RemoveHeader (header);

  if ((header.GetDestination () != GetBroadcast ()) &&
      (header.GetDestination () != GetAddress ()))
    {
      return false;
    }

  switch (m_encapMode)
    {
    case DIX:
      param = header.GetLengthType ();
      break;
    case LLC:
      {
        LlcSnapHeader llc;
        p->RemoveHeader (llc);
        param = llc.GetType ();
      }
      break;
    case ILLEGAL:
    default:
      NS_FATAL_ERROR ("CsmaNetDevice::ProcessHeader(): Unknown packet encapsulation mode");
      break;
    }
  return true;
}
#endif

void CsmaNetDevice::TransmitStart() {
  NS_LOG_FUNCTION_NOARGS();

  NS_ASSERT_MSG(m_currentPkt,
                "CsmaNetDevice::TransmitStart(): m_currentPkt not set");

  NS_LOG_LOGIC("m_currentPkt = " << m_currentPkt);
  NS_LOG_LOGIC("UID = " << m_currentPkt->GetUid());

  if (!IsSendEnabled()) {
    m_phyTxDropTrace(m_currentPkt);
    m_currentPkt = nullptr;
    return;
  }

  NS_ASSERT_MSG((m_txMachineState == READY) || (m_txMachineState == BACKOFF),
                "Must be READY to transmit. Tx state is: " << m_txMachineState);

  if (m_channel->GetState() != IDLE) {
    m_txMachineState = BACKOFF;

    if (m_backoff.MaxRetriesReached()) {
      TransmitAbort();
    } else {
      m_macTxBackoffTrace(m_currentPkt);

      m_backoff.IncrNumRetries();
      Time backoffTime = m_backoff.GetBackoffTime();

      NS_LOG_LOGIC("Channel busy, backing off for " << backoffTime.As(Time::S));

      Simulator::Schedule(backoffTime, &CsmaNetDevice::TransmitStart, this);
    }
  } else {
    m_phyTxBeginTrace(m_currentPkt);
    if (!m_channel->TransmitStart(m_currentPkt, m_deviceId)) {
      NS_LOG_WARN("Channel TransmitStart returns an error");
      m_phyTxDropTrace(m_currentPkt);
      m_currentPkt = nullptr;
      m_txMachineState = READY;
    } else {
      m_backoff.ResetBackoffTime();
      m_txMachineState = BUSY;

      Time tEvent = m_bps.CalculateBytesTxTime(m_currentPkt->GetSize());
      NS_LOG_LOGIC("Schedule TransmitCompleteEvent in " << tEvent.As(Time::S));
      Simulator::Schedule(tEvent, &CsmaNetDevice::TransmitCompleteEvent, this);
    }
  }
}

void CsmaNetDevice::TransmitAbort() {
  NS_LOG_FUNCTION_NOARGS();

  NS_ASSERT_MSG(m_currentPkt,
                "CsmaNetDevice::TransmitAbort(): m_currentPkt zero");
  NS_LOG_LOGIC("m_currentPkt=" << m_currentPkt);
  NS_LOG_LOGIC("Pkt UID is " << m_currentPkt->GetUid() << ")");

  m_phyTxDropTrace(m_currentPkt);
  m_currentPkt = nullptr;

  NS_ASSERT_MSG(
      m_txMachineState == BACKOFF,
      "Must be in BACKOFF state to abort.  Tx state is: " << m_txMachineState);

  m_backoff.ResetBackoffTime();
  m_txMachineState = READY;

  if (m_queue->IsEmpty()) {
    return;
  } else {
    Ptr<Packet> packet = m_queue->Dequeue();
    NS_ASSERT_MSG(packet, "CsmaNetDevice::TransmitAbort(): IsEmpty false but "
                          "no Packet on queue?");
    m_currentPkt = packet;
    m_snifferTrace(m_currentPkt);
    m_promiscSnifferTrace(m_currentPkt);
    TransmitStart();
  }
}

void CsmaNetDevice::TransmitCompleteEvent() {
  NS_LOG_FUNCTION_NOARGS();

  NS_ASSERT_MSG(
      m_txMachineState == BUSY,
      "CsmaNetDevice::transmitCompleteEvent(): Must be BUSY if transmitting");
  NS_ASSERT(m_channel->GetState() == TRANSMITTING);
  m_txMachineState = GAP;

  NS_ASSERT_MSG(m_currentPkt,
                "CsmaNetDevice::TransmitCompleteEvent(): m_currentPkt zero");
  NS_LOG_LOGIC("m_currentPkt=" << m_currentPkt);
  NS_LOG_LOGIC("Pkt UID is " << m_currentPkt->GetUid() << ")");

  m_channel->TransmitEnd();
  m_phyTxEndTrace(m_currentPkt);
  m_currentPkt = nullptr;

  NS_LOG_LOGIC("Schedule TransmitReadyEvent in "
               << m_tInterframeGap.As(Time::S));

  Simulator::Schedule(m_tInterframeGap, &CsmaNetDevice::TransmitReadyEvent,
                      this);
}

void CsmaNetDevice::TransmitReadyEvent() {
  NS_LOG_FUNCTION_NOARGS();

  NS_ASSERT_MSG(
      m_txMachineState == GAP,
      "CsmaNetDevice::TransmitReadyEvent(): Must be in interframe gap");
  m_txMachineState = READY;

  NS_ASSERT_MSG(!m_currentPkt,
                "CsmaNetDevice::TransmitReadyEvent(): m_currentPkt nonzero");

  if (m_queue->IsEmpty()) {
    return;
  } else {
    Ptr<Packet> packet = m_queue->Dequeue();
    NS_ASSERT_MSG(packet, "CsmaNetDevice::TransmitReadyEvent(): IsEmpty false "
                          "but no Packet on queue?");
    m_currentPkt = packet;
    m_snifferTrace(m_currentPkt);
    m_promiscSnifferTrace(m_currentPkt);
    TransmitStart();
  }
}

bool CsmaNetDevice::Attach(Ptr<CsmaChannel> ch) {
  NS_LOG_FUNCTION(this << &ch);

  m_channel = ch;

  m_deviceId = m_channel->Attach(this);

  m_bps = m_channel->GetDataRate();

  m_tInterframeGap = m_bps.CalculateBytesTxTime(96 / 8);

  NotifyLinkUp();
  return true;
}

void CsmaNetDevice::SetQueue(Ptr<Queue<Packet>> q) {
  NS_LOG_FUNCTION(q);
  m_queue = q;
}

void CsmaNetDevice::SetReceiveErrorModel(Ptr<ErrorModel> em) {
  NS_LOG_FUNCTION(em);
  m_receiveErrorModel = em;
}

void CsmaNetDevice::Receive(Ptr<Packet> packet,
                            Ptr<CsmaNetDevice> senderDevice) {
  NS_LOG_FUNCTION(packet << senderDevice);
  NS_LOG_LOGIC("UID is " << packet->GetUid());

  if (senderDevice == this) {
    return;
  }

  m_phyRxEndTrace(packet);

  if (!IsReceiveEnabled()) {
    m_phyRxDropTrace(packet);
    return;
  }

  if (m_receiveErrorModel && m_receiveErrorModel->IsCorrupt(packet)) {
    NS_LOG_LOGIC("Dropping pkt due to error model ");
    m_phyRxDropTrace(packet);
    return;
  }

  Ptr<Packet> originalPacket = packet->Copy();

  EthernetTrailer trailer;
  packet->RemoveTrailer(trailer);
  if (Node::ChecksumEnabled()) {
    trailer.EnableFcs(true);
  }

  bool crcGood = trailer.CheckFcs(packet);
  if (!crcGood) {
    NS_LOG_INFO("CRC error on Packet " << packet);
    m_phyRxDropTrace(packet);
    return;
  }

  EthernetHeader header(false);
  packet->RemoveHeader(header);

  NS_LOG_LOGIC("Pkt source is " << header.GetSource());
  NS_LOG_LOGIC("Pkt destination is " << header.GetDestination());

  uint16_t protocol;
  if (header.GetLengthType() <= 1500) {
    NS_ASSERT(packet->GetSize() >= header.GetLengthType());
    uint32_t padlen = packet->GetSize() - header.GetLengthType();
    NS_ASSERT(padlen <= 46);
    if (padlen > 0) {
      packet->RemoveAtEnd(padlen);
    }

    LlcSnapHeader llc;
    packet->RemoveHeader(llc);
    protocol = llc.GetType();
  } else {
    protocol = header.GetLengthType();
  }

  PacketType packetType;

  if (header.GetDestination().IsBroadcast()) {
    packetType = PACKET_BROADCAST;
  } else if (header.GetDestination().IsGroup()) {
    packetType = PACKET_MULTICAST;
  } else if (header.GetDestination() == m_address) {
    packetType = PACKET_HOST;
  } else {
    packetType = PACKET_OTHERHOST;
  }

  m_promiscSnifferTrace(originalPacket);
  if (!m_promiscRxCallback.IsNull()) {
    m_macPromiscRxTrace(originalPacket);
    m_promiscRxCallback(this, packet, protocol, header.GetSource(),
                        header.GetDestination(), packetType);
  }

  if (packetType != PACKET_OTHERHOST) {
    m_snifferTrace(originalPacket);
    m_macRxTrace(originalPacket);
    m_rxCallback(this, packet, protocol, header.GetSource());
  }
}

Ptr<Queue<Packet>> CsmaNetDevice::GetQueue() const {
  NS_LOG_FUNCTION_NOARGS();
  return m_queue;
}

void CsmaNetDevice::NotifyLinkUp() {
  NS_LOG_FUNCTION_NOARGS();
  m_linkUp = true;
  m_linkChangeCallbacks();
}

void CsmaNetDevice::SetIfIndex(const uint32_t index) {
  NS_LOG_FUNCTION(index);
  m_ifIndex = index;
}

uint32_t CsmaNetDevice::GetIfIndex() const {
  NS_LOG_FUNCTION_NOARGS();
  return m_ifIndex;
}

Ptr<Channel> CsmaNetDevice::GetChannel() const {
  NS_LOG_FUNCTION_NOARGS();
  return m_channel;
}

void CsmaNetDevice::SetAddress(Address address) {
  NS_LOG_FUNCTION_NOARGS();
  m_address = Mac48Address::ConvertFrom(address);
}

Address CsmaNetDevice::GetAddress() const {
  NS_LOG_FUNCTION_NOARGS();
  return m_address;
}

bool CsmaNetDevice::IsLinkUp() const {
  NS_LOG_FUNCTION_NOARGS();
  return m_linkUp;
}

void CsmaNetDevice::AddLinkChangeCallback(Callback<void> callback) {
  NS_LOG_FUNCTION(&callback);
  m_linkChangeCallbacks.ConnectWithoutContext(callback);
}

bool CsmaNetDevice::IsBroadcast() const {
  NS_LOG_FUNCTION_NOARGS();
  return true;
}

Address CsmaNetDevice::GetBroadcast() const {
  NS_LOG_FUNCTION_NOARGS();
  return Mac48Address("ff:ff:ff:ff:ff:ff");
}

bool CsmaNetDevice::IsMulticast() const {
  NS_LOG_FUNCTION_NOARGS();
  return true;
}

Address CsmaNetDevice::GetMulticast(Ipv4Address multicastGroup) const {
  NS_LOG_FUNCTION(multicastGroup);

  Mac48Address ad = Mac48Address::GetMulticast(multicastGroup);

  NS_LOG_LOGIC("multicast address is " << ad);

  return ad;
}

bool CsmaNetDevice::IsPointToPoint() const {
  NS_LOG_FUNCTION_NOARGS();
  return false;
}

bool CsmaNetDevice::IsBridge() const {
  NS_LOG_FUNCTION_NOARGS();
  return false;
}

bool CsmaNetDevice::Send(Ptr<Packet> packet, const Address &dest,
                         uint16_t protocolNumber) {
  NS_LOG_FUNCTION(packet << dest << protocolNumber);
  return SendFrom(packet, m_address, dest, protocolNumber);
}

bool CsmaNetDevice::SendFrom(Ptr<Packet> packet, const Address &src,
                             const Address &dest, uint16_t protocolNumber) {
  NS_LOG_FUNCTION(packet << src << dest << protocolNumber);
  NS_LOG_LOGIC("packet =" << packet);
  NS_LOG_LOGIC("UID is " << packet->GetUid() << ")");

  NS_ASSERT(IsLinkUp());

  if (!IsSendEnabled()) {
    m_macTxDropTrace(packet);
    return false;
  }

  Mac48Address destination = Mac48Address::ConvertFrom(dest);
  Mac48Address source = Mac48Address::ConvertFrom(src);
  AddHeader(packet, source, destination, protocolNumber);

  m_macTxTrace(packet);

  if (!m_queue->Enqueue(packet)) {
    m_macTxDropTrace(packet);
    return false;
  }

  if (m_txMachineState == READY) {
    if (!m_queue->IsEmpty()) {
      Ptr<Packet> packet = m_queue->Dequeue();
      NS_ASSERT_MSG(
          packet,
          "CsmaNetDevice::SendFrom(): IsEmpty false but no Packet on queue?");
      m_currentPkt = packet;
      m_promiscSnifferTrace(m_currentPkt);
      m_snifferTrace(m_currentPkt);
      TransmitStart();
    }
  }
  return true;
}

Ptr<Node> CsmaNetDevice::GetNode() const {
  NS_LOG_FUNCTION_NOARGS();
  return m_node;
}

void CsmaNetDevice::SetNode(Ptr<Node> node) {
  NS_LOG_FUNCTION(node);

  m_node = node;
}

bool CsmaNetDevice::NeedsArp() const {
  NS_LOG_FUNCTION_NOARGS();
  return true;
}

void CsmaNetDevice::SetReceiveCallback(NetDevice::ReceiveCallback cb) {
  NS_LOG_FUNCTION(&cb);
  m_rxCallback = cb;
}

Address CsmaNetDevice::GetMulticast(Ipv6Address addr) const {
  Mac48Address ad = Mac48Address::GetMulticast(addr);

  NS_LOG_LOGIC("MAC IPv6 multicast address is " << ad);
  return ad;
}

void CsmaNetDevice::SetPromiscReceiveCallback(
    NetDevice::PromiscReceiveCallback cb) {
  NS_LOG_FUNCTION(&cb);
  m_promiscRxCallback = cb;
}

bool CsmaNetDevice::SupportsSendFrom() const {
  NS_LOG_FUNCTION_NOARGS();
  return true;
}

int64_t CsmaNetDevice::AssignStreams(int64_t stream) {
  return m_backoff.AssignStreams(stream);
}

} // namespace ns3
