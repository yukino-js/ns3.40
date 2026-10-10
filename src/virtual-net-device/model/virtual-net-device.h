
#ifndef VIRTUAL_NET_DEVICE_H
#define VIRTUAL_NET_DEVICE_H

#include "ns3/address.h"
#include "ns3/callback.h"
#include "ns3/net-device.h"
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"

namespace ns3 {

class VirtualNetDevice : public NetDevice {
public:
  typedef Callback<bool, Ptr<Packet>, const Address &, const Address &,
                   uint16_t>
      SendCallback;

  static TypeId GetTypeId();
  VirtualNetDevice();

  ~VirtualNetDevice() override;

  void SetSendCallback(SendCallback transmitCb);

  void SetNeedsArp(bool needsArp);

  void SetIsPointToPoint(bool isPointToPoint);

  void SetSupportsSendFrom(bool supportsSendFrom);

  bool SetMtu(const uint16_t mtu) override;

  bool Receive(Ptr<Packet> packet, uint16_t protocol, const Address &source,
               const Address &destination, PacketType packetType);

  void SetIfIndex(const uint32_t index) override;
  uint32_t GetIfIndex() const override;
  Ptr<Channel> GetChannel() const override;
  void SetAddress(Address address) override;
  Address GetAddress() const override;
  uint16_t GetMtu() const override;
  bool IsLinkUp() const override;
  void AddLinkChangeCallback(Callback<void> callback) override;
  bool IsBroadcast() const override;
  Address GetBroadcast() const override;
  bool IsMulticast() const override;
  Address GetMulticast(Ipv4Address multicastGroup) const override;
  Address GetMulticast(Ipv6Address addr) const override;
  bool IsPointToPoint() const override;
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
  bool IsBridge() const override;

protected:
  void DoDispose() override;

private:
  Address m_myAddress;
  SendCallback m_sendCb;
  TracedCallback<Ptr<const Packet>> m_macRxTrace;
  TracedCallback<Ptr<const Packet>> m_macTxTrace;
  TracedCallback<Ptr<const Packet>> m_macPromiscRxTrace;
  TracedCallback<Ptr<const Packet>> m_snifferTrace;
  TracedCallback<Ptr<const Packet>> m_promiscSnifferTrace;
  Ptr<Node> m_node;
  ReceiveCallback m_rxCallback;
  PromiscReceiveCallback m_promiscRxCallback;
  std::string m_name;
  uint32_t m_index;
  uint16_t m_mtu;
  bool m_needsArp;
  bool m_supportsSendFrom;
  bool m_isPointToPoint;
};

} // namespace ns3

#endif
