#ifndef SIMPLE_NET_DEVICE_H
#define SIMPLE_NET_DEVICE_H

#include "data-rate.h"
#include "mac48-address.h"
#include "queue-fwd.h"

#include "ns3/event-id.h"
#include "ns3/net-device.h"
#include "ns3/traced-callback.h"

#include <stdint.h>
#include <string>

namespace ns3 {

class SimpleChannel;
class Node;
class ErrorModel;

class SimpleNetDevice : public NetDevice {
public:
  static TypeId GetTypeId();
  SimpleNetDevice();

  void Receive(Ptr<Packet> packet, uint16_t protocol, Mac48Address to,
               Mac48Address from);

  void SetChannel(Ptr<SimpleChannel> channel);

  void SetQueue(Ptr<Queue<Packet>> queue);

  Ptr<Queue<Packet>> GetQueue() const;

  void SetReceiveErrorModel(Ptr<ErrorModel> em);

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
  Ptr<SimpleChannel> m_channel;
  NetDevice::ReceiveCallback m_rxCallback;
  NetDevice::PromiscReceiveCallback m_promiscCallback;
  Ptr<Node> m_node;
  uint16_t m_mtu;
  uint32_t m_ifIndex;
  Mac48Address m_address;
  Ptr<ErrorModel> m_receiveErrorModel;

  TracedCallback<Ptr<const Packet>> m_phyRxDropTrace;

  void StartTransmission();

  void FinishTransmission(Ptr<Packet> packet);

  bool m_linkUp;

  bool m_pointToPointMode;

  Ptr<Queue<Packet>> m_queue;
  DataRate m_bps;
  EventId FinishTransmissionEvent;

  TracedCallback<> m_linkChangeCallbacks;
};

} // namespace ns3

#endif
