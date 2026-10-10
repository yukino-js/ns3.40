
#ifndef POINT_TO_POINT_NET_DEVICE_H
#define POINT_TO_POINT_NET_DEVICE_H

#include "ns3/address.h"
#include "ns3/callback.h"
#include "ns3/data-rate.h"
#include "ns3/mac48-address.h"
#include "ns3/net-device.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/packet.h"
#include "ns3/ptr.h"
#include "ns3/queue-fwd.h"
#include "ns3/traced-callback.h"

#include <cstring>

namespace ns3 {

class PointToPointChannel;
class ErrorModel;

class PointToPointNetDevice : public NetDevice {
public:
  static TypeId GetTypeId();

  PointToPointNetDevice();

  ~PointToPointNetDevice() override;

  PointToPointNetDevice &operator=(const PointToPointNetDevice &) = delete;
  PointToPointNetDevice(const PointToPointNetDevice &) = delete;

  void SetDataRate(DataRate bps);

  void SetInterframeGap(Time t);

  bool Attach(Ptr<PointToPointChannel> ch);

  void SetQueue(Ptr<Queue<Packet>> queue);

  Ptr<Queue<Packet>> GetQueue() const;

  void SetReceiveErrorModel(Ptr<ErrorModel> em);

  void Receive(Ptr<Packet> p);

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
  void DoMpiReceive(Ptr<Packet> p);

private:
  void DoDispose() override;

  Address GetRemote() const;

  void AddHeader(Ptr<Packet> p, uint16_t protocolNumber);

  bool ProcessHeader(Ptr<Packet> p, uint16_t &param);

  bool TransmitStart(Ptr<Packet> p);

  void TransmitComplete();

  void NotifyLinkUp();

  enum TxMachineState { READY, BUSY };

  TxMachineState m_txMachineState;

  DataRate m_bps;

  Time m_tInterframeGap;

  Ptr<PointToPointChannel> m_channel;

  Ptr<Queue<Packet>> m_queue;

  Ptr<ErrorModel> m_receiveErrorModel;

  TracedCallback<Ptr<const Packet>> m_macTxTrace;

  TracedCallback<Ptr<const Packet>> m_macTxDropTrace;

  TracedCallback<Ptr<const Packet>> m_macPromiscRxTrace;

  TracedCallback<Ptr<const Packet>> m_macRxTrace;

  TracedCallback<Ptr<const Packet>> m_macRxDropTrace;

  TracedCallback<Ptr<const Packet>> m_phyTxBeginTrace;

  TracedCallback<Ptr<const Packet>> m_phyTxEndTrace;

  TracedCallback<Ptr<const Packet>> m_phyTxDropTrace;

  TracedCallback<Ptr<const Packet>> m_phyRxBeginTrace;

  TracedCallback<Ptr<const Packet>> m_phyRxEndTrace;

  TracedCallback<Ptr<const Packet>> m_phyRxDropTrace;

  TracedCallback<Ptr<const Packet>> m_snifferTrace;

  TracedCallback<Ptr<const Packet>> m_promiscSnifferTrace;

  Ptr<Node> m_node;
  Mac48Address m_address;
  NetDevice::ReceiveCallback m_rxCallback;
  NetDevice::PromiscReceiveCallback m_promiscCallback;
  uint32_t m_ifIndex;
  bool m_linkUp;
  TracedCallback<> m_linkChangeCallbacks;

  static const uint16_t DEFAULT_MTU = 1500;

  uint32_t m_mtu;

  Ptr<Packet> m_currentPkt;

  static uint16_t PppToEther(uint16_t protocol);

  static uint16_t EtherToPpp(uint16_t protocol);
};

} // namespace ns3

#endif
