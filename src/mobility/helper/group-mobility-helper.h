
#ifndef GROUP_MOBILITY_HELPER_H
#define GROUP_MOBILITY_HELPER_H

#include "ns3/attribute.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"

#include <vector>

namespace ns3 {

class PositionAllocator;
class MobilityModel;

class GroupMobilityHelper {
public:
  GroupMobilityHelper();

  ~GroupMobilityHelper();

  void SetReferencePositionAllocator(Ptr<PositionAllocator> allocator);

  template <typename... Ts>
  void SetReferencePositionAllocator(std::string type, Ts &&...args);

  void SetMemberPositionAllocator(Ptr<PositionAllocator> allocator);

  template <typename... Ts>
  void SetMemberPositionAllocator(std::string type, Ts &&...args);

  void SetReferenceMobilityModel(Ptr<MobilityModel> mobility);

  template <typename... Ts>
  void SetReferenceMobilityModel(std::string type, Ts &&...args);

  template <typename... Ts>
  void SetMemberMobilityModel(std::string type, Ts &&...args);

  void Install(Ptr<Node> node);
  void Install(std::string nodeName);

  void Install(NodeContainer container);

  int64_t AssignStreams(NodeContainer c, int64_t stream);

private:
  NS_LOG_TEMPLATE_DECLARE;

  bool m_referencePositionSet{false};
  Ptr<MobilityModel> m_referenceMobility;
  Ptr<PositionAllocator> m_referencePosition;
  ObjectFactory m_memberMobilityFactory;
  Ptr<PositionAllocator> m_memberPosition;
};

template <typename... Ts>
void GroupMobilityHelper::SetReferencePositionAllocator(std::string type,
                                                        Ts &&...args) {
  ObjectFactory pos(type, std::forward<Ts>(args)...);
  m_referencePosition = pos.Create()->GetObject<PositionAllocator>();
  NS_ABORT_MSG_IF(!m_referencePosition,
                  "Unable to create allocator from TypeId " << type);
}

template <typename... Ts>
void GroupMobilityHelper::SetMemberPositionAllocator(std::string type,
                                                     Ts &&...args) {
  ObjectFactory pos(type, std::forward<Ts>(args)...);
  m_memberPosition = pos.Create()->GetObject<PositionAllocator>();
  NS_ABORT_MSG_IF(!m_memberPosition,
                  "Unable to create allocator from TypeId " << type);
}

template <typename... Ts>
void GroupMobilityHelper::SetReferenceMobilityModel(std::string type,
                                                    Ts &&...args) {
  NS_LOG_FUNCTION(this << type);
  ObjectFactory mob(type, std::forward<Ts>(args)...);
  m_referenceMobility = mob.Create()->GetObject<MobilityModel>();
  NS_ABORT_MSG_IF(!m_referenceMobility,
                  "Unable to create mobility from TypeId " << type);
}

template <typename... Ts>
void GroupMobilityHelper::SetMemberMobilityModel(std::string type,
                                                 Ts &&...args) {
  NS_LOG_FUNCTION(this << type);
  m_memberMobilityFactory.SetTypeId(type);
  m_memberMobilityFactory.Set(std::forward<Ts>(args)...);
}

} // namespace ns3

#endif
