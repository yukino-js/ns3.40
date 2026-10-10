
#include "olsr-header.h"

#include "ns3/assert.h"
#include "ns3/log.h"

#include <cmath>

#define IPV4_ADDRESS_SIZE 4
#define OLSR_MSG_HEADER_SIZE 12
#define OLSR_PKT_HEADER_SIZE 4

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("OlsrHeader");

namespace olsr {

#define OLSR_C 0.0625

uint8_t SecondsToEmf(double seconds) {
  int a;
  int b = 0;

  NS_ASSERT_MSG(seconds >= OLSR_C,
                "SecondsToEmf - Can not convert a value less than OLSR_C");

  for (b = 1; (seconds / OLSR_C) >= (1 << b); ++b) {
  }
  NS_ASSERT((seconds / OLSR_C) < (1 << b));
  b--;
  NS_ASSERT((seconds / OLSR_C) >= (1 << b));

  double tmp = 16 * (seconds / (OLSR_C * (1 << b)) - 1);

  a = (int)std::ceil(tmp - 0.5);

  if (a == 16) {
    b += 1;
    a = 0;
  }

  NS_ASSERT(a >= 0 && a < 16);
  NS_ASSERT(b >= 0 && b < 16);

  return (uint8_t)((a << 4) | b);
}

double EmfToSeconds(uint8_t olsrFormat) {
  int a = (olsrFormat >> 4);
  int b = (olsrFormat & 0xf);
  return OLSR_C * (1 + a / 16.0) * (1 << b);
}

NS_OBJECT_ENSURE_REGISTERED(PacketHeader);

PacketHeader::PacketHeader() {}

PacketHeader::~PacketHeader() {}

TypeId PacketHeader::GetTypeId() {
  static TypeId tid = TypeId("ns3::olsr::PacketHeader")
                          .SetParent<Header>()
                          .SetGroupName("Olsr")
                          .AddConstructor<PacketHeader>();
  return tid;
}

TypeId PacketHeader::GetInstanceTypeId() const { return GetTypeId(); }

uint32_t PacketHeader::GetSerializedSize() const {
  return OLSR_PKT_HEADER_SIZE;
}

void PacketHeader::Print(std::ostream &os) const {
  os << "len: " << m_packetLength << " seqNo: " << m_packetSequenceNumber;
}

void PacketHeader::Serialize(Buffer::Iterator start) const {
  Buffer::Iterator i = start;
  i.WriteHtonU16(m_packetLength);
  i.WriteHtonU16(m_packetSequenceNumber);
}

uint32_t PacketHeader::Deserialize(Buffer::Iterator start) {
  Buffer::Iterator i = start;
  m_packetLength = i.ReadNtohU16();
  m_packetSequenceNumber = i.ReadNtohU16();
  return GetSerializedSize();
}

NS_OBJECT_ENSURE_REGISTERED(MessageHeader);

MessageHeader::MessageHeader() : m_messageType(MessageHeader::MessageType(0)) {}

MessageHeader::~MessageHeader() {}

TypeId MessageHeader::GetTypeId() {
  static TypeId tid = TypeId("ns3::olsr::MessageHeader")
                          .SetParent<Header>()
                          .SetGroupName("Olsr")
                          .AddConstructor<MessageHeader>();
  return tid;
}

TypeId MessageHeader::GetInstanceTypeId() const { return GetTypeId(); }

uint32_t MessageHeader::GetSerializedSize() const {
  uint32_t size = OLSR_MSG_HEADER_SIZE;
  switch (m_messageType) {
  case MID_MESSAGE:
    size += m_message.mid.GetSerializedSize();
    break;
  case HELLO_MESSAGE:
    NS_LOG_DEBUG("Hello Message Size: " << size << " + "
                                        << m_message.hello.GetSerializedSize());
    size += m_message.hello.GetSerializedSize();
    break;
  case TC_MESSAGE:
    size += m_message.tc.GetSerializedSize();
    break;
  case HNA_MESSAGE:
    size += m_message.hna.GetSerializedSize();
    break;
  default:
    NS_ASSERT(false);
  }
  return size;
}

void MessageHeader::Print(std::ostream &os) const {
  switch (m_messageType) {
  case HELLO_MESSAGE:
    os << "type: HELLO";
    break;
  case TC_MESSAGE:
    os << "type: TC";
    break;
  case MID_MESSAGE:
    os << "type: MID";
    break;
  case HNA_MESSAGE:
    os << "type: HNA";
    break;
  }

  os << " TTL: " << +m_timeToLive;
  os << " Orig: " << m_originatorAddress;
  os << " SeqNo: " << m_messageSequenceNumber;
  os << " Validity: " << +m_vTime;
  os << " Hop count: " << +m_hopCount;
  os << " Size: " << m_messageSize;

  switch (m_messageType) {
  case MID_MESSAGE:
    m_message.mid.Print(os);
    break;
  case HELLO_MESSAGE:
    m_message.hello.Print(os);
    break;
  case TC_MESSAGE:
    m_message.tc.Print(os);
    break;
  case HNA_MESSAGE:
    m_message.hna.Print(os);
    break;
  default:
    NS_ASSERT(false);
  }
}

void MessageHeader::Serialize(Buffer::Iterator start) const {
  Buffer::Iterator i = start;
  i.WriteU8(m_messageType);
  i.WriteU8(m_vTime);
  i.WriteHtonU16(GetSerializedSize());
  i.WriteHtonU32(m_originatorAddress.Get());
  i.WriteU8(m_timeToLive);
  i.WriteU8(m_hopCount);
  i.WriteHtonU16(m_messageSequenceNumber);

  switch (m_messageType) {
  case MID_MESSAGE:
    m_message.mid.Serialize(i);
    break;
  case HELLO_MESSAGE:
    m_message.hello.Serialize(i);
    break;
  case TC_MESSAGE:
    m_message.tc.Serialize(i);
    break;
  case HNA_MESSAGE:
    m_message.hna.Serialize(i);
    break;
  default:
    NS_ASSERT(false);
  }
}

uint32_t MessageHeader::Deserialize(Buffer::Iterator start) {
  uint32_t size;
  Buffer::Iterator i = start;
  m_messageType = (MessageType)i.ReadU8();
  NS_ASSERT(m_messageType >= HELLO_MESSAGE && m_messageType <= HNA_MESSAGE);
  m_vTime = i.ReadU8();
  m_messageSize = i.ReadNtohU16();
  m_originatorAddress = Ipv4Address(i.ReadNtohU32());
  m_timeToLive = i.ReadU8();
  m_hopCount = i.ReadU8();
  m_messageSequenceNumber = i.ReadNtohU16();
  size = OLSR_MSG_HEADER_SIZE;
  switch (m_messageType) {
  case MID_MESSAGE:
    size += m_message.mid.Deserialize(i, m_messageSize - OLSR_MSG_HEADER_SIZE);
    break;
  case HELLO_MESSAGE:
    size +=
        m_message.hello.Deserialize(i, m_messageSize - OLSR_MSG_HEADER_SIZE);
    break;
  case TC_MESSAGE:
    size += m_message.tc.Deserialize(i, m_messageSize - OLSR_MSG_HEADER_SIZE);
    break;
  case HNA_MESSAGE:
    size += m_message.hna.Deserialize(i, m_messageSize - OLSR_MSG_HEADER_SIZE);
    break;
  default:
    NS_ASSERT(false);
  }
  return size;
}

uint32_t MessageHeader::Mid::GetSerializedSize() const {
  return this->interfaceAddresses.size() * IPV4_ADDRESS_SIZE;
}

void MessageHeader::Mid::Print(std::ostream &os) const {
  bool first = true;
  os << " [";
  for (const auto &iAddr : interfaceAddresses) {
    if (first) {
      first = false;
    } else {
      os << ", ";
    }
    os << iAddr;
  }
  os << "]";
}

void MessageHeader::Mid::Serialize(Buffer::Iterator start) const {
  Buffer::Iterator i = start;

  for (auto iter = this->interfaceAddresses.begin();
       iter != this->interfaceAddresses.end(); iter++) {
    i.WriteHtonU32(iter->Get());
  }
}

uint32_t MessageHeader::Mid::Deserialize(Buffer::Iterator start,
                                         uint32_t messageSize) {
  Buffer::Iterator i = start;

  this->interfaceAddresses.clear();
  NS_ASSERT(messageSize % IPV4_ADDRESS_SIZE == 0);

  int numAddresses = messageSize / IPV4_ADDRESS_SIZE;
  this->interfaceAddresses.erase(this->interfaceAddresses.begin(),
                                 this->interfaceAddresses.end());
  for (int n = 0; n < numAddresses; ++n) {
    this->interfaceAddresses.emplace_back(i.ReadNtohU32());
  }
  return GetSerializedSize();
}

uint32_t MessageHeader::Hello::GetSerializedSize() const {
  uint32_t size = 4;
  for (auto iter = this->linkMessages.begin(); iter != this->linkMessages.end();
       iter++) {
    const LinkMessage &lm = *iter;
    size += 4;
    size += IPV4_ADDRESS_SIZE * lm.neighborInterfaceAddresses.size();
  }
  return size;
}

void MessageHeader::Hello::Print(std::ostream &os) const {
  os << " Interval: " << +hTime << " (" << EmfToSeconds(hTime) << "s)";
  os << " Willingness: " << willingness;

  for (const auto &ilinkMessage : linkMessages) {
    const LinkMessage &lm = ilinkMessage;
    os << " Link code: " << +(lm.linkCode);
    os << " [";
    bool first = true;
    for (const auto &neigh_iter : lm.neighborInterfaceAddresses) {
      if (first) {
        first = false;
      } else {
        os << ", ";
      }
      os << neigh_iter;
    }
    os << "]";
  }
}

void MessageHeader::Hello::Serialize(Buffer::Iterator start) const {
  Buffer::Iterator i = start;

  i.WriteU16(0);
  i.WriteU8(this->hTime);
  i.WriteU8(static_cast<uint8_t>(this->willingness));

  for (auto iter = this->linkMessages.begin(); iter != this->linkMessages.end();
       iter++) {
    const LinkMessage &lm = *iter;

    i.WriteU8(lm.linkCode);
    i.WriteU8(0);

    i.WriteHtonU16(4 +
                   lm.neighborInterfaceAddresses.size() * IPV4_ADDRESS_SIZE);

    for (auto neigh_iter = lm.neighborInterfaceAddresses.begin();
         neigh_iter != lm.neighborInterfaceAddresses.end(); neigh_iter++) {
      i.WriteHtonU32(neigh_iter->Get());
    }
  }
}

uint32_t MessageHeader::Hello::Deserialize(Buffer::Iterator start,
                                           uint32_t messageSize) {
  Buffer::Iterator i = start;

  NS_ASSERT(messageSize >= 4);

  this->linkMessages.clear();

  uint16_t helloSizeLeft = messageSize;

  i.ReadNtohU16();
  this->hTime = i.ReadU8();
  this->willingness = Willingness(i.ReadU8());

  helloSizeLeft -= 4;

  while (helloSizeLeft) {
    LinkMessage lm;
    NS_ASSERT(helloSizeLeft >= 4);
    lm.linkCode = i.ReadU8();
    i.ReadU8();
    uint16_t lmSize = i.ReadNtohU16();
    NS_ASSERT((lmSize - 4) % IPV4_ADDRESS_SIZE == 0);
    for (int n = (lmSize - 4) / IPV4_ADDRESS_SIZE; n; --n) {
      lm.neighborInterfaceAddresses.emplace_back(i.ReadNtohU32());
    }
    helloSizeLeft -= lmSize;
    this->linkMessages.push_back(lm);
  }

  return messageSize;
}

uint32_t MessageHeader::Tc::GetSerializedSize() const {
  return 4 + this->neighborAddresses.size() * IPV4_ADDRESS_SIZE;
}

void MessageHeader::Tc::Print(std::ostream &os) const {
  os << " Adv. SeqNo: " << ansn;
  os << " [";
  bool first = true;
  for (const auto &iAddr : neighborAddresses) {
    if (first) {
      first = false;
    } else {
      os << ", ";
    }
    os << iAddr;
  }
  os << "]";
}

void MessageHeader::Tc::Serialize(Buffer::Iterator start) const {
  Buffer::Iterator i = start;

  i.WriteHtonU16(this->ansn);
  i.WriteHtonU16(0);

  for (auto iter = this->neighborAddresses.begin();
       iter != this->neighborAddresses.end(); iter++) {
    i.WriteHtonU32(iter->Get());
  }
}

uint32_t MessageHeader::Tc::Deserialize(Buffer::Iterator start,
                                        uint32_t messageSize) {
  Buffer::Iterator i = start;

  this->neighborAddresses.clear();
  NS_ASSERT(messageSize >= 4);

  this->ansn = i.ReadNtohU16();
  i.ReadNtohU16();

  NS_ASSERT((messageSize - 4) % IPV4_ADDRESS_SIZE == 0);
  int numAddresses = (messageSize - 4) / IPV4_ADDRESS_SIZE;
  this->neighborAddresses.clear();
  for (int n = 0; n < numAddresses; ++n) {
    this->neighborAddresses.emplace_back(i.ReadNtohU32());
  }

  return messageSize;
}

uint32_t MessageHeader::Hna::GetSerializedSize() const {
  return 2 * this->associations.size() * IPV4_ADDRESS_SIZE;
}

void MessageHeader::Hna::Print(std::ostream &os) const {
  os << " [";
  bool first = true;
  for (const auto &iAssoc : associations) {
    if (first) {
      first = false;
    } else {
      os << ", ";
    }
    os << iAssoc.address << "/" << iAssoc.mask.GetPrefixLength();
  }
  os << "]";
}

void MessageHeader::Hna::Serialize(Buffer::Iterator start) const {
  Buffer::Iterator i = start;

  for (size_t n = 0; n < this->associations.size(); ++n) {
    i.WriteHtonU32(this->associations[n].address.Get());
    i.WriteHtonU32(this->associations[n].mask.Get());
  }
}

uint32_t MessageHeader::Hna::Deserialize(Buffer::Iterator start,
                                         uint32_t messageSize) {
  Buffer::Iterator i = start;

  NS_ASSERT(messageSize % (IPV4_ADDRESS_SIZE * 2) == 0);
  int numAddresses = messageSize / IPV4_ADDRESS_SIZE / 2;
  this->associations.clear();
  for (int n = 0; n < numAddresses; ++n) {
    Ipv4Address address(i.ReadNtohU32());
    Ipv4Mask mask(i.ReadNtohU32());
    this->associations.push_back((Association){address, mask});
  }
  return messageSize;
}

} // namespace olsr
} // namespace ns3
