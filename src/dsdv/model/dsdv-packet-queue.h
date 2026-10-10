
#ifndef DSDV_PACKETQUEUE_H
#define DSDV_PACKETQUEUE_H

#include "ns3/ipv4-routing-protocol.h"
#include "ns3/simulator.h"

#include <vector>

namespace ns3 {
namespace dsdv {
class QueueEntry {
public:
  typedef Ipv4RoutingProtocol::UnicastForwardCallback UnicastForwardCallback;
  typedef Ipv4RoutingProtocol::ErrorCallback ErrorCallback;

  QueueEntry(Ptr<const Packet> pa = nullptr, const Ipv4Header &h = Ipv4Header(),
             UnicastForwardCallback ucb = UnicastForwardCallback(),
             ErrorCallback ecb = ErrorCallback())
      : m_packet(pa), m_header(h), m_ucb(ucb), m_ecb(ecb),
        m_expire(Seconds(0)) {}

  bool operator==(const QueueEntry &o) const {
    return ((m_packet == o.m_packet) &&
            (m_header.GetDestination() == o.m_header.GetDestination()) &&
            (m_expire == o.m_expire));
  }

  UnicastForwardCallback GetUnicastForwardCallback() const { return m_ucb; }

  void SetUnicastForwardCallback(UnicastForwardCallback ucb) { m_ucb = ucb; }

  ErrorCallback GetErrorCallback() const { return m_ecb; }

  void SetErrorCallback(ErrorCallback ecb) { m_ecb = ecb; }

  Ptr<const Packet> GetPacket() const { return m_packet; }

  void SetPacket(Ptr<const Packet> p) { m_packet = p; }

  Ipv4Header GetIpv4Header() const { return m_header; }

  void SetIpv4Header(Ipv4Header h) { m_header = h; }

  void SetExpireTime(Time exp) { m_expire = exp + Simulator::Now(); }

  Time GetExpireTime() const { return m_expire - Simulator::Now(); }

private:
  Ptr<const Packet> m_packet;
  Ipv4Header m_header;
  UnicastForwardCallback m_ucb;
  ErrorCallback m_ecb;
  Time m_expire;
};

class PacketQueue {
public:
  PacketQueue() {}

  bool Enqueue(QueueEntry &entry);
  bool Dequeue(Ipv4Address dst, QueueEntry &entry);
  void DropPacketWithDst(Ipv4Address dst);
  bool Find(Ipv4Address dst);
  uint32_t GetCountForPacketsWithDst(Ipv4Address dst);
  uint32_t GetSize();

  uint32_t GetMaxQueueLen() const { return m_maxLen; }

  void SetMaxQueueLen(uint32_t len) { m_maxLen = len; }

  uint32_t GetMaxPacketsPerDst() const { return m_maxLenPerDst; }

  void SetMaxPacketsPerDst(uint32_t len) { m_maxLenPerDst = len; }

  Time GetQueueTimeout() const { return m_queueTimeout; }

  void SetQueueTimeout(Time t) { m_queueTimeout = t; }

private:
  std::vector<QueueEntry> m_queue;
  void Purge();
  void Drop(QueueEntry en, std::string reason);
  uint32_t m_maxLen;
  uint32_t m_maxLenPerDst;
  Time m_queueTimeout;
};
} // namespace dsdv
} // namespace ns3
#endif
