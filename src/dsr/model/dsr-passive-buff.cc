
#include "dsr-passive-buff.h"

#include "ns3/ipv4-route.h"
#include "ns3/log.h"
#include "ns3/socket.h"

#include <algorithm>
#include <functional>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("DsrPassiveBuffer");

namespace dsr {

NS_OBJECT_ENSURE_REGISTERED(DsrPassiveBuffer);

TypeId DsrPassiveBuffer::GetTypeId() {
  static TypeId tid = TypeId("ns3::dsr::DsrPassiveBuffer")
                          .SetParent<Object>()
                          .SetGroupName("Dsr")
                          .AddConstructor<DsrPassiveBuffer>();
  return tid;
}

DsrPassiveBuffer::DsrPassiveBuffer() {}

DsrPassiveBuffer::~DsrPassiveBuffer() {}

uint32_t DsrPassiveBuffer::GetSize() {
  Purge();
  return m_passiveBuffer.size();
}

bool DsrPassiveBuffer::Enqueue(DsrPassiveBuffEntry &entry) {
  Purge();
  for (auto i = m_passiveBuffer.begin(); i != m_passiveBuffer.end(); ++i) {

    if ((i->GetPacket()->GetUid() == entry.GetPacket()->GetUid()) &&
        (i->GetSource() == entry.GetSource()) &&
        (i->GetNextHop() == entry.GetNextHop()) &&
        (i->GetDestination() == entry.GetDestination()) &&
        (i->GetIdentification() == entry.GetIdentification()) &&
        (i->GetFragmentOffset() == entry.GetFragmentOffset()) &&
        (i->GetSegsLeft() == entry.GetSegsLeft() + 1)) {
      return false;
    }
  }

  entry.SetExpireTime(m_passiveBufferTimeout);
  if (m_passiveBuffer.size() >= m_maxLen) {
    Drop(m_passiveBuffer.front(), "Drop the most aged packet");
    m_passiveBuffer.erase(m_passiveBuffer.begin());
  }
  m_passiveBuffer.push_back(entry);
  return true;
}

bool DsrPassiveBuffer::AllEqual(DsrPassiveBuffEntry &entry) {
  for (auto i = m_passiveBuffer.begin(); i != m_passiveBuffer.end(); ++i) {

    if ((i->GetPacket()->GetUid() == entry.GetPacket()->GetUid()) &&
        (i->GetSource() == entry.GetSource()) &&
        (i->GetNextHop() == entry.GetNextHop()) &&
        (i->GetDestination() == entry.GetDestination()) &&
        (i->GetIdentification() == entry.GetIdentification()) &&
        (i->GetFragmentOffset() == entry.GetFragmentOffset()) &&
        (i->GetSegsLeft() == entry.GetSegsLeft() + 1)) {
      i = m_passiveBuffer.erase(i);
      return true;
    }
  }
  return false;
}

bool DsrPassiveBuffer::Dequeue(Ipv4Address dst, DsrPassiveBuffEntry &entry) {
  Purge();
  for (auto i = m_passiveBuffer.begin(); i != m_passiveBuffer.end(); ++i) {
    if (i->GetDestination() == dst) {
      entry = *i;
      i = m_passiveBuffer.erase(i);
      NS_LOG_DEBUG("Packet size while dequeuing "
                   << entry.GetPacket()->GetSize());
      return true;
    }
  }
  return false;
}

bool DsrPassiveBuffer::Find(Ipv4Address dst) {
  for (auto i = m_passiveBuffer.begin(); i != m_passiveBuffer.end(); ++i) {
    if (i->GetDestination() == dst) {
      NS_LOG_DEBUG("Found the packet");
      return true;
    }
  }
  return false;
}

struct IsExpired {
  bool operator()(const DsrPassiveBuffEntry &e) const {
    return (e.GetExpireTime() < Seconds(0));
  }
};

void DsrPassiveBuffer::Purge() {
  NS_LOG_DEBUG("The passive buffer size " << m_passiveBuffer.size());
  IsExpired pred;
  for (auto i = m_passiveBuffer.begin(); i != m_passiveBuffer.end(); ++i) {
    if (pred(*i)) {
      NS_LOG_DEBUG("Dropping Queue Packets");
      Drop(*i, "Drop out-dated packet ");
    }
  }
  m_passiveBuffer.erase(
      std::remove_if(m_passiveBuffer.begin(), m_passiveBuffer.end(), pred),
      m_passiveBuffer.end());
}

void DsrPassiveBuffer::Drop(DsrPassiveBuffEntry en, std::string reason) {
  NS_LOG_LOGIC(reason << en.GetPacket()->GetUid() << " "
                      << en.GetDestination());
}

void DsrPassiveBuffer::DropLink(DsrPassiveBuffEntry en, std::string reason) {
  NS_LOG_LOGIC(reason << en.GetPacket()->GetUid() << " " << en.GetSource()
                      << " " << en.GetNextHop());
}
} // namespace dsr
} // namespace ns3
