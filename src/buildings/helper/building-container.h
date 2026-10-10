#ifndef BUILDING_CONTAINER_H
#define BUILDING_CONTAINER_H

#include <ns3/building.h>

#include <stdint.h>
#include <vector>

namespace ns3 {

class BuildingContainer {
public:
  typedef std::vector<Ptr<Building>>::const_iterator Iterator;

  BuildingContainer();

  BuildingContainer(Ptr<Building> building);

  BuildingContainer(std::string buildingName);

  Iterator Begin() const;

  Iterator End() const;

  uint32_t GetN() const;

  Ptr<Building> Get(uint32_t i) const;

  void Create(uint32_t n);

  void Add(BuildingContainer other);

  void Add(Ptr<Building> building);

  void Add(std::string buildingName);

  static BuildingContainer GetGlobal();

private:
  std::vector<Ptr<Building>> m_buildings;
};

} // namespace ns3

#endif
