#ifndef BUILDING_H
#define BUILDING_H

#include <ns3/attribute-helper.h>
#include <ns3/attribute.h>
#include <ns3/box.h>
#include <ns3/object.h>
#include <ns3/simple-ref-count.h>
#include <ns3/vector.h>

namespace ns3 {

class Building : public Object {
public:
  static TypeId GetTypeId();
  void DoDispose() override;

  enum BuildingType_t { Residential, Office, Commercial };

  enum ExtWallsType_t {
    Wood,
    ConcreteWithWindows,
    ConcreteWithoutWindows,
    StoneBlocks
  };

  Building(double xMin, double xMax, double yMin, double yMax, double zMin,
           double zMax);

  Building();

  ~Building() override;

  uint32_t GetId() const;

  void SetBoundaries(Box box);

  void SetBuildingType(Building::BuildingType_t t);

  void SetExtWallsType(Building::ExtWallsType_t t);

  void SetNFloors(uint16_t nfloors);

  void SetNRoomsX(uint16_t nroomx);

  void SetNRoomsY(uint16_t nroomy);

  Box GetBoundaries() const;

  BuildingType_t GetBuildingType() const;

  ExtWallsType_t GetExtWallsType() const;

  uint16_t GetNFloors() const;

  uint16_t GetNRoomsX() const;

  uint16_t GetNRoomsY() const;

  bool IsInside(Vector position) const;

  uint16_t GetRoomX(Vector position) const;

  uint16_t GetRoomY(Vector position) const;

  uint16_t GetFloor(Vector position) const;
  bool IsIntersect(const Vector &l1, const Vector &l2) const;

private:
  Box m_buildingBounds;

  uint16_t m_floors;
  uint16_t m_roomsX;
  uint16_t m_roomsY;

  uint32_t m_buildingId;
  BuildingType_t m_buildingType;
  ExtWallsType_t m_externalWalls;
};

} // namespace ns3

#endif
