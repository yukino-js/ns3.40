

#ifndef NS3_CHANNEL_H
#define NS3_CHANNEL_H

#include "ns3/object.h"
#include "ns3/ptr.h"

#include <stdint.h>
#include <string>

namespace ns3 {

class NetDevice;

class Channel : public Object {
public:
  static TypeId GetTypeId();

  Channel();
  ~Channel() override;

  uint32_t GetId() const;

  virtual std::size_t GetNDevices() const = 0;
  virtual Ptr<NetDevice> GetDevice(std::size_t i) const = 0;

private:
  uint32_t m_id;
};

} // namespace ns3

#endif
