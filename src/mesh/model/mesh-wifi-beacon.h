
#ifndef MESH_WIFI_BEACON_H
#define MESH_WIFI_BEACON_H

#include "mesh-information-element-vector.h"

#include "ns3/mgt-headers.h"
#include "ns3/object.h"
#include "ns3/packet.h"

namespace ns3 {

class WifiMacHeader;
class Time;

class MeshWifiBeacon {
public:
  MeshWifiBeacon(Ssid ssid, AllSupportedRates rates, uint64_t us);

  MgtBeaconHeader BeaconHeader() const { return m_header; }

  void AddInformationElement(Ptr<WifiInformationElement> ie);

  WifiMacHeader CreateHeader(Mac48Address address, Mac48Address mpAddress);
  Time GetBeaconInterval() const;
  Ptr<Packet> CreatePacket();

private:
  MgtBeaconHeader m_header;
  MeshInformationElementVector m_elements;
};

} // namespace ns3

#endif
