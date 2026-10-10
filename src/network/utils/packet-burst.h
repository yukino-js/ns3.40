
#ifndef PACKET_BURST_H
#define PACKET_BURST_H

#include "ns3/object.h"

#include <list>
#include <stdint.h>

namespace ns3 {

class Packet;

class PacketBurst : public Object {
public:
  static TypeId GetTypeId();
  PacketBurst();
  ~PacketBurst() override;
  Ptr<PacketBurst> Copy() const;
  void AddPacket(Ptr<Packet> packet);
  std::list<Ptr<Packet>> GetPackets() const;
  uint32_t GetNPackets() const;
  uint32_t GetSize() const;

  std::list<Ptr<Packet>>::const_iterator Begin() const;
  std::list<Ptr<Packet>>::const_iterator End() const;

  typedef void (*TracedCallback)(Ptr<const PacketBurst> burst);

private:
  void DoDispose() override;
  std::list<Ptr<Packet>> m_packets;
};
} // namespace ns3

#endif
