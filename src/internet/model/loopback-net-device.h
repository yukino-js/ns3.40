#ifndef LOOPBACK_NET_DEVICE_H
#define LOOPBACK_NET_DEVICE_H

#include "ns3/mac48-address.h"
#include "ns3/net-device.h"

#include <stdint.h>
#include <string>

namespace ns3 {

class Node;

class LoopbackNetDevice : public NetDevice {
public:
  static TypeId GetTypeId();
  LoopbackNetDevice();

  void SetIfIndex(const uint32_t index) override;
  uint32_t GetIfIndex() const override;
  Ptr<Channel> GetChannel() const override;
  void SetAddress(Address address) override;
  Address GetAddress() const override;
  bool SetMtu(const uint16_t mtu) override;
  uint16_t GetMtu() const override;
  bool IsLinkUp() const override;
  void AddLinkChangeCallback(Callback<void> callback) override;
  bool IsBroadcast() const override;
  Address GetBroadcast() const override;
  bool IsMulticast() const override;
  Address GetMulticast(Ipv4Address multicastGroup) const override;
  bool IsPointToPoint() const override;
  bool IsBridge() const override;
  bool Send(Ptr<Packet> packet, const Address &dest,
            uint16_t protocolNumber) override;
  bool SendFrom(Ptr<Packet> packet, const Address &source, const Address &dest,
                uint16_t protocolNumber) override;
  Ptr<Node> GetNode() const override;
  void SetNode(Ptr<Node> node) override;
  bool NeedsArp() const override;
  void SetReceiveCallback(NetDevice::ReceiveCallback cb) override;

  Address GetMulticast(Ipv6Address addr) const override;

  void SetPromiscReceiveCallback(PromiscReceiveCallback cb) override;
  bool SupportsSendFrom() const override;

protected:
  void DoDispose() override;

private:
  void Receive(Ptr<Packet> packet, uint16_t protocol, Mac48Address to,
               Mac48Address from);

  NetDevice::ReceiveCallback m_rxCallback;

  NetDevice::PromiscReceiveCallback m_promiscCallback;

  Ptr<Node> m_node;
  uint16_t m_mtu;
  uint32_t m_ifIndex;
  Mac48Address m_address;
};

} // namespace ns3

#endif
