
#ifndef IPV4_PACKET_PROBE_H
#define IPV4_PACKET_PROBE_H

#include "ipv4.h"

#include "ns3/boolean.h"
#include "ns3/callback.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/probe.h"
#include "ns3/simulator.h"
#include "ns3/traced-value.h"

namespace ns3 {

class Ipv4PacketProbe : public Probe {
public:
  static TypeId GetTypeId();
  Ipv4PacketProbe();
  ~Ipv4PacketProbe() override;

  void SetValue(Ptr<const Packet> packet, Ptr<Ipv4> ipv4, uint32_t interface);

  static void SetValueByPath(std::string path, Ptr<const Packet> packet,
                             Ptr<Ipv4> ipv4, uint32_t interface);

  bool ConnectByObject(std::string traceSource, Ptr<Object> obj) override;

  void ConnectByPath(std::string path) override;

private:
  void TraceSink(Ptr<const Packet> packet, Ptr<Ipv4> ipv4, uint32_t interface);

  ns3::TracedCallback<Ptr<const Packet>, Ptr<Ipv4>, uint32_t> m_output;
  ns3::TracedCallback<uint32_t, uint32_t> m_outputBytes;

  Ptr<const Packet> m_packet;

  Ptr<Ipv4> m_ipv4;

  uint32_t m_interface;

  uint32_t m_packetSizeOld;
};

} // namespace ns3

#endif
