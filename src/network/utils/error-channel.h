#ifndef ERROR_CHANNEL_H
#define ERROR_CHANNEL_H

#include "error-model.h"
#include "mac48-address.h"
#include "simple-channel.h"

#include "ns3/channel.h"
#include "ns3/nstime.h"

#include <vector>

namespace ns3 {

class SimpleNetDevice;
class Packet;

class ErrorChannel : public SimpleChannel {
public:
  static TypeId GetTypeId();
  ErrorChannel();

  void Send(Ptr<Packet> p, uint16_t protocol, Mac48Address to,
            Mac48Address from, Ptr<SimpleNetDevice> sender) override;

  void Add(Ptr<SimpleNetDevice> device) override;

  std::size_t GetNDevices() const override;
  Ptr<NetDevice> GetDevice(std::size_t i) const override;

  void SetJumpingTime(Time delay);

  void SetJumpingMode(bool mode);

  void SetDuplicateTime(Time delay);

  void SetDuplicateMode(bool mode);

private:
  std::vector<Ptr<SimpleNetDevice>> m_devices;
  Time m_jumpingTime;
  uint8_t m_jumpingState;
  bool m_jumping;
  Time m_duplicateTime;
  bool m_duplicate;
  uint8_t m_duplicateState;
};

} // namespace ns3

#endif
