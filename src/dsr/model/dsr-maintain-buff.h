
#ifndef DSR_MAINTAIN_BUFF_H
#define DSR_MAINTAIN_BUFF_H

#include "dsr-option-header.h"

#include "ns3/ipv4-header.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/simulator.h"

#include <vector>

namespace ns3 {
namespace dsr {
struct LinkKey {
  Ipv4Address m_source;
  Ipv4Address m_destination;
  Ipv4Address m_ourAdd;
  Ipv4Address m_nextHop;

  bool operator<(const LinkKey &o) const {
    if (m_source < o.m_source) {
      return true;
    }
    if (o.m_source < m_source) {
      return false;
    }
    if (m_destination < o.m_destination) {
      return true;
    }
    if (o.m_destination < m_destination) {
      return false;
    }
    if (m_ourAdd < o.m_ourAdd) {
      return true;
    }
    if (o.m_ourAdd < m_ourAdd) {
      return false;
    }
    if (m_nextHop < o.m_nextHop) {
      return true;
    }
    if (o.m_nextHop < m_nextHop) {
      return false;
    }
    return false;
  }
};

struct NetworkKey {
  uint16_t m_ackId;
  Ipv4Address m_ourAdd;
  Ipv4Address m_nextHop;
  Ipv4Address m_source;
  Ipv4Address m_destination;

  bool operator<(const NetworkKey &o) const {
    if (m_ackId < o.m_ackId) {
      return true;
    }
    if (o.m_ackId < m_ackId) {
      return false;
    }
    if (m_source < o.m_source) {
      return true;
    }
    if (o.m_source < m_source) {
      return false;
    }
    if (m_destination < o.m_destination) {
      return true;
    }
    if (o.m_destination < m_destination) {
      return false;
    }
    if (m_ourAdd < o.m_ourAdd) {
      return true;
    }
    if (o.m_ourAdd < m_ourAdd) {
      return false;
    }
    if (m_nextHop < o.m_nextHop) {
      return true;
    }
    if (o.m_nextHop < m_nextHop) {
      return false;
    }
    return false;
  }
};

struct PassiveKey {
  uint16_t m_ackId;
  Ipv4Address m_source;
  Ipv4Address m_destination;
  uint8_t m_segsLeft;

  bool operator<(const PassiveKey &o) const {
    if (m_ackId < o.m_ackId) {
      return true;
    }
    if (o.m_ackId < m_ackId) {
      return false;
    }
    if (m_source < o.m_source) {
      return true;
    }
    if (o.m_source < m_source) {
      return false;
    }
    if (m_destination < o.m_destination) {
      return true;
    }
    if (o.m_destination < m_destination) {
      return false;
    }
    if (m_segsLeft < o.m_segsLeft) {
      return true;
    }
    if (o.m_segsLeft < m_segsLeft) {
      return false;
    }
    return false;
  }
};

class DsrMaintainBuffEntry {
public:
  DsrMaintainBuffEntry(Ptr<const Packet> packet = nullptr,
                       Ipv4Address ourAddress = Ipv4Address(),
                       Ipv4Address nextHop = Ipv4Address(),
                       Ipv4Address src = Ipv4Address(),
                       Ipv4Address dst = Ipv4Address(), uint16_t ackId = 0,
                       uint8_t segsLeft = 0, Time expire = Simulator::Now())
      : m_packet(packet), m_ourAdd(ourAddress), m_nextHop(nextHop), m_src(src),
        m_dst(dst), m_ackId(ackId), m_segsLeft(segsLeft),
        m_expire(expire + Simulator::Now()) {}

  Ptr<const Packet> GetPacket() const { return m_packet; }

  void SetPacket(Ptr<const Packet> p) { m_packet = p; }

  Ipv4Address GetOurAdd() const { return m_ourAdd; }

  void SetOurAdd(Ipv4Address us) { m_ourAdd = us; }

  Ipv4Address GetNextHop() const { return m_nextHop; }

  void SetNextHop(Ipv4Address n) { m_nextHop = n; }

  Ipv4Address GetDst() const { return m_dst; }

  void SetDst(Ipv4Address n) { m_dst = n; }

  Ipv4Address GetSrc() const { return m_src; }

  void SetSrc(Ipv4Address s) { m_src = s; }

  uint16_t GetAckId() const { return m_ackId; }

  void SetAckId(uint16_t ackId) { m_ackId = ackId; }

  uint8_t GetSegsLeft() const { return m_segsLeft; }

  void SetSegsLeft(uint8_t segs) { m_segsLeft = segs; }

  void SetExpireTime(Time exp) { m_expire = exp + Simulator::Now(); }

  Time GetExpireTime() const { return m_expire - Simulator::Now(); }

private:
  Ptr<const Packet> m_packet;
  Ipv4Address m_ourAdd;
  Ipv4Address m_nextHop;
  Ipv4Address m_src;
  Ipv4Address m_dst;
  uint16_t m_ackId;
  uint8_t m_segsLeft;
  Time m_expire;
};

class DsrMaintainBuffer {
public:
  DsrMaintainBuffer() {}

  bool Enqueue(DsrMaintainBuffEntry &entry);
  bool Dequeue(Ipv4Address dst, DsrMaintainBuffEntry &entry);
  void DropPacketWithNextHop(Ipv4Address nextHop);
  bool Find(Ipv4Address nextHop);
  uint32_t GetSize();

  uint32_t GetMaxQueueLen() const { return m_maxLen; }

  void SetMaxQueueLen(uint32_t len) { m_maxLen = len; }

  Time GetMaintainBufferTimeout() const { return m_maintainBufferTimeout; }

  void SetMaintainBufferTimeout(Time t) { m_maintainBufferTimeout = t; }

  bool AllEqual(DsrMaintainBuffEntry &entry);
  bool LinkEqual(DsrMaintainBuffEntry &entry);
  bool NetworkEqual(DsrMaintainBuffEntry &entry);
  bool PromiscEqual(DsrMaintainBuffEntry &entry);

private:
  std::vector<DsrMaintainBuffEntry> m_maintainBuffer;
  std::vector<NetworkKey> m_allNetworkKey;
  void Purge();
  uint32_t m_maxLen;
  Time m_maintainBufferTimeout;
};

} // namespace dsr
} // namespace ns3
#endif
