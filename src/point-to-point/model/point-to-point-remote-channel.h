

#ifndef POINT_TO_POINT_REMOTE_CHANNEL_H
#define POINT_TO_POINT_REMOTE_CHANNEL_H

#include "point-to-point-channel.h"

namespace ns3 {

class PointToPointRemoteChannel : public PointToPointChannel {
public:
  static TypeId GetTypeId();

  PointToPointRemoteChannel();

  ~PointToPointRemoteChannel() override;

  bool TransmitStart(Ptr<const Packet> p, Ptr<PointToPointNetDevice> src,
                     Time txTime) override;
};

} // namespace ns3

#endif
