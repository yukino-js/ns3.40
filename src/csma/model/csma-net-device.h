
#ifndef CSMA_NET_DEVICE_H
#define CSMA_NET_DEVICE_H

#include "backoff.h"

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

class CsmaChannel;
class ErrorModel;

class CsmaNetDevice : public NetDevice {
public:
  static TypeId GetTypeId();

  enum EncapsulationMode {
    ILLEGAL,
    DIX,
    LLC,
  };

  CsmaNetDevice();

  ~CsmaNetDevice() override;

  void SetInterframeGap(Time t);

  void SetBackoffParams(Time slotTime, uint32_t minSlots, uint32_t maxSlots,
                        uint32_t maxRetries, uint32_t ceiling);

  bool Attach(Ptr<CsmaChannel> ch);

  void SetQueue(Ptr<Queue<Packet>> queue);

  Ptr<Queue<Packet>> GetQueue() const;

  void SetReceiveErrorModel(Ptr<ErrorModel> em);

  void Receive(Ptr<Packet> p, Ptr<CsmaNetDevice> sender);

  bool IsSendEnabled() const;

  void SetSendEnable(bool enable);

  bool IsReceiveEnabled() const;

  void SetReceiveEnable(bool enable);

  void SetEncapsulationMode(CsmaNetDevice::EncapsulationMode mode);

  CsmaNetDevice::EncapsulationMode GetEncapsulationMode();

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

  int64_t AssignStreams(int64_t stream);

protected:
  void DoDispose() override;

  void AddHeader(Ptr<Packet> p, Mac48Address source, Mac48Address dest,
                 uint16_t protocolNumber);

private:
  CsmaNetDevice &operator=(const CsmaNetDevice &o);

  CsmaNetDevice(const CsmaNetDevice &o);

  void Init(bool sendEnable, bool receiveEnable);

  void TransmitStart();

  void TransmitCompleteEvent();

  void TransmitReadyEvent();

  void TransmitAbort();

  void NotifyLinkUp();

  uint32_t m_deviceId;

  bool m_sendEnable;

  bool m_receiveEnable;

  enum TxMachineState { READY, BUSY, GAP, BACKOFF };

  TxMachineState m_txMachineState;

  EncapsulationMode m_encapMode;

  DataRate m_bps;

  Time m_tInterframeGap;

  Backoff m_backoff;

  Ptr<Packet> m_currentPkt;

  Ptr<CsmaChannel> m_channel;

  Ptr<Queue<Packet>> m_queue;

  Ptr<ErrorModel> m_receiveErrorModel;

  TracedCallback<Ptr<const Packet>> m_macTxTrace;

  TracedCallback<Ptr<const Packet>> m_macTxDropTrace;

  TracedCallback<Ptr<const Packet>> m_macPromiscRxTrace;

  TracedCallback<Ptr<const Packet>> m_macRxTrace;

  TracedCallback<Ptr<const Packet>> m_macRxDropTrace;

  TracedCallback<Ptr<const Packet>> m_macTxBackoffTrace;

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

  NetDevice::PromiscReceiveCallback m_promiscRxCallback;

  uint32_t m_ifIndex;

  bool m_linkUp;

  TracedCallback<> m_linkChangeCallbacks;

  static const uint16_t DEFAULT_MTU = 1500;

  uint32_t m_mtu;
};

} // namespace ns3

#endif
