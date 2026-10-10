
#ifndef DSR_ERRORBUFF_H
#define DSR_ERRORBUFF_H

#include "ns3/ipv4-routing-protocol.h"
#include "ns3/simulator.h"

#include <vector>

namespace ns3 {
namespace dsr {
class DsrErrorBuffEntry {
public:
  DsrErrorBuffEntry(Ptr<const Packet> pa = nullptr,
                    Ipv4Address d = Ipv4Address(),
                    Ipv4Address s = Ipv4Address(),
                    Ipv4Address n = Ipv4Address(), Time exp = Simulator::Now(),
                    uint8_t p = 0)
      : m_packet(pa), m_dst(d), m_source(s), m_nextHop(n),
        m_expire(exp + Simulator::Now()), m_protocol(p) {}

  bool operator==(const DsrErrorBuffEntry &o) const {
    return ((m_packet == o.m_packet) && (m_source == o.m_source) &&
            (m_nextHop == o.m_nextHop) && (m_dst == o.m_dst) &&
            (m_expire == o.m_expire));
  }

  Ptr<const Packet> GetPacket() const { return m_packet; }

  void SetPacket(Ptr<const Packet> p) { m_packet = p; }

  Ipv4Address GetDestination() const { return m_dst; }

  void SetDestination(Ipv4Address d) { m_dst = d; }

  Ipv4Address GetSource() const { return m_source; }

  void SetSource(Ipv4Address s) { m_source = s; }

  Ipv4Address GetNextHop() const { return m_nextHop; }

  void SetNextHop(Ipv4Address n) { m_nextHop = n; }

  void SetExpireTime(Time exp) { m_expire = exp + Simulator::Now(); }

  Time GetExpireTime() const { return m_expire - Simulator::Now(); }

  void SetProtocol(uint8_t p) { m_protocol = p; }

  uint8_t GetProtocol() const { return m_protocol; }

private:
  Ptr<const Packet> m_packet;
  Ipv4Address m_dst;
  Ipv4Address m_source;
  Ipv4Address m_nextHop;
  Time m_expire;
  uint8_t m_protocol;
};

class DsrErrorBuffer {
public:
  DsrErrorBuffer() {}

  bool Enqueue(DsrErrorBuffEntry &entry);
  bool Dequeue(Ipv4Address dst, DsrErrorBuffEntry &entry);
  void DropPacketForErrLink(Ipv4Address source, Ipv4Address nextHop);
  bool Find(Ipv4Address dst);
  uint32_t GetSize();

  uint32_t GetMaxQueueLen() const { return m_maxLen; }

  void SetMaxQueueLen(uint32_t len) { m_maxLen = len; }

  Time GetErrorBufferTimeout() const { return m_errorBufferTimeout; }

  void SetErrorBufferTimeout(Time t) { m_errorBufferTimeout = t; }

  std::vector<DsrErrorBuffEntry> &GetBuffer() { return m_errorBuffer; }

private:
  std::vector<DsrErrorBuffEntry> m_errorBuffer;
  void Purge();
  void Drop(DsrErrorBuffEntry en, std::string reason);
  void DropLink(DsrErrorBuffEntry en, std::string reason);
  uint32_t m_maxLen;
  Time m_errorBufferTimeout;
};

} // namespace dsr
} // namespace ns3

#endif
