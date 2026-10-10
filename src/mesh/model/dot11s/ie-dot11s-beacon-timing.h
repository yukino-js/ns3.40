
#ifndef WIFI_TIMING_ELEMENT_H
#define WIFI_TIMING_ELEMENT_H

#include "ns3/mesh-information-element-vector.h"
#include "ns3/nstime.h"

#include <vector>

namespace ns3 {
namespace dot11s {
class IeBeaconTimingUnit : public SimpleRefCount<IeBeaconTimingUnit> {
public:
  IeBeaconTimingUnit();
  void SetAid(uint8_t aid);
  void SetLastBeacon(uint16_t lastBeacon);
  void SetBeaconInterval(uint16_t beaconInterval);

  uint8_t GetAid() const;
  uint16_t GetLastBeacon() const;
  uint16_t GetBeaconInterval() const;

private:
  uint8_t m_aid;
  uint16_t m_lastBeacon;
  uint16_t m_beaconInterval;
  friend bool operator==(const IeBeaconTimingUnit &a,
                         const IeBeaconTimingUnit &b);
};

class IeBeaconTiming : public WifiInformationElement {
public:
  typedef std::vector<Ptr<IeBeaconTimingUnit>> NeighboursTimingUnitsList;

  IeBeaconTiming();
  NeighboursTimingUnitsList GetNeighboursTimingElementsList();
  void AddNeighboursTimingElementUnit(uint16_t aid, Time last_beacon,
                                      Time beacon_interval);
  void DelNeighboursTimingElementUnit(uint16_t aid, Time last_beacon,
                                      Time beacon_interval);
  void ClearTimingElement();

  WifiInformationElementId ElementId() const override;
  uint16_t GetInformationFieldSize() const override;
  void SerializeInformationField(Buffer::Iterator i) const override;
  uint16_t DeserializeInformationField(Buffer::Iterator i,
                                       uint16_t length) override;
  void Print(std::ostream &os) const override;

  bool operator==(const WifiInformationElement &a) const override;

private:
  static uint16_t TimestampToU16(Time x);
  static uint16_t BeaconIntervalToU16(Time x);
  static uint8_t AidToU8(uint16_t x);

  NeighboursTimingUnitsList m_neighbours;
  uint16_t m_numOfUnits;
};

bool operator==(const IeBeaconTimingUnit &a, const IeBeaconTimingUnit &b);
std::ostream &operator<<(std::ostream &os, const IeBeaconTiming &beaconTiming);
} // namespace dot11s
} // namespace ns3
#endif
