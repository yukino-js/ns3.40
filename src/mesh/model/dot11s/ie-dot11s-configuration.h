
#ifndef MESH_CONFIGURATION_H
#define MESH_CONFIGURATION_H

#include "ns3/mesh-information-element-vector.h"

namespace ns3 {
namespace dot11s {

enum Dot11sPathSelectionProtocol {
  PROTOCOL_HWMP = 0x01,
};

enum Dot11sPathSelectionMetric {
  METRIC_AIRTIME = 0x01,
};

enum Dot11sCongestionControlMode {
  CONGESTION_SIGNALING = 0x01,
  CONGESTION_NULL = 0x00,
};

enum Dot11sSynchronizationProtocolIdentifier {
  SYNC_NEIGHBOUR_OFFSET = 0x01,
  SYNC_NULL = 0x00,
};

enum Dot11sAuthenticationProtocol {
  AUTH_NULL = 0x00,
  AUTH_SAE = 0x01,
  AUTH_IEEE = 0x02,
};

class Dot11sMeshCapability {
public:
  Dot11sMeshCapability();
  uint8_t GetSerializedSize() const;
  Buffer::Iterator Serialize(Buffer::Iterator i) const;
  Buffer::Iterator Deserialize(Buffer::Iterator i);
  uint8_t GetUint8() const;
  bool acceptPeerLinks;
  bool MCCASupported;
  bool MCCAEnabled;
  bool forwarding;
  bool beaconTimingReport;
  bool TBTTAdjustment;
  bool powerSaveLevel;
  bool Is(uint8_t cap, uint8_t n) const;
  friend bool operator==(const Dot11sMeshCapability &a,
                         const Dot11sMeshCapability &b);
};

class IeConfiguration : public WifiInformationElement {
public:
  IeConfiguration();
  void SetRouting(Dot11sPathSelectionProtocol routingId);
  void SetMetric(Dot11sPathSelectionMetric metricId);
  bool IsHWMP();
  bool IsAirtime();
  void SetNeighborCount(uint8_t neighbors);
  uint8_t GetNeighborCount() const;
  const Dot11sMeshCapability &MeshCapability();

  WifiInformationElementId ElementId() const override;
  uint16_t GetInformationFieldSize() const override;
  void SerializeInformationField(Buffer::Iterator i) const override;
  uint16_t DeserializeInformationField(Buffer::Iterator i,
                                       uint16_t length) override;
  void Print(std::ostream &os) const override;

private:
  Dot11sPathSelectionProtocol m_APSPId;
  Dot11sPathSelectionMetric m_APSMId;
  Dot11sCongestionControlMode m_CCMId;
  Dot11sSynchronizationProtocolIdentifier m_SPId;
  Dot11sAuthenticationProtocol m_APId;
  Dot11sMeshCapability m_meshCap;
  uint8_t m_neighbors;
  friend bool operator==(const IeConfiguration &a, const IeConfiguration &b);
};

bool operator==(const IeConfiguration &a, const IeConfiguration &b);
bool operator==(const Dot11sMeshCapability &a, const Dot11sMeshCapability &b);
std::ostream &operator<<(std::ostream &os, const IeConfiguration &config);
} // namespace dot11s
} // namespace ns3
#endif
