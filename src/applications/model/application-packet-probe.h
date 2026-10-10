
#ifndef APPLICATION_PACKET_PROBE_H
#define APPLICATION_PACKET_PROBE_H

#include "ns3/application.h"
#include "ns3/boolean.h"
#include "ns3/callback.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/probe.h"
#include "ns3/simulator.h"
#include "ns3/traced-value.h"

namespace ns3 {

class ApplicationPacketProbe : public Probe {
public:
  static TypeId GetTypeId();
  ApplicationPacketProbe();
  ~ApplicationPacketProbe() override;

  void SetValue(Ptr<const Packet> packet, const Address &address);

  static void SetValueByPath(std::string path, Ptr<const Packet> packet,
                             const Address &address);

  bool ConnectByObject(std::string traceSource, Ptr<Object> obj) override;

  void ConnectByPath(std::string path) override;

private:
  void TraceSink(Ptr<const Packet> packet, const Address &address);

  TracedCallback<Ptr<const Packet>, const Address &> m_output;
  TracedCallback<uint32_t, uint32_t> m_outputBytes;

  Ptr<const Packet> m_packet;

  Address m_address;

  uint32_t m_packetSizeOld;
};

} // namespace ns3

#endif
