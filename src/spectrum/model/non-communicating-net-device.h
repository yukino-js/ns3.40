
#ifndef NON_COMMUNICATING_NET_DEVICE_H
#define NON_COMMUNICATING_NET_DEVICE_H

#include <ns3/address.h>
#include <ns3/callback.h>
#include <ns3/net-device.h>
#include <ns3/node.h>
#include <ns3/packet.h>
#include <ns3/ptr.h>
#include <ns3/traced-callback.h>

#include <cstring>

namespace ns3 {

class SpectrumChannel;
class Channel;
class SpectrumErrorModel;

class NonCommunicatingNetDevice : public NetDevice {
public:
  static TypeId GetTypeId();

  NonCommunicatingNetDevice();
  ~NonCommunicatingNetDevice() override;

  void SetChannel(Ptr<Channel> c);

  void SetPhy(Ptr<Object> phy);

  Ptr<Object> GetPhy() const;

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
  bool Send(Ptr<Packet> packet, const Address &dest,
            uint16_t protocolNumber) override;
  bool SendFrom(Ptr<Packet> packet, const Address &source, const Address &dest,
                uint16_t protocolNumber) override;
  Ptr<Node> GetNode() const override;
  void SetNode(Ptr<Node> node) override;
  bool NeedsArp() const override;
  void SetReceiveCallback(NetDevice::ReceiveCallback cb) override;
  Address GetMulticast(Ipv4Address addr) const override;
  Address GetMulticast(Ipv6Address addr) const override;
  void SetPromiscReceiveCallback(PromiscReceiveCallback cb) override;
  bool SupportsSendFrom() const override;

private:
  void DoDispose() override;

  Ptr<Node> m_node;
  Ptr<Channel> m_channel;
  uint32_t m_ifIndex;
  Ptr<Object> m_phy;
};

} // namespace ns3

#endif
