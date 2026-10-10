
#ifndef ALOHA_NOACK_NET_DEVICE_H
#define ALOHA_NOACK_NET_DEVICE_H

#include "ns3/queue-fwd.h"
#include <ns3/address.h>
#include <ns3/callback.h>
#include <ns3/generic-phy.h>
#include <ns3/mac48-address.h>
#include <ns3/net-device.h>
#include <ns3/node.h>
#include <ns3/nstime.h>
#include <ns3/packet.h>
#include <ns3/ptr.h>
#include <ns3/traced-callback.h>

#include <cstring>

namespace ns3 {

class SpectrumChannel;
class Channel;
class SpectrumErrorModel;

class AlohaNoackNetDevice : public NetDevice {
public:
  enum State { IDLE, TX, RX };

  static TypeId GetTypeId();

  AlohaNoackNetDevice();
  ~AlohaNoackNetDevice() override;

  virtual void SetQueue(Ptr<Queue<Packet>> queue);

  void NotifyTransmissionEnd(Ptr<const Packet>);

  void NotifyReceptionStart();

  void NotifyReceptionEndError();

  void NotifyReceptionEndOk(Ptr<Packet> p);

  void SetChannel(Ptr<Channel> c);

  void SetGenericPhyTxStartCallback(GenericPhyTxStartCallback c);

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
  void NotifyGuardIntervalEnd();
  void DoDispose() override;

  void StartTransmission();

  Ptr<Queue<Packet>> m_queue;

  TracedCallback<Ptr<const Packet>> m_macTxTrace;
  TracedCallback<Ptr<const Packet>> m_macTxDropTrace;
  TracedCallback<Ptr<const Packet>> m_macPromiscRxTrace;
  TracedCallback<Ptr<const Packet>> m_macRxTrace;

  Ptr<Node> m_node;
  Ptr<Channel> m_channel;

  Mac48Address m_address;

  NetDevice::ReceiveCallback m_rxCallback;
  NetDevice::PromiscReceiveCallback m_promiscRxCallback;

  GenericPhyTxStartCallback m_phyMacTxStartCallback;

  TracedCallback<> m_linkChangeCallbacks;

  uint32_t m_ifIndex;
  mutable uint32_t m_mtu;
  bool m_linkUp;

  State m_state;
  Ptr<Packet> m_currentPkt;
  Ptr<Object> m_phy;
};

} // namespace ns3

#endif
