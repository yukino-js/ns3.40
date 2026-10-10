
#ifndef DOT11S_PEER_MAN_H
#define DOT11S_PEER_MAN_H

#include "ie-dot11s-beacon-timing.h"
#include "ie-dot11s-peer-management.h"
#include "peer-link.h"

#include "ns3/event-id.h"
#include "ns3/mac48-address.h"
#include "ns3/net-device.h"
#include "ns3/nstime.h"
#include "ns3/traced-value.h"

#include <map>

namespace ns3 {
class MeshPointDevice;
class UniformRandomVariable;

namespace dot11s {
class PeerManagementProtocolMac;
class PeerLink;
class IeMeshId;
class IePeerManagement;
class IeConfiguration;

class PeerManagementProtocol : public Object {
public:
  PeerManagementProtocol();
  ~PeerManagementProtocol() override;

  PeerManagementProtocol(const PeerManagementProtocol &) = delete;
  PeerManagementProtocol &operator=(const PeerManagementProtocol &) = delete;

  static TypeId GetTypeId();
  void DoDispose() override;
  bool Install(Ptr<MeshPointDevice> mp);
  Ptr<IeBeaconTiming> GetBeaconTimingElement(uint32_t interface);
  void ReceiveBeacon(uint32_t interface, Mac48Address peerAddress,
                     Time beaconInterval, Ptr<IeBeaconTiming> beaconTiming);

  void ReceivePeerLinkFrame(uint32_t interface, Mac48Address peerAddress,
                            Mac48Address peerMeshPointAddress, uint16_t aid,
                            IePeerManagement peerManagementElement,
                            IeConfiguration meshConfig);
  void ConfigurationMismatch(uint32_t interface, Mac48Address peerAddress);
  void TransmissionFailure(uint32_t interface, const Mac48Address peerAddress);
  void TransmissionSuccess(uint32_t interface, const Mac48Address peerAddress);
  bool IsActiveLink(uint32_t interface, Mac48Address peerAddress);

  void SetPeerLinkStatusCallback(
      Callback<void, Mac48Address, Mac48Address, uint32_t, bool> cb);
  Ptr<PeerLink> FindPeerLink(uint32_t interface, Mac48Address peerAddress);
  std::vector<Ptr<PeerLink>> GetPeerLinks() const;
  std::vector<Mac48Address> GetPeers(uint32_t interface) const;
  Mac48Address GetAddress();
  uint8_t GetNumberOfLinks() const;
  void SetMeshId(std::string s);
  Ptr<IeMeshId> GetMeshId() const;
  void SetBeaconCollisionAvoidance(bool enable);
  bool GetBeaconCollisionAvoidance() const;
  void NotifyBeaconSent(uint32_t interface, Time beaconInterval);

  void Report(std::ostream &os) const;
  void ResetStats();
  int64_t AssignStreams(int64_t stream);

  typedef void (*LinkOpenCloseTracedCallback)(Mac48Address src,
                                              const Mac48Address dst);

private:
  void DoInitialize() override;

  struct BeaconInfo {
    uint16_t aid;
    Time referenceTbtt;
    Time beaconInterval;
  };

  typedef std::vector<Ptr<PeerLink>> PeerLinksOnInterface;
  typedef std::map<uint32_t, PeerLinksOnInterface> PeerLinksMap;
  typedef std::map<Mac48Address, BeaconInfo> BeaconsOnInterface;
  typedef std::map<uint32_t, BeaconsOnInterface> BeaconInfoMap;
  typedef std::map<uint32_t, Ptr<PeerManagementProtocolMac>>
      PeerManagementProtocolMacMap;

  Ptr<PeerLink> InitiateLink(uint32_t interface, Mac48Address peerAddress,
                             Mac48Address peerMeshPointAddress);
  bool ShouldSendOpen(uint32_t interface, Mac48Address peerAddress) const;
  bool ShouldAcceptOpen(uint32_t interface, Mac48Address peerAddress,
                        PmpReasonCode &reasonCode) const;
  void PeerLinkStatus(uint32_t interface, Mac48Address peerAddress,
                      Mac48Address peerMeshPointAddress,
                      PeerLink::PeerState ostate, PeerLink::PeerState nstate);
  void CheckBeaconCollisions(uint32_t interface);
  void ShiftOwnBeacon(uint32_t interface);
  Time TuToTime(int x);
  int TimeToTu(Time x);

  void NotifyLinkOpen(Mac48Address peerMp, Mac48Address peerIface,
                      Mac48Address myIface, uint32_t interface);
  void NotifyLinkClose(Mac48Address peerMp, Mac48Address peerIface,
                       Mac48Address myIface, uint32_t interface);

private:
  PeerManagementProtocolMacMap m_plugins;
  Mac48Address m_address;
  Ptr<IeMeshId> m_meshId;

  uint16_t m_lastAssocId;
  uint16_t m_lastLocalLinkId;
  uint8_t m_maxNumberOfPeerLinks;
  bool m_enableBca;
  uint16_t m_maxBeaconShift;
  std::map<uint32_t, Time> m_lastBeacon;
  std::map<uint32_t, Time> m_beaconInterval;

  PeerLinksMap m_peerLinks;
  Callback<void, Mac48Address, Mac48Address, uint32_t, bool>
      m_peerStatusCallback;

  typedef TracedCallback<Mac48Address, Mac48Address> LinkEventCallback;
  LinkEventCallback m_linkOpenTraceSrc;
  LinkEventCallback m_linkCloseTraceSrc;

  struct Statistics {
    uint16_t linksTotal;
    uint16_t linksOpened;
    uint16_t linksClosed;

    Statistics(uint16_t t = 0);
    void Print(std::ostream &os) const;
  };

  Statistics m_stats;

  Ptr<UniformRandomVariable> m_beaconShift;
};

} // namespace dot11s
} // namespace ns3
#endif
