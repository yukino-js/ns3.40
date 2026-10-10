
#ifndef BUILDINGS_CHANNEL_CONDITION_MODEL_H
#define BUILDINGS_CHANNEL_CONDITION_MODEL_H

#include "ns3/channel-condition-model.h"

namespace ns3 {

class MobilityModel;

class BuildingsChannelConditionModel : public ChannelConditionModel {
public:
  static TypeId GetTypeId();

  BuildingsChannelConditionModel();

  ~BuildingsChannelConditionModel() override;

  Ptr<ChannelCondition>
  GetChannelCondition(Ptr<const MobilityModel> a,
                      Ptr<const MobilityModel> b) const override;

  int64_t AssignStreams(int64_t stream) override;

private:
  bool IsLineOfSightBlocked(const Vector &l1, const Vector &l2) const;
};

} // namespace ns3

#endif
