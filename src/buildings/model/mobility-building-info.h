#ifndef MOBILITY_BUILDING_INFO_H
#define MOBILITY_BUILDING_INFO_H

#include "building.h"

#include <ns3/box.h>
#include <ns3/constant-velocity-helper.h>
#include <ns3/mobility-model.h>
#include <ns3/object.h>
#include <ns3/ptr.h>
#include <ns3/simple-ref-count.h>

#include <map>

namespace ns3 {

class MobilityBuildingInfo : public Object {
public:
  static TypeId GetTypeId();
  MobilityBuildingInfo();

  MobilityBuildingInfo(Ptr<Building> building);

  bool IsIndoor();

  void SetIndoor(Ptr<Building> building, uint8_t nfloor, uint8_t nroomx,
                 uint8_t nroomy);

  void SetIndoor(uint8_t nfloor, uint8_t nroomx, uint8_t nroomy);

  void SetOutdoor();

  uint8_t GetFloorNumber();

  uint8_t GetRoomNumberX();

  uint8_t GetRoomNumberY();

  Ptr<Building> GetBuilding();
  void MakeConsistent(Ptr<MobilityModel> mm);

protected:
  void DoInitialize() override;

private:
  Ptr<Building> m_myBuilding;
  bool m_indoor;
  uint8_t m_nFloor;
  uint8_t m_roomX;
  uint8_t m_roomY;
  Vector m_cachedPosition;
};

} // namespace ns3

#endif
