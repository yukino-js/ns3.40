
#ifndef TAP_BRIDGE_H
#define TAP_BRIDGE_H

#include "ns3/address.h"
#include "ns3/callback.h"
#include "ns3/data-rate.h"
#include "ns3/event-id.h"
#include "ns3/fd-reader.h"
#include "ns3/mac48-address.h"
#include "ns3/net-device.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/packet.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"

#include <cstring>

namespace ns3 {

class TapBridgeFdReader : public FdReader {
private:
  FdReader::Data DoRead() override;
};

class Node;

class TapBridge : public NetDevice {
public:
  static TypeId GetTypeId();

  enum Mode {
    ILLEGAL,
    CONFIGURE_LOCAL,
    USE_LOCAL,
    USE_BRIDGE,
  };

  TapBridge();
  ~TapBridge() override;

  Ptr<NetDevice> GetBridgedNetDevice();

  void SetBridgedNetDevice(Ptr<NetDevice> bridgedDevice);

  void Start(Time tStart);

  void Stop(Time tStop);

  void SetMode(TapBridge::Mode mode);

  TapBridge::Mode GetMode();

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

  bool ReceiveFromBridgedDevice(Ptr<NetDevice> device, Ptr<const Packet> packet,
                                uint16_t protocol, const Address &src,
                                const Address &dst, PacketType packetType);

  bool DiscardFromBridgedDevice(Ptr<NetDevice> device, Ptr<const Packet> packet,
                                uint16_t protocol, const Address &src);

private:
  void CreateTap();

  void StartTapDevice();

  void StopTapDevice();

  void ReadCallback(uint8_t *buf, ssize_t len);

  void ForwardToBridgedDevice(uint8_t *buf, ssize_t len);

  Ptr<Packet> Filter(Ptr<Packet> packet, Address *src, Address *dst,
                     uint16_t *type);

  void NotifyLinkUp();

  NetDevice::ReceiveCallback m_rxCallback;

  NetDevice::PromiscReceiveCallback m_promiscRxCallback;

  Ptr<Node> m_node;

  uint32_t m_ifIndex;

  uint16_t m_mtu;

  int m_sock;

  EventId m_startEvent;

  EventId m_stopEvent;

  Ptr<TapBridgeFdReader> m_fdReader;

  Mode m_mode;

  Mac48Address m_address;

  Time m_tStart;

  Time m_tStop;

  std::string m_tapDeviceName;

  Ipv4Address m_tapGateway;

  Ipv4Address m_tapIp;
  Mac48Address m_tapMac;

  Ipv4Mask m_tapNetmask;

  Ptr<NetDevice> m_bridgedDevice;
  bool m_ns3AddressRewritten;

  uint8_t *m_packetBuffer;

  uint32_t m_nodeId;

  bool m_linkUp;

  bool m_verbose;

  TracedCallback<> m_linkChangeCallbacks;
};

} // namespace ns3

#endif
