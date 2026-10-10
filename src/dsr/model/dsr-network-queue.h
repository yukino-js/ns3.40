
#ifndef DSR_NETWORK_QUEUE_H
#define DSR_NETWORK_QUEUE_H

#include "dsr-option-header.h"

#include "ns3/ipv4-header.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/simulator.h"

#include <vector>

namespace ns3 {
namespace dsr {

enum DsrMessageType { DSR_CONTROL_PACKET = 1, DSR_DATA_PACKET = 2 };

class DsrNetworkQueueEntry {
public:
  DsrNetworkQueueEntry(Ptr<const Packet> pa = nullptr,
                       Ipv4Address s = Ipv4Address(),
                       Ipv4Address n = Ipv4Address(),
                       Time exp = Simulator::Now(), Ptr<Ipv4Route> r = nullptr)
      : m_packet(pa), m_srcAddr(s), m_nextHopAddr(n), tstamp(exp),
        m_ipv4Route(r) {}

  bool operator==(const DsrNetworkQueueEntry &o) const {
    return ((m_packet == o.m_packet) && (m_srcAddr == o.m_srcAddr) &&
            (m_nextHopAddr == o.m_nextHopAddr) && (tstamp == o.tstamp) &&
            (m_ipv4Route == o.m_ipv4Route));
  }

  Ptr<const Packet> GetPacket() const { return m_packet; }

  void SetPacket(Ptr<const Packet> p) { m_packet = p; }

  Ptr<Ipv4Route> GetIpv4Route() const { return m_ipv4Route; }

  void SetIpv4Route(Ptr<Ipv4Route> route) { m_ipv4Route = route; }

  Ipv4Address GetSourceAddress() const { return m_srcAddr; }

  void SetSourceAddress(Ipv4Address addr) { m_srcAddr = addr; }

  Ipv4Address GetNextHopAddress() const { return m_nextHopAddr; }

  void SetNextHopAddress(Ipv4Address addr) { m_nextHopAddr = addr; }

  Time GetInsertedTimeStamp() const { return tstamp; }

  void SetInsertedTimeStamp(Time time) { tstamp = time; }

private:
  Ptr<const Packet> m_packet;
  Ipv4Address m_srcAddr;
  Ipv4Address m_nextHopAddr;
  Time tstamp;
  Ptr<Ipv4Route> m_ipv4Route;
};

class DsrNetworkQueue : public Object {
public:
  static TypeId GetTypeId();

  DsrNetworkQueue();
  DsrNetworkQueue(uint32_t maxLen, Time maxDelay);
  ~DsrNetworkQueue() override;

  bool FindPacketWithNexthop(Ipv4Address nextHop, DsrNetworkQueueEntry &entry);
  bool Find(Ipv4Address nextHop);
  bool Enqueue(DsrNetworkQueueEntry &entry);
  bool Dequeue(DsrNetworkQueueEntry &entry);
  uint32_t GetSize();

  void SetMaxNetworkSize(uint32_t maxSize);
  void SetMaxNetworkDelay(Time delay);
  uint32_t GetMaxNetworkSize() const;
  Time GetMaxNetworkDelay() const;
  void Flush();

  std::vector<DsrNetworkQueueEntry> &GetQueue() { return m_dsrNetworkQueue; }

private:
  void Cleanup();
  std::vector<DsrNetworkQueueEntry> m_dsrNetworkQueue;
  uint32_t m_size;
  uint32_t m_maxSize;
  Time m_maxDelay;
};

} // namespace dsr
} // namespace ns3

#endif
