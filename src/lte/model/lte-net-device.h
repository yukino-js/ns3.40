
#ifndef LTE_NET_DEVICE_H
#define LTE_NET_DEVICE_H

#include <ns3/event-id.h>
#include <ns3/mac64-address.h>
#include <ns3/net-device.h>
#include <ns3/nstime.h>
#include <ns3/traced-callback.h>

namespace ns3 {

class Node;
class Packet;

class LteNetDevice : public NetDevice {
public:
  static TypeId GetTypeId();

  LteNetDevice();
  ~LteNetDevice() override;

  LteNetDevice(const LteNetDevice &) = delete;
  LteNetDevice &operator=(const LteNetDevice &) = delete;

  void DoDispose() override;

  void SetIfIndex(const uint32_t index) override;
  uint32_t GetIfIndex() const override;
  Ptr<Channel> GetChannel() const override;
  bool SetMtu(const uint16_t mtu) override;
  uint16_t GetMtu() const override;
  void SetAddress(Address address) override;
  Address GetAddress() const override;
  bool IsLinkUp() const override;
  void AddLinkChangeCallback(Callback<void> callback) override;
  bool IsBroadcast() const override;
  Address GetBroadcast() const override;
  bool IsMulticast() const override;
  bool IsPointToPoint() const override;
  bool IsBridge() const override;
  Ptr<Node> GetNode() const override;
  void SetNode(Ptr<Node> node) override;
  bool NeedsArp() const override;
  void SetReceiveCallback(NetDevice::ReceiveCallback cb) override;
  Address GetMulticast(Ipv4Address addr) const override;
  Address GetMulticast(Ipv6Address addr) const override;
  void SetPromiscReceiveCallback(PromiscReceiveCallback cb) override;
  bool SendFrom(Ptr<Packet> packet, const Address &source, const Address &dest,
                uint16_t protocolNumber) override;
  bool SupportsSendFrom() const override;

  void Receive(Ptr<Packet> p);

protected:
  NetDevice::ReceiveCallback m_rxCallback;

private:
  Ptr<Node> m_node;

  TracedCallback<> m_linkChangeCallbacks;

  uint32_t m_ifIndex;
  bool m_linkUp;
  mutable uint16_t m_mtu;

  Mac64Address m_address;
};

} // namespace ns3

#endif
