
#ifndef BUILDING_LIST_H_
#define BUILDING_LIST_H_

#include "ns3/ptr.h"

#include <vector>

namespace ns3 {

class Building;

class BuildingList {
public:
  typedef std::vector<Ptr<Building>>::const_iterator Iterator;

  static uint32_t Add(Ptr<Building> building);
  static Iterator Begin();
  static Iterator End();
  static Ptr<Building> GetBuilding(uint32_t n);
  static uint32_t GetNBuildings();
};

} // namespace ns3

#endif
