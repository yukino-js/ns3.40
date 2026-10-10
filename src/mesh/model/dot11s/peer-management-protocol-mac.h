
#ifndef PEER_MANAGEMENT_PROTOCOL_MAC_H
#define PEER_MANAGEMENT_PROTOCOL_MAC_H

#include "ns3/mesh-wifi-interface-mac-plugin.h"

namespace ns3 {
class MeshWifiInterfaceMac;
class WifiMpdu;
enum WifiMacDropReason : uint8_t;

namespace dot11s {
class PeerManagementProtocol;
class IeConfiguration;
class IePeerManagement;
class PeerManagementProtocol;

class PeerManagementProtocolMac : public MeshWifiInterfaceMacPlugin {
public:
  PeerManagementProtocolMac(uint32_t interface,
                            Ptr<PeerManagementProtocol> protocol);
  ~PeerManagementProtocolMac() override;

  void SetParent(Ptr<MeshWifiInterfaceMac> parent) override;
  bool Receive(Ptr<Packet> packet, const WifiMacHeader &header) override;
  bool UpdateOutcomingFrame(Ptr<Packet> packet, WifiMacHeader &header,
                            Mac48Address from, Mac48Address to) override;
  void UpdateBeacon(MeshWifiBeacon &beacon) const override;
  int64_t AssignStreams(int64_t stream) override;

  void Report(std::ostream &) const;
  void ResetStats();
  uint32_t GetLinkMetric(Mac48Address peerAddress);

private:
  PeerManagementProtocolMac &operator=(const PeerManagementProtocolMac &peer);
  PeerManagementProtocolMac(const PeerManagementProtocolMac &);

  friend class PeerManagementProtocol;
  friend class PeerLink;

  struct PlinkFrameStart {
    uint8_t subtype;
    uint16_t aid;
    SupportedRates rates;
    uint16_t qos;
  };

  Ptr<Packet> CreatePeerLinkOpenFrame();
  Ptr<Packet> CreatePeerLinkConfirmFrame();
  Ptr<Packet> CreatePeerLinkCloseFrame();
  PlinkFrameStart ParsePlinkFrame(Ptr<const Packet> packet);
  void TxError(WifiMacDropReason reason, Ptr<const WifiMpdu> mpdu);
  void TxOk(Ptr<const WifiMpdu> mpdu);
  void SetBeaconShift(Time shift);
  void SetPeerManagerProtocol(Ptr<PeerManagementProtocol> protocol);
  void SendPeerLinkManagementFrame(Mac48Address peerAddress,
                                   Mac48Address peerMpAddress, uint16_t aid,
                                   IePeerManagement peerElement,
                                   IeConfiguration meshConfig);
  Mac48Address GetAddress() const;

  struct Statistics {
    uint16_t txOpen;
    uint16_t txConfirm;
    uint16_t txClose;
    uint16_t rxOpen;
    uint16_t rxConfirm;
    uint16_t rxClose;
    uint16_t dropped;
    uint16_t brokenMgt;
    uint16_t txMgt;
    uint32_t txMgtBytes;
    uint16_t rxMgt;
    uint32_t rxMgtBytes;
    uint16_t beaconShift;

    Statistics();
    void Print(std::ostream &os) const;
  };

private:
  Statistics m_stats;
  Ptr<MeshWifiInterfaceMac> m_parent;
  uint32_t m_ifIndex;
  Ptr<PeerManagementProtocol> m_protocol;
};

} // namespace dot11s
} // namespace ns3

#endif
