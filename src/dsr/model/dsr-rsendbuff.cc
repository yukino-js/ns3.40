
#include "dsr-rsendbuff.h"

#include "ns3/ipv4-route.h"
#include "ns3/log.h"
#include "ns3/socket.h"

#include <algorithm>
#include <functional>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("DsrSendBuffer");

namespace dsr {

uint32_t DsrSendBuffer::GetSize() {
  Purge();
  return m_sendBuffer.size();
}

bool DsrSendBuffer::Enqueue(DsrSendBuffEntry &entry) {
  Purge();
  for (auto i = m_sendBuffer.begin(); i != m_sendBuffer.end(); ++i) {

    if ((i->GetPacket()->GetUid() == entry.GetPacket()->GetUid()) &&
        (i->GetDestination() == entry.GetDestination())) {
      return false;
    }
  }

  entry.SetExpireTime(m_sendBufferTimeout);
  if (m_sendBuffer.size() >= m_maxLen) {
    Drop(m_sendBuffer.front(), "Drop the most aged packet");
    m_sendBuffer.erase(m_sendBuffer.begin());
  }
  m_sendBuffer.push_back(entry);
  return true;
}

void DsrSendBuffer::DropPacketWithDst(Ipv4Address dst) {
  NS_LOG_FUNCTION(this << dst);
  Purge();
  for (auto i = m_sendBuffer.begin(); i != m_sendBuffer.end(); ++i) {
    if (i->GetDestination() == dst) {
      Drop(*i, "DropPacketWithDst");
    }
  }
  auto new_end = std::remove_if(
      m_sendBuffer.begin(), m_sendBuffer.end(),
      [&](const DsrSendBuffEntry &en) { return en.GetDestination() == dst; });
  m_sendBuffer.erase(new_end, m_sendBuffer.end());
}

bool DsrSendBuffer::Dequeue(Ipv4Address dst, DsrSendBuffEntry &entry) {
  Purge();
  for (auto i = m_sendBuffer.begin(); i != m_sendBuffer.end(); ++i) {
    if (i->GetDestination() == dst) {
      entry = *i;
      i = m_sendBuffer.erase(i);
      NS_LOG_DEBUG("Packet size while dequeuing "
                   << entry.GetPacket()->GetSize());
      return true;
    }
  }
  return false;
}

bool DsrSendBuffer::Find(Ipv4Address dst) {
  for (auto i = m_sendBuffer.begin(); i != m_sendBuffer.end(); ++i) {
    if (i->GetDestination() == dst) {
      NS_LOG_DEBUG("Found the packet");
      return true;
    }
  }
  return false;
}

struct IsExpired {
  bool operator()(const DsrSendBuffEntry &e) const {
    return (e.GetExpireTime() < Seconds(0));
  }
};

void DsrSendBuffer::Purge() {
  NS_LOG_INFO("The send buffer size " << m_sendBuffer.size());
  IsExpired pred;
  for (auto i = m_sendBuffer.begin(); i != m_sendBuffer.end(); ++i) {
    if (pred(*i)) {
      NS_LOG_DEBUG("Dropping Queue Packets");
      Drop(*i, "Drop out-dated packet ");
    }
  }
  m_sendBuffer.erase(
      std::remove_if(m_sendBuffer.begin(), m_sendBuffer.end(), pred),
      m_sendBuffer.end());
}

void DsrSendBuffer::Drop(DsrSendBuffEntry en, std::string reason) {
  NS_LOG_LOGIC(reason << en.GetPacket()->GetUid() << " "
                      << en.GetDestination());
}
} // namespace dsr
} // namespace ns3
