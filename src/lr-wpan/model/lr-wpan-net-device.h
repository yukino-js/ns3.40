#ifndef LR_WPAN_NET_DEVICE_H
#define LR_WPAN_NET_DEVICE_H

#include "lr-wpan-mac.h"

#include <ns3/net-device.h>
#include <ns3/traced-callback.h>

namespace ns3 {

class LrWpanPhy;
class LrWpanCsmaCa;
class SpectrumChannel;
class Node;

class LrWpanNetDevice : public NetDevice {
public:
  static TypeId GetTypeId();

  LrWpanNetDevice();
  ~LrWpanNetDevice() override;

  enum PseudoMacAddressMode_e { RFC4944, RFC6282 };

  void SetMac(Ptr<LrWpanMac> mac);

  void SetPhy(Ptr<LrWpanPhy> phy);

  void SetCsmaCa(Ptr<LrWpanCsmaCa> csmaca);

  void SetChannel(Ptr<SpectrumChannel> channel);

  Ptr<LrWpanMac> GetMac() const;

  Ptr<LrWpanPhy> GetPhy() const;

  Ptr<LrWpanCsmaCa> GetCsmaCa() const;

  void SetIfIndex(const uint32_t index) override;
  uint32_t GetIfIndex() const override;
  Ptr<Channel> GetChannel() const override;
  void SetAddress(Address address) override;
  Address GetAddress() const override;

  void SetPanAssociation(uint16_t panId, Mac64Address coordExtAddr,
                         Mac16Address coordShortAddr,
                         Mac16Address assignedShortAddr);

  bool SetMtu(const uint16_t mtu) override;
  uint16_t GetMtu() const override;
  bool IsLinkUp() const override;
  void AddLinkChangeCallback(Callback<void> callback) override;
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

  void McpsDataIndication(McpsDataIndicationParams params, Ptr<Packet> pkt);

  int64_t AssignStreams(int64_t stream);

private:
  void DoDispose() override;
  void DoInitialize() override;

  void LinkUp();

  void LinkDown();

  Ptr<SpectrumChannel> DoGetChannel() const;

  void CompleteConfig();

  Mac48Address BuildPseudoMacAddress(uint16_t panId,
                                     Mac16Address shortAddr) const;

  Ptr<LrWpanMac> m_mac;

  Ptr<LrWpanPhy> m_phy;

  Ptr<LrWpanCsmaCa> m_csmaca;

  Ptr<Node> m_node;

  bool m_configComplete;

  bool m_useAcks;

  bool m_linkUp;

  uint32_t m_ifIndex;

  TracedCallback<> m_linkChanges;

  ReceiveCallback m_receiveCallback;

  PseudoMacAddressMode_e m_pseudoMacMode;
};

} // namespace ns3

#endif
