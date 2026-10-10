#ifndef MOCK_NET_DEVICE_H
#define MOCK_NET_DEVICE_H

#include "ns3/net-device.h"
#include "ns3/traced-callback.h"

#include <stdint.h>
#include <string>

namespace ns3 {

class Node;

class MockNetDevice : public NetDevice {
public:
  static TypeId GetTypeId();
  MockNetDevice();

  void Receive(Ptr<Packet> packet, uint16_t protocol, Address to, Address from,
               NetDevice::PacketType packetType);

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
  Address GetMulticast(Ipv6Address addr) const override;
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
  void SetPromiscReceiveCallback(PromiscReceiveCallback cb) override;
  bool SupportsSendFrom() const override;

  void SetSendCallback(PromiscReceiveCallback cb);

protected:
  void DoDispose() override;

private:
  NetDevice::ReceiveCallback m_rxCallback;
  NetDevice::PromiscReceiveCallback m_promiscCallback;
  NetDevice::PromiscReceiveCallback m_sendCallback;
  Ptr<Node> m_node;
  uint16_t m_mtu;
  uint32_t m_ifIndex;
  Address m_address;

  bool m_linkUp;
  bool m_pointToPointMode;

  TracedCallback<> m_linkChangeCallbacks;
};

} // namespace ns3

#endif
