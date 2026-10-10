
#ifndef WIFI_NET_DEVICE_H
#define WIFI_NET_DEVICE_H

#include "wifi-standards.h"

#include "ns3/net-device.h"
#include "ns3/traced-callback.h"

#include <vector>

namespace ns3 {

class WifiRemoteStationManager;
class WifiPhy;
class WifiMac;
class HtConfiguration;
class VhtConfiguration;
class HeConfiguration;
class EhtConfiguration;

static const uint16_t MAX_MSDU_SIZE = 2304;

class WifiNetDevice : public NetDevice {
public:
  static TypeId GetTypeId();

  WifiNetDevice();
  ~WifiNetDevice() override;

  WifiNetDevice(const WifiNetDevice &o) = delete;
  WifiNetDevice &operator=(const WifiNetDevice &) = delete;

  void SetStandard(WifiStandard standard);
  WifiStandard GetStandard() const;

  void SetMac(const Ptr<WifiMac> mac);
  void SetPhy(const Ptr<WifiPhy> phy);
  void SetPhys(const std::vector<Ptr<WifiPhy>> &phys);
  void SetRemoteStationManager(const Ptr<WifiRemoteStationManager> manager);
  void SetRemoteStationManagers(
      const std::vector<Ptr<WifiRemoteStationManager>> &managers);
  Ptr<WifiMac> GetMac() const;
  Ptr<WifiPhy> GetPhy() const;
  virtual Ptr<WifiPhy> GetPhy(uint8_t i) const;
  virtual const std::vector<Ptr<WifiPhy>> &GetPhys() const;
  uint8_t GetNPhys() const;
  Ptr<WifiRemoteStationManager> GetRemoteStationManager() const;
  Ptr<WifiRemoteStationManager> GetRemoteStationManager(uint8_t linkId) const;
  virtual const std::vector<Ptr<WifiRemoteStationManager>> &
  GetRemoteStationManagers() const;
  uint8_t GetNRemoteStationManagers() const;

  void SetHtConfiguration(Ptr<HtConfiguration> htConfiguration);
  Ptr<HtConfiguration> GetHtConfiguration() const;
  void SetVhtConfiguration(Ptr<VhtConfiguration> vhtConfiguration);
  Ptr<VhtConfiguration> GetVhtConfiguration() const;
  void SetHeConfiguration(Ptr<HeConfiguration> heConfiguration);
  Ptr<HeConfiguration> GetHeConfiguration() const;
  void SetEhtConfiguration(Ptr<EhtConfiguration> ehtConfiguration);
  Ptr<EhtConfiguration> GetEhtConfiguration() const;

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
  Ptr<Node> GetNode() const override;
  void SetNode(const Ptr<Node> node) override;
  bool NeedsArp() const override;
  void SetReceiveCallback(NetDevice::ReceiveCallback cb) override;
  Address GetMulticast(Ipv6Address addr) const override;
  bool SendFrom(Ptr<Packet> packet, const Address &source, const Address &dest,
                uint16_t protocolNumber) override;
  void SetPromiscReceiveCallback(PromiscReceiveCallback cb) override;
  bool SupportsSendFrom() const override;

protected:
  void DoDispose() override;
  void DoInitialize() override;
  void ForwardUp(Ptr<const Packet> packet, Mac48Address from, Mac48Address to);

private:
  void LinkUp();
  void LinkDown();
  void CompleteConfig();

  Ptr<Node> m_node;
  std::vector<Ptr<WifiPhy>> m_phys;
  Ptr<WifiMac> m_mac;
  std::vector<Ptr<WifiRemoteStationManager>> m_stationManagers;
  Ptr<HtConfiguration> m_htConfiguration;
  Ptr<VhtConfiguration> m_vhtConfiguration;
  Ptr<HeConfiguration> m_heConfiguration;
  Ptr<EhtConfiguration> m_ehtConfiguration;
  NetDevice::ReceiveCallback m_forwardUp;
  NetDevice::PromiscReceiveCallback m_promiscRx;

  TracedCallback<Ptr<const Packet>, Mac48Address> m_rxLogger;
  TracedCallback<Ptr<const Packet>, Mac48Address> m_txLogger;

  WifiStandard m_standard;
  uint32_t m_ifIndex;
  bool m_linkUp;
  TracedCallback<> m_linkChanges;
  mutable uint16_t m_mtu;
  bool m_configComplete;
};

} // namespace ns3

#endif
