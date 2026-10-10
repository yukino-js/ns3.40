
#ifndef PACKET_PROBE_H
#define PACKET_PROBE_H

#include "ns3/boolean.h"
#include "ns3/callback.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/probe.h"
#include "ns3/simulator.h"
#include "ns3/traced-value.h"

namespace ns3 {

class PacketProbe : public Probe {
public:
  static TypeId GetTypeId();
  PacketProbe();
  ~PacketProbe() override;

  void SetValue(Ptr<const Packet> packet);

  static void SetValueByPath(std::string path, Ptr<const Packet> packet);

  bool ConnectByObject(std::string traceSource, Ptr<Object> obj) override;

  void ConnectByPath(std::string path) override;

private:
  void TraceSink(Ptr<const Packet> packet);

  TracedCallback<Ptr<const Packet>> m_output;
  TracedCallback<uint32_t, uint32_t> m_outputBytes;

  Ptr<const Packet> m_packet;

  uint32_t m_packetSizeOld;
};

} // namespace ns3

#endif
