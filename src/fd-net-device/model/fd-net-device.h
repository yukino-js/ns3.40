
#ifndef FD_NET_DEVICE_H
#define FD_NET_DEVICE_H

#include "ns3/address.h"
#include "ns3/callback.h"
#include "ns3/data-rate.h"
#include "ns3/event-id.h"
#include "ns3/fd-reader.h"
#include "ns3/mac48-address.h"
#include "ns3/net-device.h"
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"

#include <mutex>
#include <queue>
#include <utility>

namespace ns3 {

class FdNetDeviceFdReader : public FdReader {
public:
  FdNetDeviceFdReader();

  void SetBufferSize(uint32_t bufferSize);

private:
  FdReader::Data DoRead() override;

  uint32_t m_bufferSize;
};

class Node;

class FdNetDevice : public NetDevice {
public:
  static TypeId GetTypeId();

  enum EncapsulationMode {
    DIX,
    LLC,
    DIXPI,
  };

  FdNetDevice();

  ~FdNetDevice() override;

  FdNetDevice(const FdNetDevice &) = delete;

  void SetEncapsulationMode(FdNetDevice::EncapsulationMode mode);

  FdNetDevice::EncapsulationMode GetEncapsulationMode() const;

  void SetFileDescriptor(int fd);

  void Start(Time tStart);

  void Stop(Time tStop);

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

  virtual void SetIsBroadcast(bool broadcast);
  virtual void SetIsMulticast(bool multicast);

  virtual ssize_t Write(uint8_t *buffer, size_t length);

protected:
  void DoInitialize() override;

  void DoDispose() override;

  int GetFileDescriptor() const;

  virtual uint8_t *AllocateBuffer(size_t len);

  virtual void FreeBuffer(uint8_t *buf);

  void ReceiveCallback(uint8_t *buf, ssize_t len);

  std::mutex m_pendingReadMutex;

  std::queue<std::pair<uint8_t *, ssize_t>> m_pendingQueue;

private:
  void StartDevice();

  void StopDevice();

  virtual Ptr<FdReader> DoCreateFdReader();

  virtual void DoFinishStartingDevice();

  virtual void DoFinishStoppingDevice();

  void ForwardUp();

  bool TransmitStart(Ptr<Packet> p);

  void NotifyLinkUp();

  Ptr<Node> m_node;

  uint32_t m_nodeId;

  uint32_t m_ifIndex;

  uint16_t m_mtu;

  int m_fd;

  Ptr<FdReader> m_fdReader;

  Mac48Address m_address;

  EncapsulationMode m_encapMode;

  bool m_linkUp;

  TracedCallback<> m_linkChangeCallbacks;

  bool m_isBroadcast;

  bool m_isMulticast;

  uint32_t m_maxPendingReads;

  Time m_tStart;

  Time m_tStop;

  EventId m_startEvent;
  EventId m_stopEvent;

  NetDevice::ReceiveCallback m_rxCallback;

  NetDevice::PromiscReceiveCallback m_promiscRxCallback;

  TracedCallback<Ptr<const Packet>> m_macTxTrace;

  TracedCallback<Ptr<const Packet>> m_macTxDropTrace;

  TracedCallback<Ptr<const Packet>> m_macPromiscRxTrace;

  TracedCallback<Ptr<const Packet>> m_macRxTrace;

  TracedCallback<Ptr<const Packet>> m_macRxDropTrace;

  TracedCallback<Ptr<const Packet>> m_phyTxDropTrace;

  TracedCallback<Ptr<const Packet>> m_phyRxDropTrace;

  TracedCallback<Ptr<const Packet>> m_snifferTrace;

  TracedCallback<Ptr<const Packet>> m_promiscSnifferTrace;
};

} // namespace ns3

#endif
