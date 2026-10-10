
#ifndef MESH_WIFI_INTERFACE_MAC_PLUGIN_H
#define MESH_WIFI_INTERFACE_MAC_PLUGIN_H

#include "mesh-wifi-beacon.h"

#include "ns3/mac48-address.h"
#include "ns3/packet.h"
#include "ns3/simple-ref-count.h"

namespace ns3 {

class MeshWifiInterfaceMac;

class MeshWifiInterfaceMacPlugin
    : public SimpleRefCount<MeshWifiInterfaceMacPlugin> {
public:
  virtual ~MeshWifiInterfaceMacPlugin() {};
  virtual void SetParent(Ptr<MeshWifiInterfaceMac> parent) = 0;
  virtual bool Receive(Ptr<Packet> packet, const WifiMacHeader &header) = 0;
  virtual bool UpdateOutcomingFrame(Ptr<Packet> packet, WifiMacHeader &header,
                                    Mac48Address from, Mac48Address to) = 0;
  virtual void UpdateBeacon(MeshWifiBeacon &beacon) const = 0;
  virtual int64_t AssignStreams(int64_t stream) = 0;
};

} // namespace ns3

#endif
