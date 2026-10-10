#ifndef SIMPLE_CHANNEL_H
#define SIMPLE_CHANNEL_H

#include "mac48-address.h"

#include "ns3/channel.h"
#include "ns3/nstime.h"

#include <map>
#include <vector>

namespace ns3 {

class SimpleNetDevice;
class Packet;

class SimpleChannel : public Channel {
public:
  static TypeId GetTypeId();
  SimpleChannel();

  virtual void Send(Ptr<Packet> p, uint16_t protocol, Mac48Address to,
                    Mac48Address from, Ptr<SimpleNetDevice> sender);

  virtual void Add(Ptr<SimpleNetDevice> device);

  virtual void BlackList(Ptr<SimpleNetDevice> from, Ptr<SimpleNetDevice> to);

  virtual void UnBlackList(Ptr<SimpleNetDevice> from, Ptr<SimpleNetDevice> to);

  std::size_t GetNDevices() const override;
  Ptr<NetDevice> GetDevice(std::size_t i) const override;

private:
  Time m_delay;
  std::vector<Ptr<SimpleNetDevice>> m_devices;
  std::map<Ptr<SimpleNetDevice>, std::vector<Ptr<SimpleNetDevice>>>
      m_blackListedDevices;
};

} // namespace ns3

#endif
