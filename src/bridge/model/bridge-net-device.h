
#ifndef BRIDGE_NET_DEVICE_H
#define BRIDGE_NET_DEVICE_H

#include "bridge-channel.h"

#include "ns3/mac48-address.h"
#include "ns3/net-device.h"
#include "ns3/nstime.h"

#include <map>
#include <stdint.h>
#include <string>

namespace ns3 {

class Node;

class BridgeNetDevice : public NetDevice {
public:
  static TypeId GetTypeId();
  BridgeNetDevice();
  ~BridgeNetDevice() override;

  BridgeNetDevice(const BridgeNetDevice &) = delete;
  BridgeNetDevice &operator=(const BridgeNetDevice &) = delete;

  void AddBridgePort(Ptr<NetDevice> bridgePort);

  uint32_t GetNBridgePorts() const;

  Ptr<NetDevice> GetBridgePort(uint32_t n) const;

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
  void SetPromiscReceiveCallback(NetDevice::PromiscReceiveCallback cb) override;
  bool SupportsSendFrom() const override;
  Address GetMulticast(Ipv6Address addr) const override;

protected:
  void DoDispose() override;

  void ReceiveFromDevice(Ptr<NetDevice> device, Ptr<const Packet> packet,
                         uint16_t protocol, const Address &source,
                         const Address &destination, PacketType packetType);

  void ForwardUnicast(Ptr<NetDevice> incomingPort, Ptr<const Packet> packet,
                      uint16_t protocol, Mac48Address src, Mac48Address dst);

  void ForwardBroadcast(Ptr<NetDevice> incomingPort, Ptr<const Packet> packet,
                        uint16_t protocol, Mac48Address src, Mac48Address dst);

  void Learn(Mac48Address source, Ptr<NetDevice> port);

  Ptr<NetDevice> GetLearnedState(Mac48Address source);

private:
  NetDevice::ReceiveCallback m_rxCallback;
  NetDevice::PromiscReceiveCallback m_promiscRxCallback;

  Mac48Address m_address;
  Time m_expirationTime;

  struct LearnedState {
    Ptr<NetDevice> associatedPort;
    Time expirationTime;
  };

  std::map<Mac48Address, LearnedState> m_learnState;
  Ptr<Node> m_node;
  Ptr<BridgeChannel> m_channel;
  std::vector<Ptr<NetDevice>> m_ports;
  uint32_t m_ifIndex;
  uint16_t m_mtu;
  bool m_enableLearning;
};

} // namespace ns3

#endif
