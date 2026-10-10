
#ifndef DSR_SENDBUFF_H
#define DSR_SENDBUFF_H

#include "ns3/ipv4-routing-protocol.h"
#include "ns3/simulator.h"

#include <vector>

namespace ns3 {
namespace dsr {
class DsrSendBuffEntry {
public:
  DsrSendBuffEntry(Ptr<const Packet> pa = nullptr,
                   Ipv4Address d = Ipv4Address(), Time exp = Simulator::Now(),
                   uint8_t p = 0)
      : m_packet(pa), m_dst(d), m_expire(exp + Simulator::Now()),
        m_protocol(p) {}

  bool operator==(const DsrSendBuffEntry &o) const {
    return ((m_packet == o.m_packet) && (m_dst == o.m_dst) &&
            (m_expire == o.m_expire));
  }

  Ptr<const Packet> GetPacket() const { return m_packet; }

  void SetPacket(Ptr<const Packet> p) { m_packet = p; }

  Ipv4Address GetDestination() const { return m_dst; }

  void SetDestination(Ipv4Address d) { m_dst = d; }

  void SetExpireTime(Time exp) { m_expire = exp + Simulator::Now(); }

  Time GetExpireTime() const { return m_expire - Simulator::Now(); }

  void SetProtocol(uint8_t p) { m_protocol = p; }

  uint8_t GetProtocol() const { return m_protocol; }

private:
  Ptr<const Packet> m_packet;
  Ipv4Address m_dst;
  Time m_expire;
  uint8_t m_protocol;
};

class DsrSendBuffer {
public:
  DsrSendBuffer() {}

  bool Enqueue(DsrSendBuffEntry &entry);
  bool Dequeue(Ipv4Address dst, DsrSendBuffEntry &entry);
  void DropPacketWithDst(Ipv4Address dst);
  bool Find(Ipv4Address dst);
  uint32_t GetSize();

  uint32_t GetMaxQueueLen() const { return m_maxLen; }

  void SetMaxQueueLen(uint32_t len) { m_maxLen = len; }

  Time GetSendBufferTimeout() const { return m_sendBufferTimeout; }

  void SetSendBufferTimeout(Time t) { m_sendBufferTimeout = t; }

  std::vector<DsrSendBuffEntry> &GetBuffer() { return m_sendBuffer; }

private:
  std::vector<DsrSendBuffEntry> m_sendBuffer;
  void Purge();

  void Drop(DsrSendBuffEntry en, std::string reason);

  uint32_t m_maxLen;
  Time m_sendBufferTimeout;
};

} // namespace dsr
} // namespace ns3

#endif
