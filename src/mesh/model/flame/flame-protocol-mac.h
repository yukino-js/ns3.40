
#ifndef FLAME_PROTOCOL_MAC_H
#define FLAME_PROTOCOL_MAC_H

#include "ns3/mesh-wifi-interface-mac.h"

namespace ns3 {
namespace flame {
class FlameProtocol;

class FlameProtocolMac : public MeshWifiInterfaceMacPlugin {
public:
  FlameProtocolMac(Ptr<FlameProtocol> protocol);
  ~FlameProtocolMac() override;

  void SetParent(Ptr<MeshWifiInterfaceMac> parent) override;
  bool Receive(Ptr<Packet> packet, const WifiMacHeader &header) override;
  bool UpdateOutcomingFrame(Ptr<Packet> packet, WifiMacHeader &header,
                            Mac48Address from, Mac48Address to) override;
  void UpdateBeacon(MeshWifiBeacon &beacon) const override {};

  int64_t AssignStreams(int64_t stream) override { return 0; }

  uint16_t GetChannelId() const;
  void Report(std::ostream &os) const;
  void ResetStats();

private:
  Ptr<FlameProtocol> m_protocol;
  Ptr<MeshWifiInterfaceMac> m_parent;

  struct Statistics {
    uint16_t txUnicast;
    uint16_t txBroadcast;
    uint32_t txBytes;
    uint16_t rxUnicast;
    uint16_t rxBroadcast;
    uint32_t rxBytes;

    void Print(std::ostream &os) const;
    Statistics();
  };

  Statistics m_stats;
};
} // namespace flame
} // namespace ns3
#endif
