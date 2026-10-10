
#ifndef UAN_NET_DEVICE_H
#define UAN_NET_DEVICE_H

#include "ns3/mac8-address.h"
#include "ns3/net-device.h"
#include "ns3/pointer.h"
#include "ns3/traced-callback.h"

#include <list>

namespace ns3 {

class UanChannel;
class UanPhy;
class UanMac;
class UanTransducer;

class UanNetDevice : public NetDevice {
public:
  typedef std::list<Ptr<UanPhy>> UanPhyList;
  typedef std::list<Ptr<UanTransducer>> UanTransducerList;

  static TypeId GetTypeId();

  UanNetDevice();
  ~UanNetDevice() override;

  void SetMac(Ptr<UanMac> mac);

  void SetPhy(Ptr<UanPhy> phy);

  void SetChannel(Ptr<UanChannel> channel);

  Ptr<UanMac> GetMac() const;

  Ptr<UanPhy> GetPhy() const;

  Ptr<UanTransducer> GetTransducer() const;
  void SetTransducer(Ptr<UanTransducer> trans);

  void Clear();

  void SetSleepMode(bool sleep);

  void SetIfIndex(const uint32_t index) override;
  uint32_t GetIfIndex() const override;
  Ptr<Channel> GetChannel() const override;
  Address GetAddress() const override;
  bool SetMtu(const uint16_t mtu) override;
  uint16_t GetMtu() const override;
  bool IsLinkUp() const override;
  bool IsBroadcast() const override;
  Address GetBroadcast() const override;
  bool IsMulticast() const override;
  Address GetMulticast(Ipv4Address multicastGroup) const override;
  Address GetMulticast(Ipv6Address addr) const override;
  bool IsBridge() const override;
  bool IsPointToPoint() const override;
  bool Send(Ptr<Packet> packet, const Address &dest,
            uint16_t protocolNumber) override;
  bool SendFrom(Ptr<Packet> packet, const Address &source, const Address &dest,
                uint16_t protocolNumber) override;
  Ptr<Node> GetNode() const override;
  void SetNode(Ptr<Node> node) override;
  bool NeedsArp() const override;
  void SetReceiveCallback(NetDevice::ReceiveCallback cb) override;
  void SetPromiscReceiveCallback(PromiscReceiveCallback cb) override;
  bool SupportsSendFrom() const override;
  void AddLinkChangeCallback(Callback<void> callback) override;
  void SetAddress(Address address) override;

  uint32_t GetTxModeIndex();

  void SetTxModeIndex(uint32_t txModeIndex);

  typedef void (*RxTxTracedCallback)(Ptr<const Packet> packet,
                                     Mac8Address address);

private:
  virtual void ForwardUp(Ptr<Packet> pkt, uint16_t protocolNumber,
                         const Mac8Address &src);

  Ptr<UanChannel> DoGetChannel() const;

  Ptr<UanTransducer> m_trans;
  Ptr<Node> m_node;
  Ptr<UanChannel> m_channel;
  Ptr<UanMac> m_mac;
  Ptr<UanPhy> m_phy;

  uint32_t m_ifIndex;
  uint16_t m_mtu;
  bool m_linkup;
  TracedCallback<> m_linkChanges;
  ReceiveCallback m_forwardUp;

  TracedCallback<Ptr<const Packet>, Mac8Address> m_rxLogger;
  TracedCallback<Ptr<const Packet>, Mac8Address> m_txLogger;

  bool m_cleared;

protected:
  void DoDispose() override;
  void DoInitialize() override;
};

} // namespace ns3

#endif
