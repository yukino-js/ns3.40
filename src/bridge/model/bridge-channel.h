
#ifndef BRIDGE_CHANNEL_H
#define BRIDGE_CHANNEL_H

#include "ns3/channel.h"
#include "ns3/net-device.h"

#include <vector>

namespace ns3 {

class BridgeChannel : public Channel {
public:
  static TypeId GetTypeId();
  BridgeChannel();
  ~BridgeChannel() override;

  BridgeChannel(const BridgeChannel &) = delete;
  BridgeChannel &operator=(const BridgeChannel &) = delete;

  void AddChannel(Ptr<Channel> bridgedChannel);

  std::size_t GetNDevices() const override;
  Ptr<NetDevice> GetDevice(std::size_t i) const override;

private:
  std::vector<Ptr<Channel>> m_bridgedChannels;
};

} // namespace ns3

#endif
