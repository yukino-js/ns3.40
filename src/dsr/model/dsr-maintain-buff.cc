
#include "dsr-maintain-buff.h"

#include "ns3/ipv4-route.h"
#include "ns3/log.h"
#include "ns3/socket.h"

#include <algorithm>
#include <functional>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("DsrMaintainBuffer");

namespace dsr {

uint32_t DsrMaintainBuffer::GetSize() {
  Purge();
  return m_maintainBuffer.size();
}

bool DsrMaintainBuffer::Enqueue(DsrMaintainBuffEntry &entry) {
  Purge();
  for (auto i = m_maintainBuffer.begin(); i != m_maintainBuffer.end(); ++i) {

    if ((i->GetNextHop() == entry.GetNextHop()) &&
        (i->GetOurAdd() == entry.GetOurAdd()) &&
        (i->GetSrc() == entry.GetSrc()) && (i->GetDst() == entry.GetDst()) &&
        (i->GetAckId() == entry.GetAckId()) &&
        (i->GetSegsLeft() == entry.GetSegsLeft())) {
      NS_LOG_DEBUG("Same maintenance entry found");
      return false;
    }
  }

  entry.SetExpireTime(m_maintainBufferTimeout);
  if (m_maintainBuffer.size() >= m_maxLen) {
    NS_LOG_DEBUG("Drop the most aged packet");
    m_maintainBuffer.erase(m_maintainBuffer.begin());
  }
  m_maintainBuffer.push_back(entry);
  return true;
}

void DsrMaintainBuffer::DropPacketWithNextHop(Ipv4Address nextHop) {
  NS_LOG_FUNCTION(this << nextHop);
  Purge();
  NS_LOG_INFO("Drop Packet With next hop " << nextHop);

  auto new_end =
      std::remove_if(m_maintainBuffer.begin(), m_maintainBuffer.end(),
                     [&](const DsrMaintainBuffEntry &en) {
                       return en.GetNextHop() == nextHop;
                     });
  m_maintainBuffer.erase(new_end, m_maintainBuffer.end());
}

bool DsrMaintainBuffer::Dequeue(Ipv4Address nextHop,
                                DsrMaintainBuffEntry &entry) {
  Purge();
  for (auto i = m_maintainBuffer.begin(); i != m_maintainBuffer.end(); ++i) {
    if (i->GetNextHop() == nextHop) {
      entry = *i;
      i = m_maintainBuffer.erase(i);
      NS_LOG_DEBUG("Packet size while dequeuing "
                   << entry.GetPacket()->GetSize());
      return true;
    }
  }
  return false;
}

bool DsrMaintainBuffer::Find(Ipv4Address nextHop) {
  for (auto i = m_maintainBuffer.begin(); i != m_maintainBuffer.end(); ++i) {
    if (i->GetNextHop() == nextHop) {
      NS_LOG_DEBUG("Found the packet in maintenance buffer");
      return true;
    }
  }
  return false;
}

bool DsrMaintainBuffer::AllEqual(DsrMaintainBuffEntry &entry) {
  for (auto i = m_maintainBuffer.begin(); i != m_maintainBuffer.end(); ++i) {

    if ((i->GetOurAdd() == entry.GetOurAdd()) &&
        (i->GetNextHop() == entry.GetNextHop()) &&
        (i->GetSrc() == entry.GetSrc()) && (i->GetDst() == entry.GetDst()) &&
        (i->GetAckId() == entry.GetAckId()) &&
        (i->GetSegsLeft() == entry.GetSegsLeft())) {
      i = m_maintainBuffer.erase(i);
      return true;
    }
  }
  return false;
}

bool DsrMaintainBuffer::NetworkEqual(DsrMaintainBuffEntry &entry) {
  for (auto i = m_maintainBuffer.begin(); i != m_maintainBuffer.end(); ++i) {

    if ((i->GetOurAdd() == entry.GetOurAdd()) &&
        (i->GetNextHop() == entry.GetNextHop()) &&
        (i->GetSrc() == entry.GetSrc()) && (i->GetDst() == entry.GetDst()) &&
        (i->GetAckId() == entry.GetAckId())) {
      i = m_maintainBuffer.erase(i);
      return true;
    }
  }
  return false;
}

bool DsrMaintainBuffer::PromiscEqual(DsrMaintainBuffEntry &entry) {
  NS_LOG_DEBUG("The maintenance buffer size " << m_maintainBuffer.size());
  for (auto i = m_maintainBuffer.begin(); i != m_maintainBuffer.end(); ++i) {

    if ((i->GetSrc() == entry.GetSrc()) && (i->GetDst() == entry.GetDst()) &&
        (i->GetSegsLeft() == entry.GetSegsLeft()) &&
        (i->GetAckId() == entry.GetAckId())) {
      i = m_maintainBuffer.erase(i);
      return true;
    }
  }
  return false;
}

bool DsrMaintainBuffer::LinkEqual(DsrMaintainBuffEntry &entry) {
  NS_LOG_DEBUG("The maintenance buffer size " << m_maintainBuffer.size());
  for (auto i = m_maintainBuffer.begin(); i != m_maintainBuffer.end(); ++i) {

    if ((i->GetSrc() == entry.GetSrc()) && (i->GetDst() == entry.GetDst()) &&
        (i->GetOurAdd() == entry.GetOurAdd()) &&
        (i->GetNextHop() == entry.GetNextHop())) {
      i = m_maintainBuffer.erase(i);
      return true;
    }
  }
  return false;
}

struct IsExpired {
  bool operator()(const DsrMaintainBuffEntry &e) const {
    return (e.GetExpireTime() < Seconds(0));
  }
};

void DsrMaintainBuffer::Purge() {
  NS_LOG_DEBUG("Purging Maintenance Buffer");
  IsExpired pred;
  m_maintainBuffer.erase(
      std::remove_if(m_maintainBuffer.begin(), m_maintainBuffer.end(), pred),
      m_maintainBuffer.end());
}

} // namespace dsr
} // namespace ns3
