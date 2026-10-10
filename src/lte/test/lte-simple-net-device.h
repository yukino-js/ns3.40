
#ifndef LTE_SIMPLE_NET_DEVICE_H
#define LTE_SIMPLE_NET_DEVICE_H

#include "ns3/error-model.h"
#include "ns3/event-id.h"
#include "ns3/lte-rlc.h"
#include "ns3/node.h"
#include "ns3/simple-channel.h"
#include "ns3/simple-net-device.h"

namespace ns3 {

class LteSimpleNetDevice : public SimpleNetDevice {
public:
  static TypeId GetTypeId();

  LteSimpleNetDevice();
  LteSimpleNetDevice(Ptr<Node> node);

  ~LteSimpleNetDevice() override;
  void DoDispose() override;

  bool Send(Ptr<Packet> packet, const Address &dest,
            uint16_t protocolNumber) override;

protected:
  void DoInitialize() override;
};

} // namespace ns3

#endif
