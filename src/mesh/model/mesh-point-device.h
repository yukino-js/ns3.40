
#ifndef L2ROUTING_NET_DEVICE_H
#define L2ROUTING_NET_DEVICE_H

#include "mesh-l2-routing-protocol.h"

#include "ns3/bridge-channel.h"
#include "ns3/mac48-address.h"
#include "ns3/net-device.h"
#include "ns3/node.h"
#include "ns3/random-variable-stream.h"

namespace ns3 {

class MeshPointDevice : public NetDevice {
public:
  static TypeId GetTypeId();
  MeshPointDevice();
  ~MeshPointDevice() override;

  void AddInterface(Ptr<NetDevice> port);
  uint32_t GetNInterfaces() const;
  Ptr<NetDevice> GetInterface(uint32_t id) const;
  std::vector<Ptr<NetDevice>> GetInterfaces() const;

  void SetRoutingProtocol(Ptr<MeshL2RoutingProtocol> protocol);
  Ptr<MeshL2RoutingProtocol> GetRoutingProtocol() const;

  void SetIfIndex(const uint32_t index) override;
  uint32_t GetIfIndex() const override;
  Ptr<Channel> GetChannel() const override;
  Address GetAddress() const override;
  void SetAddress(Address a) override;
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
  void DoDispose() override;

  void Report(std::ostream &os) const;
  void ResetStats();

  int64_t AssignStreams(int64_t stream);

  Time GetForwardingDelay() const;

private:
  void ReceiveFromDevice(Ptr<NetDevice> device, Ptr<const Packet> packet,
                         uint16_t protocol, const Address &source,
                         const Address &destination, PacketType packetType);
  void Forward(Ptr<NetDevice> incomingPort, Ptr<const Packet> packet,
               uint16_t protocol, const Mac48Address src,
               const Mac48Address dst);
  void DoSend(bool success, Ptr<Packet> packet, Mac48Address src,
              Mac48Address dst, uint16_t protocol, uint32_t iface);

private:
  NetDevice::ReceiveCallback m_rxCallback;
  NetDevice::PromiscReceiveCallback m_promiscRxCallback;
  Mac48Address m_address;
  Ptr<Node> m_node;
  std::vector<Ptr<NetDevice>> m_ifaces;
  uint32_t m_ifIndex;
  uint16_t m_mtu;
  Ptr<BridgeChannel> m_channel;
  Ptr<MeshL2RoutingProtocol> m_routingProtocol;
  Ptr<RandomVariableStream> m_forwardingRandomVariable;

  struct Statistics {
    uint32_t unicastData;
    uint32_t unicastDataBytes;
    uint32_t broadcastData;
    uint32_t broadcastDataBytes;

    Statistics();
  };

  Statistics m_rxStats;
  Statistics m_txStats;
  Statistics m_fwdStats;
};
} // namespace ns3
#endif
