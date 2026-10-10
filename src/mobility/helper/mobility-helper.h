
#ifndef MOBILITY_HELPER_H
#define MOBILITY_HELPER_H

#include "ns3/attribute.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/position-allocator.h"

#include <vector>

namespace ns3 {

class PositionAllocator;
class MobilityModel;

class MobilityHelper {
public:
  MobilityHelper();

  ~MobilityHelper();

  void SetPositionAllocator(Ptr<PositionAllocator> allocator);

  template <typename... Ts>
  void SetPositionAllocator(std::string type, Ts &&...args);

  template <typename... Ts>
  void SetMobilityModel(std::string type, Ts &&...args);

  void PushReferenceMobilityModel(Ptr<Object> reference);
  void PushReferenceMobilityModel(std::string referenceName);
  void PopReferenceMobilityModel();

  std::string GetMobilityModelType() const;

  void Install(Ptr<Node> node) const;
  void Install(std::string nodeName) const;

  void Install(NodeContainer container) const;

  void InstallAll() const;

  static void EnableAscii(Ptr<OutputStreamWrapper> stream, uint32_t nodeid);
  static void EnableAscii(Ptr<OutputStreamWrapper> stream, NodeContainer n);
  static void EnableAsciiAll(Ptr<OutputStreamWrapper> stream);
  int64_t AssignStreams(NodeContainer c, int64_t stream);

  static double GetDistanceSquaredBetween(Ptr<Node> n1, Ptr<Node> n2);

private:
  static void CourseChanged(Ptr<OutputStreamWrapper> stream,
                            Ptr<const MobilityModel> mobility);
  std::vector<Ptr<MobilityModel>> m_mobilityStack;
  ObjectFactory m_mobility;
  Ptr<PositionAllocator> m_position;
};

template <typename... Ts>
void MobilityHelper::SetPositionAllocator(std::string type, Ts &&...args) {
  ObjectFactory pos(type, std::forward<Ts>(args)...);
  m_position = pos.Create()->GetObject<PositionAllocator>();
}

template <typename... Ts>
void MobilityHelper::SetMobilityModel(std::string type, Ts &&...args) {
  m_mobility.SetTypeId(type);
  m_mobility.Set(std::forward<Ts>(args)...);
}

} // namespace ns3

#endif
