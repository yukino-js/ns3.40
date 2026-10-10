#ifndef BUILDING_ALLOCATOR_H
#define BUILDING_ALLOCATOR_H

#include "building-container.h"

#include "ns3/object-factory.h"
#include "ns3/object.h"
#include "ns3/position-allocator.h"
#include "ns3/vector.h"

namespace ns3 {

class Building;

class GridBuildingAllocator : public Object {
public:
  GridBuildingAllocator();
  ~GridBuildingAllocator() override;

  static TypeId GetTypeId();

  void SetBuildingAttribute(std::string n, const AttributeValue &v);

  BuildingContainer Create(uint32_t n) const;

private:
  void PushAttributes() const;

  mutable uint32_t m_current;
  GridPositionAllocator::LayoutType m_layoutType;
  double m_xMin;
  double m_yMin;
  uint32_t m_n;
  double m_lengthX;
  double m_lengthY;
  double m_deltaX;
  double m_deltaY;
  double m_height;

  mutable ObjectFactory m_buildingFactory;
  Ptr<GridPositionAllocator> m_lowerLeftPositionAllocator;
  Ptr<GridPositionAllocator> m_upperRightPositionAllocator;
};

} // namespace ns3

#endif
