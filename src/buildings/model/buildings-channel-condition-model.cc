
#include "buildings-channel-condition-model.h"

#include "building-list.h"
#include "mobility-building-info.h"

#include "ns3/log.h"
#include "ns3/mobility-model.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("BuildingsChannelConditionModel");

NS_OBJECT_ENSURE_REGISTERED(BuildingsChannelConditionModel);

TypeId BuildingsChannelConditionModel::GetTypeId() {
  static TypeId tid = TypeId("ns3::BuildingsChannelConditionModel")
                          .SetParent<ChannelConditionModel>()
                          .SetGroupName("Buildings")
                          .AddConstructor<BuildingsChannelConditionModel>();
  return tid;
}

BuildingsChannelConditionModel::BuildingsChannelConditionModel()
    : ChannelConditionModel() {}

BuildingsChannelConditionModel::~BuildingsChannelConditionModel() {}

Ptr<ChannelCondition> BuildingsChannelConditionModel::GetChannelCondition(
    Ptr<const MobilityModel> a, Ptr<const MobilityModel> b) const {
  NS_LOG_FUNCTION(this);
  Ptr<MobilityBuildingInfo> a1 = a->GetObject<MobilityBuildingInfo>();
  Ptr<MobilityBuildingInfo> b1 = b->GetObject<MobilityBuildingInfo>();
  NS_ASSERT_MSG(
      a1 && b1,
      "BuildingsChannelConditionModel only works with MobilityBuildingInfo");

  Ptr<ChannelCondition> cond = CreateObject<ChannelCondition>();

  bool isAIndoor = a1->IsIndoor();
  bool isBIndoor = b1->IsIndoor();

  if (!isAIndoor && !isBIndoor) {
    cond->SetO2iCondition(ChannelCondition::O2iConditionValue::O2O);

    bool blocked = IsLineOfSightBlocked(a->GetPosition(), b->GetPosition());
    NS_LOG_DEBUG("a and b are outdoor, blocked " << blocked);
    if (!blocked) {
      NS_LOG_DEBUG("Set LOS");
      cond->SetLosCondition(ChannelCondition::LosConditionValue::LOS);
    } else {
      cond->SetLosCondition(ChannelCondition::LosConditionValue::NLOS);
    }
  } else if (isAIndoor && isBIndoor) {
    cond->SetO2iCondition(ChannelCondition::O2iConditionValue::I2I);

    if (a1->GetBuilding() == b1->GetBuilding()) {
      NS_LOG_DEBUG("a and b are indoor in the same building");
      cond->SetLosCondition(ChannelCondition::LosConditionValue::LOS);
    } else {
      NS_LOG_DEBUG("a and b are indoor in different buildings");
      cond->SetLosCondition(ChannelCondition::LosConditionValue::NLOS);

      ChannelCondition::O2iLowHighConditionValue lowHighLossConditionA1;
      ChannelCondition::O2iLowHighConditionValue lowHighLossConditionB1;

      lowHighLossConditionA1 =
          a1->GetBuilding()->GetExtWallsType() ==
                      Building::ExtWallsType_t::Wood ||
                  a1->GetBuilding()->GetExtWallsType() ==
                      Building::ExtWallsType_t::ConcreteWithWindows
              ? ChannelCondition::O2iLowHighConditionValue::LOW
              : ChannelCondition::O2iLowHighConditionValue::HIGH;

      lowHighLossConditionB1 =
          b1->GetBuilding()->GetExtWallsType() ==
                      Building::ExtWallsType_t::Wood ||
                  b1->GetBuilding()->GetExtWallsType() ==
                      Building::ExtWallsType_t::ConcreteWithWindows
              ? ChannelCondition::O2iLowHighConditionValue::LOW
              : ChannelCondition::O2iLowHighConditionValue::HIGH;

      if (lowHighLossConditionA1 ==
              ChannelCondition::O2iLowHighConditionValue::HIGH ||
          lowHighLossConditionB1 ==
              ChannelCondition::O2iLowHighConditionValue::HIGH) {
        cond->SetO2iLowHighCondition(
            ChannelCondition::O2iLowHighConditionValue::HIGH);
      } else {
        cond->SetO2iLowHighCondition(
            ChannelCondition::O2iLowHighConditionValue::LOW);
      }
    }
  } else {
    cond->SetO2iCondition(ChannelCondition::O2iConditionValue::O2I);

    NS_LOG_DEBUG("a is indoor and b outdoor or vice-versa");
    cond->SetLosCondition(ChannelCondition::LosConditionValue::NLOS);

    ChannelCondition::O2iLowHighConditionValue lowHighLossCondition;
    if (isAIndoor) {
      lowHighLossCondition =
          a1->GetBuilding()->GetExtWallsType() ==
                      Building::ExtWallsType_t::Wood ||
                  a1->GetBuilding()->GetExtWallsType() ==
                      Building::ExtWallsType_t::ConcreteWithWindows
              ? ChannelCondition::O2iLowHighConditionValue::LOW
              : ChannelCondition::O2iLowHighConditionValue::HIGH;

      cond->SetO2iLowHighCondition(lowHighLossCondition);
    } else {
      lowHighLossCondition =
          b1->GetBuilding()->GetExtWallsType() ==
                      Building::ExtWallsType_t::Wood ||
                  b1->GetBuilding()->GetExtWallsType() ==
                      Building::ExtWallsType_t::ConcreteWithWindows
              ? ChannelCondition::O2iLowHighConditionValue::LOW
              : ChannelCondition::O2iLowHighConditionValue::HIGH;
      cond->SetO2iLowHighCondition(lowHighLossCondition);
    }
  }

  return cond;
}

bool BuildingsChannelConditionModel::IsLineOfSightBlocked(
    const ns3::Vector &l1, const ns3::Vector &l2) const {
  for (auto bit = BuildingList::Begin(); bit != BuildingList::End(); ++bit) {
    if ((*bit)->IsIntersect(l1, l2)) {
      return true;
    }
  }

  return false;
}

int64_t BuildingsChannelConditionModel::AssignStreams(int64_t) { return 0; }

} // namespace ns3
