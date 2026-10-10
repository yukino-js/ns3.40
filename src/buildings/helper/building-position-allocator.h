#ifndef BUILDING_POSITION_ALLOCATOR_H
#define BUILDING_POSITION_ALLOCATOR_H

#include "ns3/random-variable-stream.h"
#include <ns3/node-container.h>
#include <ns3/position-allocator.h>
#include <ns3/ptr.h>

namespace ns3 {

class Building;
class UniformRandomVariable;

class RandomBuildingPositionAllocator : public PositionAllocator {
public:
  RandomBuildingPositionAllocator();

  static TypeId GetTypeId();

  Vector GetNext() const override;

  int64_t AssignStreams(int64_t stream) override;

private:
  bool m_withReplacement;
  mutable std::vector<Ptr<Building>> m_buildingListWithoutReplacement;

  Ptr<UniformRandomVariable> m_rand;
};

class OutdoorPositionAllocator : public PositionAllocator {
public:
  OutdoorPositionAllocator();

  static TypeId GetTypeId();

  Vector GetNext() const override;

  void SetX(Ptr<RandomVariableStream> x);
  void SetY(Ptr<RandomVariableStream> y);
  void SetZ(Ptr<RandomVariableStream> z);

  int64_t AssignStreams(int64_t stream) override;

private:
  Ptr<RandomVariableStream> m_x;
  Ptr<RandomVariableStream> m_y;
  Ptr<RandomVariableStream> m_z;

  uint32_t m_maxAttempts;
};

class RandomRoomPositionAllocator : public PositionAllocator {
public:
  RandomRoomPositionAllocator();

  static TypeId GetTypeId();

  Vector GetNext() const override;

  int64_t AssignStreams(int64_t stream) override;

private:
  struct RoomInfo {
    Ptr<Building> b;
    uint32_t roomx;
    uint32_t roomy;
    uint32_t floor;
  };

  mutable std::vector<RoomInfo> m_roomListWithoutReplacement;

  Ptr<UniformRandomVariable> m_rand;
};

class SameRoomPositionAllocator : public PositionAllocator {
public:
  SameRoomPositionAllocator();

  SameRoomPositionAllocator(NodeContainer c);

  static TypeId GetTypeId();

  Vector GetNext() const override;

  int64_t AssignStreams(int64_t) override;

private:
  NodeContainer m_nodes;
  mutable NodeContainer::Iterator m_nodeIt;

  Ptr<UniformRandomVariable> m_rand;
};

class FixedRoomPositionAllocator : public PositionAllocator {
public:
  FixedRoomPositionAllocator(uint32_t x, uint32_t y, uint32_t z,
                             Ptr<Building> b);
  static TypeId GetTypeId();
  Vector GetNext() const override;

  int64_t AssignStreams(int64_t) override;

private:
  uint32_t roomx;
  uint32_t roomy;
  uint32_t floor;

  Ptr<Building> bptr;

  Ptr<UniformRandomVariable> m_rand;
};

} // namespace ns3

#endif
