
#ifndef DSR_PASSIVEBUFF_H
#define DSR_PASSIVEBUFF_H

#include "ns3/ipv4-routing-protocol.h"
#include "ns3/simulator.h"

#include <vector>

namespace ns3 {
namespace dsr {
class DsrPassiveBuffEntry {
public:
  DsrPassiveBuffEntry(Ptr<const Packet> pa = nullptr,
                      Ipv4Address d = Ipv4Address(),
                      Ipv4Address s = Ipv4Address(),
                      Ipv4Address n = Ipv4Address(), uint16_t i = 0,
                      uint16_t f = 0, uint8_t seg = 0,
                      Time exp = Simulator::Now(), uint8_t p = 0)
      : m_packet(pa), m_dst(d), m_source(s), m_nextHop(n), m_identification(i),
        m_fragmentOffset(f), m_segsLeft(seg), m_expire(exp + Simulator::Now()),
        m_protocol(p) {}

  bool operator==(const DsrPassiveBuffEntry &o) const {
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

  uint16_t GetIdentification() const { return m_identification; }

  void SetIdentification(uint16_t i) { m_identification = i; }

  uint16_t GetFragmentOffset() const { return m_fragmentOffset; }

  void SetFragmentOffset(uint16_t f) { m_fragmentOffset = f; }

  uint8_t GetSegsLeft() const { return m_segsLeft; }

  void SetSegsLeft(uint8_t seg) { m_segsLeft = seg; }

  void SetExpireTime(Time exp) { m_expire = exp + Simulator::Now(); }

  Time GetExpireTime() const { return m_expire - Simulator::Now(); }

  void SetProtocol(uint8_t p) { m_protocol = p; }

  uint8_t GetProtocol() const { return m_protocol; }

private:
  Ptr<const Packet> m_packet;
  Ipv4Address m_dst;
  Ipv4Address m_source;
  Ipv4Address m_nextHop;
  uint16_t m_identification;
  uint16_t m_fragmentOffset;
  uint8_t m_segsLeft;
  Time m_expire;
  uint8_t m_protocol;
};

class DsrPassiveBuffer : public Object {
public:
  static TypeId GetTypeId();

  DsrPassiveBuffer();
  ~DsrPassiveBuffer() override;

  bool Enqueue(DsrPassiveBuffEntry &entry);
  bool Dequeue(Ipv4Address dst, DsrPassiveBuffEntry &entry);
  bool Find(Ipv4Address dst);
  bool AllEqual(DsrPassiveBuffEntry &entry);
  uint32_t GetSize();

  uint32_t GetMaxQueueLen() const { return m_maxLen; }

  void SetMaxQueueLen(uint32_t len) { m_maxLen = len; }

  Time GetPassiveBufferTimeout() const { return m_passiveBufferTimeout; }

  void SetPassiveBufferTimeout(Time t) { m_passiveBufferTimeout = t; }

private:
  std::vector<DsrPassiveBuffEntry> m_passiveBuffer;
  void Purge();
  void Drop(DsrPassiveBuffEntry en, std::string reason);
  void DropLink(DsrPassiveBuffEntry en, std::string reason);
  uint32_t m_maxLen;
  Time m_passiveBufferTimeout;

  static bool LinkEqual(DsrPassiveBuffEntry en,
                        const std::vector<Ipv4Address> link) {
    return ((en.GetSource() == link[0]) && (en.GetNextHop() == link[1]));
  }
};

} // namespace dsr
} // namespace ns3

#endif
